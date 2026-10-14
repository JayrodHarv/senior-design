#pragma once

#include <HTTPClient.h>

class SupabaseClient
{
public:
    // PATCH public.beam_state (id = Config::BEAM_ROW_ID) with {"broken": ...}.
    // The database webhook on UPDATE calls the Resend edge function.
    bool updateBeamState(bool broken);

private:
    void addHeaders(HTTPClient& http);
};
