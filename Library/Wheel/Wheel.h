/*
 * Wheel.h
 *
 *  Created on: Nov 25, 2015
 *      Author: pneumoman
 */

#ifndef WHEEL_H_
#define WHEEL_H_


#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

class Wheel {
public:

	Wheel(uint16_t nSteps,uint16_t ledQty);
	uint32_t getColor(uint16_t p);
	void getColor(uint16_t ledNo, uint16_t step,uint8_t &red,uint8_t &grn,uint8_t &blu);
	void setSize(uint16_t nx,uint16_t ledQty);
        char * getSettings();
	virtual ~Wheel();
private:
	uint16_t _n;/*steps*/
	uint16_t _nLeds;
	uint16_t _d;
	uint16_t _q1;
	uint16_t _q2;
	uint16_t _q3;
	uint16_t _q4;
	uint16_t _q5;
uint8_t getHue(uint16_t p, uint16_t qtr, bool up);
uint32_t getColorNum(uint8_t r,uint8_t g, uint8_t b);
void getColor(uint16_t p,uint8_t &red,uint8_t &grn,uint8_t &blu);
	Wheel();
};

#endif /* WHEEL_H_ */

