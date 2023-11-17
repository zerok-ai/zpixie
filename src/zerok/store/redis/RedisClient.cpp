#include "RedisClient.h"
#include "log.h"

#include <stdarg.h>

using namespace std;

RedisReplyPtr RedisClient::redisCommand(const char* format, ...) {
  RedisReplyPtr reply;
  va_list ap;

  va_start(ap, format);
  reply = redisvCommand(format, ap);
  va_end(ap);

  return reply;
}

RedisReplyPtr RedisClient::redisvCommand(const char* format, va_list ap) {
  void* reply = 0;
  PooledSocket socket(inst);

  if (socket.notNull()) {
    reply = redis_vcommand(socket, inst, format, ap);
  } else {
    log_(L_ERROR | L_CONS,
         "Can not get socket from redis connection pool, server down? or not enough connection?");
  }

  return RedisReplyPtr(reply);
}

RedisClient& RedisClient::operator=(const RedisClient& other) {
  // Check for self-assignment
  if (this == &other) {
    return *this;
  }

  // Release current resources if needed
  redis_pool_destroy(inst);  // Assuming you need to release the existing resources

  // Copy data from 'other' to 'this'
  inst = nullptr;       // Set to nullptr to ensure correct behavior in case of exceptions during
                        // resource allocation
  initialized = false;  // Reset the initialization flag

  if (redis_pool_create(other.inst->config, &inst) >= 0) {
    initialized = true;
  } else {
    // Handle failure to create a new pool (throw an exception or log an error)
    log_(L_ERROR | L_CONS, "Failed to create a new Redis connection pool during assignment");
  }

  // Ensure the assignment is complete
  return *this;
}