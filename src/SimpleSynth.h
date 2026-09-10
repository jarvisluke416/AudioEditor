#pragma once

#include "AudioBuffer.h"

class SimpleSynth
{
public:
    static void renderNote(
        EditorAudioBuffer& buffer,
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

private:
    static double midiToFrequency(int midiNote);
};
