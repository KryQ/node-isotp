#include <napi.h>
#include "Isotp.hpp"
#include <thread>

class IsotpWrapper : public Napi::ObjectWrap<IsotpWrapper> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports); //Init function for setting the export key to JS
  IsotpWrapper(const Napi::CallbackInfo& info); //Constructor to initialise

 private:
  static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
  //Napi::Value connect(const Napi::CallbackInfo& info); //wrapped getValue function 

  Napi::Value send(const Napi::CallbackInfo& info); //wrapped add function
  Napi::Value read(const Napi::CallbackInfo& info); //wrapped add function

  Isotp *isotp_; //internal instance of actualclass used to perform actual operations.
  Napi::Function emitter_;
};