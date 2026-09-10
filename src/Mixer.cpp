#include "Mixer.h"
#include <algorithm>
#include <cmath>

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

    constexpr double PI =
        3.14159265358979323846;

    const double angle =
        (static_cast<double>(pan) + 1.0) *
        0.25 *
        PI;

    const float leftGain =
        static_cast<float>(std::cos(angle)) * volume;

    const float rightGain =
        static_cast<float>(std::sin(angle)) * volume;

    for (std::size_t i = 0; i < source.size(); ++i)
    {
        const std::size_t destinationSample =
            startSample + i;

        if (destinationSample >= destination.size())
            break;

        destination.add(
            destinationSample,
            source.left()[i] * leftGain,
            source.right()[i] * rightGain
        );
    }
}
