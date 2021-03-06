#pragma once

#include "renderer.h"
#include <SLAudio.h>

class SLAudioRenderer : public IAudioRenderer
{
public:
    SLAudioRenderer();

    virtual ~SLAudioRenderer();

    virtual bool prepareForPlayback(const OPUS_MULTISTREAM_CONFIGURATION* opusConfig);

    virtual void* getAudioBuffer(int* size);

    virtual bool submitAudio(int bytesWritten);

private:
    static void slLogCallback(void* context, ESLAudioLog logLevel, const char* message);

    CSLAudioContext* m_AudioContext;
    CSLAudioStream* m_AudioStream;

    int m_AudioBufferSize;
};
