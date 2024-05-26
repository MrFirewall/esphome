#pragma once

#include "esphome/core/component.h"
#include "../proxy_base/proxy_base.h"
#include "peer_receiver.h"
#include "../proxy_base/log_tag.h"
#include <vector>
#include "sensor_holder.h"
#include "esphome/components/ota/ota_component.h"
#include "../proxy_base/wifi_holder.h"

namespace esphome
{
  namespace proxy_transmitter
  {
    class ProxyTransmitterComponent : public proxy_base::ProxyBaseComponent, public proxy_base::WiFiHolder
    {
    private:
      int loop_run_ = false;
      void first_loop();
    protected:
      proxy_base::LogTag *TAG = new proxy_base::LogTag("PeerReceiver");
      PeerReceiver *peer_receiver_ = new PeerReceiver();

    public:
      void set_peer_receiver(PeerReceiver *peer_receiver) { peer_receiver_ = peer_receiver; }
      // PROCESSOR priority so we can kill off the wifi component before it starts...
      float get_setup_priority() const override { return setup_priority::PROCESSOR; }
      void proxy_loop();
      void proxy_setup();
    };
  } // namespace proxy_transmitter
} // namespace esphome
