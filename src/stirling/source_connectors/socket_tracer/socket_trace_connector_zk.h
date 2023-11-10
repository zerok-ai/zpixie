/*
 * Copyright 2018- The Pixie Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/stirling/source_connectors/socket_tracer/socket_trace_connector.h"

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <filesystem>
#include <utility>
// #include <thread>

#include <absl/container/flat_hash_map.h>
#include <absl/strings/match.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/delimited_message_util.h>
#include <magic_enum.hpp>

#include "src/common/base/base.h"
#include "src/common/base/utils.h"
#include "src/common/json/json.h"
#include "src/common/system/proc_pid_path.h"
#include "src/common/system/socket_info.h"
#include "src/shared/metadata/metadata.h"
#include "src/stirling/bpf_tools/macros.h"
#include "src/stirling/bpf_tools/utils.h"
#include "src/stirling/source_connectors/socket_tracer/bcc_bpf_intf/go_grpc_types.hpp"
#include "src/stirling/source_connectors/socket_tracer/bcc_bpf_intf/socket_trace.hpp"
#include "src/stirling/source_connectors/socket_tracer/conn_stats.h"
#include "src/stirling/source_connectors/socket_tracer/proto/sock_event.pb.h"
#include "src/stirling/source_connectors/socket_tracer/protocols/http/utils.h"
#include "src/stirling/source_connectors/socket_tracer/protocols/http2/grpc.h"
#include "src/stirling/utils/linux_headers.h"
#include "src/stirling/utils/proc_path_tools.h"
#include "src/zerok/filters/query/Query.h"
#include "src/zerok/filters/query/ZkTraceInfo.h"
#include "src/zerok/filters/query/QueryBuilder.h"
#include "src/zerok/filters/query/QueryExecutor.h"
// #include "src/zerok/store/redis.h"

namespace px {
  namespace stirling {

    using px::utils::ToJSONString;

    class ZkRulesExecutor{
      private:
        static int64_t calculateLatency(int64_t req_timestamp_ns, int64_t resp_timestamp_ns) {
          int64_t latency_ns = 0;
          if (req_timestamp_ns > 0 && resp_timestamp_ns > 0) {
            latency_ns = resp_timestamp_ns - req_timestamp_ns;
            LOG_IF_EVERY_N(WARNING, latency_ns < 0, 100)
                << absl::Substitute("Negative latency implies req resp mismatch [t_req=$0, t_resp=$1].",
                                    req_timestamp_ns, resp_timestamp_ns);
          }
          return latency_ns;
        }

      public:
        static void init(){
          zk::ZkQueryExecutor::init();
        }

        static std::string extractTraceparentValue(const std::string& queryString) {
            std::string traceparentValue = "ZK_NULL";

            std::size_t traceparentStart = queryString.find("traceparent:");
            if (traceparentStart == std::string::npos) {
                return traceparentValue;
            }

            traceparentStart += strlen("traceparent:");

            std::size_t traceparentEnd = queryString.find(",", traceparentStart);
            if (traceparentEnd == std::string::npos) {
                return traceparentValue;
            }

            traceparentValue = queryString.substr(traceparentStart, traceparentEnd - traceparentStart);
            return traceparentValue;
        }

        static std::string extractResultRows(const std::string& queryString) {
            std::string resultRows = "0";

            /* TODO: (avin) explore if we can extract this in a better way. */
            std::size_t resultRowsStart = queryString.find("Resultset rows = ");
            if (resultRowsStart == std::string::npos) {
                return resultRows;
            }

            resultRowsStart += strlen("Resultset rows = ");

            std::size_t resultRowsEnd = queryString.find(" > ", resultRowsStart);
            if (resultRowsEnd == std::string::npos) {
                return resultRows;
            }

            resultRows = queryString.substr(resultRowsStart, resultRowsEnd - resultRowsStart);
            return resultRows;
        }

        //returns passthrough value - as in if the given record is allowed to be writtent o the db (apache arrow)
        static zk::ZkTraceInfo httpEvaluate(const ConnTracker& conn_tracker, protocols::http::Message& req_message, 
            protocols::http::Message& resp_message, HTTPContentType content_type, md::UPID upid){
          init();
          (void)req_message;
          (void)resp_message;
          
          return httpEvaluate(resp_message.timestamp_ns, upid, conn_tracker.remote_endpoint().AddrStr(), 
            conn_tracker.remote_endpoint().port(), conn_tracker.role(), 1, resp_message.minor_version, 
            ToJSONString(req_message.headers), content_type, req_message.req_method, req_message.req_path, 
            resp_message.resp_status, resp_message.resp_message, req_message.body_size, req_message.body,
            resp_message.body_size, resp_message.body, ToJSONString(resp_message.headers), calculateLatency(req_message.timestamp_ns, resp_message.timestamp_ns));
        }

        static zk::ZkTraceInfo httpEvaluate(const std::map<std::string, std::string>& propsMap){
          // std::string myString = "";
          // for (const auto& pair : propsMap) {
          //     myString += pair.first + ": " + pair.second + "@@@@";
          // }
          // LOG(INFO) << "AVIN_DEBUG05__SocketTraceConnector::AppendMessage myString " << myString;
          zk::ZkTraceInfo data = zk::ZkQueryExecutor::apply("HTTP", propsMap);
          return data;
        }

        static zk::ZkTraceInfo httpEvaluate(uint64_t time, md::UPID upid, std::string remoteAddr, 
            int remotePort, int traceRole, int majorVersion, int minorVersion, std::string reqHeadesJson, 
            HTTPContentType content_type, std::string reqMethod, std::string reqPath, int64_t respStatus, 
            std::string respMessage, size_t reqBodySize, std::string reqBody, size_t respBodySize, 
            std::string respBody, std::string respHeadersJson, int64_t latency){
              std::map<std::string, std::string> propsMap;

              //TODO:ZEROK Remove the following debug values
              //Debug values START
              propsMap["zk_req_type"] = "HTTP";
              propsMap["int_field"] = "35";
              // propsMap["trace_role"] = "server";
              propsMap["remote_addr"] = "10.0.0.4";
              propsMap["key_value_field"] = "{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value2\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value3\":\"HTTP\"}}}";
              //Debug values END 

              propsMap["time_"] = std::to_string(static_cast<long>(time));
              // propsMap["upid"] = std::to_string(absl::Uint128High64(upid.value())) + std::to_string(absl::Uint128Low64(upid.value()));
              propsMap["upid"] = upid.String();
              // Note that there is a string copy here,
              // But std::move is not allowed because we re-use conn object.
              propsMap["remote_addr"] = remoteAddr;
              propsMap["remote_port"] = std::to_string(remotePort);
              int traceRoleInt = traceRole;
              std::string traceRoleString = "";
              if(traceRoleInt == 2){
                traceRoleString = "server";
              }else if(traceRoleInt == 1){
                traceRoleString = "client";
              }
              propsMap["trace_role"] = traceRoleString;//std::to_string(conn_tracker.role());
              propsMap["major_version"] = std::to_string(majorVersion);
              propsMap["minor_version"] = std::to_string(minorVersion);
              propsMap["req_headers"] = reqHeadesJson;
              propsMap["content_type"] = std::to_string(static_cast<uint64_t>(content_type));
              propsMap["req_method"] = reqMethod;
              propsMap["req_path"] = reqPath;
              propsMap["resp_status"] = std::to_string(respStatus);
              propsMap["resp_message"] = respMessage;
              propsMap["req_body_size"] = std::to_string(reqBodySize);
              propsMap["req_body"] = reqBody;
              propsMap["resp_body_size"] = std::to_string(respBodySize);
              propsMap["resp_headers"] = respHeadersJson;
              propsMap["resp_body"] = respBody;

              propsMap["latency"] = std::to_string(latency);

              return httpEvaluate(propsMap);
        }

        static zk::ZkTraceInfo httpEvaluate(int64_t resp_status, const ConnTracker& conn_tracker, protocols::http2::HalfStream* req_stream,
          protocols::http2::HalfStream* resp_stream, HTTPContentType content_type, md::UPID upid){
          init();
          (void)req_stream;
          (void)resp_stream;
          // zk::ZkStore zkStore;
          // zkStore.connect();
          std::map<std::string, std::string> propsMap;
          //TODO:ZEROK Remove the following debug values
          //Debug values START
          propsMap["zk_req_type"] = "HTTP";
          propsMap["int_field"] = "35";
          // propsMap["trace_role"] = "server";
          propsMap["remote_addr"] = "10.0.0.4";
          propsMap["key_value_field"] = "{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value2\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value3\":\"HTTP\"}}}";
          //Debug values END 

          propsMap["time_"] = std::to_string(static_cast<long>(resp_stream->timestamp_ns));
          // propsMap["upid"] = std::to_string(absl::Uint128High64(upid.value())) + std::to_string(absl::Uint128Low64(upid.value()));
          propsMap["upid"] = upid.String();
          // Note that there is a string copy here,
          // But std::move is not allowed because we re-use conn object.
          propsMap["remote_addr"] = conn_tracker.remote_endpoint().AddrStr();
          propsMap["remote_port"] = std::to_string(conn_tracker.remote_endpoint().port());
          int traceRoleInt = conn_tracker.role();
          std::string traceRoleString = "";
          if(traceRoleInt == 2){
            traceRoleString = "server";
          }else if(traceRoleInt == 1){
            traceRoleString = "client";
          }
          propsMap["trace_role"] = traceRoleString;//std::to_string(conn_tracker.role());
          propsMap["major_version"] = std::to_string(1);
          propsMap["minor_version"] = std::to_string(0);
          propsMap["req_headers"] = ToJSONString(req_stream->headers());
          propsMap["content_type"] = std::to_string(static_cast<uint64_t>(content_type));
          propsMap["req_method"] = req_stream->headers().ValueByKey(protocols::http2::headers::kMethod);
          propsMap["req_path"] = req_stream->headers().ValueByKey(":path");
          propsMap["resp_status"] = std::to_string(resp_status);
          propsMap["resp_message"] = "OK";
          propsMap["req_body_size"] = std::to_string(req_stream->original_data_size());
          propsMap["req_body"] = req_stream->ConsumeData();
          propsMap["resp_body_size"] = std::to_string(resp_stream->original_data_size());
          propsMap["resp_headers"] = ToJSONString(resp_stream->headers());
          propsMap["resp_body"] = resp_stream->ConsumeData();

          propsMap["latency"] = std::to_string(calculateLatency(req_stream->timestamp_ns, resp_stream->timestamp_ns));

          return httpEvaluate(propsMap);
        }
    };
  }
}
