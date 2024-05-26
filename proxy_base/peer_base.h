#pragma once

#include "esphome/core/hal.h"
#include <esp_now.h>
#include "proxy_messages.h"
#include "log_tag.h"
#include <vector>
#include "peer_mac_address.h"
#include "peer_state.h"
#include <string.h>
#include <queue>


namespace esphome
{
  namespace proxy_base
  {

    class PeerBase
    {
    private:
      int espnow_channel_;
      esp_now_peer_info_t peer_info_;
      peer_state state_ = PS_UNSET;
      bool awaiting_send_callback_ = false;
      int awaiting_send_callback_start_ms_;
      // This is a hack since I cannot find a way of passing a method pointer to the ESPNow callbacks...
      static std::vector<PeerBase *> *global_peer_list_;
      void on_data_send_callback(esp_now_send_status_t status);
      void on_data_recv_callback(const uint8_t *incomingData, int len);
      static PeerBase *find_peer_in_global_peer_list(PeerMacAddress *peer);
      
      std::queue<proxy_message *> *proxy_message_incoming_queue_ = new std::queue<proxy_message *>();
      std::queue<proxy_message *> *proxy_message_outgoing_queue_ = new std::queue<proxy_message *>();
      std::queue<esp_now_send_status_t> *proxy_message_sendack_queue_ = new std::queue<esp_now_send_status_t>();
      bool process_proxy_message_sendack_queue();
      bool process_proxy_message_outgoing_queue();
      bool process_proxy_message_incoming_queue();

    public:

      void set_espnow_channel(int espnow_channel) { espnow_channel_ = espnow_channel; }
      int get_espnow_channel() { return espnow_channel_; }
      void set_name(const char *name) { name_ = name; }
      void set_mac_address(uint64_t mac_address_uint64_t) { mac_address_.set_from_uint64_t(mac_address_uint64_t); }
      bool espnow_add_peer();
      const char *get_name() { return name_; }
      // Callbacks from ESPNow
      static void call_on_data_send_callback(const uint8_t *mac_addr, esp_now_send_status_t status);
      static void call_on_data_recv_callback(const uint8_t *mac_addr, const uint8_t *incomingData, int len);
      void loop();

    protected:
      LogTag *TAG = new LogTag("PeerBase");


      PeerMacAddress mac_address_;
      const char *name_;
      int last_state_change_millis_;
      void set_state(peer_state state);
      peer_state get_state();
      void reset_state(const char *reason);
      void send_proxy_message(proxy_message *message);
      virtual void handle_received_proxy_message(proxy_message *message) = 0;
      virtual void peer_workflow_loop() = 0;
    };
  } // namespace proxy_base
} // namespace esphome
