/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    Array.h: 2-dim. float-array and direction-array

Copyright (C) 2009 Mischa Lusteck
Copyright (C) 2017 Alejandro Sirgo

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

#ifndef ARRAY_H
#define ARRAY_H

#include <math.h>
#include "ConstDefs.h"
#include "PointClass.h"
#include "ArraysTemplate.h"

using namespace std;

class RandGen
{
   long rx,ry;
public:
   RandGen(void)    { rx = 639286192; ry = 207155721; }
   RandGen(long xx) { rx = xx; ry = rx*(rx>>5) + 1130958141 + (rx>>16); }
   RandGen(long xx, long yy)  { rx = xx; ry = yy;}
   ~RandGen(void) {}

 
   long RandL(void) {
      rx = (rx^ry) + 78511087 + (ry>>16); 
      ry = ry*(rx>>5) + 1130958141  + (rx>>16);
      return ry; 
   } 

   long RandL(long n) {
      rx = (rx^ry) + 78511087 + (ry>>16) + n; 
      ry = ry*(rx>>5) + 1130958141  + (rx>>16);
      return ry; 
   } 

   float RandF(void) { return float((RandL()>>5)&65535) * (1.0 / 65535.0); } 

 };

class DirArray;

class MyArray : public BasicArray<PFloat> {
public:
   MyArray(void) : BasicArray<PFloat>() {}
   MyArray(int sx,int sy) : BasicArray<PFloat>(sx,sy) {}
   MyArray(MyArray & aSrc) : BasicArray<PFloat>(aSrc) {}

   float GetF(int x, int y)        { return Get(x, y).toF(); }
   void  Set(int x, int y, PFloat p) { BasicArray<PFloat>::Set(x, y, p); }
   void  Add(int x, int y, PFloat p) { BasicArray<PFloat>::Add(x, y, p); }
   void  Sub(int x, int y, PFloat p) { BasicArray<PFloat>::Sub(x, y, p); }
   void  Set(int x, int y, float p) { BasicArray<PFloat>::Set(x, y, PFloat(p)); }
   void  Add(int x, int y, float p) { BasicArray<PFloat>::Add(x, y, PFloat(p)); }
   void  Sub(int x, int y, float p) { BasicArray<PFloat>::Sub(x, y, PFloat(p)); }

   MyArray *SmoothDouble(void)
      { return (MyArray *) BasicArray<PFloat>::SmoothDouble(); }
   MyArray *ShrinkHalf(void)     
      { return (MyArray *) BasicArray<PFloat>::ShrinkHalf(); }
   MyArray *Shrink(int sizeXNew, int sizeYNew)   
      { return (MyArray *) BasicArray<PFloat>::Shrink(sizeXNew,sizeYNew); }
   MyArray *SplitLowFreq(int lenExp)
      { return (MyArray *) BasicArray<PFloat>::SplitLowFreq(lenExp); }
   MyArray *Smooth(void)
      { return (MyArray *) BasicArray<PFloat>::Smooth(); }
   
   DirArray *Gradient(void);
   
   void FillWithHill(void);
   void FillWithDots(void);
   void OperateDir(DirArray *dirArr,RandGen & rGen);

private:
   void Tst(void);
};


// ---------------------------------------------------------------
// ---------------------------------------------------------------


class DirArray : public BasicArray<Point2> {
public:
   DirArray(void) : BasicArray<Point2> () {}
   DirArray(int sx,int sy) : BasicArray<Point2> (sx,sy) {}

   DirArray *SmoothDouble(void)   
      { return (DirArray *) BasicArray<Point2>::SmoothDouble(); }
   DirArray *ShrinkHalf(void)     
      { return (DirArray *) BasicArray<Point2>::ShrinkHalf(); }
   DirArray *Shrink(int sizeXNew, int sizeYNew)   
      { return (DirArray *) BasicArray<Point2>::Shrink(sizeXNew,sizeYNew); }
   DirArray *SplitLowFreq(int lenExp)
      { return (DirArray *) BasicArray<Point2>::SplitLowFreq(lenExp); }
   DirArray *Smooth(void)
      { return (DirArray *) BasicArray<Point2>::Smooth(); }
   
   DirArray *Func0(void);
   DirArray *Func(void);
   void Normalize(void);
   void GradToDir(void); 
   void SmoothenWithDir(void);
private:

   inline float NFunc(Point2 n1, Point2 n2) {
      float w;
      w = 1.0 - (n1 - n2).Norm()*10.0;
      if(w<0.0)w=0.0;
      //w=w*w*(3.0 - 2.0*w);
      w*=w;
      w*=w;
      return w;
   }
   inline float NFunc0(Point2 n1, Point2 n2) {
      float w = n1*n2;
      w = 0.5 + 0.5*w;
      w = 6.0*w - 5.0;
      if(w<0.0) w=0.0;
      w = w*w;
      w = w*w;
      return w;
   }
};


#endif
