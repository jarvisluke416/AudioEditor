#pragma once
#include "AudioBuffer.h"
#include <string>

class WavWriter
{
public:
    static bool write(
        const std::string& filename,
        const EditorAudioBuffer& buffer
    );
};
