#include "Parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

// ==================================================
// Parse .song project file
// ==================================================

Project Parser::parseFile(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        throw std::runtime_error(
            "Could not open project: " + filename
        );
    }

    Project project;

    Track* currentTrack = nullptr;

    std::string line;

    while (std::getline(file, line))
    {
        // ------------------------------------------
        // Remove comments
        // ------------------------------------------

        const auto comment = line.find('#');

        if (comment != std::string::npos)
            line = line.substr(0, comment);

        std::stringstream ss(line);

        std::string command;

        if (!(ss >> command))
            continue;

        // ==================================================
        // TEMPO
        // ==================================================

        if (command == "TEMPO")
        {
            double tempo;

            if (!(ss >> tempo))
            {
                throw std::runtime_error(
                    "Invalid TEMPO value."
                );
            }

            if (tempo < 1.0 || tempo > 480.0)
            {
                throw std::runtime_error(
                    "Tempo must be between 1 and 480 BPM."
                );
            }

            project.tempo = tempo;

            continue;
        }

        // ==================================================
        // TRACK
        // ==================================================

        if (command == "TRACK")
        {
            int number;
            std::string type;

            if (!(ss >> number >> type))
            {
                throw std::runtime_error(
                    "Invalid TRACK declaration."
                );
            }

            project.tracks.emplace_back();

            currentTrack = &project.tracks.back();

            currentTrack->number = number;

            if (type == "INSTRUMENT")
            {
                currentTrack->type =
                    TrackType::Instrument;
            }
            else if (type == "AUDIO")
            {
                currentTrack->type =
                    TrackType::Audio;
            }
            else
            {
                throw std::runtime_error(
                    "Unknown track type: " + type
                );
            }

            continue;
        }

        // ==================================================
        // Make sure a TRACK exists
        // ==================================================

        if (currentTrack == nullptr)
        {
            throw std::runtime_error(
                "Found data before a TRACK declaration."
            );
        }

        // ==================================================
        // LENGTH
        // ==================================================

        if (command == "LENGTH")
        {
            if (!(ss >> currentTrack->lengthBeats))
            {
                throw std::runtime_error(
                    "Invalid LENGTH value."
                );
            }

            if (currentTrack->lengthBeats < 0.0)
            {
                throw std::runtime_error(
                    "Track length cannot be negative."
                );
            }

            continue;
        }

        // ==================================================
        // VOLUME
        // ==================================================

        if (command == "VOLUME")
        {
            if (!(ss >> currentTrack->volume))
            {
                throw std::runtime_error(
                    "Invalid VOLUME value."
                );
            }

            if (currentTrack->volume < 0.0)
            {
                throw std::runtime_error(
                    "Track volume cannot be negative."
                );
            }

            continue;
        }

        // ==================================================
        // PAN
        // ==================================================

        if (command == "PAN")
        {
            if (!(ss >> currentTrack->pan))
            {
                throw std::runtime_error(
                    "Invalid PAN value."
                );
            }

            if (currentTrack->pan < -1.0 ||
                currentTrack->pan > 1.0)
            {
                throw std::runtime_error(
                    "Track pan must be between -1 and 1."
                );
            }

            continue;
        }

        // ==================================================
        // AUDIO
        //
        // New preferred syntax:
        //
        // AUDIO "vocal.wav" 0
        //
        // This allows audio to live on the same track
        // as instruments and drums.
        // ==================================================

        if (command == "AUDIO")
        {
            std::string audioFilename;
            double startBeat = 0.0;

            if (!(ss >> audioFilename))
            {
                throw std::runtime_error(
                    "Missing audio filename."
                );
            }

            if (!(ss >> startBeat))
            {
                throw std::runtime_error(
                    "Missing AUDIO start beat."
                );
            }

            if (startBeat < 0.0)
            {
                throw std::runtime_error(
                    "AUDIO start beat cannot be negative."
                );
            }

            // Remove surrounding quotes.
            if (audioFilename.size() >= 2 &&
                audioFilename.front() == '"' &&
                audioFilename.back() == '"')
            {
                audioFilename =
                    audioFilename.substr(
                        1,
                        audioFilename.size() - 2
                    );
            }

            currentTrack->audioFile =
                audioFilename;

            currentTrack->audioStartBeat =
                startBeat;

            continue;
        }

        // ==================================================
        // Legacy AUDIO FILE
        //
        // FILE "vocal.wav"
        // ==================================================

        if (command == "FILE")
        {
            std::string audioFilename;

            if (!(ss >> audioFilename))
            {
                throw std::runtime_error(
                    "Missing audio filename."
                );
            }

            if (audioFilename.size() >= 2 &&
                audioFilename.front() == '"' &&
                audioFilename.back() == '"')
            {
                audioFilename =
                    audioFilename.substr(
                        1,
                        audioFilename.size() - 2
                    );
            }

            currentTrack->audioFile =
                audioFilename;

            continue;
        }

        // ==================================================
        // Legacy AUDIO START
        //
        // START 0
        // ==================================================

        if (command == "START")
        {
            if (!(ss >> currentTrack->audioStartBeat))
            {
                throw std::runtime_error(
                    "Invalid START value."
                );
            }

            if (currentTrack->audioStartBeat < 0.0)
            {
                throw std::runtime_error(
                    "Audio START cannot be negative."
                );
            }

            continue;
        }

        // ==================================================
        // INSTRUMENT TRACK DATA
        // ==================================================

        if (currentTrack->type ==
            TrackType::Instrument)
        {
            // ==================================================
            // DRUM
            // ==================================================

            if (command == "DRUM")
            {
                std::string drumName;
                double startBeat;

                if (!(ss >> drumName >> startBeat))
                {
                    throw std::runtime_error(
                        "Invalid DRUM event."
                    );
                }

                if (startBeat < 0.0)
                {
                    throw std::runtime_error(
                        "Drum start beat cannot be negative."
                    );
                }

                DrumEvent event;

                event.drum =
                    parseDrum(drumName);

                event.startBeat =
                    startBeat;

                currentTrack->drums.push_back(
                    event
                );

                continue;
            }

            // ==================================================
            // NOTE
            // ==================================================

            std::string noteName;
            double startBeat;
            double duration;

            if (!(ss >> noteName >>
                  startBeat >>
                  duration))
            {
                throw std::runtime_error(
                    "Invalid instrument event: " +
                    command
                );
            }

            if (startBeat < 0.0)
            {
                throw std::runtime_error(
                    "Note start beat cannot be negative."
                );
            }

            if (duration <= 0.0)
            {
                throw std::runtime_error(
                    "Note duration must be greater than zero."
                );
            }

            NoteEvent event;

            event.instrument =
                parseInstrument(command);

            event.midiNote =
                noteToMidi(noteName);

            event.startBeat =
                startBeat;

            event.durationBeats =
                duration;

            currentTrack->notes.push_back(
                event
            );

            continue;
        }

        // ==================================================
        // Unknown command
        // ==================================================

        throw std::runtime_error(
            "Unknown command: " + command
        );
    }

    return project;
}

// ==================================================
// Convert musical note to MIDI
// ==================================================

int Parser::noteToMidi(
    const std::string& note)
{
    if (note.size() < 2)
    {
        throw std::runtime_error(
            "Invalid note: " + note
        );
    }

    char letter =
        static_cast<char>(
            std::toupper(
                static_cast<unsigned char>(
                    note[0]
                )
            )
        );

    int semitone = 0;

    switch (letter)
    {
        case 'C':
            semitone = 0;
            break;

        case 'D':
            semitone = 2;
            break;

        case 'E':
            semitone = 4;
            break;

        case 'F':
            semitone = 5;
            break;

        case 'G':
            semitone = 7;
            break;

        case 'A':
            semitone = 9;
            break;

        case 'B':
            semitone = 11;
            break;

        default:
            throw std::runtime_error(
                "Invalid note: " + note
            );
    }

    std::size_t index = 1;

    // ------------------------------------------
    // Sharp
    // ------------------------------------------

    if (index < note.size() &&
        note[index] == '#')
    {
        semitone++;
        index++;
    }

    // ------------------------------------------
    // Flat
    // ------------------------------------------

    else if (index < note.size() &&
             note[index] == 'b')
    {
        semitone--;
        index++;
    }

    if (index >= note.size())
    {
        throw std::runtime_error(
            "Missing octave: " + note
        );
    }

    int octave;

    try
    {
        octave =
            std::stoi(
                note.substr(index)
            );
    }
    catch (...)
    {
        throw std::runtime_error(
            "Invalid octave: " + note
        );
    }

    if (octave < -1 || octave > 9)
    {
        throw std::runtime_error(
            "Octave out of range: " + note
        );
    }

    return (octave + 1) * 12 +
           semitone;
}

// ==================================================
// Parse instrument
// ==================================================

InstrumentType Parser::parseInstrument(
    const std::string& name)
{
    if (name == "GUITAR")
        return InstrumentType::Guitar;

    if (name == "ELECTRICGUITAR")
        return InstrumentType::ElectricGuitar;

    if (name == "PIANO")
        return InstrumentType::Piano;

    if (name == "ELECTRICPIANO")
        return InstrumentType::ElectricPiano;

    if (name == "BASS")
        return InstrumentType::Bass;

    if (name == "ORGAN")
        return InstrumentType::Organ;

    if (name == "SYNTHLEAD")
        return InstrumentType::SynthLead;

    if (name == "SYNTHPAD")
        return InstrumentType::SynthPad;

    if (name == "STRINGS")
        return InstrumentType::Strings;

    if (name == "FLUTE")
        return InstrumentType::Flute;

    if (name == "BRASS")
        return InstrumentType::Brass;

    if (name == "BELL")
        return InstrumentType::Bell;

    throw std::runtime_error(
        "Unknown instrument: " + name
    );
}

// ==================================================
// Parse drums
// ==================================================

DrumType Parser::parseDrum(
    const std::string& name)
{
    if (name == "KICK")
        return DrumType::Kick;

    if (name == "SNARE")
        return DrumType::Snare;

    if (name == "HIHAT")
        return DrumType::HiHat;

    if (name == "OPENHIHAT")
        return DrumType::OpenHiHat;

    if (name == "CLAP")
        return DrumType::Clap;

    if (name == "RIMSHOT")
        return DrumType::Rimshot;

    if (name == "TOM")
        return DrumType::Tom;

    if (name == "LOWTOM")
        return DrumType::LowTom;

    if (name == "MIDTOM")
        return DrumType::MidTom;

    if (name == "HIGHTOM")
        return DrumType::HighTom;

    if (name == "CRASH")
        return DrumType::Crash;

    if (name == "RIDE")
        return DrumType::Ride;

    if (name == "COWBELL")
        return DrumType::Cowbell;

    if (name == "TAMBOURINE")
        return DrumType::Tambourine;

    if (name == "SHAKER")
        return DrumType::Shaker;

    throw std::runtime_error(
        "Unknown drum: " + name
    );
}