/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    FractTab.h: an array for fractal deformation of the enlarge-kernels

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

#ifndef FRACTTAB_H
#define FRACTTAB_H

#include <math.h>
#include "ConstDefs.h"
#include "PointClass.h"
#include "Array.h"
#include <iostream>
using namespace std;

const int FRACTTABEXP  = 9;
const int FRACTTABLEN  = (1<<FRACTTABEXP);
const int FRACTTABMASK = (1<<FRACTTABEXP)-1;
const int RandTabExp  = 12;
const int RandTabLen  = (1<<RandTabExp);
const int RandTabMask = (1<<RandTabExp)-1;

class FractTab {
   BasicArray<PFloat> fTab;
   RandGen randG;
   int *randTab;
   float scaleF, invScaleF;

public:
   FractTab( float sF );
   ~FractTab( void );

   // get random kernel center ( for rand seeds in centerX, centerY )
   void GetKerCenter( int & centerX, int & centerY, float & centerV ) {
      RandPerm( centerX, centerY );
      centerX &= FRACTTABMASK;
      centerY &= FRACTTABMASK;
      centerV = fTab.Get( centerX, centerY ).toF();
   }
   void CreateTab( void );
   float GetT  ( int x, int y ) {
      x &= FRACTTABMASK; y &= FRACTTABMASK;
      return fTab.Get( x, y ).toF();
   }

private:
   void AddRand( BasicArray<PFloat> *a0, float rFakt);
   void SaveTab( void );
   void CreateRandTab( void );
   int  Rand( int r ) { return randTab[ r & RandTabMask ]; }
   void RandPerm( int & r1, int & r2 ) { // (r1,r2) -> (rand1(r1,r1),rand2(r1,r2))
      r1 = (r1>>8) + Rand( r1 );
      r2 = (r2>>8) + Rand( r2 );
      r1 = Rand( r1^r2 );
      r2 = Rand( r1+r2 );
   }
};

#endif // FRACTTAB_H
