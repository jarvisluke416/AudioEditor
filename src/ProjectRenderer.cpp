#include "ProjectRenderer.h"
#include "AudioFile.h"
#include "Mixer.h"
#include "SimpleSynth.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

// ==================================================
// Project rendering
// ==================================================

EditorAudioBuffer ProjectRenderer::render(
    const Project& project,
    int sampleRate)
{
    // ==================================================
    // Validate input
    // ==================================================

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

    if (project.tracks.empty())
    {
        throw std::runtime_error(
            "No track data loaded. "
            "The project contains zero tracks."
        );
    }

    // ==================================================
    // Tempo conversion
    // ==================================================

    const double secondsPerBeat =
        60.0 / project.tempo;

    // ==================================================
    // Find initial project length
    // ==================================================

    double totalBeats = 0.0;

    for (const auto& track : project.tracks)
    {
        totalBeats =
            std::max(
                totalBeats,
                track.lengthBeats
            );

        // ------------------------------------------
        // Notes
        // ------------------------------------------

        for (const auto& note : track.notes)
        {
            if (note.durationBeats > 0.0)
            {
                totalBeats =
                    std::max(
                        totalBeats,
                        note.startBeat +
                        note.durationBeats
                    );
            }
        }

        // ------------------------------------------
        // Drums
        // ------------------------------------------

        for (const auto& drum : track.drums)
        {
            totalBeats =
                std::max(
                    totalBeats,
                    drum.startBeat + 1.0
                );
        }

        // ------------------------------------------
        // Audio tracks
        // ------------------------------------------

        if (track.type == TrackType::Audio)
        {
            if (track.audioFile.empty())
            {
                throw std::runtime_error(
                    "Audio track " +
                    std::to_string(track.number) +
                    " has no FILE specified."
                );
            }

            totalBeats =
                std::max(
                    totalBeats,
                    track.audioStartBeat + 1.0
                );
        }
    }

    if (totalBeats <= 0.0)
        totalBeats = 1.0;

    // ==================================================
    // Load audio files first.
    //
    // This lets us determine their real duration.
    // ==================================================

    struct LoadedAudio
    {
        const Track* track = nullptr;
        EditorAudioBuffer audio;
    };

    std::vector<LoadedAudio> loadedAudio;

    for (const auto& track : project.tracks)
    {
        if (track.type != TrackType::Audio)
            continue;

        try
        {
            EditorAudioBuffer audio =
                AudioFile::load(
                    track.audioFile,
                    sampleRate
                );

            if (audio.size() == 0)
            {
                throw std::runtime_error(
                    "The decoded audio file contains zero samples."
                );
            }

            // ------------------------------------------
            // Extend project length to include audio.
            // ------------------------------------------

            const double audioSeconds =
                static_cast<double>(audio.size()) /
                static_cast<double>(sampleRate);

            const double audioBeats =
                audioSeconds /
                secondsPerBeat;

            totalBeats =
                std::max(
                    totalBeats,
                    track.audioStartBeat +
                    audioBeats
                );

            loadedAudio.push_back(
                LoadedAudio{
                    &track,
                    std::move(audio)
                }
            );
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(
                "Audio track " +
                std::to_string(track.number) +
                " failed to load file:\n" +
                track.audioFile +
                "\n\nReason: " +
                e.what()
            );
        }
        catch (...)
        {
            throw std::runtime_error(
                "Audio track " +
                std::to_string(track.number) +
                " failed to load file:\n" +
                track.audioFile
            );
        }
    }

    // ==================================================
    // Calculate final output size.
    // ==================================================

    const double totalSeconds =
        totalBeats * secondsPerBeat;

    const std::size_t totalSamples =
        static_cast<std::size_t>(
            std::ceil(
                totalSeconds *
                static_cast<double>(sampleRate)
            )
        );

    if (totalSamples == 0)
    {
        throw std::runtime_error(
            "Project produced zero audio samples."
        );
    }

    // ==================================================
    // Create master output buffer.
    // ==================================================

    EditorAudioBuffer output(
        totalSamples,
        sampleRate
    );

    bool renderedInstrumentData = false;
    bool renderedAudioData = false;

    // ==================================================
    // Render every track.
    // ==================================================

    for (const auto& track : project.tracks)
    {
        // ==================================================
        // INSTRUMENT TRACK
        // ==================================================

        if (track.type == TrackType::Instrument)
        {
            EditorAudioBuffer trackBuffer(
                totalSamples,
                sampleRate
            );

            // ------------------------------------------
            // Render melodic instruments.
            // ------------------------------------------

            for (const auto& note : track.notes)
            {
                if (note.durationBeats <= 0.0)
                    continue;

                SimpleSynth::renderNote(
                    trackBuffer,
                    note.instrument,
                    note.midiNote,
                    note.startBeat,
                    note.durationBeats,
                    project.tempo
                );

                renderedInstrumentData = true;
            }

            // ------------------------------------------
            // Render drums.
            // ------------------------------------------

            for (const auto& drum : track.drums)
            {
                switch (drum.drum)
                {
                    case DrumType::Kick:
                    {
                        SimpleSynth::renderKick(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::Snare:
                    {
                        SimpleSynth::renderSnare(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::HiHat:
                    {
                        SimpleSynth::renderHiHat(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo,
                            false
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::OpenHiHat:
                    {
                        SimpleSynth::renderHiHat(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo,
                            true
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::Clap:
                    {
                        SimpleSynth::renderClap(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::Rimshot:
                    {
                        SimpleSynth::renderRimshot(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::LowTom:
                    {
                        SimpleSynth::renderTom(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo,
                            110.0
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::MidTom:
                    {
                        SimpleSynth::renderTom(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo,
                            180.0
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::HighTom:
                    {
                        SimpleSynth::renderTom(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo,
                            280.0
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::Tom:
                    {
                        SimpleSynth::renderTom(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo,
                            190.0
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::Crash:
                    {
                        SimpleSynth::renderCrash(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::Ride:
                    {
                        SimpleSynth::renderRide(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::Cowbell:
                    {
                        SimpleSynth::renderCowbell(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::Tambourine:
                    {
                        SimpleSynth::renderTambourine(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo
                        );

                        renderedInstrumentData = true;
                        break;
                    }

                    case DrumType::Shaker:
                    {
                        SimpleSynth::renderShaker(
                            trackBuffer,
                            drum.startBeat,
                            project.tempo
                        );

                        renderedInstrumentData = true;
                        break;
                    }
                }
            }

            // ------------------------------------------
            // Mix instrument track into master.
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
        // AUDIO TRACK
        // ==================================================

        else if (track.type == TrackType::Audio)
        {
            const auto loaded =
                std::find_if(
                    loadedAudio.begin(),
                    loadedAudio.end(),
                    [&track](const LoadedAudio& item)
                    {
                        return item.track == &track;
                    }
                );

            if (loaded == loadedAudio.end())
            {
                throw std::runtime_error(
                    "Audio track " +
                    std::to_string(track.number) +
                    " was not loaded."
                );
            }

            const EditorAudioBuffer& audio =
                loaded->audio;

            // ------------------------------------------
            // Convert start beat to sample position.
            // ------------------------------------------

            const double startSeconds =
                track.audioStartBeat *
                secondsPerBeat;

            std::size_t startSample = 0;

            if (startSeconds > 0.0)
            {
                startSample =
                    static_cast<std::size_t>(
                        startSeconds *
                        static_cast<double>(sampleRate)
                    );
            }

            // ------------------------------------------
            // Mix audio into master.
            // ------------------------------------------

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

            renderedAudioData = true;
        }
    }

    // ==================================================
    // Verify that something rendered.
    // ==================================================

    if (!renderedInstrumentData &&
        !renderedAudioData)
    {
        throw std::runtime_error(
            "No playable track data was rendered. "
            "Check that the project contains notes, "
            "drums, or valid audio tracks."
        );
    }

    // ==================================================
    // MASTER PROCESSING
    //
    // Strong volume boost followed by soft limiting.
    // This keeps the mix loud without allowing huge
    // values to blow up the WAV.
    // ==================================================

    constexpr float masterGain = 1.65f;

    for (std::size_t i = 0;
         i < output.size();
         ++i)
    {
        float left =
            output.left()[i] *
            masterGain;

        float right =
            output.right()[i] *
            masterGain;

        // ------------------------------------------
        // Soft saturation / limiter.
        // ------------------------------------------

        left = std::tanh(left);
        right = std::tanh(right);

        output.left()[i] = left;
        output.right()[i] = right;
    }

    return output;
}
