/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    Array.cpp: 2-dim. float-array and direction-array

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

#include <string>
#include <iostream>
#include "Array.h"
#include "ArraysTemplateDefs.h"

template class BasicArray<PFloat>;   // explicit instantiation
template class BasicArray<Point2>;   // explicit instantiation

DirArray *MyArray::Gradient(void) {
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();

   float p00,p10,p20;
   float p01,p11,p21;
   float p02,p12,p22;
   DirArray *gradArray = new DirArray(sizeX,sizeY);
   float dx,dy;
   
   for(y=1;y<sizeY-1;y++) {
      for(x=1;x<sizeX-1;x++) {
         p00 = Get(x-1,y-1).toF(); p10 = Get(x  ,y-1).toF(); p20 = Get(x+1,y-1).toF();
         p01 = Get(x-1,y  ).toF(); p11 = Get(x  ,y  ).toF(); p21 = Get(x+1,y  ).toF();
         p02 = Get(x-1,y+1).toF(); p12 = Get(x  ,y+1).toF(); p22 = Get(x+1,y+1).toF();
         dx = p20 - p10 + 2.0*(p21 - p11) + p22 - p12;
         dy = p02 - p01 + 2.0*(p12 - p11) + p22 - p21;
         dx *= 0.25;
         dy *= 0.25;
         gradArray->Set(x,y,Point2(dx,dy));
      }
   }

   return gradArray;
}

void MyArray::FillWithHill(void) {
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();
   float scaleF = 2.0/float(sizeX);
   
   for (y=0; y<sizeY; y++ )   {
      float yf = float(y - (sizeY>>1) )*scaleF;
      for (x=0; x<sizeX; x++ )   {
         float xf = float(x - (sizeX>>1) )*scaleF;
         float w = 2.0*(xf*xf + yf*yf);
         if(w>1.0) w=2.0 - w;
         if(w<0.0)w=0.0;
         Set(x,y, PFloat(w) );
      }
   }
}

void MyArray::FillWithDots(void) {
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();
   float scaleF = 2.0/float(sizeX);
   
   for (y=0; y<sizeY; y++ )   {
      for (x=0; x<sizeX; x++ )   {
         if( (x&3) || (y&3) )
            Set(x,y, PFloat(0.0) );
         else
            Set(x,y, PFloat(1.0) );
      }
   }
}

void MyArray::OperateDir(DirArray *dirArr, RandGen & rGen) {
   int x,y,a;
   int sizeX = SizeX(), sizeY = SizeY();
   const float kTab[5] = { 1.0, 4.0, 6.0, 4.0, 1.0 };   
   float ker[25];
   float w,dw;
   Point2 dir;
   MyArray *newArr = new MyArray(sizeX,sizeY);   
    
   for (y=2; y<sizeY-2; y++ )   {
      for (x=2; x<sizeX-2; x++ )   {
         int kx,ky;
         float totalWeight = 0.0,normF,val;
         a=0;
         dir = dirArr->Get( x , y );
         float phi = dir.Angle()*0.5 * (2.0*PI/360.0);
         float cc =cos(phi), ss =sin(phi);
         for(ky=0;ky<5;ky++) {
            for(kx=0;kx<5;kx++) {
               w = kTab[kx] * kTab[ky] *(1.0/256.0);
               dw = float(kx-2)*cc + float(ky-2)*ss;
               dw = 1.0*dw*dw;
               dw = 1.0 - dw;
               if(dw<0.0)dw = 0.0;
               w *= dw;
               ker[a++] = w;
               totalWeight += w;
            }
         }
         if(totalWeight>0.0) 
            normF = 1.0/totalWeight;
         else
            normF = 0.0;
         for(a=0;a<5*5;a++)
            ker[a]*=normF;
         
         val = 0.0;
         for(ky=0;ky<5;ky++) {
            for(kx=0;kx<5;kx++) {
               val += ker[ky*5 + kx] * GetF( x+kx-2, y+ky-2 );
            }
         }
         float centerV = GetF(x,y);
         val = centerV - 0.5*(val - centerV);
         val += 0.0001*(rGen.RandF() - 0.5);
         if(val<0.0)val=0.0;
         else if(val>1.0)val=1.0;
         
         newArr->Set(x,y, PFloat(val) );
      }
   }

   for (y=2; y<sizeY-2; y++ )   {
      for (x=2; x<sizeX-2; x++ )   {
         Set(x,y,newArr->Get(x,y));
      }
   }
   
   delete newArr;
}


void DirArray::SmoothenWithDir(void) {
   int x,y,ax,ay,a;
   int sizeX = SizeX(), sizeY = SizeY();
   

   DirArray dir(*this);

   for (y=1; y<sizeY-1; y++ )   {
      for (x=1; x<sizeX-1; x++ )   {
         float w[9],ww,wTotal;
         
         a=0;
         wTotal = 0.0;
         Point2 dM;// = dir.Get(x,y);
         for(ay=0;ay<3;ay++) {
            for(ax=0;ax<3;ax++) {
               dM = Point2(float(ax-1),float(ay-1));
               dM = dM.DoubleDir();
               ww = 1.0 + 20.0*dM*dir.Get(x-1+ax,y-1+ay);
               if(ww<0.000001)
                  ww=0.000001;
               if(ax==1)ww*=2.0;
               if(ay==1)ww*=2.0;
               w[a] = ww;
               wTotal += ww;
               a++;
            }
         }
         ww = 1.0/wTotal;
         for(a=0;a<9;a++)
            w[a]*=ww;
         
         a=0;
         Point2 pSmooth(0.0);
         for(ay=0;ay<3;ay++) {
            for(ax=0;ax<3;ax++) {
               pSmooth += w[a]*dir.Get(x-1+ax,y-1+ay);
               a++;
            }
         }
         Set(x,y,pSmooth);
      }
   }
}

// ---------------------------------------------------------------
// ---------------------------------------------------------------

void DirArray::GradToDir(void) {
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();

   for(y=0;y<sizeY;y++) {
      for(x=0;x<sizeX;x++) {
         float hx;
         Point2 d;
         d = Get(x,y);
         hx = d.x; d.x = d.y; d.y = -hx;
         Set(x,y,d.DoubleDir());
      }
   }
}

DirArray *DirArray::Func(void) {
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();

   DirArray *newArr = new DirArray(sizeX,sizeY);
   for(y=1;y<sizeY-1;y++) {
      for(x=1;x<sizeX-1;x++) {
         Point2 d;
         Point2 d00,d10,d20;
         Point2 d01,d11,d21;
         Point2 d02,d12,d22;
         Point2 n00,n10,n20;
         Point2 n01,n11,n21;
         Point2 n02,n12,n22;

         d00 = Get(x-1,y-1); d10 = Get(x  ,y-1); d20 = Get(x+1,y-1);
         d01 = Get(x-1,y  ); d11 = Get(x  ,y  ); d21 = Get(x+1,y  );
         d02 = Get(x-1,y+1); d12 = Get(x  ,y+1); d22 = Get(x+1,y+1);
         n00 = d00.Normalized(); n10 = d10.Normalized(); n20 = d20.Normalized();
         n01 = d01.Normalized(); n11 = d11.Normalized(); n21 = d21.Normalized();
         n02 = d02.Normalized(); n12 = d12.Normalized(); n22 = d22.Normalized();

         d  =      NFunc(n00,n11)*d00 + 2.0*NFunc(n10,n11)*d10 + NFunc(n20,n11)*d20;
         d += 2.0*(NFunc(n01,n11)*d01          +                 NFunc(n21,n11)*d21 );
         d +=      NFunc(n02,n11)*d02 + 2.0*NFunc(n12,n11)*d12 + NFunc(n22,n11)*d22;
         d*=0.05;
         newArr->Set(x,y,d11 + d);
      }
   }
   return newArr;
}

DirArray *DirArray::Func0(void) {
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();

   DirArray *newArr = new DirArray(sizeX,sizeY);
   for(y=1;y<sizeY-1;y++) {
      for(x=1;x<sizeX-1;x++) {
         Point2 d;
         Point2 d00,d10,d20;
         Point2 d01,d11,d21;
         Point2 d02,d12,d22;
         Point2 n00,n10,n20;
         Point2 n01,n11,n21;
         Point2 n02,n12,n22;
         float  f00,f10,f20;
         float  f01,f11,f21;
         float  f02,f12,f22;
         d00 = Get(x-1,y-1); d10 = Get(x  ,y-1); d20 = Get(x+1,y-1);
         d01 = Get(x-1,y  ); d11 = Get(x  ,y  ); d21 = Get(x+1,y  );
         d02 = Get(x-1,y+1); d12 = Get(x  ,y+1); d22 = Get(x+1,y+1);
         n00 = d00.Normalized(); n10 = d10.Normalized(); n20 = d20.Normalized();
         n01 = d01.Normalized(); n11 = d11.Normalized(); n21 = d21.Normalized();
         n02 = d02.Normalized(); n12 = d12.Normalized(); n22 = d22.Normalized();
         f00 = NFunc0(n00,n11); f10 = NFunc0(n10,n11); f20 = NFunc0(n20,n11);
         f01 = NFunc0(n01,n11); f11 = NFunc0(n11,n11); f21 = NFunc0(n21,n11);
         f02 = NFunc0(n02,n11); f12 = NFunc0(n12,n11); f22 = NFunc0(n22,n11);
         d  =      f00*d00 + 2.0*f10*d10 + f20*d20;
         d += 2.0*(f01*d01       +         f21*d21 );
         d +=      f02*d02 + 2.0*f12*d12 + f22*d22;
         newArr->Set(x,y,d11 + 0.02*d);
      }
   }
   return newArr;
}

void DirArray::Normalize(void) {
   int x,y;
   int sizeX = SizeX(), sizeY = SizeY();

   for(y=0;y<sizeY;y++) {
      for(x=0;x<sizeX;x++) {
         Point2 d;
         d  = Get(x,y);
         d.Normalize();
         Set(x,y,d);
      }
   }
}

// ---------------------------------------------------------------
// ---------------------------------------------------------------
//-------------------------------------------------------------

