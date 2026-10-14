#pragma once

// Boundary between the digital filter and the rest of the firmware.
//
// The filter implementation only has to provide these two functions.
// The filter owns the LED; main.cpp only uses beamPresent() for Supabase.
// beamPresent() must return true while the transmitter's frequency is
// detected and must be safe to call from loop() (e.g. read a volatile
// flag that the filter's sampling ISR/task updates).
namespace IrFilter
{
void begin();
bool beamPresent();
}
