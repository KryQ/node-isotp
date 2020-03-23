#include "IsotpWrapper.hpp"
#include <iostream>
#include <thread>

Napi::FunctionReference IsotpWrapper::constructor;

Napi::Object IsotpWrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "IsotpWrapper", {
    //InstanceMethod("connect", &IsotpWrapper::connect),
    InstanceMethod("send", &IsotpWrapper::send),
    InstanceMethod("read", &IsotpWrapper::read),
    //InstanceMethod("single_read", &IsotpWrapper::single_read)
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
  if (length != 1) {
    Napi::Error::New(env, "2 arguments expected").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  Napi::String interface = info[0].As<Napi::String>();
  
  this->isotp_ = new Isotp(interface.Utf8Value());
}

Napi::Value IsotpWrapper::send(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 3 || !info[0].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  if (!info[1].IsNumber() || !info[2].IsNumber()) {
    Napi::TypeError::New(env, "Numbero expected").ThrowAsJavaScriptException();
  }

  Napi::String tx = info[0].As<Napi::String>();
  std::string tx_buff = tx.Utf8Value();

  Napi::Number txId = info[1].As<Napi::Number>();
  Napi::Number rxId = info[2].As<Napi::Number>();

  int bytes = this->isotp_->send(tx_buff.c_str(), tx_buff.length(), txId.Uint32Value(), rxId.Uint32Value());

  return Napi::Number::New(info.Env(), bytes);
}

Napi::Value IsotpWrapper::read(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if ( info.Length() < 3 ) {
    throw Napi::TypeError::New( env, "Expected two arguments" );
  }
  else if ( !info[0].IsFunction() ) {
    throw Napi::TypeError::New( env, "Expected first arg to be function" );
  }
  else if (!info[1].IsNumber() || !info[2].IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }

  int txId = info[1].As<Napi::Number>().Uint32Value();
  int rxId = info[2].As<Napi::Number>().Uint32Value();

  int thread_id = rand() % 1000;
  reading_threads_map.insert ( std::pair<int, std::thread>(thread_id, std::thread()) );

  // Create a ThreadSafeFunction
  tsfn_map.insert( std::pair<int, Napi::ThreadSafeFunction>(thread_id, Napi::ThreadSafeFunction::New(
      env,
      info[0].As<Napi::Function>(),  // JavaScript function called asynchronously
      "Resource Name",         // Name
      0,                       // Unlimited queue
      1,                       // Only one thread will use this initially
      [this, thread_id, env ]( Napi::Env ) {        // Finalizer used to clean threads up
        this->reading_threads_map[thread_id].join();
      }
    )
  ));

  // Create a native thread
  reading_threads_map[thread_id] = std::thread( [this, thread_id, txId, rxId] {
    auto callback = [this, txId, rxId]( Napi::Env env, Napi::Function jsCallback, IsotpWrapper::Frame* frame ) {
      Napi::Object obj = Napi::Object::New(env);
      Napi::Buffer<char> test = Napi::Buffer<char>::New(env, frame->buff, frame->len);
      obj.Set("data", test);
      obj.Set("len", frame->len);
      obj.Set("txId", txId);
      obj.Set("rxId", rxId);

      jsCallback.Call( {obj} );
    };

    IsotpWrapper::Frame frame;
    int sock = this->isotp_->connect(txId, rxId);
    
    while (1)
    {
      frame.len = this->isotp_->read(frame.buff, sock);
      if(frame.len > 0) {
        napi_status status = this->tsfn_map[thread_id].BlockingCall( &frame, callback );
        if ( status != napi_ok )
        {
          // Handle error
          break;
        }
      }
      else {
        break;
      }
    }

    // Release the thread-safe function
    //delete buff;
    this->tsfn_map[thread_id].Release();
  } );

  return Napi::Number::New(env, thread_id);
}