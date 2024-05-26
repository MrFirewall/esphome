#include "proxy_base.h"
#include "esphome/core/log.h"

#include <WiFi.h>
#include "log_tag.h"
#include "esp_result_decoder.h"

// Useful to hold off during debugging to wait for other components to finish spamming the log. 
#define STARTUP_DELAY 0

namespace esphome
{
  namespace proxy_base
  {
    bool ProxyBaseComponent::setup_espnow()
    {
      espnow_is_setup_ = false;
      ESPResultDecoder::check_esp_result_code(esp_now_init(), "esp_now_init");

      ESPResultDecoder::check_esp_result_code(esp_now_register_send_cb(PeerBase::call_on_data_send_callback), "esp_now_register_send_cb");
      ESPResultDecoder::check_esp_result_code(esp_now_register_recv_cb(PeerBase::call_on_data_recv_callback), "esp_now_register_recv_cb");

      espnow_is_setup_ = true;

      return true;
    }

    void ProxyBaseComponent::setup()
    {
      ESP_LOGD(TAG->get_tag(), "((((((((((((((((((((((((((((((((((((((((ENTER PROXY BASE SETUP)))))))))))))))))))))))))))))))))))");
      proxy_setup();
      setup_ms_ = millis();
    }

    void ProxyBaseComponent::loop()
    {
      if (millis() < (setup_ms_ + STARTUP_DELAY))
      {
        return;
      }
      proxy_loop();
    }

  } // namespace proxy_base
} // namespace esphome
