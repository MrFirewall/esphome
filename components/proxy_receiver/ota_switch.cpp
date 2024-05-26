#include "ota_switch.h"
#include "esphome/core/log.h"

namespace esphome
{
  namespace proxy_receiver
  {

    OTASwitchComponent::OTASwitchComponent()
    {
      this->set_entity_category(ENTITY_CATEGORY_DIAGNOSTIC);
    }

    void OTASwitchComponent::write_state(bool state)
    {
      _deviceSideState = state;
      this->publish_state(state);
    }

    bool OTASwitchComponent::assumed_state()
    {
      return false; // Dont show on AND off, since the state is known and not assumed.
    }

    void OTASwitchComponent::setup()
    {
      write_state(_deviceSideState);
    }

    void OTASwitchComponent::set(bool state)
    {
      this->write_state(state);
    }
  } // namespace proxy_receiver
} // namespace esphome
