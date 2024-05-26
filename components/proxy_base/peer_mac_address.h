#pragma once

#include "esphome/core/hal.h"
#include <esp_now.h>

namespace esphome
{
  namespace proxy_base
  {
    class PeerMacAddress
    {
    private:
      void update_as_string();

    public:
      char as_string[3 * ESP_NOW_ETH_ALEN + 1] = "00:00:00:00:00:00";
      uint8_t as_uint8_t_array[ESP_NOW_ETH_ALEN];
      void set_from_uint64_t(uint64_t value);
      void set_from_uint8_t_array(const uint8_t *value);
      bool mac_address_equals(PeerMacAddress *other);

    protected:
    };
  } // namespace proxy_base
} // namespace esphome
