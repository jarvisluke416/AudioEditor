#include "TimelineComponent.h"

#include <algorithm>
#include <cmath>

// ==================================================
// Constructor
// ==================================================

TimelineComponent::TimelineComponent()
{
    setOpaque(true);
}

// ==================================================
// Set project
// ==================================================

void TimelineComponent::setProject(
    const Project* project)
{
    project_ = project;

    repaint();
}

// ==================================================
// Paint
// ==================================================

void TimelineComponent::paint(
    juce::Graphics& g)
{
    // ==================================================
    // Background
    // ==================================================

    g.fillAll(
        juce::Colour::fromRGB(
            24,
            24,
            28));

    // ==================================================
    // Layout constants
    // ==================================================

    constexpr int rulerHeight = 32;
    constexpr int trackHeight = 80;

    constexpr double pixelsPerBeat = 100.0;

    const int width = getWidth();
    const int height = getHeight();

    // ==================================================
    // Timeline ruler
    // ==================================================

    g.setColour(
        juce::Colour::fromRGB(
            32,
            32,
            38));

    g.fillRect(
        0,
        0,
        width,
        rulerHeight);

    // ==================================================
    // No project
    // ==================================================

    if (project_ == nullptr)
    {
        g.setColour(
            juce::Colours::lightgrey);

        g.setFont(16.0f);

        g.drawText(
            "No project loaded",
            20,
            rulerHeight + 20,
            width - 40,
            30,
            juce::Justification::centredLeft);

        return;
    }

    // ==================================================
    // Calculate visible beats
    // ==================================================

    const double visibleBeats =
        static_cast<double>(width) /
        pixelsPerBeat;

    double totalBeats = 0.0;

    for (const auto& track : project_->tracks)
    {
        totalBeats =
            std::max(
                totalBeats,
                track.lengthBeats);

        for (const auto& note : track.notes)
        {
            totalBeats =
                std::max(
                    totalBeats,
                    note.startBeat +
                    note.durationBeats);
        }

        for (const auto& drum : track.drums)
        {
            totalBeats =
                std::max(
                    totalBeats,
                    drum.startBeat + 1.0);
        }

        if (track.type == TrackType::Audio)
        {
            totalBeats =
                std::max(
                    totalBeats,
                    track.audioStartBeat + 4.0);
        }
    }

    totalBeats =
        std::max(
            totalBeats,
            visibleBeats);

    // ==================================================
    // Beat grid
    // ==================================================

    for (int beat = 0;
         static_cast<double>(beat) <= totalBeats;
         ++beat)
    {
        const int x =
            static_cast<int>(
                beat * pixelsPerBeat);

        // Every fourth beat is slightly stronger.
        if (beat % 4 == 0)
        {
            g.setColour(
                juce::Colour::fromRGB(
                    70,
                    70,
                    78));
        }
        else
        {
            g.setColour(
                juce::Colour::fromRGB(
                    45,
                    45,
                    52));
        }

        g.drawVerticalLine(
            x,
            static_cast<float>(rulerHeight),
            static_cast<float>(height));

        // ----------------------------------------------
        // Beat number
        // ----------------------------------------------

        g.setColour(
            juce::Colours::lightgrey);

        g.setFont(13.0f);

        g.drawText(
            juce::String(beat),
            x + 5,
            0,
            50,
            rulerHeight,
            juce::Justification::centredLeft);
    }

    // ==================================================
    // Track lanes
    // ==================================================

    for (std::size_t trackIndex = 0;
         trackIndex < project_->tracks.size();
         ++trackIndex)
    {
        const auto& track =
            project_->tracks[trackIndex];

        const int y =
            rulerHeight +
            static_cast<int>(
                trackIndex * trackHeight);

        // ----------------------------------------------
        // Track background
        // ----------------------------------------------

        if (trackIndex % 2 == 0)
        {
            g.setColour(
                juce::Colour::fromRGB(
                    29,
                    29,
                    34));
        }
        else
        {
            g.setColour(
                juce::Colour::fromRGB(
                    26,
                    26,
                    31));
        }

        g.fillRect(
            0,
            y,
            width,
            trackHeight);

        // ----------------------------------------------
        // Track separator
        // ----------------------------------------------

        g.setColour(
            juce::Colour::fromRGB(
                55,
                55,
                62));

        g.drawHorizontalLine(
            y,
            0.0f,
            static_cast<float>(width));

        // ==================================================
        // MIDI notes
        // ==================================================

        for (const auto& note : track.notes)
        {
            const int noteX =
                static_cast<int>(
                    note.startBeat *
                    pixelsPerBeat);

            const int noteWidth =
                std::max(
                    4,
                    static_cast<int>(
                        note.durationBeats *
                        pixelsPerBeat));

            // Different colour per instrument.
            g.setColour(
                juce::Colour::fromRGB(
                    70,
                    150,
                    240));

            g.fillRoundedRectangle(
                static_cast<float>(noteX + 2),
                static_cast<float>(y + 25),
                static_cast<float>(
                    noteWidth - 4),
                24.0f,
                4.0f);

            // MIDI note number
            g.setColour(
                juce::Colours::white);

            g.setFont(11.0f);

            g.drawText(
                juce::String(
                    note.midiNote),
                noteX + 5,
                y + 25,
                noteWidth - 10,
                24,
                juce::Justification::centred);
        }

        // ==================================================
        // Drum events
        // ==================================================

        for (const auto& drum : track.drums)
        {
            const int drumX =
                static_cast<int>(
                    drum.startBeat *
                    pixelsPerBeat);

            g.setColour(
                juce::Colour::fromRGB(
                    240,
                    160,
                    70));

            g.fillEllipse(
                static_cast<float>(
                    drumX - 6),
                static_cast<float>(
                    y + 34),
                12.0f,
                12.0f);
        }

        // ==================================================
        // Audio region
        // ==================================================

        if (track.type == TrackType::Audio &&
            !track.audioFile.empty())
        {
            const int audioX =
                static_cast<int>(
                    track.audioStartBeat *
                    pixelsPerBeat);

            const int audioWidth =
                std::max(
                    80,
                    static_cast<int>(
                        4.0 *
                        pixelsPerBeat));

            g.setColour(
                juce::Colour::fromRGB(
                    70,
                    190,
                    120));

            g.fillRoundedRectangle(
                static_cast<float>(
                    audioX + 2),
                static_cast<float>(
                    y + 15),
                static_cast<float>(
                    audioWidth - 4),
                50.0f,
                5.0f);

            g.setColour(
                juce::Colours::white);

            g.setFont(12.0f);

            g.drawText(
                juce::File(
                    track.audioFile)
                    .getFileName(),
                audioX + 8,
                y + 20,
                audioWidth - 16,
                20,
                juce::Justification::centredLeft);
        }

        // ==================================================
        // Track number
        // ==================================================

        g.setColour(
            juce::Colour::fromRGBA(
                255,
                255,
                255,
                90));

        g.setFont(11.0f);

        g.drawText(
            "Track " +
            juce::String(track.number),
            8,
            y + 4,
            80,
            16,
            juce::Justification::centredLeft);
    }

    // ==================================================
    // Playhead
    //
    // Currently fixed at beat 0.
    // We will connect this to the transport later.
    // ==================================================

    constexpr double playheadBeat = 0.0;

    const int playheadX =
        static_cast<int>(
            playheadBeat *
            pixelsPerBeat);

    g.setColour(
        juce::Colours::red);

    g.drawVerticalLine(
        playheadX,
        0.0f,
        static_cast<float>(height));

    // ==================================================
    // Ruler top border
    // ==================================================

    g.setColour(
        juce::Colour::fromRGB(
            70,
            70,
            78));

    g.drawHorizontalLine(
        rulerHeight,
        0.0f,
        static_cast<float>(width));
}

// ==================================================
// Resized
// ==================================================

void TimelineComponent::resized()
{
    // No child components yet.
}
