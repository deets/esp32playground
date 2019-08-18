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

// hardware setup on the newjoy baseboard
const gpio_num_t CE = static_cast<gpio_num_t>(19);

const gpio_num_t CS = static_cast<gpio_num_t>(5);
const gpio_num_t MISO = static_cast<gpio_num_t>(22);
const gpio_num_t MOSI = static_cast<gpio_num_t>(23);
const gpio_num_t SCK = static_cast<gpio_num_t>(18);
const auto NRF24_SPI_SPEED = 2 * 1000*1000;

void hub()
{
  ESP_LOGI("hub", "I'm the hub!");

  NRF24 nrf24(CE, CS, SCK, MOSI, MISO, NRF24_SPI_SPEED, hub_address.data());

  TickType_t last_wake_time;
  const TickType_t period =  pdMS_TO_TICKS(1000);
  last_wake_time = xTaskGetTickCount();

  ESP_LOGI("hub", "config, should be 12: %i", nrf24.reg_read(NRF24_CONFIG));
  nrf24.stop_listening();
  nrf24.open_tx_pipe(spoke_address.data(), 32);

  while(1)
  {
    nrf24.stop_listening();
    const auto send_error = nrf24.send((const uint8_t*)"PING\0", 5);
    nrf24.start_listening();
    switch(send_error)
    {
    case NRF24_SEND_ERROR_NONE:
      ESP_LOGE("hub", "sending error none!");
      break;
    case NRF24_SEND_ERROR_OK:
      ESP_LOGI("hub", "sending ok!");
      break;
    case NRF24_SEND_ERROR_MAX_RT:
    case NRF24_SEND_ERROR_SPURIOUS:
      ESP_LOGE("hub", "error sending!");
      break;
    }
    vTaskDelayUntil(&last_wake_time, period);
  }
}

void spoke()
{

  ESP_LOGI("spoke", "I'm the spoke!");

  NRF24 nrf24(CE, CS, SCK, MOSI, MISO, NRF24_SPI_SPEED, spoke_address.data());

  ESP_LOGI("spoke", "config, should be 12: %i", nrf24.reg_read(NRF24_CONFIG));

  TickType_t last_wake_time;
  const TickType_t ms1 =  pdMS_TO_TICKS(10);
  last_wake_time = xTaskGetTickCount();

  nrf24.start_listening();

  while(1)
  {
    bool received = false;
    for(auto i=0; i < 10; ++i)
    {
      if(nrf24.any())
      {
        received = true;
        break;
      }
      vTaskDelayUntil(&last_wake_time, ms1);
    }
    if(received)
    {
      std::array<unsigned char, 32> buffer;
      const auto count = nrf24.recv(buffer.data(), buffer.size());
      ESP_LOGI("spoke", "recv: %s, count: %i", buffer.data(), count);
    }
    else
    {
      //ESP_LOGE("spoke", "nothing for 100ms!");
    }
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
