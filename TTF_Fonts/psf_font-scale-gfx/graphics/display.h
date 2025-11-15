#ifndef _display_H_
#define _display_H_

#include "stdint.h"
#include "color.h"

/* *** Function prototypes. *** */
uint16_t Display_Get_WIDTH(void);
uint16_t Display_Get_HEIGHT(void);
void Display_Set_WIDTH(uint16_t x);
void Display_Set_HEIGHT(uint16_t y);

#endif /* _display_H_ */

