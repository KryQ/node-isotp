#include "IsotpWrapper.hpp"
#include <iostream>
#include <thread>

Napi::FunctionReference IsotpWrapper::constructor;

Napi::Object IsotpWrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "IsotpWrapper", {
    //InstanceMethod("connect", &IsotpWrapper::connect),
    InstanceMethod("send", &IsotpWrapper::send),
    InstanceMethod("read", &IsotpWrapper::read)
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

std::thread nativeThread;
  Napi::ThreadSafeFunction tsfn;

Napi::Value IsotpWrapper::read(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if ( info.Length() < 2 )
  {
    throw Napi::TypeError::New( env, "Expected two arguments" );
  }
  else if ( !info[0].IsFunction() )
  {
    throw Napi::TypeError::New( env, "Expected first arg to be function" );
  }
  else if ( !info[1].IsNumber() )
  {
    throw Napi::TypeError::New( env, "Expected second arg to be number" );
  }

  // Create a ThreadSafeFunction
  tsfn = Napi::ThreadSafeFunction::New(
      env,
      info[0].As<Napi::Function>(),  // JavaScript function called asynchronously
      "Resource Name",         // Name
      0,                       // Unlimited queue
      1,                       // Only one thread will use this initially
      []( Napi::Env ) {        // Finalizer used to clean threads up
        nativeThread.join();
      } );

  // Create a native thread
  nativeThread = std::thread( [this] {
    auto callback = []( Napi::Env env, Napi::Function jsCallback, char* value ) {
      // Transform native data into JS data, passing it to the provided 
      // `jsCallback` -- the TSFN's JavaScript function.
      jsCallback.Call( {Napi::String::New( env, std::string(value) )} );
    };

    char buff[4096];
    while (1)
    {
      if(this->isotp_->read(buff)>0){
        napi_status status = tsfn.BlockingCall( buff, callback );
        if ( status != napi_ok )
        {
          // Handle error
          break;
        }
      }
    }

    // Release the thread-safe function
    delete buff;
    tsfn.Release();
  } );

  return Napi::Boolean::New(env, true);
}