#include "tensor.h"

using std::cout;
using std::endl;

namespace torchjs
{
torch::NoGradGuard no_grad_tensor;
Nan::Persistent<v8::FunctionTemplate> Tensor::constructor;

Tensor::Tensor(){};
Tensor::~Tensor(){};

NAN_MODULE_INIT(Tensor::Init)
{
  torch::NoGradGuard no_grad;
  v8::Local<v8::FunctionTemplate> ctor =
      Nan::New<v8::FunctionTemplate>(Tensor::New);
  Tensor::constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Tensor").ToLocalChecked());

  // link our getters and setter to the object property
  Nan::SetAccessor(ctor->InstanceTemplate(),
                   Nan::New("data").ToLocalChecked(), Tensor::HandleGetters,
                   Tensor::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(),
                   Nan::New("sizes").ToLocalChecked(), Tensor::HandleGetters,
                   Tensor::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(),
                   Nan::New("type").ToLocalChecked(), Tensor::HandleGetters,
                   Tensor::HandleSetters);
  Nan::SetPrototypeMethod(ctor, "cuda", cuda);
  Nan::SetPrototypeMethod(ctor, "cpu", cpu);
  // Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("y").ToLocalChecked(),
  // Tensor::HandleGetters, Tensor::HandleSetters);
  // Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("z").ToLocalChecked(),
  // Tensor::HandleGetters, Tensor::HandleSetters);

  // Nan::SetPrototypeMethod(tpl, "setTensor", setTensor);
  // Nan::SetPrototypeMethod(tpl, "getTensor", getTensor);

  target->Set(Nan::New("Tensor").ToLocalChecked(), Nan::GetFunction(ctor).ToLocalChecked());
}

void Tensor::setTensor(at::Tensor tensor) { this->mTensor = tensor; }
torch::Tensor Tensor::getTensor() { return this->mTensor; }

v8::Local<v8::Object> Tensor::NewInstance()
{
  v8::Local<v8::Function> constructorFunc =
      Nan::GetFunction(Nan::New(Tensor::constructor)).ToLocalChecked();
  const int argc = 0;
  v8::Local<v8::Value> argv[] = {};
  return Nan::NewInstance(constructorFunc, argc, argv).ToLocalChecked();
  // return Nan::NewInstance(cons, 0, {}).ToLocalChecked();
}

NAN_METHOD(Tensor::toString)
{
  Tensor *obj = ObjectWrap::Unwrap<Tensor>(info.Holder());
  std::stringstream ss;
  at::IntArrayRef sizes = obj->mTensor.sizes();
  auto option = obj->mTensor.options();
  ss << "Tensor[Type=" << option.dtype() << "_" << option.device() << ", ";
  ss << "Size=" << sizes << std::endl;
  info.GetReturnValue().Set(Nan::New(ss.str()).ToLocalChecked());
}

NAN_METHOD(Tensor::New)
{
  if (!info.IsConstructCall())
  {
    return Nan::ThrowError(
        Nan::New("Tensor::New - called without new keyword").ToLocalChecked());
  }

  Tensor *obj = new Tensor();
  obj->Wrap(info.Holder());
  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(ones)
{
  // Sanity checking of the arguments
  if (info.Length() < 2)
    return Nan::ThrowError(
        Nan::New("Wrong number of arguments").ToLocalChecked());
  if (!info[0]->IsArray() || !info[1]->IsBoolean())
    return Nan::ThrowError(Nan::New("Wrong argument types").ToLocalChecked());
  // Retrieving parameters (require_grad and tensor shape)
  const bool require_grad = Nan::To<bool>(info[1]).FromJust();
  const v8::Local<v8::Array> array = info[0].As<v8::Array>();
  const uint32_t length = array->Length();
  // Convert from v8::Array to std::vector
  std::vector<int64_t> dims;
  for (int i = 0; i < length; i++)
  {
    // v8::Local<v8::Value> v;
    int32_t d = array->Get(i)->Int32Value(Nan::GetCurrentContext()).FromJust();
    dims.push_back(d);
  }
  // Call the libtorch and create a new torchjs::Tensor object
  // wrapping the new torch::Tensor that was created by torch::ones
  at::Tensor v = torch::ones(dims, torch::requires_grad(require_grad));
  auto newinst = Tensor::NewInstance();
  Tensor *obj = Nan::ObjectWrap::Unwrap<Tensor>(newinst);
  obj->setTensor(v);
  info.GetReturnValue().Set(newinst);
}

NAN_METHOD(zeros)
{
  // Sanity checking of the arguments
  if (info.Length() < 2)
    return Nan::ThrowError(
        Nan::New("Wrong number of arguments").ToLocalChecked());
  if (!info[0]->IsArray() || !info[1]->IsBoolean())
    return Nan::ThrowError(Nan::New("Wrong argument types").ToLocalChecked());
  // Retrieving parameters (require_grad and tensor shape)
  const bool require_grad = Nan::To<bool>(info[1]).FromJust();
  const v8::Local<v8::Array> array = info[0].As<v8::Array>();
  const uint32_t length = array->Length();
  // Convert from v8::Array to std::vector
  std::vector<int64_t> dims;
  for (int i = 0; i < length; i++)
  {
    // v8::Local<v8::Value> v;
    int32_t d = array->Get(i)->Int32Value(Nan::GetCurrentContext()).FromJust();
    dims.push_back(d);
  }
  // Call the libtorch and create a new torchjs::Tensor object
  // wrapping the new torch::Tensor that was created by torch::zeros
  at::Tensor v = torch::zeros(dims, torch::requires_grad(require_grad));
  auto newinst = Tensor::NewInstance();
  Tensor *obj = Nan::ObjectWrap::Unwrap<Tensor>(newinst);
  obj->setTensor(v);
  info.GetReturnValue().Set(newinst);
}

template <typename T>
struct V8TypedArrayTraits; // no generic case
template <>
struct V8TypedArrayTraits<v8::Float32Array>
{
  typedef float value_type;
};
template <>
struct V8TypedArrayTraits<v8::Float64Array>
{
  typedef double value_type;
};
// etc. v8 doesn't export anything to make this nice.

template <typename T>
v8::Local<T> createTypedArray(size_t size)
{
  size_t byteLength = size * sizeof(typename V8TypedArrayTraits<T>::value_type);
  v8::Local<v8::ArrayBuffer> buffer =
      v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), byteLength);
  v8::Local<T> result = T::New(buffer, 0, size);
  return result;
};

NAN_GETTER(Tensor::HandleGetters)
{
  Tensor *self = Nan::ObjectWrap::Unwrap<Tensor>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "data")
  {
    uint64_t numel = self->mTensor.numel();
    v8::Local<v8::Float32Array> array =
        createTypedArray<v8::Float32Array>(numel);

    Nan::TypedArrayContents<float> typedArrayContents(array);
    float *dst = *typedArrayContents;

    torch::Tensor x = self->mTensor.contiguous();
    auto x_p = x.data_ptr<float>();
    numel = x.numel();

    for (size_t i = 0; i < numel; i++)
    {
      dst[i] = x_p[i];
    }

    info.GetReturnValue().Set(array);
  }
  else if (propertyName == "sizes")
  {
    uint8_t dim = self->mTensor.dim();
    at::IntArrayRef sizes = self->mTensor.sizes();
    v8::Local<v8::Array> array = Nan::New<v8::Array>(dim);
    for (size_t i = 0; i < dim; i++)
    {
      uint32_t size = sizes[i];
      Nan::Set(array, i, Nan::New(size));
    }
    info.GetReturnValue().Set(array);
  }
  else if (propertyName == "type")
  {
    std::stringstream ss;
    auto option = self->mTensor.options();
    ss << option.dtype() << "_" << option.device();
    v8::Local<v8::String> type = Nan::New<v8::String>(ss.str()).ToLocalChecked();
    info.GetReturnValue().Set(type);
  }
  else
  {
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_SETTER(Tensor::HandleSetters)
{
  Tensor *self = Nan::ObjectWrap::Unwrap<Tensor>(info.This());
  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "data")
  {
    if (!value->IsTypedArray())
    {
      return Nan::ThrowError(
          Nan::New("expected value to be a TypedArray").ToLocalChecked());
    }

    v8::Local<v8::Float32Array> array = value.As<v8::Float32Array>();
    Nan::TypedArrayContents<float> typedArrayContents(array);
    float *dst = *typedArrayContents;

    torch::Tensor x = self->mTensor.contiguous();
    auto x_p = x.data_ptr<float>();
    uint64_t numel = x.numel();

    for (size_t i = 0; i < numel; i++)
    {
      x_p[i] = dst[i];
    }
  }
}

NAN_METHOD(Tensor::cuda)
{
  torch::NoGradGuard no_grad;
  Tensor *tensor = ObjectWrap::Unwrap<Tensor>(info.Holder());
  tensor->mTensor.to(at::kCUDA);
}

NAN_METHOD(Tensor::cpu)
{
  torch::NoGradGuard no_grad;
  Tensor *tensor = ObjectWrap::Unwrap<Tensor>(info.Holder());
  tensor->mTensor.to(at::kCPU);
}

} // namespace torchjs