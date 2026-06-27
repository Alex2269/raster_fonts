#include "stdio.h"
#include "stdbool.h"

#include "display.h"

uint16_t DISPLAY_WIDTH;
uint16_t DISPLAY_HEIGHT;

uint16_t Display_Get_WIDTH(void)
{
  return DISPLAY_WIDTH;
}

uint16_t Display_Get_HEIGHT(void)
{
  return DISPLAY_HEIGHT;
}

void Display_Set_WIDTH(uint16_t x)
{
  DISPLAY_WIDTH = x;
}

void Display_Set_HEIGHT(uint16_t y)
{
  DISPLAY_HEIGHT = y;
}

