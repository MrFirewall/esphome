#pragma once

#include "esphome/core/component.h"
#include "../proxy_base/log_tag.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome
{
  namespace proxy_receiver
  {
    class ProxiedSensorComponent : public sensor::Sensor, public Component
    {
    private:
    protected:
      const char *proxy_id;
      proxy_base::LogTag *TAG = new proxy_base::LogTag("ProxiedSensorComponent");

    public:
      void set_proxied_sensor_id(const char *proxied_sensor_id) { this->proxy_id = proxied_sensor_id; }
      const char *get_proxied_sensor_id() { return this->proxy_id; }
    };

  } // namespace proxy_receiver
} // namespace esphome
