/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    ArraysTemplateDefs.h: basic template for 2-dim. arrays

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

// class T needs float-vector-operations  plus scalar product
// additionally:
//   t.Norm1()        : simple norm
//   t.Clip()         : clamping
//   t.Normalize()    :  t/Norm(t)
//   t.Normalized()   :  t/Norm(t)
//   t.AddMul(a, p) :  t += a*p
//   t.IsZero()
//   t.SetZero()

#ifndef ARRAYS_TEMPLATE_DEFS_H
#define ARRAYS_TEMPLATE_DEFS_H

#include <iostream>
#include <math.h>
#include "ImageEnlargerCode/ConstDefs.h"
#include "ArraysTemplate.h"

using namespace std;

template<class T>
BasicArray<T>::BasicArray(BasicArray<T> & aSrc) : sizeX(aSrc.sizeX) , sizeY(aSrc.sizeY) {
   buf = new T[sizeX*sizeY];
   
   int a = sizeX*sizeY;
   T *src = aSrc.buf, *dst = buf;
   while(a-- > 0) 
      *(dst++) = *(src++);
      
}

template<class T>
BasicArray<T> & BasicArray<T>::operator= (BasicArray<T> & aSrc) {
   ChangeSize(aSrc.sizeX, aSrc.sizeY);
   int a = sizeX*sizeY;
   T *src = aSrc.buf, *dst = buf;
   while(a-- > 0)
      *(dst++) = *(src++);
   return *this;
}

// Copy (sizeX,sizeY)-Array out of srcArray beginning at (srcX,srcY)
// if parts of dst are outside, fill them with border-values of srcArray
template<class T>
void BasicArray<T>::CopyFromArray(BasicArray<T> *srcArr, int srcX,int srcY) {
   int x,y,sx,sy;
   T *src, *dst;
   y=0;sy=srcY;
   dst = buf;
   
   // while dst outside: copy pixels of src-line 0
   while(sy<=0 && y<sizeY) {   
      x=0;sx=srcX;
      src = srcArr->buf;
      if(sx>0)
         src += sx;
      // while dst outside: write src-edge-pixel
	  while(sx<0 && x<sizeX) {
         *(dst++) = *src;      x++; sx++;
      }

	  while(sx < srcArr->sizeX - 1 && x<sizeX) {
         *(dst++) = *(src++);  x++; sx++;
      }

      // while dst outside: write src-edge-pixel
	  while(x<sizeX) {
         *(dst++) = *src;      x++; sx++;
      }
      y++;sy++;
   }
      
   while(sy < srcArr->sizeY - 1 && y<sizeY) {   
      x=0;sx=srcX;
      src = srcArr->buf + sy*srcArr->sizeX;
      if(sx>0)
         src += sx;
      // while dst outside: write src-edge-pixel
	  while(sx<0 && x<sizeX) {
         *(dst++) = *src;      x++; sx++;
      }

	  while(sx < srcArr->sizeX - 1 && x<sizeX) {
         *(dst++) = *(src++);  x++; sx++;
      }

      // while dst outside: write src-edge-pixel
	  while(x<sizeX) {
         *(dst++) = *src;      x++; sx++;
      }
      y++;sy++;
   }
      
   // for outside-parts: copy pixels of last src-line
   while(y<sizeY) {   
      x=0;sx=srcX;
      src = srcArr->buf + (srcArr->sizeY - 1)*srcArr->sizeX;
      if(sx>0)
         src += sx;
      // while dst outside: write src-edge-pixel
	  while(sx<0 && x<sizeX) {
         *(dst++) = *src;      x++; sx++;
      }

	  while(sx < srcArr->sizeX - 1 && x<sizeX) {
         *(dst++) = *(src++);  x++; sx++;
      }

      // while dst outside: write src-edge-pixel
	  while(x<sizeX) {
         *(dst++) = *src;      x++; sx++;
      }
      y++;sy++;
   }
}

template<class T>
BasicArray<T> *BasicArray<T>::SplitLowFreq(int lenExp) {
   int x,y;
   BasicArray<T> loArr((sizeX+(1<<lenExp))>>lenExp , (sizeY+(1<<lenExp))>>lenExp) ;

   for (y=0; y<loArr.SizeY(); y++)   {
	  for (x=0; x<loArr.SizeX(); x++)   {
		 loArr.Set(x,y, T(0.0));
      }
   }
   
   for (y=0; y<sizeY; y++)   {
	  for (x=0; x<sizeX; x++)   {
         loArr.Add(x>>lenExp,y>>lenExp,Get(x,y));
      }
   }
   float fFakt = 1.0/float(1<<(2*lenExp));
   for (y=0; y<loArr.SizeY(); y++)   {
	  for (x=0; x<loArr.SizeX(); x++)   {
		 loArr.Set(x,y, loArr.Get(x,y)*fFakt);
      }
   }
   BasicArray<T> *a2,*a3;
   a2 = loArr.SmoothDouble();
   for(int a=0;a<lenExp-1;a++) {
      a3 = a2->SmoothDouble();
      delete a2;
      a2 =a3;
   }

   a3 = new BasicArray<T>(sizeX,sizeY);

   for (y=0; y<sizeY; y++)   {
	  for (x=0; x<sizeX; x++)   {
         T w =  a2->Get(x,y);
         a3->Set(x,y,w);
         w = (Get(x,y) - w);
         Set(x,y,w);
      }
   }
   
   delete a2;
   return a3;

}

template<class T>
void BasicArray<T>::AddArray(BasicArray<T> *arr) {
   int x,y;
   for (y=0; y<sizeY; y++)   {
	  for (x=0; x<sizeX; x++)   {
         Add(x,y,arr->Get(x,y));
      }
   }
}

template<class T>
void BasicArray<T>::SubArray(BasicArray<T> *arr) {
   int x,y;
   for (y=0; y<sizeY; y++)   {
	  for (x=0; x<sizeX; x++)   {
         Add(x,y,-arr->Get(x,y));
      }
   }
}

template<class T>
void BasicArray<T>::MulArray(float f) {
   int x,y;
   for (y=0; y<sizeY; y++)   {
	  for (x=0; x<sizeX; x++)   {
         Mul(x,y,f);
      }
   }
}

template<class T>
BasicArray<T> *BasicArray<T>::ShrinkHalf(void) {
   BasicArray<T> *halfArr;
   int x,y;
   halfArr = new BasicArray<T>((sizeX+1)>>1 , (sizeY+1)>>1) ;

   for (y=0; y<halfArr->SizeY(); y++)   {
	  for (x=0; x<halfArr->SizeX(); x++)   {
		 halfArr->Set(x,y, T(0.0));
      }
   }
   
   for (y=0; y<sizeY; y++)   {
	  for (x=0; x<sizeX; x++)   {
         halfArr->Add(x>>1,y>>1,Get(x,y));
      }
   }

   for (y=0; y<halfArr->SizeY(); y++)   {
	  for (x=0; x<halfArr->SizeX(); x++)   {
         halfArr->Mul(x,y,0.25);
      }
   }
   return halfArr;
}

template<class T>
void BasicArray<T>::Sharpen(float f) {
   BasicArray<T> src(*this);
   int x,y;
   
   if(f==0.0)
      return;
      
   for(y=1;y<sizeY-1;y++) {
      for(x=1;x<sizeX-1;x++) {
         T l;
		 l  = src.Get(x  , y-1) + src.Get(x-1, y  );
		 l += src.Get(x+1, y  ) + src.Get(x  , y+1);
         l*=2.0;
		 l += src.Get(x-1, y-1) + src.Get(x+1, y-1);
		 l += src.Get(x-1, y+1) + src.Get(x+1, y+1);
         l*=(1.0/12.0);
		 l -= src.Get(x  , y  );
		 l = src.Get(x  , y  ) - f*l;
         Set(x,y,l);
      }
   }
}


template<class T>
BasicArray<T> *BasicArray<T>::SmoothDouble(void) {
   BasicArray<T> *newArray = new BasicArray<T>(sizeX*2,sizeY*2);
   T *line0, *line1, *line2;
   
   line0 = new T[2*sizeX];
   line1 = new T[2*sizeX];
   line2 = new T[2*sizeX];

	ReadLineSmoothDouble(0,line1);
	ReadLineSmoothDouble(0,line2);
   for(int y=0;y<sizeY;y++) {
      T *hh;
 	   // scroll down: swap lines and refresh line2
      hh = line0;
      line0 = line1;
      line1=line2;
      line2 = hh;  
      if(y<sizeY-1)
         ReadLineSmoothDouble(y+1,line2);
      else
         ReadLineSmoothDouble(y  ,line2);
	  for(int x=0;x<2*sizeX;x++) {
         T p0,p1,p2,p;
         p0 = line0[x];
         p1 = line1[x];
         p2 = line2[x];
         p = SmoothFunc(p0,p1,p2);
		 newArray->Set(x, 2*y   , p);
         p = SmoothFunc(p2,p1,p0);
		 newArray->Set(x, 2*y+1 , p);
      }
   }
   delete[] line0;
   delete[] line1;
   delete[] line2;
   return newArray;
}

template<class T>
void BasicArray<T>::ReadLineSmoothDouble(int y,T *line) {
   int x;
   T *src,p0,p1,p2;
   
   src = buf + y*sizeX;
   p0 = *src;
   p1 = *src;
   p2 = *(src+1);
   *(line++) = SmoothFunc(p0,p1,p2);
   *(line++) = SmoothFunc(p2,p1,p0);
   for(x=1;x<sizeX-1;x++) {
      p0 = *src;
      p1 = *(src+1);
      p2 = *(src+2);
      *(line++) = SmoothFunc(p0,p1,p2);
      *(line++) = SmoothFunc(p2,p1,p0);
      src++;
   }
   p0 = *src;
   p1 = *(src+1);
   p2 = *(src+1);
   *(line++) = SmoothFunc(p0,p1,p2);
   *(line++) = SmoothFunc(p2,p1,p0);
}

template<class T>
BasicArray<T> *BasicArray<T>::SmoothDoubleTorus(void) {
   BasicArray<T> *newArray = new BasicArray<T>(sizeX*2,sizeY*2);
   T *line0, *line1, *line2;

   line0 = new T[2*sizeX];
   line1 = new T[2*sizeX];
   line2 = new T[2*sizeX];

   ReadLineSmoothDoubleTorus(sizeY-1,line1);
   ReadLineSmoothDoubleTorus(0,line2);
   for(int y=0;y<sizeY;y++) {
      T *hh;
      // scroll down: swap lines and refresh line2
      hh = line0;
      line0 = line1;
      line1=line2;
      line2 = hh;
      if(y<sizeY-1)
         ReadLineSmoothDoubleTorus(y+1,line2);
      else
         ReadLineSmoothDoubleTorus(0  ,line2);
	  for(int x=0;x<2*sizeX;x++) {
         T p0,p1,p2,p;
         p0 = line0[x];
         p1 = line1[x];
         p2 = line2[x];
         p = SmoothFunc(p0,p1,p2);
		 newArray->Set(x, 2*y   , p);
         p = SmoothFunc(p2,p1,p0);
		 newArray->Set(x, 2*y+1 , p);
      }
   }
   delete[] line0;
   delete[] line1;
   delete[] line2;
   return newArray;
}

template<class T>
void BasicArray<T>::ReadLineSmoothDoubleTorus(int y,T *line) {
   int x;
   T *src,p0,p1,p2;
   T *pFirst, *pLast;
   pFirst = buf + y*sizeX;
   pLast  = pFirst + sizeX-1;

   src = pFirst;
   p0 = *pLast;
   p1 = *src;
   p2 = *(src+1);
   *(line++) = SmoothFunc(p0,p1,p2);
   *(line++) = SmoothFunc(p2,p1,p0);
   for(x=1;x<sizeX-1;x++) {
      p0 = *src;
      p1 = *(src+1);
      p2 = *(src+2);
      *(line++) = SmoothFunc(p0,p1,p2);
      *(line++) = SmoothFunc(p2,p1,p0);
      src++;
   }
   p0 = *src;
   p1 = *(src+1);
   p2 = *pFirst;
   *(line++) = SmoothFunc(p0,p1,p2);
   *(line++) = SmoothFunc(p2,p1,p0);
}

template<class T>
BasicArray<T> *BasicArray<T>::Smooth(void) {
   int x,y;
   BasicArray<T> *a4 = new BasicArray<T>(sizeX,sizeY);

   for (y=1; y<sizeY-1; y++)   {
	  for (x=1; x<sizeX-1; x++)   {
         T pSmooth;
         
         pSmooth  =       Get(x-1,y-1) + 2.0*Get(x  ,y-1) + Get(x+1,y-1);
		 pSmooth += 2.0*(Get(x-1,y ) + 2.0*Get(x  ,y ) + Get(x+1,y ));
         pSmooth +=       Get(x-1,y+1) + 2.0*Get(x  ,y+1) + Get(x+1,y+1);
         pSmooth*=0.0625;
         a4->Set(x,y,pSmooth);
      }
   }
   return a4;
}

template<class T>
void BasicArray<T>::Smoothen(void) {
   int x,y;
   BasicArray<T> src(*this);

   for (y=1; y<sizeY-1; y++)   {
	  for (x=1; x<sizeX-1; x++)   {
         T pSmooth;
         
         pSmooth  =       src.Get(x-1,y-1) + 2.0*src.Get(x  ,y-1) + src.Get(x+1,y-1);
		 pSmooth += 2.0*(src.Get(x-1,y ) + 2.0*src.Get(x  ,y ) + src.Get(x+1,y ));
         pSmooth +=       src.Get(x-1,y+1) + 2.0*src.Get(x  ,y+1) + src.Get(x+1,y+1);
         pSmooth*=0.0625;
         Set(x,y,pSmooth);
      }
   }
}

template<class T>
BasicArray<T> *BasicArray<T>::Shrink(int sizeXNew, int sizeYNew) {
   if(sizeXNew<=0)sizeXNew = 1;
   if(sizeYNew<=0)sizeYNew = 1;
   
   float scaleX = float(sizeXNew)/float(sizeX);
   float scaleY = float(sizeYNew)/float(sizeY);
   T *line;
   int y,xBig,yBig;
   float floorY,ff;
   BasicArray<T> *dst;
   
   dst = new BasicArray<T>(sizeXNew,sizeYNew);
   line = new T[sizeXNew];

   yBig = 0;
   floorY = 0.0;
   for(y=0;y<sizeY-1;y++) {
	  ShrinkLine(y, scaleX, sizeXNew, line);
      ff = floorY + scaleY - 1.0;
      if(ff>0) {  // stepping into new bigPixel reached
         for(xBig=0;xBig<sizeXNew;xBig++) {
			dst->Add(xBig, yBig  , (scaleY - ff)*line[xBig]);
			dst->Add(xBig, yBig+1, ff*line[xBig]);
         }
         floorY-=1.0;
         ++yBig;
      }
      else
         for(xBig=0;xBig<sizeXNew;xBig++)
			 dst->Add(xBig, yBig  , scaleY*line[xBig]);
      floorY += scaleY;
   }
   ShrinkLine(sizeY-1, scaleX, sizeXNew, line);
   for(xBig=0;xBig<sizeXNew;xBig++)
	   dst->Add(xBig, sizeYNew-1, scaleY*line[xBig]);

   delete[] line;
   return dst;
}

template<class T>
void BasicArray<T>::ShrinkLine(int y, float scaleF, int sizeXNew, T *line) {
   int x,xBig;
   float floorX = 0.0;  // left border of current smallPixel

   for(xBig=0;xBig<sizeXNew;xBig++)
      line[xBig] = T(0.0);
   
   xBig = 0;
   floorX = 0.0;
   for(x=0;x<sizeX-1;x++) {
      float ff = floorX + scaleF - 1.0;
      if(ff>0) {  // stepping into new bigPixel reached
		 line[xBig] += (scaleF - ff)*Get(x,y);
         ++xBig;
         floorX-=1.0;
         line[xBig] += ff*Get(x,y);
      }
      else
         line[xBig] += scaleF*Get(x,y);
      floorX += scaleF;
   }
   line[sizeXNew-1] += scaleF*Get(sizeX-1,y);
}

template<class T>
BasicArray<T> *BasicArray<T>::BlockyPixEnlarge(int sizeXNew, int sizeYNew) {
   if(sizeXNew<=0)sizeXNew = 1;
   if(sizeYNew<=0)sizeYNew = 1;
   
   float scaleX = float(sizeX)/float(sizeXNew);
   float scaleY = float(sizeY)/float(sizeYNew);
   cout<<" < "<< scaleX<<" "<<scaleY<<" > \n";
   int x,y,xSrc,ySrc;
   float floorX,floorY,ffx,ffy;
   BasicArray<T> *dst;
   
   dst = new BasicArray<T>(sizeXNew,sizeYNew);

   ySrc = 0;
   floorY = 0.0;
   for(y=0;y<sizeYNew-1;y++) {
      ffy = floorY + scaleY - 1.0;
      floorX = 0.0;
      xSrc = 0;
      for(x=0;x<sizeXNew-1;x++) {
         ffx = floorX + scaleX - 1.0;
         
         T val,val1;

		 if(ffx > 0.0) {
            val = (1.0 - ffx) * Get(xSrc,ySrc) + ffx*Get(xSrc+1,ySrc);
			if(ffy > 0.0) {
               val1 = (1.0 - ffx) * Get(xSrc,ySrc+1) + ffx*Get(xSrc+1,ySrc+1);
               val += ffy*(val1-val);
            }
         }
         else {
            val = Get(xSrc,ySrc);
			if(ffy > 0.0)
			   val += ffy*(Get(xSrc,ySrc+1) - val);
         }
      
         dst->Set(x,y,val);
         floorX += scaleX;
		 if(floorX>= 1.0) {
            floorX-=1.0;
            xSrc++;
         }
      }
      floorY += scaleY;
	  if(floorY>= 1.0) {
         floorY-=1.0;
         ySrc++;
      }
   }

   return dst;
}

template<class T>
void BasicArray<T>::Clamp01(void) {
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();
   for (y=0; y<sizeY; y++)   {
	  for (x=0; x<sizeX; x++)   {
         T p = Get(x,y);
         p.Clip();
         Set(x,y,p);
      }
   }
}


template<class T>
void BasicArray<T>::HiSharpen(float f) {
   BasicArray<T> src(*this);
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();

   for(y=1;y<sizeY-1;y++) {
      for(x=1;x<sizeX-1;x++) {
         float dd=0.0,ff;
         T   l,c;
		 c = src.Get(x  , y);
		 l = src.Get(x  , y-1) - c; dd+=l.Norm1();
		 l = src.Get(x-1, y  ) - c; dd+=l.Norm1();
		 l = src.Get(x+1, y  ) - c; dd+=l.Norm1();
		 l = src.Get(x  , y+1) - c; dd+=l.Norm1();
         dd*=2.0;
		 l = src.Get(x-1, y-1) - c; dd+=l.Norm1();
		 l = src.Get(x+1, y-1) - c; dd+=l.Norm1();
		 l = src.Get(x-1, y+1) - c; dd+=l.Norm1();
		 l = src.Get(x+1, y+1) - c; dd+=l.Norm1();
         dd*=(1.0/12.0);

         dd = 10.0*f*dd;
         if(dd>1.0)dd=1.0;
         dd = pow_f(dd,2.0);
         //dd = dd*dd*(3.0 - 2.0*dd);

         ff = c.Norm1();
         ff = 1.0/(10.0*ff+ 0.001) - 1.0/3.0001;
		 Mul(x, y, 1.0 + 0.2*dd*ff);
      }
   }
}

template<class T>
void BasicArray<T>::ReduceNoise(float reduceF) {
   BasicArray<T> *hiF,*loF;
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();

   if(reduceF<=0.0)
      return;
   reduceF =1.0/reduceF;

   hiF = new BasicArray<T>(*this);
   loF = hiF->SplitLowFreq(1);

   for(y=0;y<sizeY;y++) {
      for(x=0;x<sizeX;x++) {
         T p;
         float w,dd;

         p = hiF->Get(x,y);
         dd = p.Norm1() * 5.0*reduceF;
         if(dd<1.0) {
            w = dd;
            w*=(2.0 - w);
            w*=(2.0 - w);
            w*=(2.0 - w);

            // create smooth ending
            if(w<0.2) {
               w = w*5.0;
               w = 0.1*w*w + 0.1;
            }
            w =  1.1*(w - 1.0) + 1.0;

            p *= (1.0 - w);
			Sub(x, y, p);
         }
      }
   }
   delete hiF;
   delete loF;
}

#endif
