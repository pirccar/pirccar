#include <stdint.h>
#ifndef _pca9685_H
#define _pca9685_H

#define MODE1 0x00
#define MODE2 0x01
#define SUBADR1 0x02
#define SUBADR2 0x03
#define SUBADR3 0x04
#define ALLCALLADR 0x05
#define LED0 0x06
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09
#define LED_MULTIPLYER 4
#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD
#define PRE_SCALE 0xFE
#define CLOCK_FREQ 25000000.0
#define CLK_DIV BCM2835_I2C_CLOCK_DIVIDER_148

#define MAX_LEN 32

void PCA9685_init();
void setPWMFreq(int freq);
void setPWMOnOff(uint8_t led, int on_value, int off_value);
void setPWM(uint8_t led, int off_value);
int getPWMOff(uint8_t led);
int getPWMOn(uint8_t led);

void PCA9685_reset();

#endif