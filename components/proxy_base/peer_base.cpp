#include "peer_base.h"
#include "esphome/core/log.h"
#include "esp_result_decoder.h"

#define ESP_SEND_CALLBACK_TIMEOUT_MS 8000

namespace esphome
{
  namespace proxy_base
  {
    std::vector<PeerBase *> *PeerBase::global_peer_list_ = new std::vector<PeerBase *>();

    //
    // Add ESPNow peer
    //

    bool PeerBase::espnow_add_peer()
    {
      ESP_LOGD(TAG->get_tag(), "Add peer %s", mac_address_.as_string);

      memcpy(&peer_info_.peer_addr, &mac_address_.as_uint8_t_array, sizeof(peer_info_.peer_addr));

      // peer_info_.peer_addr = mac_address.as_uint8_t_array;
      peer_info_.channel = espnow_channel_;
      peer_info_.encrypt = false;

      ESPResultDecoder::check_esp_result_code(esp_now_add_peer(&peer_info_), "esp_now_add_peer");
      global_peer_list_->push_back(this);

      last_state_change_millis_ = millis();
      reset_state("ESP Now Peer Added");

      return true;
    }

    //
    // Send Proxy Message
    //
    void PeerBase::send_proxy_message(proxy_message *message)
    {
      message->time_stamp = millis();
      proxy_message *queue_message = (proxy_message *)malloc(sizeof(proxy_message));

      memcpy(queue_message, message, sizeof(proxy_message));
      proxy_message_outgoing_queue_->push(queue_message);
    }

    //
    // Static callbacks (will find correct instance and call non static flavour)
    //
    PeerBase *PeerBase::find_peer_in_global_peer_list(PeerMacAddress *peer)
    {
      for (int i = 0; i < global_peer_list_->size(); i++)
      {
        if (global_peer_list_->at(i)->mac_address_.mac_address_equals(peer))
          return global_peer_list_->at(i);
      }

      return NULL;
    }

    void PeerBase::call_on_data_send_callback(const uint8_t *mac_addr, esp_now_send_status_t status)
    {
      PeerMacAddress callback_peer;
      callback_peer.set_from_uint8_t_array(mac_addr);

      PeerBase *peer = find_peer_in_global_peer_list(&callback_peer);
      if (peer == NULL)
      {
        ESP_LOGD("PeerBaseComponent", "Received unexpected DataSent callback from unknown peer %s", callback_peer.as_string);
        return;
      }
      peer->on_data_send_callback(status);
    }

    void PeerBase::call_on_data_recv_callback(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
    {

      PeerMacAddress callback_peer;

      callback_peer.set_from_uint8_t_array(mac_addr);

      PeerBase *peer = find_peer_in_global_peer_list(&callback_peer);
      if (peer == NULL)
      {
        ESP_LOGD("PeerBaseComponent", "Received unexpected DataRecv callback from unknown peer %s", callback_peer.as_string);
        return;
      }
      peer->on_data_recv_callback(incomingData, len);
    }

    //
    // Non static callbacks
    //
    void PeerBase::on_data_send_callback(esp_now_send_status_t status)
    {
      proxy_message_sendack_queue_->push(status);
    }

    void PeerBase::on_data_recv_callback(const uint8_t *incomingData, int len)
    {
      // Enqueue message as this is called from a high priority wifi thread, and documentation says not to spend too long in here...
      proxy_message *message = (proxy_message *)malloc(sizeof(proxy_message));

      memcpy(message, incomingData, sizeof(proxy_message));
      proxy_message_incoming_queue_->push(message);
    }

    //
    // Loop - this is called from the transmitter/receiver component
    //
    void PeerBase::loop()
    {
      if (process_proxy_message_sendack_queue())
      {
        return; // Dont do anything else as we are not supposed to block for long in loop
      }

      if (process_proxy_message_outgoing_queue())
      {
        return; // Dont do anything else as we are not supposed to block for long in loop
      }

      if (process_proxy_message_incoming_queue())
      {
        return;
      }

      peer_workflow_loop();
    }

    //
    // Process message queues
    //
    bool PeerBase::process_proxy_message_sendack_queue()
    {
      // If we have been waiting too long for a send ack, give up, reset to not waiting, so on next loop it will move on
      if (awaiting_send_callback_)
      {
        // See if we have times out awaiting the ack
        if ((millis() - awaiting_send_callback_start_ms_) > ESP_SEND_CALLBACK_TIMEOUT_MS)
        {
          ESP_LOGD(TAG->get_tag(), "No ESP Send callback after %dms", ESP_SEND_CALLBACK_TIMEOUT_MS);
          awaiting_send_callback_ = false;
          return true; // Dont do anything else in this loop, but next loop will continue normally
        }
      }

      if (proxy_message_sendack_queue_->size() > 0)
      {
        // Process first sendack queue item
        esp_now_send_status_t next_sendack = proxy_message_sendack_queue_->front();
        proxy_message_sendack_queue_->pop();
        if (!awaiting_send_callback_)
        {
          ESP_LOGD(TAG->get_tag(), "+ UNEXPECTED ACK %s - %s", mac_address_.as_string, (next_sendack == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"));
        }
        else
        {
          ESP_LOGD(TAG->get_tag(), "+ ACK %s - %s", mac_address_.as_string, (next_sendack == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"));
          awaiting_send_callback_ = false;
        }
        return true; // Dont do anything else in this loop as we have processed a sendack and (maybe) set the awaiting flag to not waiting anymore.
      }

      if (awaiting_send_callback_)
      {
        return true; // Dont allow loop to do other stuff as we are still waiting for a send ACK and havent timed out
      }
      // If we are here, we are not awaiting sendack, and we have nothing in the sendack queue, so continue to next bit of loop
      return false;
    }

    bool PeerBase::process_proxy_message_outgoing_queue()
    {

      if (proxy_message_outgoing_queue_->size() == 0)
      {
        return false; // Nothing to send
      }

      proxy_message *next_message = proxy_message_outgoing_queue_->front();
      proxy_message_outgoing_queue_->pop();

      std::string desc;
      describe_proxy_message(&desc, next_message);

      ESP_LOGD(TAG->get_tag(), "> %s %s", name_, desc.c_str());

      ESPResultDecoder::check_esp_result_code(esp_now_send(peer_info_.peer_addr, (uint8_t *)next_message, sizeof(proxy_message)), "esp_now_send");

      awaiting_send_callback_ = true;
      awaiting_send_callback_start_ms_ = millis();

      // And free it
      free(next_message);

      return true;
    }

    bool PeerBase::process_proxy_message_incoming_queue()
    {
      if (proxy_message_incoming_queue_->size() == 0)
      {
        return false;
      }

      proxy_message *next_message = proxy_message_incoming_queue_->front();
      proxy_message_incoming_queue_->pop();

      if (next_message->message_protocol_version != MESSAGE_PROTOCOL_VERSION)
      {
        ESP_LOGD(TAG->get_tag(), "!!!! ERROR Received message with protocol version %d but version %d was expected. This means the receiver and the transmitter are not running the same version of the ESP Proxy component. Ignoring.", next_message->message_protocol_version, MESSAGE_PROTOCOL_VERSION);
        free(next_message);
        return true; // Dont want to do any more processing with this peer on this loop
      }

      std::string desc;
      describe_proxy_message(&desc, next_message);

      ESP_LOGD(TAG->get_tag(), "< %s %s", name_, desc.c_str());

      handle_received_proxy_message(next_message);

      // And free it
      free(next_message);

      return true; // HAve processed message, so tell loop to not do other stuff
    }

    //
    // State
    //
    peer_state PeerBase::get_state()
    {
      return state_;
    }

    void PeerBase::set_state(peer_state state)
    {
      std::string old_desc;
      describe_peer_state(&old_desc, state_);

      state_ = state;

      std::string new_desc;
      describe_peer_state(&new_desc, state_);

      ESP_LOGD(TAG->get_tag(), "---> %s (was %s - after %f3s)", new_desc.c_str(), old_desc.c_str(), (millis() - last_state_change_millis_) / 1000.0);

      last_state_change_millis_ = millis();
    }

    void PeerBase::reset_state(const char *reason)
    {
      ESP_LOGD(TAG->get_tag(), "*** Resetting state - %s", reason);
      set_state(PS_READY);
    }
  } // namespace proxy_base
} // namespace esphome
