#pragma once

#include <string>
#include <vector>

// ==================================================
// Instrument types
// ==================================================

enum class InstrumentType
{
    Guitar,
    ElectricGuitar,
    Piano,
    ElectricPiano,
    Bass,
    Organ,
    SynthLead,
    SynthPad,
    Strings,
    Flute,
    Brass,
    Bell
};

// ==================================================
// Drum types
// ==================================================

enum class DrumType
{
    Kick,
    Snare,
    HiHat,
    OpenHiHat,
    Clap,
    Rimshot,
    Tom,
    LowTom,
    MidTom,
    HighTom,
    Crash,
    Ride,
    Cowbell,
    Tambourine,
    Shaker
};

// ==================================================
// Track types
// ==================================================

enum class TrackType
{
    Instrument,
    Audio
};

// ==================================================
// Note event
// ==================================================

struct NoteEvent
{
    InstrumentType instrument =
        InstrumentType::Guitar;

    int midiNote = 60;

    double startBeat = 0.0;

    double durationBeats = 1.0;
};

// ==================================================
// Drum event
// ==================================================

struct DrumEvent
{
    DrumType drum =
        DrumType::Kick;

    double startBeat = 0.0;
};

// ==================================================
// Track
// ==================================================

struct Track
{
    int number = 0;

    TrackType type =
        TrackType::Instrument;

    double lengthBeats = 0.0;

    double volume = 1.0;

    double pan = 0.0;

    std::vector<NoteEvent> notes;

    std::vector<DrumEvent> drums;

    std::string audioFile;

    double audioStartBeat = 0.0;
};

// ==================================================
// Project
// ==================================================

struct Project
{
    double tempo = 120.0;

    std::vector<Track> tracks;
};
