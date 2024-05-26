#pragma once

#include <esp_err.h>
#include "esphome/core/log.h"

namespace esphome
{
    namespace proxy_base
    {
        class ESPResultDecoder
        {
        private:
            static const char *decode_esp_result(esp_err_t result);

        public:
            static void check_esp_result_code(esp_err_t result, const char *operation);
            static void check_esp_result_bool(bool result, const char *operation);

        protected:
        };
    } // namespace proxy_base
} // namespace esphome
