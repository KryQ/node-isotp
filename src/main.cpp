#include <napi.h>
#include "IsotpWrapper.hpp"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return IsotpWrapper::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)