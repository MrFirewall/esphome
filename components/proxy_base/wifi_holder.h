#pragma once

#include "esphome/components/wifi/wifi_component.h"

namespace esphome
{
    namespace proxy_base
    {

        class WiFiHolder
        {
        private:
            wifi::WiFiComponent *wifi_;

        public:
            void set_wifi(wifi::WiFiComponent *wifi) { wifi_ = wifi; }
            wifi::WiFiComponent *get_wifi() { return wifi_; }
        };
    }
}
