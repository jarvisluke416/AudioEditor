#include "SimpleSynth.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace
{
    constexpr double PI =
        3.14159265358979323846;

    double sine(double phase)
    {
        return std::sin(phase);
    }

    double saw(double phase)
    {
        const double cycles =
            phase / (2.0 * PI);

        return 2.0 *
               (cycles -
                std::floor(cycles + 0.5));
    }

    double square(double phase)
    {
        return std::sin(phase) >= 0.0
            ? 1.0
            : -1.0;
    }

    double triangle(double phase)
    {
        const double cycles =
            phase / (2.0 * PI);

        return 2.0 *
               std::abs(
                   2.0 *
                   (cycles -
                    std::floor(cycles + 0.5)))
               - 1.0;
    }

    float clampAudio(float value)
    {
        return std::clamp(
            value,
            -1.0f,
            1.0f);
    }
}

// ==================================================
// MIDI -> frequency
// ==================================================

double SimpleSynth::midiToFrequency(
    int midiNote)
{
    return 440.0 *
           std::pow(
               2.0,
               (midiNote - 69) / 12.0);
}

// ==================================================
// Beat -> sample
// ==================================================

std::size_t SimpleSynth::beatToSample(
    const EditorAudioBuffer& buffer,
    double beat,
    double bpm)
{
    if (bpm <= 0.0 || beat < 0.0)
        return 0;

    return static_cast<std::size_t>(
        beat *
        (60.0 / bpm) *
        static_cast<double>(
            buffer.sampleRate()));
}

// ==================================================
// Soft clipping
// ==================================================

float SimpleSynth::softClip(float value)
{
    return static_cast<float>(
        std::tanh(
            static_cast<double>(value)));
}

// ==================================================
// Melodic instruments
// ==================================================

void SimpleSynth::renderNote(
    EditorAudioBuffer& buffer,
    InstrumentType instrument,
    int midiNote,
    double startBeat,
    double durationBeats,
    double bpm)
{
    if (bpm <= 0.0 ||
        durationBeats <= 0.0)
        return;

    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    const std::size_t durationSamples =
        static_cast<std::size_t>(
            durationBeats *
            (60.0 / bpm) *
            buffer.sampleRate());

    const double frequency =
        midiToFrequency(midiNote);

    const double sampleRate =
        static_cast<double>(
            buffer.sampleRate());

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
            sampleRate;

        const double normalized =
            durationSamples > 0
                ? static_cast<double>(i) /
                  static_cast<double>(durationSamples)
                : 0.0;

        double value = 0.0;

        // ==========================================
        // GUITAR
        // ==========================================

        if (instrument ==
            InstrumentType::Guitar)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double harmonics =
                sine(phase) +
                0.32 * sine(phase * 2.0) +
                0.16 * sine(phase * 3.0) +
                0.08 * sine(phase * 4.0);

            const double attack =
                std::min(
                    1.0,
                    t / 0.006);

            const double envelope =
                attack *
                std::exp(-2.8 * t);

            value =
                harmonics *
                envelope *
                0.48;
        }

        // ==========================================
        // ELECTRIC GUITAR
        // ==========================================

        else if (instrument ==
                 InstrumentType::ElectricGuitar)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            double wave =
                0.65 * saw(phase) +
                0.25 * sine(phase * 2.0) +
                0.10 * sine(phase * 3.0);

            wave =
                std::tanh(
                    wave * 3.0);

            const double envelope =
                std::min(
                    1.0,
                    t / 0.004) *
                std::exp(-2.0 * t);

            value =
                wave *
                envelope *
                0.34;
        }

        // ==========================================
        // PIANO
        // ==========================================

        else if (instrument ==
                 InstrumentType::Piano)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double piano =
                sine(phase) +
                0.50 * sine(phase * 2.0) +
                0.25 * sine(phase * 3.0) +
                0.12 * sine(phase * 4.0) +
                0.06 * sine(phase * 5.0);

            const double attack =
                std::exp(-18.0 * t);

            const double body =
                std::exp(-1.7 * t);

            value =
                piano *
                (0.35 * attack +
                 0.75 * body) *
                0.30;
        }

        // ==========================================
        // ELECTRIC PIANO
        // ==========================================

        else if (instrument ==
                 InstrumentType::ElectricPiano)
        {
            const double carrier =
                2.0 * PI *
                frequency *
                t;

            const double modulation =
                3.5 *
                sine(
                    2.0 * PI *
                    frequency *
                    2.0 *
                    t);

            const double wave =
                sine(
                    carrier +
                    modulation);

            const double envelope =
                (1.0 -
                 std::exp(-12.0 * t)) *
                std::exp(-1.1 * t);

            value =
                wave *
                envelope *
                0.38;
        }

        // ==========================================
        // BASS
        // ==========================================

        else if (instrument ==
                 InstrumentType::Bass)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double fundamental =
                sine(phase);

            const double harmonic =
                0.35 *
                sine(phase * 2.0);

            const double sub =
                0.30 *
                sine(
                    2.0 *
                    PI *
                    (frequency * 0.5) *
                    t);

            const double attack =
                std::min(
                    1.0,
                    t / 0.012);

            const double envelope =
                attack *
                std::exp(-0.9 * t);

            value =
                (fundamental +
                 harmonic +
                 sub) *
                envelope *
                0.55;
        }

        // ==========================================
        // ORGAN
        // ==========================================

        else if (instrument ==
                 InstrumentType::Organ)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double organ =
                sine(phase) +
                0.60 * sine(phase * 2.0) +
                0.35 * sine(phase * 3.0) +
                0.20 * sine(phase * 4.0) +
                0.12 * sine(phase * 6.0);

            const double attack =
                std::min(
                    1.0,
                    t / 0.06);

            const double release =
                normalized > 0.82
                    ? (1.0 - normalized) / 0.18
                    : 1.0;

            value =
                organ *
                attack *
                release *
                0.23;
        }

        // ==========================================
        // SYNTH LEAD
        // ==========================================

        else if (instrument ==
                 InstrumentType::SynthLead)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double wave =
                0.72 * saw(phase) +
                0.18 * square(phase * 0.5) +
                0.10 * sine(phase * 2.0);

            const double envelope =
                std::min(
                    1.0,
                    t / 0.015) *
                std::exp(-0.35 * t);

            value =
                wave *
                envelope *
                0.34;
        }

        // ==========================================
        // SYNTH PAD
        // ==========================================

        else if (instrument ==
                 InstrumentType::SynthPad)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double detunedPhase =
                2.0 * PI *
                (frequency * 1.006) *
                t;

            const double wave =
                0.5 * saw(phase) +
                0.5 * saw(detunedPhase);

            const double attack =
                std::min(
                    1.0,
                    t / 0.35);

            const double release =
                normalized > 0.75
                    ? (1.0 - normalized) / 0.25
                    : 1.0;

            value =
                wave *
                attack *
                release *
                0.24;
        }

        // ==========================================
        // STRINGS
        // ==========================================

        else if (instrument ==
                 InstrumentType::Strings)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double vibrato =
                0.004 *
                sine(
                    2.0 * PI *
                    5.2 *
                    t);

            const double bowed =
                triangle(
                    phase +
                    vibrato);

            const double harmonic =
                0.30 *
                sine(phase * 2.0);

            const double attack =
                std::min(
                    1.0,
                    t / 0.22);

            const double release =
                normalized > 0.8
                    ? (1.0 - normalized) / 0.2
                    : 1.0;

            value =
                (bowed +
                 harmonic) *
                attack *
                release *
                0.25;
        }

        // ==========================================
        // FLUTE
        // ==========================================

        else if (instrument ==
                 InstrumentType::Flute)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double breath =
                0.025 *
                std::sin(
                    2.0 * PI *
                    17.0 *
                    t);

            const double flute =
                sine(phase) +
                0.08 * sine(phase * 2.0) +
                breath;

            const double attack =
                std::min(
                    1.0,
                    t / 0.12);

            const double release =
                normalized > 0.85
                    ? (1.0 - normalized) / 0.15
                    : 1.0;

            value =
                flute *
                attack *
                release *
                0.30;
        }

        // ==========================================
        // BRASS
        // ==========================================

        else if (instrument ==
                 InstrumentType::Brass)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double brass =
                saw(phase) +
                0.25 * sine(phase * 2.0);

            const double attack =
                1.0 -
                std::exp(-7.0 * t);

            const double envelope =
                attack *
                std::exp(-0.65 * t);

            value =
                brass *
                envelope *
                0.27;
        }

        // ==========================================
        // BELL
        // ==========================================

        else if (instrument ==
                 InstrumentType::Bell)
        {
            const double phase =
                2.0 * PI *
                frequency *
                t;

            const double bell =
                sine(phase) +
                0.55 * sine(phase * 2.71) +
                0.32 * sine(phase * 4.16) +
                0.18 * sine(phase * 6.83);

            const double envelope =
                std::exp(-2.4 * t);

            value =
                bell *
                envelope *
                0.30;
        }

        const float output =
            softClip(
                static_cast<float>(value));

        buffer.add(
            sample,
            output,
            output);
    }
}

// ==================================================
// Kick
// ==================================================

void SimpleSynth::renderKick(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        0.50;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double frequency =
            165.0 *
            std::exp(-15.0 * t) +
            42.0;

        const double envelope =
            std::exp(-7.5 * t);

        const double body =
            sine(
                2.0 * PI *
                frequency *
                t);

        const double click =
            std::exp(-80.0 * t) *
            sine(
                2.0 * PI *
                1800.0 *
                t);

        const float value =
            softClip(
                static_cast<float>(
                    (body * 0.90 +
                     click * 0.12) *
                    envelope *
                    0.95));

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Snare
// ==================================================

void SimpleSynth::renderSnare(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        0.32;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    std::mt19937 generator(
        static_cast<unsigned>(
            startSample + 12345));

    std::uniform_real_distribution<float>
        noise(-1.0f, 1.0f);

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double noiseEnvelope =
            std::exp(-15.0 * t);

        const double bodyEnvelope =
            std::exp(-22.0 * t);

        const double body =
            sine(
                2.0 * PI *
                190.0 *
                t);

        const float value =
            softClip(
                static_cast<float>(
                    noise(generator) *
                    noiseEnvelope *
                    0.70 +
                    body *
                    bodyEnvelope *
                    0.30));

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Hi-hat
// ==================================================

void SimpleSynth::renderHiHat(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm,
    bool open)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    const double duration =
        open ? 0.50 : 0.12;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    std::mt19937 generator(
        static_cast<unsigned>(
            startSample + 8765));

    std::uniform_real_distribution<float>
        noise(-1.0f, 1.0f);

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double decay =
            open
                ? std::exp(-7.0 * t)
                : std::exp(-35.0 * t);

        const double metallic =
            square(
                2.0 * PI *
                7200.0 *
                t) *
            0.30;

        const float value =
            static_cast<float>(
                (noise(generator) +
                 metallic) *
                decay *
                0.25);

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Clap
// ==================================================

void SimpleSynth::renderClap(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        0.30;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    std::mt19937 generator(
        static_cast<unsigned>(
            startSample + 321));

    std::uniform_real_distribution<float>
        noise(-1.0f, 1.0f);

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double burst1 =
            std::exp(-80.0 *
                     std::abs(t - 0.005));

        const double burst2 =
            std::exp(-65.0 *
                     std::abs(t - 0.035));

        const double tail =
            std::exp(-18.0 * t);

        const float value =
            noise(generator) *
            static_cast<float>(
                (burst1 +
                 burst2 +
                 tail * 0.45) *
                0.45);

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Rimshot
// ==================================================

void SimpleSynth::renderRimshot(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        0.12;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double envelope =
            std::exp(-35.0 * t);

        const double tone =
            sine(
                2.0 * PI *
                1200.0 *
                t);

        const float value =
            static_cast<float>(
                tone *
                envelope *
                0.60);

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Tom
// ==================================================

void SimpleSynth::renderTom(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm,
    double frequency)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        0.45;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double pitch =
            frequency *
            std::exp(-3.0 * t);

        const double envelope =
            std::exp(-6.0 * t);

        const float value =
            static_cast<float>(
                sine(
                    2.0 * PI *
                    pitch *
                    t) *
                envelope *
                0.65);

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Crash
// ==================================================

void SimpleSynth::renderCrash(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        1.20;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    std::mt19937 generator(
        static_cast<unsigned>(
            startSample + 555));

    std::uniform_real_distribution<float>
        noise(-1.0f, 1.0f);

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double envelope =
            std::exp(-3.2 * t);

        const double metal =
            sine(
                2.0 * PI *
                5300.0 *
                t) *
            0.35;

        const float value =
            (noise(generator) +
             static_cast<float>(metal)) *
            static_cast<float>(
                envelope *
                0.20);

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Ride
// ==================================================

void SimpleSynth::renderRide(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        0.80;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double envelope =
            std::exp(-3.0 * t);

        const double metal =
            sine(
                2.0 * PI *
                3100.0 *
                t) +
            0.50 *
            sine(
                2.0 * PI *
                5100.0 *
                t);

        const float value =
            static_cast<float>(
                metal *
                envelope *
                0.18);

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Cowbell
// ==================================================

void SimpleSynth::renderCowbell(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        0.25;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    for (std::size_t i = 0;
         i < samples;
         ++i)
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

        const double tone =
            square(
                2.0 * PI *
                540.0 *
                t) *
            0.5 +
            square(
                2.0 * PI *
                800.0 *
                t) *
            0.5;

        const float value =
            static_cast<float>(
                tone *
                envelope *
                0.35);

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Tambourine
// ==================================================

void SimpleSynth::renderTambourine(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        0.30;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    std::mt19937 generator(
        static_cast<unsigned>(
            startSample + 777));

    std::uniform_real_distribution<float>
        noise(-1.0f, 1.0f);

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double envelope =
            std::exp(-13.0 * t);

        const float value =
            noise(generator) *
            static_cast<float>(
                envelope *
                0.30);

        buffer.add(
            sample,
            value,
            value);
    }
}

// ==================================================
// Shaker
// ==================================================

void SimpleSynth::renderShaker(
    EditorAudioBuffer& buffer,
    double startBeat,
    double bpm)
{
    const std::size_t startSample =
        beatToSample(
            buffer,
            startBeat,
            bpm);

    constexpr double duration =
        0.18;

    const std::size_t samples =
        static_cast<std::size_t>(
            duration *
            buffer.sampleRate());

    std::mt19937 generator(
        static_cast<unsigned>(
            startSample + 999));

    std::uniform_real_distribution<float>
        noise(-1.0f, 1.0f);

    for (std::size_t i = 0;
         i < samples;
         ++i)
    {
        const std::size_t sample =
            startSample + i;

        if (sample >= buffer.size())
            break;

        const double t =
            static_cast<double>(i) /
            buffer.sampleRate();

        const double envelope =
            std::exp(-22.0 * t);

        const float value =
            noise(generator) *
            static_cast<float>(
                envelope *
                0.20);

        buffer.add(
            sample,
            value,
            value);
    }
}
