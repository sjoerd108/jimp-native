#include "wrapInvert.hpp"
#include "../modules/invert.hpp"
#include "../util/doAsync.hpp"

void wrapInvert (const Napi::CallbackInfo& info, ReferenceFactory& referenceFactory) {
    Napi::Env env = info.Env();

    if(info.Length() < 2 || info.Length() > 3) {
        throw Napi::Error::New(env, "Invalid number of arguments");
    }

    if(!info[0].IsBuffer()) {
        throw Napi::Error::New(env, "Image has to be of type buffer");
    }

    std::optional<Napi::Function> callback;
    if (info.Length() == 3 && info[2].IsFunction()) {
        callback = info[2].As<Napi::Function>();
    }

    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    uint8_t* pixelData = buffer.Data();
    size_t bufferLength = buffer.ElementLength();

    if(bufferLength % 4 != 0) {
        throw Napi::Error::New(env, "Buffer length must be divisible by four (RGBA)");
    }

    if (callback) {
        auto imageBufferReference = referenceFactory.ref(env, buffer);

        doAsync(
            env,
            callback.value(),
            [pixelData, bufferLength]() mutable {
                invert(pixelData, bufferLength / 4);
            },
            [imageBufferReference](Napi::Env env, Napi::Function callback, auto err) mutable {
                imageBufferReference.unref();
                callback.Call({ err ? Napi::String::New(env, err.value()) : env.Null() });
            }
        );
    } else {
        invert(pixelData, bufferLength / 4);
    }
}
