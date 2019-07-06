#include "u8g2_esp32_hal.h"

#include <u8g2.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// DISPLAY
#define DISPLAY_DC   25
#define DISPLAY_RST  18
#define DISPLAY_CLK  19
#define DISPLAY_CS   22
#define DISPLAY_MOSI 23
#define DISPLAY_SPEED (500*1000)

extern "C" void app_main();

void app_main()
{
  u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
  u8g2_esp32_hal.clk   = (gpio_num_t)DISPLAY_CLK;
  u8g2_esp32_hal.mosi  = (gpio_num_t)DISPLAY_MOSI;
  u8g2_esp32_hal.cs    = (gpio_num_t)DISPLAY_CS;
  u8g2_esp32_hal.dc    = (gpio_num_t)DISPLAY_DC;
  u8g2_esp32_hal.reset = (gpio_num_t)DISPLAY_RST;

  u8g2_esp32_hal_init(u8g2_esp32_hal);

  u8g2_t u8g2; // a structure which will contain all the data for one display
  u8g2_Setup_sh1106_128x64_noname_f(
    &u8g2,
    U8G2_R0,
    u8g2_esp32_spi_byte_cb,
    u8g2_esp32_gpio_and_delay_cb);  // init u8g2 structure

  u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,

  u8g2_SetPowerSave(&u8g2, 0); // wake up display

  TickType_t last_wake_time;
  const TickType_t period =  pdMS_TO_TICKS(16);
  last_wake_time = xTaskGetTickCount();

  int x = 0;
  while(1)
  {
    ESP_LOGI("main", "loop");
  u8g2_ClearBuffer(&u8g2);
  u8g2_DrawBox(&u8g2, x,20, 20, 30);
  x = (x + 1) % (128 - 20);
  u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
  u8g2_DrawStr(&u8g2, 0,15,"Hello World!");
  u8g2_SendBuffer(&u8g2);

    vTaskDelayUntil(&last_wake_time, period);
  }
}
