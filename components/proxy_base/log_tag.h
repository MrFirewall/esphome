#pragma once

#include "esphome/core/hal.h"

namespace esphome
{
  namespace proxy_base
  {
    class LogTag
    {
    private:
      const char *tag_suffix_;
      char *tag_{NULL};

    public:
      LogTag(const char *tag_suffix);
      char *get_tag();

    protected:
    };
  } // namespace proxy_base
} // namespace esphome
