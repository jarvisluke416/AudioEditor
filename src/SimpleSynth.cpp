#include "SimpleSynth.h"
#include <cmath>
#include <random>
#include <algorithm>

namespace
{
    constexpr double PI =
        3.14159265358979323846;
}

double SimpleSynth::midiToFrequency(int midiNote)
{
    return 440.0 *
           std::pow(
               2.0,
               (midiNote - 69) / 12.0
           );
}

void SimpleSynth::renderNote(
    EditorAudioBuffer& buffer,
    int midiNote,
    double startBeat,
    double durationBeats,
    double bpm)
{
    const double secondsPerBeat =
        60.0 / bpm;

    const std::size_t startSample =
        static_cast<std::size_t>(
            startBeat *
            secondsPerBeat *
            buffer.sampleRate()
        );

    const std::size_t durationSamples =
        static_cast<std::size_t>(
            durationBeats *
            secondsPerBeat *
            buffer.sampleRate()
        );

    const double frequency =
        midiToFrequency(midiNote);

    for (std::size_t i = 0;
         i < durationSamples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        // Simple guitar-like harmonic tone.
        const double fundamental =
            std::sin(
                2.0 * PI * frequency * t
            );

        const double harmonic2 =
            0.35 * std::sin(
                2.0 * PI * frequency * 2.0 * t
            );

        const double harmonic3 =
            0.15 * std::sin(
                2.0 * PI * frequency * 3.0 * t
            );

        // Quick attack + decay.
        const double attack =
            std::min(1.0, t / 0.01);

        const double decay =
            std::exp(-3.0 * t);

        const float value =
            static_cast<float>(
                (fundamental +
                 harmonic2 +
                 harmonic3) *
                attack *
                decay *
                0.18
            );

        buffer.add(sample, value, value);
    }
}

void SimpleSynth::renderKick(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const double startSeconds =
        startBeat * 60.0 / bpm;

    const std::size_t startSample =
        static_cast<std::size_t>(
            startSeconds *
            buffer.sampleRate()
        );

    constexpr double duration = 0.35;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate()
        );

    for (std::size_t i = 0; i < samples; ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double frequency =
            120.0 * std::exp(-12.0 * t);

        const double envelope =
            std::exp(-10.0 * t);

        const float value =
            static_cast<float>(
                std::sin(
                    2.0 * PI *
                    frequency *
                    t
                ) *
                envelope *
                0.45
            );

        buffer.add(sample, value, value);
    }
}

void SimpleSynth::renderSnare(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const double startSeconds =
        startBeat * 60.0 / bpm;

    const std::size_t startSample =
        static_cast<std::size_t>(
            startSeconds *
            buffer.sampleRate()
        );

    constexpr double duration = 0.20;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate()
        );

    std::mt19937 generator(42);
    std::uniform_real_distribution<float>
        noise(-1.0f, 1.0f);

    for (std::size_t i = 0; i < samples; ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double envelope =
            std::exp(-18.0 * t);

        const float value =
            noise(generator) *
            static_cast<float>(
                envelope * 0.18
            );

        buffer.add(sample, value, value);
    }
}
