#pragma once

#include <vector>
#include <stdexcept>

struct TempoChange
{
    double beat;
    double bpm;
};

class TempoMap
{
public:
    TempoMap()
    {
        changes.push_back({0.0, 120.0});
    }

    void setInitialTempo(double bpm)
    {
        validateTempo(bpm);

        changes[0].bpm = bpm;
    }

    void addTempoChange(double beat, double bpm)
    {
        validateTempo(bpm);

        if (beat < 0.0)
            throw std::runtime_error("Tempo change beat cannot be negative.");

        changes.push_back({beat, bpm});
    }

    double beatToSeconds(double targetBeat) const
    {
        if (targetBeat <= 0.0)
            return 0.0;

        double seconds = 0.0;

        for (size_t i = 0; i < changes.size(); ++i)
        {
            const auto& current = changes[i];

            const double nextBeat =
                (i + 1 < changes.size())
                    ? changes[i + 1].beat
                    : targetBeat;

            if (targetBeat <= current.beat)
                break;

            const double endBeat =
                std::min(targetBeat, nextBeat);

            const double beats =
                endBeat - current.beat;

            if (beats > 0.0)
                seconds += beats * (60.0 / current.bpm);

            if (targetBeat <= nextBeat)
                break;
        }

        return seconds;
    }

    double getInitialTempo() const
    {
        return changes.front().bpm;
    }

private:

    std::vector<TempoChange> changes;

    static void validateTempo(double bpm)
    {
        if (bpm < 1.0 || bpm > 480.0)
        {
            throw std::runtime_error(
                "Tempo must be between 1 and 480 BPM."
            );
        }
    }
};
