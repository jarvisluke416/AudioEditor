#pragma once
#include "Project.h"
#include "AudioBuffer.h"

class ProjectRenderer
{
public:
    static EditorAudioBuffer render(
        const Project& project,
        int sampleRate = 48000
    );
};
