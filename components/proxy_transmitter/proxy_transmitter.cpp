#include "proxy_transmitter.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <WiFi.h>
#include "../proxy_base/proxy_messages.h"
#include "../proxy_base/esp_result_decoder.h"

namespace esphome
{
  namespace proxy_transmitter
  {
    void ProxyTransmitterComponent::proxy_setup()
    {
      // This runs before wifi component starts up...
      get_wifi()->mark_failed(); // To kill it

      int channel = peer_receiver_->get_espnow_channel();

      ESP_ERROR_CHECK(esp_netif_init());
      ESP_ERROR_CHECK(esp_event_loop_create_default());
      wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
      ESP_ERROR_CHECK(esp_wifi_init(&cfg));
      ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
      ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
      ESP_ERROR_CHECK(esp_wifi_start());

      setup_espnow();

      peer_receiver_->espnow_add_peer();

      proxy_base::ESPResultDecoder::check_esp_result_code(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE), "WiFi.channel");
    }

    void ProxyTransmitterComponent::proxy_loop()
    {
      peer_receiver_->loop();
    }
  } // namespace proxy_transmitter
} // namespace esphome
