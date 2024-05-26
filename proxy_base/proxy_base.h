#pragma once

#include "esphome/core/component.h"
#include <esp_now.h>
#include "proxy_messages.h"
#include "peer_base.h"

namespace esphome
{
  namespace proxy_base
  {

    class ProxyBaseComponent : public Component
    {
    private:
      int setup_ms_;

    public:
      void loop() override;
      void setup() override;

    protected:
      LogTag *TAG = new LogTag("ProxyBaseComponent");

      virtual void proxy_loop() = 0;
      virtual void proxy_setup() = 0;
      bool espnow_is_setup_{false};
      bool setup_espnow();
    };

  } // namespace proxy_base
} // namespace esphome
