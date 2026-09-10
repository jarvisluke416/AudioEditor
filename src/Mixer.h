#pragma once

#include "AudioBuffer.h"

#include <cstddef>

class Mixer
{
public:
    // Mix source into destination starting at startSample.
    // volume = 1.0 means normal volume.
    static void mix(
        EditorAudioBuffer& destination,
        const EditorAudioBuffer& source,
        std::size_t startSample,
        float volume = 1.0f
    );

    // Mix with volume and stereo pan.
    // pan = -1.0 = full left
    // pan =  0.0 = center
    // pan =  1.0 = full right
    static void mixPanned(
        EditorAudioBuffer& destination,
        const EditorAudioBuffer& source,
        std::size_t startSample,
        float volume,
        float pan
    );
};
