#include "peer_mac_address.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include <WiFi.h>

namespace esphome
{
  namespace proxy_base
  {
    void PeerMacAddress::set_from_uint64_t(uint64_t value)
    {
      const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&value);
      // Its little endian on our ESPs so cant just use as is..
      for (int i = 0; i < ESP_NOW_ETH_ALEN; i++)
      {
        as_uint8_t_array[i] = bytes[ESP_NOW_ETH_ALEN - 1 - i];
      }
      update_as_string();
    }

    void PeerMacAddress::set_from_uint8_t_array(const uint8_t *value)
    {
      memcpy(&as_uint8_t_array, value, sizeof(as_uint8_t_array));
      update_as_string();
    }

    bool PeerMacAddress::mac_address_equals(PeerMacAddress *other)
    {
      for (int i = 0; i < ESP_NOW_ETH_ALEN; i++)
      {
        if (as_uint8_t_array[i] != other->as_uint8_t_array[i])
          return false;
      }
      return true;
    }

    void PeerMacAddress::update_as_string()
    {
      for (int i = 0; i < ESP_NOW_ETH_ALEN; i++)
      {
        if (i < (ESP_NOW_ETH_ALEN - 1))
          sprintf(&as_string[i * 3], "%02X:", as_uint8_t_array[i]);
        else
          sprintf(&as_string[i * 3], "%02X", as_uint8_t_array[i]);
      }
    }

  } // namespace proxy_base
} // namespace esphome