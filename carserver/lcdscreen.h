#pragma once
#include <string>
#include <stdint.h>


class LcdScreen
{
public:
	LcdScreen(void);
	
	void init(void);
	void showLogo(void);
	void clear(void);
	
	void printText(std::string output, int x = 0, int y = 0);
	void printImage(const uint8_t* img, int x = 0, int y = 0, int w = 84, int h = 48);
private:
};