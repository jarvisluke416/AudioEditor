#pragma once

#include "AudioBuffer.h"

#include <string>

class AudioFile
{
public:
    static EditorAudioBuffer load(
        const std::string& filename,
        int sampleRate = 48000
    );
};
