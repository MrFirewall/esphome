#pragma once

#include "esphome/core/component.h"
#include "../proxy_base/log_tag.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"

namespace esphome
{
  namespace proxy_receiver
  {
    class OTASwitchComponent : public switch_::Switch, public Component
    {
    protected:
      proxy_base::LogTag *TAG = new proxy_base::LogTag("OTASwitchComponent");
      bool assumed_state() override;
      void write_state(bool state) override;

    private:
      std::function<void(bool state)> _on_change;
      bool _deviceSideState;
      void setup() override;

    public:
      void set(bool state);
    };

  } // namespace proxy_receiver
} // namespace esphome
