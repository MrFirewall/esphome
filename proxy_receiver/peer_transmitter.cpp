#include "peer_transmitter.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <WiFi.h>

namespace esphome
{
  namespace proxy_receiver
  {
    void PeerTransmitter::handle_received_proxy_message(proxy_base::proxy_message *message)
    {
      if (message->message_type == proxy_base::MT_CHECKIN)
      {
        ESP_LOGD(TAG->get_tag(), "");
        ESP_LOGD(TAG->get_tag(), "------------------------------");
        ESP_LOGD(TAG->get_tag(), "******* CHECKIN FROM %s", get_name());
        ESP_LOGD(TAG->get_tag(), "------------------------------");

        // Send a response
        proxy_base::proxy_message send_msg;
        send_msg.message_type = proxy_base::MT_ACK_CHECKIN;
        send_msg.checkin_response.enter_ota_mode = ota_switch->state;
        send_proxy_message(&send_msg);
        // Switch off OTA mode switch once message sent... we could wait for an ack, but then
        // the ack might get lost even though the reboot has happened, and thus it  would enter OTA mode
        // next time it does a full boot. Two generals problem.
        if (ota_switch->state)
        {
          ota_switch->set(false);
        }
      }

      if (message->message_type == proxy_base::MT_SEND_SENSOR_STATE)
      {
        // Update local state...
        update_proxied_sensor(message);
        // Send a response
        proxy_base::proxy_message send_msg;
        send_msg.message_type = proxy_base::MT_ACK_SEND_SENSOR_STATE;
        memcpy(send_msg.send_sensor_state_response.proxy_id, message->send_sensor_state.proxy_id, PROXY_ID_MAX_LENGTH);

        send_proxy_message(&send_msg);
      }
    }

    void PeerTransmitter::peer_workflow_loop()
    {
      if (get_state() == proxy_base::PS_READY)
      {
        // Just waiting for incoming messages.
        return;
      }

      ESP_LOGD(TAG->get_tag(), "Unexpected state in loop - %d ", get_state());
    }

    void PeerTransmitter::update_proxied_sensor(proxy_base::proxy_message *message)
    {
      // Need to find the sensor with our proxy_id
      ProxiedSensorComponent *proxied_sensor = NULL;
      for (int i = 0; i < proxied_sensors_->size(); i++)
      {
        if (strcmp(message->send_sensor_state.proxy_id, proxied_sensors_->at(i)->get_proxied_sensor_id()) == 0)
        {
          proxied_sensor = proxied_sensors_->at(i);
          break;
        }
      }

      if (!proxied_sensor)
      {
        ESP_LOGD(TAG->get_tag(), "Received sensor state from %s (%s) with proxy_id of %s but no such proxied sensor exists.", get_name(), mac_address_.as_string, message->send_sensor_state.proxy_id);
        return;
      }
      float state = message->send_sensor_state.state;
      ESP_LOGD(TAG->get_tag(), "^^Publish state from %s (%s) with proxy_id of %s - state is %f", get_name(), mac_address_.as_string, message->send_sensor_state.proxy_id, state);
      proxied_sensor->publish_state(state);
    }

    // void PeerTransmitter::add_proxied_sensor(ProxiedSensorComponent *proxied_sensore)
    // {
    //   // Find peer transmitter with this mac address...
    //   PeerTransmitter *peer_transmitter = NULL;
    //   proxy_base::PeerMacAddress peer_mac_address;
    //   peer_mac_address.set_from_uint64_t(mac_address);

    //   for (int i = 0; i < peer_transmitters_->size(); i++)
    //   {
    //     if (peer_transmitters_->at(i)->mac_address.mac_address_equals(&peer_mac_address))
    //     {
    //       peer_transmitter = peer_transmitters_->at(i);
    //     }
    //   }

    //   if (peer_transmitter == NULL)
    //   {
    //     ESP_LOGD(TAG->get_tag(), "Error - Attempt to add a proxied sensor to transmitter with mac address %s but we dont have such a transmitter", peer_mac_address.as_string);
    //     return;
    //   }

    //   ProxiedSensorComponent *new_proxied_sensor = new ProxiedSensorComponent();
    //   new_proxied_sensor->set_name(name);
    //   new_proxied_sensor->proxy_id = proxy_id;
    //   new_proxied_sensor->set_accuracy_decimals(4);

    //   // App.register_sensor(new_proxied_sensor);
    //   // App.register_component(new_proxied_sensor);

    //   peer_transmitter->proxied_sensors->push_back(new_proxied_sensor);
    // }
  } // namespace proxy_receiver
} // namespace esphome
