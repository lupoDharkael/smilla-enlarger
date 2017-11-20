/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    ConstDefs.h:  some constants

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

#include <math.h>

#ifndef CONST_DEFS_H
#define CONST_DEFS_H

#ifndef PI
const double PI = 3.14159265358979323846;
#endif

const int enExp = 3 ;
const int enLen = (1<<enExp) ;

const int blockExp = 9;               // len of dstBlock
const int blockLen = (1<<blockExp);
const int srcBlockMargin = 9;        // srcBlock contains additional margins of borderPixels

const float similPeakThinness   = 8.0 ;      // sharper peak at 0.0 -> others are less similar
const float similPeakFlatness   = 1.5 ;      //
const float similWeightFakt     = 0.09;     // 0.09
const float similPeakFakt       = 0.7 ;      // fakt*peak + ( 1 - fakt )*linear
		                                       //
const float indieWeightFact     = 1.0 ;      // modifies weight of indies
const float indieSensitivity    = 7.0 ;      // higher value -> individuality at smaller distance
const float indieThreshold      = 1.0 ;      // threshold for summed-up individuality
		                                       //
const float selectPeakSharpness = 10.0;      // sharper peak at 0.0 -> others are less similar
                                             // and thus less weighted -> sharper image

const int kerFineExp = 4; 
const int kerFineLen = 1<<kerFineExp; 
const int diffTabLen = 1000;
const int invTabLen  = 10000;

inline float pow_f(float a,float b) {return (float)pow(a,b); }


#endif
