/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    FractTab.cpp: an array for fractal deformation of the enlarge-kernels

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

#include "FractTab.h"
#include "ArraysTemplateDefs.h"
#include <QImage>

FractTab::FractTab( float sF ) : scaleF( sF ), invScaleF( 1.0/sF ) {
   randTab = new int[RandTabLen];
   CreateRandTab();
   CreateTab();
}

FractTab::~FractTab( void ) {
   delete[] randTab;
}

void FractTab::CreateTab( void ) {
   const long startExp = 2;
   BasicArray<PFloat> *a0;

   long aLen = (1<<startExp);
   float rFakt;
   float fRatio = scaleF / float( 1<<(FRACTTABEXP-startExp) );

   a0 = new BasicArray<PFloat> ( aLen, aLen );
   for( int s=0; s<FRACTTABEXP-startExp; s++ ) {
      BasicArray<PFloat> *hh;
      hh = a0->SmoothDoubleTorus();
      delete a0;
      a0 = hh;
      fRatio *= 2.0;
      rFakt =  2.0*fRatio;
      if( rFakt>1.0 )
         rFakt = 1.0;
      else
         rFakt = rFakt*rFakt*( 3.0 - 2.0*rFakt ); // S-Func()
      rFakt *= ldexp( 1.5, FRACTTABEXP - s - startExp - 8 );
      AddRand( a0, rFakt );
   }

   fTab = *a0;
   fTab.MulArray( 1000.0*invScaleF );
   delete a0;
}

void FractTab::AddRand( BasicArray<PFloat> *a0, float rFakt) {
   for ( int y = 0; y < a0->SizeY(); y++ ) {
      for ( int x = 0; x < a0->SizeX(); x++ ) {
         a0->Add( x, y, ( randG.RandF() - 0.5 )*rFakt );
      }
   }
}

void FractTab::SaveTab( void ) {
   int x,y;
   int sizeX = fTab.SizeX(), sizeY = fTab.SizeY();
   QImage image( sizeX, sizeY, QImage::Format_ARGB32 );

   for (y=0; y<sizeY; y++ )   {
      for (x=0; x<sizeX; x++ )   {
         float vv,rr,gg,bb;
         vv = 0.1*fTab.Get(x,y).toF();
         if( vv<0 )
            vv=-vv;
         rr = vv * 8.0;
         if(rr>1.0) rr=1.0;
         rr = 255.0*rr*(2.0-rr);
         gg = vv * 2.0;
         if(gg>1.0) gg=1.0;
         gg = 255.0*gg*gg*(3.0 - 2.0*gg);
         bb = vv;
         if(bb>1.0) bb=1.0;
         bb *= 255.0*bb;

         QRgb c = qRgba( int(rr), int(gg),  int(bb), 255 );
         image.setPixel( x,y,c );
      }
   }
   image.save("/Users/mischa/Desktop/tsttab.png");
}

void FractTab::CreateRandTab( void ) {
   for( int a=0; a<RandTabLen; a++ ) {
      randTab[a] = a;
   }
   for( int c=0; c< 4*RandTabLen; c++ ) {
      long rr = 3615232;
      long s1,s2,hh;
      rr += (randG.RandL()>>5);
      s1 = rr&RandTabMask;
      rr += (randG.RandL()>>5);
      s2 = rr&RandTabMask;

      hh = randTab[s1];
      randTab[s1] = randTab[s2];
      randTab[s2] = hh;
   }
}
