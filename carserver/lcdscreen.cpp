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