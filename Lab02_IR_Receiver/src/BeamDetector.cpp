#include "BeamDetector.h"
#include "Config.h"

bool BeamDetector::update(bool beamPresent, unsigned long now)
{
    const bool brokenNow = !beamPresent;

    if (brokenNow == broken_)
    {
        pending_ = false; // glitch ended before it was confirmed
        return false;
    }

    if (!pending_)
    {
        pending_ = true;
        pendingSince_ = now;
        return false;
    }

    const uint32_t hold = brokenNow ? Config::BEAM_BREAK_CONFIRM_MS
                                    : Config::BEAM_RESTORE_CONFIRM_MS;
    if (now - pendingSince_ < hold)
    {
        return false;
    }

    broken_ = brokenNow;
    pending_ = false;
    if (!broken_) armed_ = true; // beam confirmed present at least once
    return true;
}
