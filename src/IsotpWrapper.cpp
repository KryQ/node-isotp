#include "IsotpWrapper.hpp"
#include <thread>

Napi::FunctionReference IsotpWrapper::constructor;

Napi::Object IsotpWrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "IsotpWrapper", {
    //InstanceMethod("connect", &IsotpWrapper::connect),
    InstanceMethod("send", &IsotpWrapper::send),
    InstanceMethod("startReading", &IsotpWrapper::read)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("IsotpWrapper", func);
  return exports;
}

IsotpWrapper::IsotpWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<IsotpWrapper>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();
  if (length != 4) {
    Napi::Error::New(env, "3 arguments expected").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  if (!info[1].IsNumber() || !info[2].IsNumber()) {
    Napi::TypeError::New(env, "Numbero expected").ThrowAsJavaScriptException();
  }

  if (!info[3].IsFunction()) {
    Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
  }

  Napi::String interface = info[0].As<Napi::String>();
  Napi::Number txId = info[1].As<Napi::Number>();
  Napi::Number rxId = info[2].As<Napi::Number>();

  this->isotp_ = new Isotp(interface.Utf8Value(), txId.Uint32Value(), rxId.Uint32Value());
  this->emitter_ = info[3].As<Napi::Function>();

  this->emitter_.Call({Napi::String::New(env, "created"), Napi::String::New(env, "created")});
}

Napi::Value IsotpWrapper::send(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  Napi::String tx = info[0].As<Napi::String>();
  std::string tx_buff = tx.Utf8Value();

  int bytes = this->isotp_->send(tx_buff.c_str(), tx_buff.length());

  return Napi::Number::New(info.Env(), bytes);
}

void IsotpWrapper::cont_read(Isotp *isotp, Napi::Function *emitter, Napi::Env *env) {
  char buff[4096];

  while(1) {
    if(isotp->read(buff) > 0) {
      emitter->Call({Napi::String::New(*env, "data"), Napi::String::New(*env, std::string(buff))});
      memset(&buff[0], 0, sizeof(buff));
    }
    else {
      perror("read");
    }
  }
}

Napi::Value IsotpWrapper::read(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  reading = new std::thread(IsotpWrapper::cont_read, isotp_, &emitter_, &env);

  this->emitter_.Call({Napi::String::New(env, "startedReading"), Napi::String::New(env, "lol")});
  return Napi::String::New(info.Env(), "lol");
}