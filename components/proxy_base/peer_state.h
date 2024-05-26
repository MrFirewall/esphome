#pragma once
#include <string>

namespace esphome
{
    namespace proxy_base
    {
        enum peer_state
        {
            PS_UNSET = 5,
            PS_READY = 10,
            PS_WAIT_CHECKIN_ACK = 20,
            PS_READING_SENSORS = 25,
            PS_SENDING_STATES = 30,
            PS_WAIT_SEND_STATE_ACK = 40,
            PS_COMPLETE = 50
        };

        void describe_peer_state(std::string *output, peer_state state);
    } // namespace proxy_base
} // namespace esphome
