#include "peer_state.h"

namespace esphome
{
    namespace proxy_base
    {
        void describe_peer_state(std::string *output, peer_state state)
        {
            switch (state)
            {
            case PS_UNSET:
                output->append("UNSET");
                break;
            case PS_READY:
                output->append("READY");
                break;
            case PS_WAIT_CHECKIN_ACK:
                output->append("WAIT_CHECKIN_ACK");
                break;
            case PS_READING_SENSORS:
                output->append("READING_SENSORS");
                break;
            case PS_SENDING_STATES:
                output->append("PS_SENDING_STATES");
                break;
            case PS_WAIT_SEND_STATE_ACK:
                output->append("WAIT_SEND_STATE_ACK");
                break;
            case PS_COMPLETE:
                output->append("COMPLETE");
                break;
            default:
                output->append("!! UNKONWN PEER STATE TYPE !!");
            }
        }
    } // namespace proxy_base
} // namespace esphome

// output->append("(PING)" + std::to_string(message->message_type));
