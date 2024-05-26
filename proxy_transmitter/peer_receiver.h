#pragma once

#include "../proxy_base/peer_base.h"
#include "../proxy_base/log_tag.h"
#include "../proxy_base/proxy_messages.h"
#include <vector>
#include "esphome/components/sensor/sensor.h"
#include "sensor_holder.h"
#include "../proxy_base/ota_holder.h"

namespace esphome
{
  namespace proxy_transmitter
  {
    class PeerReceiver : public proxy_base::PeerBase, public proxy_base::OtaHolder
    {
    public:
      void add_sensor(sensor::Sensor *sensor, const char *proxy_id) { sensors_->push_back(new SensorHolder(sensor, proxy_id)); };
      void peer_workflow_loop();
    private:
      int failed_ack_checkin_count_ = 0;
      std::vector<SensorHolder *> *sensors_ = new std::vector<SensorHolder *>();
      void start_sensor_reads();
      SensorHolder *get_first_unsent_sensor();
      void enter_ota_mode();
      void go_to_sleep();

    protected:
      proxy_base::LogTag *TAG = new proxy_base::LogTag("PeerReceiver");
      void handle_received_proxy_message(proxy_base::proxy_message *message);
    };

  } // namespace proxy_receiver
} // namespace esphome
