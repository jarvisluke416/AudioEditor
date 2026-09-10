#pragma once

#include "AudioBuffer.h"

#include <string>

class MixerChannel
{
public:
    MixerChannel(
        const std::string& name,
        std::size_t numSamples,
        int sampleRate = 48000
    );

    const std::string& name() const;

    EditorAudioBuffer& buffer();
    const EditorAudioBuffer& buffer() const;

    float volume() const;
    void setVolume(float volume);

    float pan() const;
    void setPan(float pan);

    bool isMuted() const;
    void setMuted(bool muted);

    bool isSolo() const;
    void setSolo(bool solo);

private:
    std::string name_;

    EditorAudioBuffer buffer_;

    float volume_ = 1.0f;
    float pan_ = 0.0f;

    bool muted_ = false;
    bool solo_ = false;
};
