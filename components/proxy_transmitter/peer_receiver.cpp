#include "peer_receiver.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <WiFi.h>
#include "esphome/core/application.h"

// Give up on ACKs after this log, and reset and try again
#define RESPONSE_TIMEOUT 4000

// Give up on reading sensors after this long
#define READ_SENSORS_TIMEOUT 10000

// Give up on sending states after this long
#define SENDING_STATE_TIMEOUT 8000

// Reboot into safe mode after this many timeouts waiting for checking ACK
#define MAX_CHECKIN_RESPONSE_TIMEOUTS 8

namespace esphome
{
  namespace proxy_transmitter
  {
    void PeerReceiver::handle_received_proxy_message(proxy_base::proxy_message *message)
    {
      if (get_state() == proxy_base::PS_WAIT_CHECKIN_ACK)
      {
        if (message->message_type == proxy_base::MT_ACK_CHECKIN)
        {
          failed_ack_checkin_count_ = 0;
          if (message->checkin_response.enter_ota_mode)
          {
            enter_ota_mode();
          }
          start_sensor_reads();
          set_state(proxy_base::PS_READING_SENSORS);
        }
        return;
      }

      if (get_state() == proxy_base::PS_WAIT_SEND_STATE_ACK)
      {
        if (message->message_type == proxy_base::MT_ACK_SEND_SENSOR_STATE)
        {
          // Should be first unsend sender...
          SensorHolder *first_unsent = get_first_unsent_sensor();
          if (first_unsent == NULL)
          {
            reset_state("Got R to T Send State Response but no sensors are unsent");
            return;
          }
          first_unsent->is_sent = true;

          SensorHolder *new_first_unsent = get_first_unsent_sensor();
          if (new_first_unsent)
          {
            set_state(proxy_base::PS_SENDING_STATES);
          }
          else
          {
            set_state(proxy_base::PS_COMPLETE);
          }
          return;
        }

        ESP_LOGD(TAG->get_tag(), "Unexpected message type %d when in state %d", message->message_type, get_state());
      }
    }

    void PeerReceiver::peer_workflow_loop()
    {

      int time_since_last_state_change_ms = millis() - last_state_change_millis_;

      if (get_state() == proxy_base::PS_READY)
      {

        ESP_LOGD(TAG->get_tag(), "");
        ESP_LOGD(TAG->get_tag(), "----------------------");
        ESP_LOGD(TAG->get_tag(), "******* READY");
        ESP_LOGD(TAG->get_tag(), "----------------------");

        // Want to send a checkin
        proxy_base::proxy_message msg;
        msg.message_type = proxy_base::MT_CHECKIN;
        send_proxy_message(&msg);
        // Set state to awaiting MT_ACK_CHECKIN
        set_state(proxy_base::PS_WAIT_CHECKIN_ACK);

        return;
      }

      if (get_state() == proxy_base::PS_WAIT_CHECKIN_ACK)
      {
        if (time_since_last_state_change_ms > RESPONSE_TIMEOUT)
        {
          failed_ack_checkin_count_++;
          if (failed_ack_checkin_count_ > MAX_CHECKIN_RESPONSE_TIMEOUTS)
          {
            ESP_LOGD(TAG->get_tag(), "Exceeded %d CHECKIN RESPONSE timeouts in a row. Rebooting to safe mode", MAX_CHECKIN_RESPONSE_TIMEOUTS);
            enter_ota_mode();
            return;
          }

          reset_state("Timeout waiting for R to T Check in respponse");
        }
        return;
      }

      if (get_state() == proxy_base::PS_READING_SENSORS)
      {
        if (time_since_last_state_change_ms > READ_SENSORS_TIMEOUT)
        {
          reset_state("Timeout reading sensors");
        }

        bool has_outstanding_reads = false;
        for (int i = 0; i < sensors_->size(); i++)
        {
          if (!sensors_->at(i)->has_state)
          {
            has_outstanding_reads = true;
            break;
          }
        }

        if (!has_outstanding_reads)
        {
          // Have finished reading, so now move on to sending states to receiver
          for (int i = 0; i < sensors_->size(); i++)
          {
            sensors_->at(i)->is_sent = false;
          }
          set_state(proxy_base::PS_SENDING_STATES);
        }
        return;
      }

      if (get_state() == proxy_base::PS_SENDING_STATES)
      {
        if (time_since_last_state_change_ms > SENDING_STATE_TIMEOUT)
        {
          reset_state("Timeout sending states");
        }

        SensorHolder *first_unsent = get_first_unsent_sensor();
        if (first_unsent == NULL)
        {
          reset_state("In PS_SENDING_STATES but nothing left to send.");
          return;
        }

        // Want to send a checkin
        proxy_base::proxy_message msg;
        msg.message_type = proxy_base::MT_SEND_SENSOR_STATE;

        // Zero out peoxy_id
        for (int i = 0; i < PROXY_ID_MAX_LENGTH; i++)
        {
          msg.send_sensor_state.proxy_id[i] = 0;
        }
        memcpy(msg.send_sensor_state.proxy_id, first_unsent->proxy_id, PROXY_ID_MAX_LENGTH - 1);
        msg.send_sensor_state.state = first_unsent->state;
        send_proxy_message(&msg);

        // Set state to awaiting PS_WAIT_SEND_STATE_ACK
        set_state(proxy_base::PS_WAIT_SEND_STATE_ACK);

        return;
      }

      if (get_state() == proxy_base::PS_WAIT_SEND_STATE_ACK)
      {
        if (time_since_last_state_change_ms > RESPONSE_TIMEOUT)
        {
          reset_state("Timeout waiting for R to T Send State Response");
        }
        return;
      }

      if (get_state() == proxy_base::PS_COMPLETE)
      {
        ESP_LOGD(TAG->get_tag(), "");
        ESP_LOGD(TAG->get_tag(), "----------------------");
        ESP_LOGD(TAG->get_tag(), "******* CYCLE COMPLETE");
        ESP_LOGD(TAG->get_tag(), "----------------------");

        handle_received_proxy_message(esphome::proxy_base::proxy_message*);

        set_state(proxy_base::PS_READY);

        return;
      }

      ESP_LOGD(TAG->get_tag(), "Unexpected state in loop - %d ", get_state());
    }

    SensorHolder *PeerReceiver::get_first_unsent_sensor()
    {
      for (int i = 0; i < sensors_->size(); i++)
      {
        if (!sensors_->at(i)->is_sent)
          return sensors_->at(i);
      }
      return NULL;
    }

    void PeerReceiver::start_sensor_reads()
    {
      ESP_LOGD(TAG->get_tag(), "Waiting for state from %d sensors ", sensors_->size());
      for (int i = 0; i < sensors_->size(); i++)
      {
        SensorHolder *sensor = sensors_->at(i);
        sensor->has_state = false;
      }
    }

    void PeerReceiver::enter_ota_mode()
    {
      ESP_LOGD(TAG->get_tag(), "*****************************************************");
      ESP_LOGD(TAG->get_tag(), "******* Enter OTA Mode set. Rebooting to safe mode...");
      ESP_LOGD(TAG->get_tag(), "*****************************************************");
      
      this->get_ota()->set_safe_mode_pending(true);
      App.safe_reboot();
      ESP_LOGD(TAG->get_tag(), "******* Reboot commamnd sent...");
    }

  } // namespace proxy_receiver
} // namespace esphome
