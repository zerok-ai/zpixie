# Copyright 2018- The Pixie Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

load("@rules_cc//cc:defs.bzl", "cc_library")

# licenses(["notice"])
# # MIT LICENSE

# exports_files(["license.txt"])

cc_library(
    name = "hiredis",
    srcs = glob(["*.c", "**/*.c"], exclude = ["examples/**", "ssl.c"],),
    hdrs = glob(["*.h", "**/*.h", "dict.c",]),
    includes = [],
    # includes = ["include"],
    # srcs = [
    #     "async.c",
    #     "hiredis.c",
    #     "net.c",
    #     "read.c",
    #     "sds.c",
    #     "*.c",
    # ],
    # hdrs = [
    #     # adding dict.c here since async.c includes it
    #     "dict.c",
    #     "alloc.h",
    #     "async.h",
    #     "dict.h",
    #     "hiredis.h",
    #     "net.h",
    #     "read.h",
    #     "sds.h",
    #     "fmacros.h",
    #     "*.h",
    # ],
    visibility = ["//visibility:public"],
)
