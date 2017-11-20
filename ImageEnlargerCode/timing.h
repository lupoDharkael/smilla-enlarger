/* ----------------------------------------------------------------

ImageEnlarger  -  resize, especially magnify bitmaps in high quality
    timing.h: time-measuring of program parts

Copyright (C) 2009 Mischa Lusteck

This program is free software;
you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation;
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.

---------------------------------------------------------------------- */

#ifndef _TIMING
#define _TIMING

#include <iostream>

using namespace std;

#include <time.h>


// Performance-Messung von Programmstuecken
//  

class Timer
{
   long timer;
   long hilf;
public:
   Timer(void) {timer=0;}
   ~Timer(void){}
   void Start(void)
      {hilf=clock();} 
   void Stop(void) 
      {timer+=clock()-hilf;}
   void Clear(void) {timer=0;}
   double Get(void) {return double(timer)/(CLOCKS_PER_SEC);}
};

#endif


