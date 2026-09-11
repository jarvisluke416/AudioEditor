#pragma once

#include <string>
#include "Project.h"

// ==================================================
// Parser
// ==================================================

class Parser
{
public:

    // ----------------------------------------------
    // Parse .song file
    // ----------------------------------------------

    static Project parseFile(
        const std::string& filename);

private:

    // ----------------------------------------------
    // Musical note conversion
    // ----------------------------------------------

    static int noteToMidi(
        const std::string& note);

    // ----------------------------------------------
    // Instrument conversion
    // ----------------------------------------------

    static InstrumentType parseInstrument(
        const std::string& name);

    // ----------------------------------------------
    // Drum conversion
    // ----------------------------------------------

    static DrumType parseDrum(
        const std::string& name);
};
