#include "u8g2_esp32_hal.h"
#include "nrf24.hh"

#include <u8g2.h>


#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <array>

// DISPLAY
#define DISPLAY_DC   25
#define DISPLAY_RST  18
#define DISPLAY_CLK  19
#define DISPLAY_CS   22
#define DISPLAY_MOSI 23
#define DISPLAY_SPEED (500*1000)

extern "C" void app_main();

namespace {

std::array<uint8_t, 6> HUB = {0x30, 0xae, 0xa4, 0x8b, 0x8d, 0xf4};
std::array<char, 5> hub_address = { 'A', 'N', 'N', 'E', '!' };
std::array<char, 5> spoke_address = { 'D', 'I', 'E', 'Z', '!' };

void hub()
{
  ESP_LOGI("hub", "I'm the hub!");

  nrf24_setup(hub_address.data());

  TickType_t last_wake_time;
  const TickType_t period =  pdMS_TO_TICKS(1000);
  last_wake_time = xTaskGetTickCount();

  ESP_LOGI("hub", "config, should be 12: %i", nrf24_reg_read(NRF24_CONFIG));
  while(1)
  {
    uint8_t *buffer;
    size_t len;
    const auto res = nrf24_hub_to_spoke(spoke_address.data(), &buffer, &len);
    switch(res)
    {
    case NRF24_HUB_ERROR_OK:
      ESP_LOGI("hub", "NRF24_HUB_ERROR_OK");
      break;
    case NRF24_HUB_SEND_FAILED:
      {
        const auto observe_tx = nrf24_reg_read(NRF24_OBSERVE_TX);
        ESP_LOGE("hub", "NRF24_HUB_SEND_FAILED, PLOS: %i", (observe_tx >> 4));
      }
      break;
    case NRF24_HUB_RX_TIMEOUT:
      ESP_LOGE("hub", "NRF24_HUB_RX_TIMEOUT");
      break;
    case NRF24_HUB_PAYLOAD_TOO_LONG:
      ESP_LOGE("hub", "NRF24_HUB_PAYLOAD_TOO_LONG");
      break;
    }

    vTaskDelayUntil(&last_wake_time, period);
  }
}

void spoke()
{

  ESP_LOGI("spoke", "I'm the spoke!");

  nrf24_setup(spoke_address.data());

  TickType_t last_wake_time;
  const TickType_t period =  pdMS_TO_TICKS(1000);
  last_wake_time = xTaskGetTickCount();

  while(1)
  {
    ESP_LOGI("spoke", "config: %i", nrf24_reg_read(NRF24_CONFIG));
    vTaskDelayUntil(&last_wake_time, period);
  }
}


} // end ns anon

void app_main()
{
  std::array<uint8_t, 6> chipid;
  esp_efuse_mac_get_default(chipid.data());
  ESP_LOG_BUFFER_HEX_LEVEL("main", chipid.data(), chipid.size(), ESP_LOG_INFO);

  if(chipid == HUB)
  {
    hub();
  }
  else
  {
    spoke();
  }

}
