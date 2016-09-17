/*
 * Wheel.cpp
 *
 *  Created on: Nov 25, 2015
 *      Author: pneumoman
 */

#include "Wheel.h"


Wheel::Wheel() {
  _n=0;_d=0; _nLeds=0;
  _q1=_q2=_q3=_q4=_q5=0;
}

Wheel::Wheel(uint16_t nSteps, uint16_t qtyLeds) {
  setSize(nSteps,qtyLeds);
}

void Wheel::getColor(uint16_t ledNo,uint16_t stepx,uint8_t &red,uint8_t &grn,uint8_t &blu)
{
  //get color component based angle(step) and spoke(led)#
        //space between spokes x spoke# + step, if we exceed the circumference restart at zero
	uint16_t p = (_d*ledNo+stepx)%_n;
	getColor(p, red, grn, blu);
}

void Wheel::getColor(uint16_t p,uint8_t &red,uint8_t &grn,uint8_t &blu)
{
  //Get color components based on position on wheel
  //pass in the previous quad as the offset instead 
  //of the current quad to simplfy calculation
	  if(p < _q1)
	  {//quad 1, red to yellow
		red=255;
		grn=getHue(p,0,true);
		blu=0;
	  }
	  else if (p < _q2)
	  {//quad 2 yellow to grn
		  red=getHue(p,_q1,false);
		  grn=255;
		  blu=0;
	  }
	  else if (p < _q3)
	  {//quad 3 grn to teal
		  red=0;
		  grn=255;
		  blu=getHue(p,_q2,true);
	  }
	  else if (p < _q4)
	  {//quad 4 teal to blue
		  red=0;
		  grn=getHue(p,_q3,false);
		  blu=255;
	  }
	  else if (p < _q5)
	  {//quad 5 blue to purple
		  red=getHue(p,_q4,true);
		  grn=0;
		  blu=255;
	  }
	  else if (p < _n)
	  {//quad 6 purple back to red
		  red=255;
		  grn=0;
		  blu=getHue(p,_q5,false);
	  }
}

uint32_t Wheel::getColor(uint16_t p)
{
   //return a 32bit color value, quite likely wrong.
	uint32_t clr=0;
	uint8_t red, grn, blu;
	getColor(p, red, grn, blu);
	clr=getColorNum(red,grn,blu);
	return clr;
}

void Wheel::setSize(uint16_t nSteps,uint16_t ledQty)
{
  // setup wheel parameters
  _n=nSteps;
  _nLeds=ledQty;
  //setup spoke spacing
  _d=nSteps/ledQty;
  //setup quadrants (hextants?)
 /* float Q0=(float)_n/6.0;
  _q1=floor(Q0);
  _q2=floor(2*Q0);
  _q3=floor(3*Q0);
  _q4=floor(4*Q0);
  _q5=floor(5*Q0);
*/
  _q1=100*_n/6;
  _q2=_q1+_q1;
  _q3=_q2+_q1;
  _q2=_q2/100;
  _q4=_q3+_q1;
  _q3=_q3/100;
  _q5=_q4+_q1;
  _q4=_q4/100;
  _q5=_q5/100;
  _q1=_q1/100;
}

uint8_t Wheel::getHue(uint16_t p, uint16_t qtr, bool up)
{
 //calculate place in color range for a component
        //chop off quadrant offset
	uint16_t p0=p-qtr;
	uint8_t hue;
        //determine place in range, linear scaling
	hue=(255*p0)/_q1;
        //depending where we are on our wheel the color value could be decending
	if(!up)
	{
		hue=255-hue;
	}

	return hue;
}

uint32_t Wheel::getColorNum(uint8_t r,uint8_t g, uint8_t b)
{
  // in theory converts color components to 32bit color number - use neopixels call if you must
	uint32_t clrno=r;
	clrno=(clrno<<8) | g;
	clrno=(clrno<<8) | b;
	return clrno;
}
char* Wheel::getSettings()
{
	char* rtn = (char*)malloc(100);//new char[70];
                   //12   34567   891123   456789   212345   678931   234567   89
	sprintf(rtn,"n:%4d | d:%4d | q1:%4d | q2:%4d | q3:%4d | q4:%4d | q5:%4d\n",_n,_d,_q1,_q2,_q3,_q4,_q5);
	return rtn;
}

Wheel::~Wheel() {
	//Auto-generated destructor stub
}

