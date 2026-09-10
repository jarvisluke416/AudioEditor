#pragma once
#include "AudioBuffer.h"
#include "Project.h"
#include <cstddef>
#include <vector>

class Mixer
{
public:
    static void mix(
        EditorAudioBuffer& destination,
        const EditorAudioBuffer& source,
        std::size_t startSample,
        float volume = 1.0f
    );

    static void mixPanned(
        EditorAudioBuffer& destination,
        const EditorAudioBuffer& source,
        std::size_t startSample,
        float volume,
        float pan
    );

    static void mixTrack(
        EditorAudioBuffer& destination,
        const Track& track,
        std::size_t startSample
    );

    static void mixTracks(
        EditorAudioBuffer& destination,
        const std::vector<Track>& tracks
    );
};
