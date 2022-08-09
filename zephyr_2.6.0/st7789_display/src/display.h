#include <stdint.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135
int display_begin();
void display_command(uint8_t cmd);
void display_data(uint8_t data);
void display_openAperture(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void display_putPixel(uint16_t x, uint16_t y, uint16_t colour);
void display_putImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *Image);
void display_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour);
int iabs(int x); // simple integer version of abs for use by graphics functions        
void display_drawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Colour);
void display_fillRectangle(uint16_t x,uint16_t y,uint16_t width, uint16_t height, uint16_t colour);
void display_drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour);
void display_fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour);
void display_print(const char *Text, uint16_t len, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour);
uint16_t display_RGBToWord(uint16_t R, uint16_t G, uint16_t B);
