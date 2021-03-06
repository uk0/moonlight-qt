#include "slaud.h"

#include <SDL.h>

SLAudioRenderer::SLAudioRenderer()
    : m_AudioContext(nullptr),
      m_AudioStream(nullptr)
{
    SLAudio_SetLogFunction(SLAudioRenderer::slLogCallback, nullptr);
}

bool SLAudioRenderer::prepareForPlayback(const OPUS_MULTISTREAM_CONFIGURATION* opusConfig)
{
    m_AudioContext = SLAudio_CreateContext();
    if (m_AudioContext == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SLAudio_CreateContext() failed");
        return false;
    }

    m_AudioBufferSize = opusConfig->samplesPerFrame * sizeof(short) * opusConfig->channelCount;
    m_AudioStream = SLAudio_CreateStream(m_AudioContext,
                                         opusConfig->sampleRate,
                                         opusConfig->channelCount,
                                         m_AudioBufferSize,
                                         1);
    if (m_AudioStream == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SLAudio_CreateStream() failed");
        return false;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Using SLAudio renderer");

    return true;
}

void* SLAudioRenderer::getAudioBuffer(int* size)
{
    SDL_assert(*size == m_AudioBufferSize);
    return SLAudio_BeginFrame(m_AudioStream);
}

SLAudioRenderer::~SLAudioRenderer()
{
    if (m_AudioStream != nullptr) {
        SLAudio_FreeStream(m_AudioStream);
    }

    if (m_AudioContext != nullptr) {
        SLAudio_FreeContext(m_AudioContext);
    }
}

bool SLAudioRenderer::submitAudio(int)
{
    SLAudio_SubmitFrame(m_AudioStream);
    return true;
}

void SLAudioRenderer::slLogCallback(void*, ESLAudioLog logLevel, const char *message)
{
    SDL_LogPriority priority;

    switch (logLevel)
    {
    case k_ESLAudioLogError:
        priority = SDL_LOG_PRIORITY_ERROR;
        break;
    case k_ESLAudioLogWarning:
        priority = SDL_LOG_PRIORITY_WARN;
        break;
    case k_ESLAudioLogInfo:
        priority = SDL_LOG_PRIORITY_INFO;
        break;
    default:
    case k_ESLAudioLogDebug:
        priority = SDL_LOG_PRIORITY_DEBUG;
        break;
    }

    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                   priority,
                   "SLAudio: %s",
                   message);
}
