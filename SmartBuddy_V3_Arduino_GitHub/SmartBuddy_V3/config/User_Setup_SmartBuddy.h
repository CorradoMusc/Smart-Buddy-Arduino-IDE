#pragma once
// REFERENCE CONFIGURATION for Bodmer TFT_eSPI User_Setup.h.
// Your existing working library setup may already be equivalent.
// Keep ONE active display driver and ONE group of active GPIO definitions.
#define USER_SETUP_INFO "Smart Buddy V3 ST7796S"
#define ST7796_DRIVER
#define TFT_WIDTH 320
#define TFT_HEIGHT 480
#define TFT_MISO -1
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 27
#define TFT_RST 33
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define SPI_FREQUENCY 20000000
#define SPI_READ_FREQUENCY 10000000
