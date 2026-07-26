#pragma once

#include "CSJUtils_Export.h"

#include <cstdint>
#include <memory>

#ifdef __cplusplus

namespace csjutils {

enum class CSJPixelFormat {
    CSJPixelFormat_NONE = 0,
    CSJPixelFormat_YUV420P,
    CSJPixelFormat_NV12,
    CSJPixelFormat_R8G8B8A8,
    CSJPixelFormat_B8G8R8A8
};

struct CSJVideoFrame {
    uint8_t       *data[4]     = {nullptr};
    int            linesize[4] = {0};
    int            width       = 0;
    int            height      = 0;
    int64_t        pts         = 0;
    double         duration    = 0.0;
    CSJPixelFormat format      = CSJPixelFormat::CSJPixelFormat_NONE;
    bool           isFile      = false; // If rendering with an image file, this field is true.

    CSJVideoFrame() {
        
    }
};

struct CSJAudioFrame {
    uint8_t *data          = nullptr;
    int      dataSize      = 0;
    int      channels      = 0;
    int      sampleRate    = 0;
    int      bitsPerSample = 0;
    int64_t  pts           = 0;
    double   duration      = 0.0;
};

extern "C" {
CSJUTILS_API CSJVideoFrame *create_video_frame();
CSJUTILS_API void           release_video_frame(CSJVideoFrame *frame);
CSJUTILS_API bool           alloc_video_buffer(CSJVideoFrame *frame, int width, int height, CSJPixelFormat format);
CSJUTILS_API CSJAudioFrame *create_audio_frame();
CSJUTILS_API void           release_audio_frame(CSJAudioFrame *frame);
CSJUTILS_API bool           alloc_audio_frame(CSJAudioFrame *frame, int channels, int sampleRate, int samplesPerChannel, int bitsPerSample);
}

using CSJVideoFramePtr = std::shared_ptr<CSJVideoFrame>;
using CSJAudioFramePtr = std::shared_ptr<CSJAudioFrame>;

inline CSJVideoFramePtr createVideoFramePtr() {
    return CSJVideoFramePtr(create_video_frame(), release_video_frame);
}

inline CSJAudioFramePtr createAudioFramePtr() {
    return CSJAudioFramePtr(create_audio_frame(), release_audio_frame);
}

} // namespace csjutils

#endif