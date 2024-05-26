#include "log_tag.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include <WiFi.h>

namespace esphome
{
  namespace proxy_base
  {
    LogTag::LogTag(const char *tag_suffix)
    {
      tag_suffix_ = tag_suffix;
    }

    char *LogTag::get_tag()
    {
      if (tag_ == NULL)
      {
        tag_ = (char *)malloc(100);
        sprintf(tag_, "[%s %s] %s", WiFi.macAddress().c_str(), App.get_name().c_str(), tag_suffix_);
      }
      return tag_;
    }

  } // namespace proxy_base
} // namespace esphome
