#include "Mixer.h"
#include <algorithm>

void Mixer::mix(
    EditorAudioBuffer& destination,
    const EditorAudioBuffer& source,
    std::size_t startSample,
    float volume)
{
    for (std::size_t i = 0; i < source.size(); ++i)
    {
        const std::size_t destinationSample =
            startSample + i;

        if (destinationSample >= destination.size())
            break;

        destination.add(
            destinationSample,
            source.left()[i] * volume,
            source.right()[i] * volume
        );
    }
}

void Mixer::mixPanned(
    EditorAudioBuffer& destination,
    const EditorAudioBuffer& source,
    std::size_t startSample,
    float volume,
    float pan)
{
    pan = std::clamp(pan, -1.0f, 1.0f);

    const float leftGain =
        0.5f * (1.0f - pan);

    const float rightGain =
        0.5f * (1.0f + pan);

    for (std::size_t i = 0; i < source.size(); ++i)
    {
        const std::size_t destinationSample =
            startSample + i;

        if (destinationSample >= destination.size())
            break;

        destination.add(
            destinationSample,
            source.left()[i] * volume * leftGain,
            source.right()[i] * volume * rightGain
        );
    }
}
