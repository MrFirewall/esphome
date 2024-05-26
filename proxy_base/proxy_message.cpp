#include "proxy_messages.h"

namespace esphome
{
    namespace proxy_base
    {
        void describe_proxy_message(std::string *output, proxy_message *message)
        {
            switch (message->message_type)
            {
            case MT_CHECKIN:
                output->append("MT_CHECKIN");
                break;
            case MT_ACK_CHECKIN:
            {
                std::string enter_ota_mode = std::to_string(message->checkin_response.enter_ota_mode);
                output->append("MT_ACK_CHECKIN {enter_ota_mode: " + enter_ota_mode + "}");
                break;
            }
            case MT_SEND_SENSOR_STATE:
            {
                std::string state = std::to_string(message->send_sensor_state.state);
                std::string proxy_id = std::string(message->send_sensor_state.proxy_id);
                output->append("MT_SEND_SENSOR_STATE {state: " + state + ", proxy_id: " + proxy_id + "}");
                break;
            }
            case MT_ACK_SEND_SENSOR_STATE:
            {
                std::string proxy_id = std::string(message->send_sensor_state.proxy_id);
                output->append("MT_ACK_SEND_SENSOR_STATE {proxy_id: " + proxy_id + "}");
                break;
            }
            default:
                output->append("!! UNKONWN MESSAGE TYPE !!");
            }
        }
    } // namespace proxy_base
} // namespace esphome

// output->append("(PING)" );
