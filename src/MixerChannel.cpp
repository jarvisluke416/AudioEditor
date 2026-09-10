#include "MixerChannel.h"

#include <algorithm>

MixerChannel::MixerChannel(

    const std::string& name,
    std::size_t numSamples,
    int sampleRate)
    : name_(name),
      buffer_(numSamples, sampleRate)
{
}

const std::string& MixerChannel::name() const
{
    return name_;
}

EditorAudioBuffer& MixerChannel::buffer()
{
    return buffer_;
}

const EditorAudioBuffer& MixerChannel::buffer() const
{
    return buffer_;
}

float MixerChannel::volume() const
{
    return volume_;
}

void MixerChannel::setVolume(float volume)
{
    volume_ = std::clamp(volume, 0.0f, 1.0f);
}

float MixerChannel::pan() const
{
    return pan_;
}

void MixerChannel::setPan(float pan)
{
    pan_ = std::clamp(pan, -1.0f, 1.0f);
}

bool MixerChannel::isMuted() const
{
    return muted_;
}

void MixerChannel::setMuted(bool muted)
{
    muted_ = muted;
}

bool MixerChannel::isSolo() const
{
    return solo_;
}

void MixerChannel::setSolo(bool solo)
{
    solo_ = solo;
}
