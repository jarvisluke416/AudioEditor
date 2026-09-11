#pragma once

#include <JuceHeader.h>
#include "Project.h"

class TimelineComponent
    : public juce::Component
{
public:
    TimelineComponent();

    void setProject(const Project* project);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    const Project* project_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        TimelineComponent
    )
};
