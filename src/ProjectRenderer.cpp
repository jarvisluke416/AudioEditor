#include "ProjectRenderer.h"
#include "AudioFile.h"
#include "Mixer.h"
#include "SimpleSynth.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

// ==================================================
// Render complete project
// ==================================================

EditorAudioBuffer ProjectRenderer::render(
const Project& project,
int sampleRate)
{
// ----------------------------------------------
// Validate render settings
// ----------------------------------------------

if (sampleRate <= 0)
{
    throw std::runtime_error(
        "Invalid sample rate."
    );
}

if (project.tempo <= 0.0)
{
    throw std::runtime_error(
        "Invalid project tempo."
    );
}

// ----------------------------------------------
// Find total project length
// ----------------------------------------------

double totalBeats = 0.0;

for (const auto& track : project.tracks)
{
    totalBeats =
        std::max(
            totalBeats,
            track.lengthBeats
        );

    // ------------------------------------------
    // Instrument notes
    // ------------------------------------------

    for (const auto& note : track.notes)
    {
        totalBeats =
            std::max(
                totalBeats,
                note.startBeat +
                note.durationBeats
            );
    }

    // ------------------------------------------
    // Drum events
    // ------------------------------------------

    for (const auto& drum : track.drums)
    {
        // Give drum events enough space for
        // their generated sound.
        totalBeats =
            std::max(
                totalBeats,
                drum.startBeat + 1.0
            );
    }

    // ------------------------------------------
    // Audio track
    // ------------------------------------------

    if (track.type == TrackType::Audio &&
        !track.audioFile.empty())
    {
        // Make sure the project has at least
        // one beat after the audio START.
        //
        // The actual audio file is decoded below.
        totalBeats =
            std::max(
                totalBeats,
                track.audioStartBeat + 1.0
            );
    }
}

if (totalBeats <= 0.0)
    totalBeats = 1.0;

// ----------------------------------------------
// Convert beats to samples
// ----------------------------------------------

const double secondsPerBeat =
    60.0 / project.tempo;

const double totalSeconds =
    totalBeats * secondsPerBeat;

const std::size_t totalSamples =
    static_cast<std::size_t>(
        std::ceil(
            totalSeconds *
            sampleRate
        )
    );

EditorAudioBuffer output(
    totalSamples,
    sampleRate
);

// ----------------------------------------------
// Render every track
// ----------------------------------------------

for (std::size_t trackIndex = 0;
     trackIndex < project.tracks.size();
     ++trackIndex)
{
    const auto& track =
        project.tracks[trackIndex];

    // ==================================================
    // Instrument track
    // ==================================================

    if (track.type == TrackType::Instrument)
    {
        EditorAudioBuffer trackBuffer(
            totalSamples,
            sampleRate
        );

        // ------------------------------------------
        // Render notes
        // ------------------------------------------

        for (const auto& note : track.notes)
        {
            SimpleSynth::renderNote(
                trackBuffer,
                note.midiNote,
                note.startBeat,
                note.durationBeats,
                project.tempo
            );
        }

        // ------------------------------------------
        // Render drums
        // ------------------------------------------

        for (const auto& drum : track.drums)
        {
            switch (drum.drum)
            {
                case DrumType::Kick:
                    SimpleSynth::renderKick(
                        trackBuffer,
                        drum.startBeat,
                        project.tempo
                    );
                    break;

                case DrumType::Snare:
                    SimpleSynth::renderSnare(
                        trackBuffer,
                        drum.startBeat,
                        project.tempo
                    );
                    break;

                case DrumType::HiHat:
                case DrumType::Crash:
                    // These sounds are not implemented yet.
                    break;
            }
        }

        // ------------------------------------------
        // Mix instrument track
        // ------------------------------------------

        Mixer::mixPanned(
            output,
            trackBuffer,
            0,
            static_cast<float>(
                track.volume
            ),
            static_cast<float>(
                track.pan
            )
        );
    }

    // ==================================================
    // Audio track
    // ==================================================

    else if (track.type == TrackType::Audio)
    {
        // ------------------------------------------
        // Check that FILE exists in the project
        // ------------------------------------------

        if (track.audioFile.empty())
        {
            throw std::runtime_error(
                "Audio track " +
                std::to_string(trackIndex + 1) +
                " does not contain a FILE."
            );
        }

        try
        {
            // --------------------------------------
            // Load audio through AudioFile/FFmpeg
            // --------------------------------------

            EditorAudioBuffer audio =
                AudioFile::load(
                    track.audioFile,
                    sampleRate
                );

            // --------------------------------------
            // Calculate starting sample
            // --------------------------------------

            const double startPosition =
                track.audioStartBeat *
                secondsPerBeat *
                sampleRate;

            if (startPosition < 0.0)
            {
                throw std::runtime_error(
                    "Audio track " +
                    std::to_string(trackIndex + 1) +
                    " has a negative START value."
                );
            }

            const std::size_t startSample =
                static_cast<std::size_t>(
                    startPosition
                );

            // --------------------------------------
            // Mix audio into project
            // --------------------------------------

            Mixer::mixPanned(
                output,
                audio,
                startSample,
                static_cast<float>(
                    track.volume
                ),
                static_cast<float>(
                    track.pan
                )
            );
        }
        catch (const std::exception& e)
        {
            // --------------------------------------
            // IMPORTANT:
            //
            // Do NOT silently ignore audio errors.
            //
            // Include:
            //   - track number
            //   - exact audio filename
            //   - original error
            // --------------------------------------

            throw std::runtime_error(
                "Could not load audio file on track " +
                std::to_string(trackIndex + 1) +
                ":\n\n" +
                track.audioFile +
                "\n\nReason:\n" +
                e.what()
            );
        }
    }
}

return output;


}