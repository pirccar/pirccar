#include "lcdscreen.h"
#include "PCD8544.h"
#include <cstdio>


LcdScreen::LcdScreen(void)
{
}
	
void LcdScreen::init(void)
{
	// pin setup
	int _din = 4;
	int _sclk = 5;
	int _dc = 3;
	int _rst = 0;
	int _cs = 2;

	LCDInit(_sclk, _din, _dc, _cs, _rst, 50);
	LCDclear();
}

void LcdScreen::showLogo(void)
{
	LCDshowLogo();
}

void LcdScreen::clear(void)
{
	printf("Hepp2!\n");
	LCDclear();
	LCDdisplay();
}

void LcdScreen::printText(std::string output, int x, int y)
{
	LCDclear();
	LCDdrawstring_P(x,y, output.c_str());
	LCDdisplay();
}

void LcdScreen::printImage(const uint8_t* img, int x, int y, int w, int h)
{
	LCDclear();
	LCDdrawbitmap(x, y, img, w, h, 255);
	LCDdisplay();
}