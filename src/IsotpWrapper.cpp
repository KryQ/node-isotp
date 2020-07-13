#include "IsotpWrapper.hpp"
#include <iostream>
#include <thread>
#include <errno.h>

Napi::FunctionReference IsotpWrapper::constructor;

Napi::Object IsotpWrapper::Init(Napi::Env env, Napi::Object exports)
{
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "IsotpWrapper", {
                                                             //InstanceMethod("connect", &IsotpWrapper::connect),
                                                             InstanceMethod("send", &IsotpWrapper::send), 
                                                             InstanceMethod("read", &IsotpWrapper::read),
                                                             InstanceMethod("stopReading", &IsotpWrapper::stopReading),
                                                             //InstanceMethod("single_read", &IsotpWrapper::single_read)
                                                         });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("IsotpWrapper", func);
  return exports;
}

IsotpWrapper::IsotpWrapper(const Napi::CallbackInfo &info) : Napi::ObjectWrap<IsotpWrapper>(info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();
  if (length != 3)
  {
    Napi::Error::New(env, "3 arguments expected").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString())
  {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  if (!info[1].IsNumber() || !info[2].IsNumber())
  {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }

  Napi::String interface = info[0].As<Napi::String>();
  Napi::Number bs = info[1].As<Napi::Number>();
  Napi::Number stmin = info[2].As<Napi::Number>();

  this->isotp_ = new Isotp(interface.Utf8Value(), bs.Uint32Value(), stmin.Uint32Value() );
}

Napi::Value IsotpWrapper::send(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 3 || !info[0].IsString())
  {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  if (!info[1].IsNumber() || !info[2].IsNumber())
  {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }

  Napi::String tx = info[0].As<Napi::String>();
  std::string tx_buff = tx.Utf8Value();

  Napi::Number txId = info[1].As<Napi::Number>();
  Napi::Number rxId = info[2].As<Napi::Number>();

  int bytes = this->isotp_->send(tx_buff.c_str(), tx_buff.length(), txId.Uint32Value(), rxId.Uint32Value());

  return Napi::Number::New(info.Env(), bytes);
}

class ReadWorker : public Napi::AsyncWorker
{
public:
  ReadWorker(Napi::Function &callback, Isotp *isotp, int &txId, int &rxId)
      : AsyncWorker(callback), isotp(isotp), txId(txId), rxId(rxId)
  {
    SuppressDestruct();

    this->sock = isotp->connect(txId, rxId);
    if (this->sock < 0)
    {
      SetError(strerror(errno));
    }
  }

  ~ReadWorker() {}
  // This code will be executed on the worker thread
  void Execute() override
  {
    int len = isotp->read(this->buff, this->sock);
    this->buff[len] = 0;

    if (len < 0)
    {
      SetError(strerror(errno));
    }
  }

  void Kill() {
    this->kill = true;
  }

  void OnOK() override
  {
    Napi::HandleScope scope(Env());
    
    if(!this->kill) {
      Callback().Call({Napi::Boolean::New(Env(), false), Napi::String::New(Env(), std::string(this->buff)), Napi::Number::New(Env(), txId), Napi::Number::New(Env(), rxId)});
      Queue();
    }
    else {
      this->isotp->disconnect(this->sock);
      Destroy();
    }
  }

private:
  Isotp *isotp;
  int sock, txId, rxId;
  char buff[5000];
  bool kill = false;
};

Napi::Value IsotpWrapper::read(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 3)
  {
    throw Napi::TypeError::New(env, "Expected two arguments");
  }
  else if (!info[0].IsFunction())
  {
    throw Napi::TypeError::New(env, "Expected first arg to be function");
  }
  else if (!info[1].IsNumber() || !info[2].IsNumber())
  {
    throw Napi::TypeError::New(env, "Number expected");
  }

  Napi::Function cb = info[0].As<Napi::Function>();
  int txId = info[1].As<Napi::Number>().Uint32Value();
  int rxId = info[2].As<Napi::Number>().Uint32Value();

  if (txId == rxId)
  {
    throw Napi::TypeError::New(env, "rxId can't be same as txTd");
  }

  ReadWorker *wk = new ReadWorker(cb, this->isotp_, txId, rxId);
  wk->Queue();

  int read_id = std::rand();
  read_workers.insert ( std::pair<int, ReadWorker*>(read_id, wk));

  return Napi::Number::New(env, read_id);
}

Napi::Value IsotpWrapper::stopReading(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    throw Napi::RangeError::New(env, "Expected one argument");
  }
  else if (!info[0].IsNumber())
  {
    throw Napi::TypeError::New(env, "Number expected");
  }

  int read_id = info[0].As<Napi::Number>().Uint32Value();

  read_workers[read_id]->Kill();
  read_workers.erase (read_id);

  return Napi::Number::New(env, 1);
}