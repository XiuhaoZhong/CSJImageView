#include "CSJMediaData.h"

#include <cstdlib>
#include <cstring>

namespace csjutils {

CSJVideoFrame *create_video_frame() {
    return new (std::nothrow) CSJVideoFrame();
}

void release_video_frame(CSJVideoFrame *frame) {
    if (!frame) {
        return ;
    }

    for (int i = 0; i < 4; i++) {
        if (frame->data[i]) {
            free(frame->data[i]);
            frame->data[i] = nullptr;
        }
    }

    delete frame;
}

bool alloc_video_buffer(CSJVideoFrame *frame, int width, int height, CSJPixelFormat format) {
    if (!frame) {
        return false;
    }

    frame->width = width;
    frame->height = height;
    frame->format = format;

    if (format == CSJPixelFormat::CSJPixelFormat_YUV420P) {
        frame->linesize[0] = width;
        frame->linesize[1] = width / 2;
        frame->linesize[2] = width / 2;

        frame->data[0] = (uint8_t*)malloc(width * height);
        frame->data[1] = (uint8_t*)malloc(width * height / 4);
        frame->data[2] = (uint8_t*)malloc(width * height / 4);
    } else if (format == CSJPixelFormat::CSJPixelFormat_NV12) {
        frame->linesize[0] = width;
        frame->linesize[1] = width;

        frame->data[0] = (uint8_t*)malloc(width * height);
        frame->data[1] = (uint8_t*)malloc(width * height / 2);
    } else if (format == CSJPixelFormat::CSJPixelFormat_R8G8B8A8 || 
                format == CSJPixelFormat::CSJPixelFormat_B8G8R8A8) {
        frame->linesize[0] = width * 4;
        frame->data[0] = (uint8_t*)malloc(width * height * 4);
    } else {
        return false;
    }

    return true;
}

CSJAudioFrame *create_audio_frame() {
    return new (std::nothrow) CSJAudioFrame();
}

void release_audio_frame(CSJAudioFrame *frame) {
    if (!frame) {
        return ;
    }

    if (frame->data) {
        free(frame->data);
        frame->data = nullptr;
    }

    delete frame;
}

bool alloc_audio_frame(CSJAudioFrame *frame, int channels, int sampleRate, int samplesPerChannel, int bitsPerSample) {
    if (!frame) {
        return false;
    }

    frame->channels = channels;
    frame->sampleRate = sampleRate;
    frame->bitsPerSample = bitsPerSample;  

    int bytesPerSample = bitsPerSample / 8;
    frame->dataSize = samplesPerChannel * channels * bytesPerSample;

    // Utils 内部 malloc
    frame->data = (uint8_t*)malloc(frame->dataSize);
    return frame->data != nullptr;
}

} // namespace csjutils