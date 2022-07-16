#ifndef __ONEWIRE_GPIO_H__
#define __ONEWIRE_GPIO_H__

unsigned char ow_reset(void);
unsigned char read_byte(void);
void write_byte(char val);
void Software_Reset(void);

#endif
