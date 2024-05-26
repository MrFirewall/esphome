#pragma once
#include <string>

// Allow an extra byte for the null terminator
// This also needs setting in the python file so people dont inject too large strings
#define PROXY_ID_MAX_LENGTH 21
#define MESSAGE_PROTOCOL_VERSION 210

namespace esphome
{
    namespace proxy_base
    {
        enum proxy_message_type
        {
            MT_CHECKIN = 20,
            MT_ACK_CHECKIN = 30,
            MT_SEND_SENSOR_STATE = 40,
            MT_ACK_SEND_SENSOR_STATE = 50
        };

        typedef struct proxy_message_checkin_response
        {
            bool enter_ota_mode;
        } proxy_message_checkin_response;

        typedef struct proxy_message_send_sensor_state
        {
            char proxy_id[PROXY_ID_MAX_LENGTH];
            float state;
        } proxy_message_send_sensor_state;

        typedef struct proxy_message_send_sensor_state_response
        {
            char proxy_id[PROXY_ID_MAX_LENGTH];
        } proxy_message_send_sensor_state_response;

        typedef struct proxy_message
        {
            int message_protocol_version = MESSAGE_PROTOCOL_VERSION;
            proxy_message_type message_type;
            int time_stamp;
            union
            {
                proxy_message_checkin_response checkin_response;
                proxy_message_send_sensor_state send_sensor_state;
                proxy_message_send_sensor_state_response send_sensor_state_response;
            };
        } proxy_message_header;

        void describe_proxy_message(std::string *output, proxy_message *message);
    } // namespace proxy_base
} // namespace esphome
