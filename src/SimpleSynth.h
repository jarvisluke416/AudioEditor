#pragma once
#include "AudioBuffer.h"
#include "Project.h"

class SimpleSynth
{
public:
    static void renderNote(
        EditorAudioBuffer& buffer,
        InstrumentType instrument,
        int midiNote,
        double startBeat,
        double durationBeats,
        double bpm
    );

    static void renderKick(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm
    );

    static void renderSnare(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm
    );

    static void renderHiHat(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm,
        bool open
    );

    static void renderClap(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm
    );

    static void renderRimshot(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm
    );

    static void renderTom(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm,
        double frequency
    );

    static void renderCrash(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm
    );

    static void renderRide(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm
    );

    static void renderCowbell(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm
    );

    static void renderTambourine(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm
    );

    static void renderShaker(
        EditorAudioBuffer& buffer,
        double startBeat,
        double bpm
    );

private:
    static double midiToFrequency(int midiNote);

    static std::size_t beatToSample(
        const EditorAudioBuffer& buffer,
        double beat,
        double bpm
    );

    static float softClip(float value);
};
