#pragma once
#include <string>
#include <vector>

// ==================================================
// Instrument types
// ==================================================

enum class InstrumentType
{
    Guitar,
    Piano,
    Bass
};

// ==================================================
// Drum types
// ==================================================

enum class DrumType
{
    Kick,
    Snare,
    HiHat,
    Crash
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
    InstrumentType instrument;

    int midiNote = 60;

    double startBeat = 0.0;

    double durationBeats = 1.0;
};

// ==================================================
// Drum event
// ==================================================

struct DrumEvent
{
    DrumType drum;

    double startBeat = 0.0;
};

// ==================================================
// Track
// ==================================================

struct Track
{
    int number = 0;

    TrackType type = TrackType::Instrument;

    double lengthBeats = 0.0;

    // Mixer controls
    double volume = 1.0;
    double pan = 0.0;


    // Instrument events
    std::vector<NoteEvent> notes;

    std::vector<DrumEvent> drums;

    // Audio track
    std::string audioFile;

    double audioStartBeat = 0.0;
};

// ==================================================
// Project
// ==================================================

struct Project
{
    // Initial project tempo.
    // Valid range: 1–480 BPM.
    double tempo = 120.0;

    std::vector<Track> tracks;
};
