#include <errno.h>
#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pca9685.h"


void PCA9685_init(){
	bcm2835_i2c_begin();
	bcm2835_i2c_setSlaveAddress(0x40);
}

void setPWMFreq(int freq){
	char cmd[2]; 
	uint8_t ret = 0;
	uint8_t prescale_val = (CLOCK_FREQ / 4096 / freq) -1;
	
	printf("Prescale: %u \n", prescale_val);
	
	cmd[0] = MODE1;
	cmd[1] = 0x10;
	
	ret = bcm2835_i2c_write((const char*)&cmd, 2);
	
	cmd[0] = PRE_SCALE;
	cmd[1] = prescale_val;
	
	ret = bcm2835_i2c_write((const char*)&cmd, 2);
	
	cmd[0] = MODE1;
	cmd[1] = 0x80;
	
	ret = bcm2835_i2c_write((const char*)&cmd, 2);
	
	cmd[0] = MODE2;
	cmd[1] = 0x04;
	
	ret = bcm2835_i2c_write((const char*)&cmd, 2);
	
	//printf("Freq Returns: %u \n", ret);
}
void setPWMOnOff(uint8_t led, int on_value, int off_value){
	char cmdOnL[2];
	char cmdOnH[2];
	char cmdOffL[2]; 
	char cmdOffH[2];
	uint8_t ret = 0;
	
	cmdOnL[0] = LED0_ON_L + LED_MULTIPLYER * (led-1);
	cmdOnL[1] = on_value & 0xFF;
	
	cmdOnH[0] = LED0_ON_H + LED_MULTIPLYER * (led-1);
	cmdOnH[1] = on_value >> 8;
	
	cmdOffL[0] = LED0_OFF_L + LED_MULTIPLYER * (led-1);
	cmdOffL[1] = off_value & 0xFF;
	
	cmdOffH[0] = LED0_OFF_H + LED_MULTIPLYER * (led-1);
	cmdOffH[1] = off_value >> 8;
	
	ret = bcm2835_i2c_write((const char*)&cmdOnL, 2);
	
	ret = bcm2835_i2c_write((const char*)&cmdOnH, 2);
	
	ret = bcm2835_i2c_write((const char*)&cmdOffL, 2);	
	
	ret = bcm2835_i2c_write((const char*)&cmdOffH, 2);	
	
	//printf("Write returns: %u \n", ret);
}
void setPWM(uint8_t led, int off_value){
	setPWMOnOff(led, 0, off_value);
}


int getPWMOn(uint8_t led){
	int val = 0;
	char buf[MAX_LEN];
	char addr = LED0_ON_H + LED_MULTIPLYER * (led-1);
	bcm2835_i2c_read_register_rs((char*)&addr, (char*)&buf, 1);
	
	val = buf[0];
	val = val & 0xf;
	val <<= 8;
	
	addr = LED0_ON_L + LED_MULTIPLYER * (led-1);
	
	bcm2835_i2c_read_register_rs((char*)&addr, (char*)&buf, 1);
	
	val += buf[0];
	
	return val;
}


int getPWMOff(uint8_t led){
	int val = 0;
	char buf[MAX_LEN];
	char addr = LED0_OFF_H + LED_MULTIPLYER * (led-1);;
	
	bcm2835_i2c_read_register_rs((char*)&addr, (char*)&buf, 1);
	
	val = buf[0];
	val = val & 0xf;
	val <<= 8;
	
	addr = LED0_OFF_L + LED_MULTIPLYER * (led-1);
	
	bcm2835_i2c_read_register_rs((char*)&addr, (char*)&buf, 1);
	
	val += buf[0];
	
	return val;
}

void PCA9685_reset(){
	char cmd[2];
	cmd[0] = MODE1;
	cmd[1] = 0x00;
	bcm2835_i2c_write((const char*)&cmd, 2);
	
	cmd[0] = MODE2;
	cmd[1] = 0x04;
	
	bcm2835_i2c_write((const char*)&cmd, 1);
}