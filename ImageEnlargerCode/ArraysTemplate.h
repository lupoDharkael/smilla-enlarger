/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    ArraysTemplate.h: basic template for 2-dim. arrays

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

#ifndef ARRAYS_TEMPLATE_H
#define ARRAYS_TEMPLATE_H

template<class T>
class BasicArray {
   
   int sizeX,sizeY;
   T *buf;
public:
   BasicArray(void)          : sizeX(0), sizeY(0)   { buf = 0; }
   BasicArray(int sx,int sy) : sizeX(sx), sizeY(sy) { buf = new T[sizeX*sizeY]; }
   BasicArray( BasicArray<T> & aSrc);
   ~BasicArray(void) { if(buf!=0) delete[] buf; }

   BasicArray<T> &operator= ( BasicArray<T> & aSrc );

   T     Get(int x, int y) const { return buf[ x + y*sizeX ]; }
   void  Set(int x, int y, T p )     { buf[ x + y*sizeX ] =  p; }
   void  Add(int x, int y, T p )     { buf[ x + y*sizeX ] += p; }
   void  Sub(int x, int y, T p )     { buf[ x + y*sizeX ] -= p; }
   void  Mul(int x, int y, float f ) { buf[ x + y*sizeX ] *= f; }
   T     DX(int x, int y)  { return 0.5*(Get(x+1,y) - Get(x-1,y)); }
   T     DY(int x, int y)  { return 0.5*(Get(x,y+1) - Get(x,y-1)); }
   T     D2X(int x, int y) { return Get(x+1,y) + Get(x-1,y) - 2.0*Get(x,y); }
   T     D2Y(int x, int y) { return Get(x,y+1) + Get(x,y-1) - 2.0*Get(x,y); }
   T     DXY(int x, int y) { return Get(x+1,y+1) - Get(x-1,y+1) - Get(x+1,y-1) + Get(x-1,y-1); }
   T     Laplace(int x,int y)  {
      T l;
      l  = Get( x  , y-1 ) + Get( x-1, y   ) + Get( x+1, y   ) + Get( x  , y+1 );
      l*=2.0;
      l += Get( x-1, y-1 ) + Get( x+1, y-1 ) + Get( x-1, y+1 ) + Get( x+1, y+1 );
      l*=(1.0/12.0);
      return l - Get(x,y);
   }
   
   int SizeX(void) const { return sizeX; }
   int SizeY(void) const { return sizeY; }
   T *Buffer(void) { return buf; }
   
   void ChangeSize( int sxNew, int syNew ) { 
      sizeX = sxNew; sizeY = syNew;
      if(buf!=0) delete[] buf;
      buf = new T[sizeX*sizeY];
   }
   void CopyFromArray(BasicArray<T> *srcArr, int srcX,int srcY);
   
   BasicArray<T> *Clip(int leftX,int topY,int sizeXNew, int sizeYNew) {
      BasicArray<T> *clipArr;
      clipArr = new BasicArray<T>(sizeXNew,sizeYNew);
      clipArr->CopyFromArray(this, leftX,topY);
      return clipArr;
   }
   
   BasicArray<T> *SmoothDouble ( void );
   BasicArray<T> *SmoothDoubleTorus ( void );
   BasicArray<T> *ShrinkHalf   ( void );
   BasicArray<T> *Shrink       ( int sizeXNew, int sizeYNew );
   BasicArray<T> *BlockyPixEnlarge( int sizeXNew, int sizeYNew );
   BasicArray<T> *SplitLowFreq ( int lenExp );
   BasicArray<T> *Smooth       ( void );
   void AddArray ( BasicArray<T> *arr );
   void SubArray ( BasicArray<T> *arr );
   void MulArray ( float f );
   void Sharpen  ( float f );
   void Smoothen ( void );
   void Clamp01  ( void );
   void ReduceNoise( float reduceF );
   void HiSharpen ( float f );


private:
   T SmoothFunc(T p0, T p1, T p2) { return 0.0625*( 5.0*p0 + 10.0*p1 + p2 ); }
   void ReadLineSmoothDouble(int y,T *line);
   void ReadLineSmoothDoubleTorus(int y,T *line);
   void ShrinkLine(int y, float scaleF, int sizeXNew, T *line);
};

#endif
