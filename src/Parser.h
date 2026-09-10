#pragma once

#include "Project.h"
#include <string>

class Parser
{
public:
    static Project parseFile(const std::string& filename);

private:
    static int noteToMidi(const std::string& note);
    static InstrumentType parseInstrument(const std::string& name);
    static DrumType parseDrum(const std::string& name);
};
