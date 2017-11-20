/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    EnlargerTemplateDefs.h: the enlarging algorithm

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

#ifndef ENLARGER_TEMPLATE_DEFS_H
#define ENLARGER_TEMPLATE_DEFS_H

#include <iostream>
#include <math.h>
#include <stdio.h>
#include "EnlargerTemplate.h"
#include "Array.h"
#include "ConstDefs.h"
#include "FractTab.h"

using namespace std;

// the different formats used:
//
// - the complete source image
//      sizeX, sizeY, format.srcWidth, format.srcHeight
//
// - the complete source enlarged by scaleX,scaleY
//      sizeXDst, sizeYDst, format.DstWidth(), format.DstHeight()
//
// - the clipping rectangle in the format structure
//      the clipping rect in the format structure uses the < DstWidth(), DstHeight() >
//      coordinate system
//      but it allows values outside [ 0, DstWidth()-1 ] x [ 0, DstHeight()-1 ]
// - the format.cliprect is changed in the BasicEnlarger structure to a new clipping rectangle
//      within the allowed borders of the full dstRect
//      additionally an offset is calculated to place the cliprect within the output
// - the output image
//      outputWidth, outputHeight
//      offsetX, offsetY are the coordinates of the changed cliprect within the output
//      might lead to black borders outside the cliprect

template<class T>
BasicEnlarger<T>::BasicEnlarger( const EnlargeFormat & format, const EnlargeParameter & param ) {
   // int srcSizeX, int srcSizeY, float scaleF) {
   int a;
   sizeX = format.srcWidth;
   sizeY = format.srcHeight;

   // for pixel-accuracy use slightly different scaleF in x,y
   sizeXDst = format.DstWidth();
   sizeYDst = format.DstHeight();
   scaleFaktX = float(sizeXDst)/float(sizeX);
   scaleFaktY = float(sizeYDst)/float(sizeY);
   //scaleFaktX = format.scaleX;    //float(sizeXDst)/float(sizeX);
   //scaleFaktY = format.scaleY;    //float(sizeYDst)/float(sizeY);
   invScaleFaktX = 1.0/scaleFaktX;
   invScaleFaktY = 1.0/scaleFaktY;

   // the 'clipping rectangle' might reach outside the dst-boundaries
   // this leads to black margins in the output
   // for calculation, a new clipping rect inside the boundaries is calculated
   // additionally, outputWidth, outputHeight give the dimensions of the result,
   // offsetX, offsetY give the topleft edge of the new cliprect within the output

   outputWidth  = format.ClipW();
   outputHeight = format.ClipH();
   CalculateClipAndOffset( format );
   onlyShrinking = ( scaleFaktX < 1.0  && scaleFaktY < 1.0 );

   randGen = new RandGen(635017,934021);
   smallToBigTabX  = new int[sizeXDst + 2*smallToBigMargin];
   smallToBigTabY  = new int[sizeYDst + 2*smallToBigMargin];
   for( a=0; a<sizeXDst+2*smallToBigMargin; a++ ) {
       smallToBigTabX[a] = int( float(a-smallToBigMargin)*invScaleFaktX );
   }
   for( a=0; a<sizeYDst+2*smallToBigMargin; a++ ) {
       smallToBigTabY[a] = int( float(a-smallToBigMargin)*invScaleFaktY );
   }

   if( OnlyShrinking() )
      return;

   sizeDstBlock = blockLen;
   sizeSrcBlockX = int ( invScaleFaktX * float(sizeDstBlock) + 0.5) + 2*srcBlockMargin;
   sizeSrcBlockY = int ( invScaleFaktY * float(sizeDstBlock) + 0.5) + 2*srcBlockMargin;
   srcBlock = new BasicArray<T>( sizeSrcBlockX, sizeSrcBlockY );
   dstBlock = new BasicArray<T>( sizeDstBlock, sizeDstBlock );

   baseWeights   = new MyArray( sizeSrcBlockX, sizeSrcBlockY );
   workMask      = new MyArray( sizeSrcBlockX, sizeSrcBlockY );
   baseParams    = new MyArray( sizeSrcBlockX, sizeSrcBlockY );
   baseIntensity = new MyArray( sizeSrcBlockX, sizeSrcBlockY );
   workMaskDst   = new MyArray( sizeDstBlock,  sizeDstBlock  );

   dX  = new BasicArray<T>( sizeSrcBlockX, sizeSrcBlockY );
   dY  = new BasicArray<T>( sizeSrcBlockX, sizeSrcBlockY );
   d2X = new BasicArray<T>( sizeSrcBlockX, sizeSrcBlockY );
   d2Y = new BasicArray<T>( sizeSrcBlockX, sizeSrcBlockY );
   dXY = new BasicArray<T>( sizeSrcBlockX, sizeSrcBlockY );
   d2L = new BasicArray<T>( sizeSrcBlockX, sizeSrcBlockY );

   selectDiffTab = new float[diffTabLen];
   centerWeightTab = new float[diffTabLen];
   invTab = new float[invTabLen];
   invTab[0] = 1000000.0;
   for(a=1;a<invTabLen;a++)
      invTab[a] = float(invTabLen-1)/float(a);

   // use different kernels in x,y-dir, because diff. scaleF in x,y
   // for sake of pixel-accuracy allow slight difference in aspect ratio
   // and thus change scaleF into slightly diff. scaleFX,scaleFY

   enlargeKernelX = new float*[sizeXDst];
   enlargeKernelY = new float*[sizeYDst];
   selectKernelX  = new float*[sizeXDst];
   selectKernelY  = new float*[sizeYDst];
   for( a=0; a<sizeXDst; a++ ) {
      enlargeKernelX[a] = new float[ 5 ];
      selectKernelX[a]  = new float[ 5 ];
   }
   for( a=0; a<sizeYDst; a++ ) {
      enlargeKernelY[a] = new float[ 5 ];
      selectKernelY[a]  = new float[ 5 ];
   }

   derivF = 0.0;
   sharpExp = 0.01;
   centerWeightF =  3.0;
   centerWExp = 3.5;
   selectPeakExp = 3.0;
   derivDiffF = 0.7;

   lineNegF = 1.0;
   linePosF = 0.15;
   ditherF = 0.1;
   dirDiffF = 1.0;
   deNoiseF = 0.0;
   preSharpenF = 0.0;
   fractNoiseF = 0.0;

   SetParameter( param );
   CreateKernels();
   CreateDiffTabs();

   fractTab = 0;   // fractTab has to be imported with SetFractTab
}

// format.clip allows exceeding bounds ( for black margins )
// this is converted to new cliprect within bounds and additional offset
template<class T>
void BasicEnlarger<T>::CalculateClipAndOffset( const EnlargeFormat & format ) {
   clipX0 = format.clipX0;
   clipY0 = format.clipY0;
   clipX1 = format.clipX1;
   clipY1 = format.clipY1;
   offsetX = offsetY = 0;
   if( clipX0 < 0 ) {
      offsetX = -clipX0;
      clipX0 = 0;
   }
   if( clipY0 < 0 ) {
      offsetY = -clipY0;
      clipY0 = 0;
   }
   if( clipX1 > sizeXDst ) {
      clipX1 = sizeXDst;
   }
   if( clipY1 > sizeYDst ) {
      clipY1 = sizeYDst;
   }
}

template<class T>
BasicEnlarger<T>::~BasicEnlarger(void) {
   int a;

   delete randGen;
   delete[] smallToBigTabX;
   delete[] smallToBigTabY;

   if( OnlyShrinking() )
      return;

   for( a=0; a<sizeXDst; a++) {
      delete[] enlargeKernelX[a];
      delete[] selectKernelX[a];
   }
   for( a=0; a<sizeYDst; a++) {
      delete[] enlargeKernelY[a];
      delete[] selectKernelY[a];
   }
   delete[] enlargeKernelX;
   delete[] enlargeKernelY;
   delete[] selectKernelX;
   delete[] selectKernelY;


   delete [] selectDiffTab;
   delete [] centerWeightTab;
   delete [] invTab;

   delete dX;
   delete dY;
   delete d2X;
   delete d2Y;
   delete dXY;
   delete d2L;

   delete baseIntensity;
   delete baseParams;
   delete baseWeights;
   delete workMask;
   delete workMaskDst;
   delete srcBlock;
   delete dstBlock;
}

// example for enlarging the clip-rect
template<class T>
void BasicEnlarger<T>::Enlarge( void ) {
   int dstX,dstY;

   if( OnlyShrinking() ) {
      ShrinkClip();
      return;
   }

   for( dstY = ClipY0(); dstY < ClipY1(); dstY+=blockLen) {
      for( dstX = ClipX0(); dstX < ClipX1(); dstX+=blockLen) {
         BlockBegin( dstX, dstY);
         ReadSrcBlock();
         SrcBlockReduceNoise();
         SrcBlockSharpen();
         EnlargeBlock();
         WriteDstBlock();
      }
   }

}

template<class T>
void BasicEnlarger<T>::SetParameter(float sharpness, float flatness) {
   const int listLen = 7;

   const float  sE_list0[listLen]      = { 16.0,  6.0,  4.0,  2.0,  1.0,  0.5,  0.1 };
   const float cWF_list0[listLen]      = {  8.0,  7.5,  7.0,  5.0,  3.0,  2.0,  1.0  };
   const float cWE_list0[listLen]      = {  6.0,  5.5,  5.0,  4.5,  4.0,  3.0,  2.0  };
   const float sPE_list0[listLen]      = {  4.0,  4.0,  3.5,  2.5,  1.5,  1.2,  1.0  };

   const float  sE_list1[listLen]      = { 12.0,  5.0,  2.5,  1.0,  0.5,  0.1,  0.01 };
   const float cWF_list1[listLen]      = {  8.0,  7.0,  5.0,  3.0,  2.0,  1.0,  1.0  };
   const float cWE_list1[listLen]      = {  6.0,  5.0,  4.5,  4.0,  3.0,  2.0,  2.0  };
   const float sPE_list1[listLen]      = {  4.0,  3.5,  2.5,  1.5,  1.2,  1.0,  1.0  };

   if( OnlyShrinking() )
      return;

   if(sharpness<0.0)sharpness = 0.0;
   else if(sharpness>1.0) sharpness=1.0;

   if(flatness<0.0)flatness = 0.0;
   else if(flatness>1.0) flatness=1.0;

   float t = (1.0 - sharpness)*float(listLen-1);
   int idx = int(t);
   t -= float(idx);

   float hh;

   derivF = 1.0 - flatness;
   hh            =   sE_list0[idx]*(1.0 - t) +  sE_list0[idx+1]*t;
   sharpExp      =   sE_list1[idx]*(1.0 - t) +  sE_list1[idx+1]*t;
   sharpExp      =   sharpExp*flatness       +  hh*(1.0 - flatness) ;

   hh            =  cWF_list0[idx]*(1.0 - t) +  cWF_list0[idx+1]*t;
   centerWeightF =  cWF_list1[idx]*(1.0 - t) +  cWF_list1[idx+1]*t;
   centerWeightF =  centerWeightF*flatness   +  hh*(1.0 - flatness) ;

   hh            =  cWE_list0[idx]*(1.0 - t) + cWE_list0[idx+1]*t;
   centerWExp    =  cWE_list1[idx]*(1.0 - t) + cWE_list1[idx+1]*t;
   centerWExp    =  centerWExp*flatness     +  hh*(1.0 - flatness) ;

   hh            =  sPE_list0[idx]*(1.0 - t) + sPE_list0[idx+1]*t;
   selectPeakExp =  sPE_list1[idx]*(1.0 - t) + sPE_list1[idx+1]*t;
   selectPeakExp =  selectPeakExp*flatness   +  hh*(1.0 - flatness) ;

   CreateDiffTabs();
}

template<class T>
void BasicEnlarger<T>::SetParameter( const EnlargeParameter & p) {
    SetParameter( p.sharp, p.flat );
    SetDeNoise ( p.deNoise );
    SetPreSharpen ( p.preSharp );
    SetDither( p.dither );
    SetFractNoise( p.fractNoise );
}

template<class T>
void BasicEnlarger<T>::EnlargeBlock(void) {
   if( OnlyShrinking() )
      return;

   CalcBaseWeights();
   BlockEnlargeSmooth();
   MaskBlockEnlargeSmooth();
   EnlargeBlockPart( dstMinBY, dstMaxBY );
   AddRandom ();
   dstBlock->Clamp01();
}

template<class T>
void BasicEnlarger<T>::EnlargeBlockPart( int dstStartBY, int dstEndBY ) {
   int dstBX, dstBY, srcBX, srcBY, srcBXNew;
   float *kerX,*kerY;
   T      modColor[25];
   if( OnlyShrinking() )
      return;

   // for each pixel in dstBlock:
   // get neighbouring BigPixels and their simil & indie weights (ReadBigPixelNeighs)
   // get weighting kernels for sx and sy pos                    ( kerX, kerY )
   // get the prev.calc. smooth-enlarged color at (dstBX,dstBY)  ( smallColor )
   // weight all neigh. bigPixels with
   //        their indies & simils
   //        the kernel kerX*kerY
   //        the weight resulting from difference of
   //          bigPixelColor and smallColor                       ( SelectWeight )
   // sum up all weighted colors and return
   //    lincomb with smoothEnlarged smallColor

   // for each bigPixel calculate quadric from derivatives
   // for inc. of dstBX calc only increment of the Quadrics
   T      quadric[5*5], quadDelta[5*5], quadD2[5*5];
   int srcXm2, srcYm2;
   float fx,fy;
   float deltaX = 1.0*invScaleFaktX;
   int a,ax,ay;
   bool lastPixelWasCalculated;   // used for quadric-refreshing

   if( dstStartBY < dstMinBY )
       dstStartBY = dstMinBY;
   if( dstEndBY > dstMaxBY )
       dstEndBY = dstMaxBY;
   for(dstBY = dstStartBY; dstBY < dstEndBY ; dstBY++ ) {
      kerY = selectKernelY[dstBY + dstBlockEdgeY];

      srcBX = CurrentSrcBlockX( 0 );
      srcBY = CurrentSrcBlockY( dstBY );
      srcYm2  = srcBY - 2 + srcBlockEdgeY;
      fy = float( dstBY + dstBlockEdgeY )*invScaleFaktY  - float( srcYm2 ) - 0.25;

      ReadBigPixelNeighs( srcBX, srcBY );

      lastPixelWasCalculated = false;   // quadric: a fresh start for a new row
      for( dstBX = dstMinBX; dstBX<dstMaxBX ; dstBX++ ) {
         kerX = selectKernelX[dstBX + dstBlockEdgeX];
         srcBXNew = CurrentSrcBlockX( dstBX );
         srcXm2 = srcBXNew - 2 + srcBlockEdgeX;
         fx = float( dstBX + dstBlockEdgeX )*invScaleFaktX  - float( srcXm2 ) - 0.25;

         if( srcBXNew > srcBX ) { // do we step forward in the src-system?
            // in this case: refresh the src-neighbour-mat
            // the matrix of 5x5 quadric-datas has to be shifted,
            // the last column is calculated
            srcBX = srcBXNew;
            ReadBigPixelNeighs( srcBX, srcBY );

            // if the last pixel was not calculated, then all
            // quadrics are newly initialized further down, else:
            // shift the quadric-data by one to the left
            if( derivF>0.0 && lastPixelWasCalculated ) {
               a=0;
               for( ay=0; ay<5; ay++ ) {
                  for( ax=0; ax<4; ax++ ) {
                     quadric[a]   = quadric[a+1];
                     quadDelta[a] = quadDelta[a+1];
                     quadD2[a]    = quadD2[a+1];

                     // increment the quadric-data
                     quadric[a]   += quadDelta[a];
                     quadDelta[a] += quadD2[a];

                     a++;
                  }
                  // the last quadric in every row is new
                  float px = fx - float(ax);
                  float py = fy - float(ay);
                  QuadricCalc(px, py, a, deltaX, quadric[a], quadDelta[a], quadD2[a]);
                  a++;
               }
            }
         }
         else if( derivF>0.0 && lastPixelWasCalculated ) {
            // increment the quadric-data
            a=0;
            for( ay=0; ay<5; ay++ ) {
               for( ax=0; ax<5; ax++ ) {
                  quadric[a]   += quadDelta[a];
                  quadDelta[a] += quadD2[a];
                  a++;
               }
            }
         }

         // Modify One Small Pixel at (dstBX,dstBY) with smallColor
         T     smallColor = dstBlock->Get( dstBX , dstBY );

         // for diffCalc fract-modify smallColor
         T     smallColorFract = smallColor;

         if( fractTab != 0 && fractNoiseF > 0.0 ) {
            float wf = MyFractTab()->GetT( dstBX + DstBlockEdgeX(), dstBY + DstBlockEdgeY() );
            smallColorFract += ( fractNoiseF*0.01*wf )*smallColorFract;
            smallColorFract.Clip();
         }


         float wMat[5*5], totalWeight, w, normF;
         float wMask = 1.0;
         T     color,diff;

         wMask = workMaskDst->GetF( dstBX, dstBY ) - 0.01;
         if(wMask>0.0) {
            wMask *= 1.5;
            if(wMask>1.0)
               wMask=1.0;
            else {
               wMask = wMask*wMask*(3.0 - 2.0*wMask);
            }
            totalWeight=0.0;

            // if last pixel was not in the mask:
            // initialize the 5x5 quadrics
            if( derivF>0.0 && !lastPixelWasCalculated ) {
               a=0;
               for( ay=0; ay<5; ay++ ) {
                  for( ax=0; ax<5; ax++ ) {
                     float px = fx - float(ax);
                     float py = fy - float(ay);
                     QuadricCalc(px, py, a, deltaX, quadric[a], quadDelta[a], quadD2[a]);
                     a++;
                  }
               }
            }
            lastPixelWasCalculated = true;

            a=0;
            // weight the 5x5 source pixels
            for( int ay=0; ay<5; ay++ ) {
               for( int ax=0; ax<5; ax++ ) {
                  if(derivF > 0.0) {
                     modColor[a] = quadric[a]  + bigPixelColor[a];
                     w = derivDiffF*quadric[a].Norm1();
                  }
                  else {
                     modColor[a] = bigPixelColor[a];
                     w=0.0;
                  }


                  //diff = modColor[a] - smallColor;
                  diff = modColor[a] - smallColorFract;
                  w  = 10.0 * ( w + diff.Norm1() ) * bigPixelIntensity[a];
                  w = bigPixelWeight[a] * kerX[ax] * kerY[ay] * SelectWeight( w );

                  // give bigPixel add. Weigt near center
                  float px = fx - float(ax);
                  float py = fy - float(ay);
                  float ww = bigPixelCenterW[a];
                  ww = 1.0 + ( CenterWeight(px*px + py*py) - 1.0 )*ww;
                  w *=  ww;



                  // deform the weight via fractTab,
                  // for each srcPixel we have deform-kernel
                  // ( FractCX,FractCY,FractCVal : center pos & center val in fractTab )
                  // kernel was selected by randomizing the coord of the srcPixel
                  if( fractTab != 0 && fractNoiseF > 0.0 ) {
                     int dx = int(px*scaleFaktX) + bigPixelFractCX[a];
                     int dy = int(py*scaleFaktY) + bigPixelFractCY[a];
                     ww = 1.0 + 1.5*fractNoiseF*( fractTab->GetT( dx, dy ) - bigPixelFractCVal[a]) ;

                     if( ww<0.01 )
                        ww = 0.01 + ( 0.01 - ww );
                     if( ww>3.0 )
                        ww = 3.0 - ( ww - 3.0 );
                     if( ww<0.01 )
                        ww = 0.01 + ( 0.01 - ww );
                     w *= ww;
                  }
                  /*
                  // wavy special effect
                  float sdx = bigPixelDX[a].x;
                  float sdy = bigPixelDY[a].x;
                  float sd = sqrt( sdx*sdx + sdy*sdy );
                  if(sd>0) {
                     float sdd = 1.0/sd;
                     sdx*=sdd;
                     sdy*=sdd;
                     sd *= 10.0;
                     if( sd>0.5 )
                        sd=0.5;
                  }
                  ww = 1.0 + sd*cos( 10.0*(sdx*px + sdy*py) );
                  modColor[a] *= ww;
                  */

                  w += 0.000000001;
                  wMat[a] = w;
                  totalWeight += w;
                  a++;
               }
            }
            normF = 1.0/totalWeight;
            T    colorR;
            color.SetZero();
            colorR.SetZero();

            for( a=0; a<25; a++ )
                wMat[a] *= normF;

            for( a=0; a<25; a++ ) {
                color.AddMul(wMat[a],modColor[a]); //color += modColor[a]*wMat[a];
            }

            diff = color - smallColor;
            smallColor += diff*wMask;

         }
         dstBlock->Set( dstBX , dstBY, smallColor );
      }
   }
}

// calculate positions, clipping
template<class T>
void BasicEnlarger<T>::BlockBegin(int dstXEdge,int dstYEdge) {
   // smallPos of upper left edge of DstBlock
   dstBlockEdgeX = dstXEdge;
   dstBlockEdgeY = dstYEdge;
   // bigPos of upper left edge of   SrcBlock: add margin
   srcBlockEdgeX = BigSrcPosX( dstXEdge) - srcBlockMargin;
   srcBlockEdgeY = BigSrcPosY( dstYEdge) - srcBlockMargin;

   // calculate clipping
   dstMinBX=0; dstMaxBX=sizeDstBlock;
   dstMinBY=0; dstMaxBY=sizeDstBlock;
   if( dstBlockEdgeX < clipX0 )
      dstMinBX = clipX0 - dstBlockEdgeX;
   if( dstBlockEdgeY < clipY0 )
      dstMinBY = clipY0 - dstBlockEdgeY;
   if( dstBlockEdgeX + sizeDstBlock >= clipX1 )
      dstMaxBX = clipX1 - dstBlockEdgeX;
   if( dstBlockEdgeY + sizeDstBlock >= clipY1 )
      dstMaxBY = clipY1 - dstBlockEdgeY;
}

template<class T>
void BasicEnlarger<T>::ReadSrcBlock( void ) {
   // copy data, pos outside src is ok, filled with margin-data
   //srcBlock->CopyFromArray(src, SrcBlockEdgeX(), SrcBlockEdgeY() );
   int x,y,sx,sy;
   if( OnlyShrinking() )
      return;

   T  *dst;
   int srcSizeX = SizeSrcX();
   int srcSizeY = SizeSrcY();;
   int blockSizeX = SizeSrcBlockX();
   int blockSizeY = SizeSrcBlockY();
   int srcEdgeX = SrcBlockEdgeX();
   int srcEdgeY = SrcBlockEdgeY();
   dst = CurrentSrcBlock()->Buffer();

   y=0;sy=srcEdgeY;
   // while dst outside: copy pixels of src-line 0
   while(sy<=0 && y < blockSizeY ) {
      x=0;sx=srcEdgeX;
      // while dst outside: write src-edge-pixel
      while( sx<0 && x < blockSizeX ) {
         ReadSrcPixel( 0         , 0   , *dst );  dst++; x++; sx++;
      }
      // copy pixels of src-line 0
      while( sx < srcSizeX - 1 && x < blockSizeX ) {
         ReadSrcPixel( sx        , 0   , *dst );  dst++; x++; sx++;
      }
      // while dst outside: write src-edge-pixel
      while( x < blockSizeX ) {
         ReadSrcPixel( srcSizeX-1, 0   , *dst );  dst++; x++; sx++;
      }
      y++;sy++;
   }

   while(sy < srcSizeY - 1 && y < blockSizeY ) {
      x=0; sx=srcEdgeX;
      // while dst outside: write src-border-pixel
      while( sx<0 && x < blockSizeX ) {
         ReadSrcPixel( 0         , sy  , *dst );  dst++; x++; sx++;
      }
      // copy pixels normally
      while( sx < srcSizeX - 1 && x < blockSizeX ) {
         ReadSrcPixel( sx        , sy  , *dst );  dst++; x++; sx++;
      }
      // while dst outside: write src-border-pixel
      while( x < blockSizeX ) {
         ReadSrcPixel( srcSizeX-1, sy  , *dst );  dst++; x++; sx++;
      }
      y++;sy++;
   }

   // for outside-parts: copy pixels of last src-line
   while( y < blockSizeY ) {
      x=0;sx=srcEdgeX;
      // while dst outside: write src-edge-pixel
      while( sx<0 && x < blockSizeX ) {
         ReadSrcPixel( 0         , srcSizeY-1 , *dst );  dst++; x++; sx++;
      }
      // copy pixels of last src-line
      while( sx < srcSizeX - 1 && x < blockSizeX ) {
         ReadSrcPixel( sx        , srcSizeY-1 , *dst );  dst++; x++; sx++;
      }
      // while dst outside: write src-edge-pixel
      while( x < blockSizeX ) {
         ReadSrcPixel( srcSizeX-1, srcSizeY-1 , *dst );  dst++; x++; sx++;
      }
      y++;sy++;
   }
}

template<class T>
void BasicEnlarger<T>::WriteDstBlock( void ) {
   // offsetX, offsetY: new addition to allow black margins in output
   int dstBX, dstBY;
   if( OnlyShrinking() )
      return;

   for( dstBY = DstMinBY(); dstBY < DstMaxBY(); dstBY++ ) {
      for( dstBX = DstMinBX(); dstBX < DstMaxBX(); dstBX++ ) {
         int dstCX =  dstBX + DstBlockEdgeX() - ClipX0() + offsetX;
         int dstCY =  dstBY + DstBlockEdgeY() - ClipY0() + offsetY;
         WriteDstPixel( dstBlock->Get( dstBX, dstBY ), dstCX, dstCY );
      }
   }
}
template<class T>
void BasicEnlarger<T>::ReadSrcLine ( int srcY, T  *srcLine ) {
   for( int srcX=0; srcX<SizeSrcX(); srcX++ ) {
      ReadSrcPixel( srcX , srcY, srcLine[ srcX ] );
   }
}

template<class T>
void BasicEnlarger<T>::WriteDstLine( int dstY, T  *dstLine ) {
   // offsetX, offsetY: new addition to allow black margins in output
   for( int dstX=ClipX0(); dstX<ClipX1(); dstX++ ) {
      WriteDstPixel( dstLine[ dstX ], dstX-ClipX0()+offsetX, dstY-ClipY0()+offsetY );
   }
}

template<class T>
void BasicEnlarger<T>::BlockEnlargeSmooth(void) {
   int a, srcBY, srcBYNew, dstBX, dstBY;
   T  *line[5], *ll[5], *hl;
   if( OnlyShrinking() )
      return;

   for(a=0;a<5;a++)
      line[a] = ll[a] = new T [sizeDstBlock];
   srcBY = CurrentSrcBlockY(0);
   for(a=0;a<5;a++)
      BlockReadLineSmooth( srcBY+a-2, line[a] );
   for( dstBY=0;dstBY<sizeDstBlock;dstBY++) {
      int dstY;
      float *kTabY;
      dstY = dstBY + dstBlockEdgeY;
      if( dstY >= 0 && dstY < sizeYDst )
         kTabY = enlargeKernelY[ dstY ];
      else
         kTabY = enlargeKernelY[0];
      srcBYNew = CurrentSrcBlockY( dstBY );

      // bigPos changed? -> scroll
      if( srcBYNew > srcBY ) {
         srcBY = srcBYNew;
         hl=line[0]; line[0]=line[1]; line[1]=line[2];
         line[2]=line[3]; line[3]=line[4]; line[4]=hl;
         BlockReadLineSmooth( srcBY+2 , line[4]);
      }
      for( dstBX=0; dstBX < sizeDstBlock; dstBX++ ) {
         T      p;
         p  = line[0][dstBX]*kTabY[0];
         p += line[1][dstBX]*kTabY[1];
         p += line[2][dstBX]*kTabY[2];
         p += line[3][dstBX]*kTabY[3];
         p += line[4][dstBX]*kTabY[4];
         dstBlock->Set(dstBX,dstBY,p);
      }
   }

   for(a=0;a<5;a++)
      delete[] ll[a];

}

template<class T>
void BasicEnlarger<T>::BlockReadLineSmooth( int srcBY, T *line ) {
   int srcBX, dstBX, dstX;
   for(dstBX = 0;dstBX<sizeDstBlock;dstBX++) {
      float *kTabX;
      T  p;

      dstX = dstBX + dstBlockEdgeX;
      if( dstX >= 0 && dstX < sizeXDst)
         kTabX = enlargeKernelX[ dstX ];
      else
         kTabX = enlargeKernelX[0];

      srcBX = CurrentSrcBlockX( dstBX );
      p  = srcBlock->Get( srcBX - 2 , srcBY) * kTabX[0];
      p += srcBlock->Get( srcBX - 1 , srcBY) * kTabX[1];
      p += srcBlock->Get( srcBX     , srcBY) * kTabX[2];
      p += srcBlock->Get( srcBX + 1 , srcBY) * kTabX[3];
      p += srcBlock->Get( srcBX + 2 , srcBY) * kTabX[4];
      line[dstBX] = p;
   }
}

template<class T>
void BasicEnlarger<T>::AddRandom( void ) {
   int dstBX,dstBY;
   if( OnlyShrinking() )
      return;

   for(dstBY = dstMinBY; dstBY<dstMaxBY ; dstBY++ ) {
      for( dstBX = dstMinBX; dstBX<dstMaxBX ; dstBX++ ) {
         float w = (2.0 * randGen->RandF() - 1.0);
         w *= randGen->RandF();
         w *= 0.5*ditherF;
         w = 1.0 + w;

         dstBlock->Mul( dstBX, dstBY, w );
      }
   }
}

//----------------

template<class T>
void BasicEnlarger<T>::MaskBlockEnlargeSmooth(void) {
   int a, srcBY, srcBYNew, dstBX, dstBY;
   float *line[5],*ll[5],*hl;

   for(a=0;a<5;a++)
      line[a] = ll[a] = new float[sizeDstBlock];
   srcBY = CurrentSrcBlockY(0);
   for(a=0;a<5;a++)
      MaskBlockReadLineSmooth( srcBY+a-2, line[a] );
   for( dstBY=0;dstBY<sizeDstBlock;dstBY++) {
      int dstY;
      float *kTabY;
      dstY = dstBY + dstBlockEdgeY;
      if( dstY >= 0 && dstY < sizeYDst )
         kTabY = enlargeKernelY[ dstY ];
      else {
         kTabY = enlargeKernelY[0];
      }
      srcBYNew = CurrentSrcBlockY(dstBY);

      // bigPos changed? -> scroll
      if( srcBYNew > srcBY ) {
         srcBY = srcBYNew;
         hl=line[0]; line[0]=line[1]; line[1]=line[2];
         line[2]=line[3]; line[3]=line[4]; line[4]=hl;
         MaskBlockReadLineSmooth( srcBY+2 , line[4]);
      }
      for( dstBX=0; dstBX < sizeDstBlock; dstBX++ ) {
         float p;
         p  = line[0][dstBX]*kTabY[0];
         p += line[1][dstBX]*kTabY[1];
         p += line[2][dstBX]*kTabY[2];
         p += line[3][dstBX]*kTabY[3];
         p += line[4][dstBX]*kTabY[4];
         workMaskDst->Set( dstBX, dstBY, p );
      }
   }

   for(a=0;a<5;a++)
      delete[] ll[a];
}

template<class T>
void BasicEnlarger<T>::MaskBlockReadLineSmooth( int srcBY, float *line ) {
   int srcBX, dstBX, dstX;
   for(dstBX = 0;dstBX<sizeDstBlock;dstBX++) {
      float *kTabX;
      float p;

      dstX = dstBX + dstBlockEdgeX;
      if( dstX >= 0 && dstX < sizeXDst)
         kTabX = enlargeKernelX[ dstX ];
      else
         kTabX = enlargeKernelX[0];

      srcBX = CurrentSrcBlockX( dstBX );
      p  = workMask->GetF( srcBX - 2 , srcBY ) * kTabX[0];
      p += workMask->GetF( srcBX - 1 , srcBY ) * kTabX[1];
      p += workMask->GetF( srcBX     , srcBY ) * kTabX[2];
      p += workMask->GetF( srcBX + 1 , srcBY ) * kTabX[3];
      p += workMask->GetF( srcBX + 2 , srcBY ) * kTabX[4];
      line[dstBX] = p;
   }
}


template<class T>
void BasicEnlarger<T>::ShrinkClip( void ) {

   T  *srcLine, *addLine, *dstLine;
   int srcY, dstX, dstY;
   int srcY0, srcY1;
   float floorY,ff;

   srcLine = new T[ sizeX    + 2 ];
   addLine = new T[ sizeXDst + 2 ];
   dstLine = new T[ sizeXDst + 2 ];
   for( dstX=0; dstX < sizeXDst; dstX++ )
       dstLine[ dstX ].SetZero();

   if( clipY0 > 0 ) {
       srcY0  = int( float(clipY0-1)/scaleFaktY + 0.5);
       floorY = float( srcY0 )*scaleFaktY;
       dstY   = int( floorY );
       floorY-= float( dstY );
   }
   else {
       srcY0  = 0;
       dstY   = 0;
       floorY = 0.0;
   }
   srcY1  = int( float( clipY1+1)/scaleFaktY + 0.5 )+1;
   if( srcY1 > sizeY )
       srcY1 = sizeY;

   for( srcY = srcY0; srcY < srcY1; srcY++ ) {
      ReadSrcLine( srcY, srcLine );   // read srcLine,  shrink it in x-direction, resulting in addLine
      ShrinkLineClip( srcLine, addLine );
      ff = floorY + scaleFaktY - 1.0;
      if(ff>0) {  // stepping into new dstLine reached, share addLine between old and new dstLine
         for( dstX=clipX0; dstX<clipX1; dstX++ ) {
            dstLine[ dstX ] += ( scaleFaktY - ff )*addLine[ dstX ];
         }
         if( dstY >= clipY0 && dstY < clipY1 ) {
            WriteDstLine( dstY, dstLine );
         }
         floorY-=1.0;
         dstY++;
         for( dstX=clipX0; dstX<clipX1; dstX++ ) { // clear dstLine, fill with rest of addLine
            dstLine[ dstX ] = ff*addLine[ dstX ];
         }
      }
      else  // addLine fully added to current dstLine ( with appropriate weight )
         for( dstX=clipX0; dstX<clipX1; dstX++ )
             dstLine[ dstX ] += scaleFaktY*addLine[ dstX ];
      floorY += scaleFaktY;
   }
   if( dstY >= clipY0 && dstY < clipY1 ) {
      WriteDstLine( dstY, dstLine );
   }

   delete[] srcLine;
   delete[] addLine;
   delete[] dstLine;
}

template<class T>
void BasicEnlarger<T>::ShrinkLineClip( T *srcLine,  T *dstLine ) {
   int srcX, dstX;
   int srcX0, srcX1;
   float floorX,ff;

   floorX = 0.0;  // left border of current smallPixel

   for( dstX = clipX0; dstX<clipX1; dstX++)
      dstLine[ dstX ].SetZero();;

   if( clipX0>0 ) {
       srcX0  = int( float(clipX0-1)/scaleFaktX + 0.5 );
       floorX = float( srcX0 )*scaleFaktX;
       dstX = int(floorX);
       floorX -= float( dstX );
   }
   else {
       srcX0 = 0;
       dstX = 0;
       floorX = 0.0;
   }
   srcX1 = int( float( clipX1+1 )/scaleFaktX + 0.5 );
   if( srcX1 > sizeX )
       srcX1 = sizeX;

   for( srcX=srcX0; srcX<srcX1; srcX++ ) {
      ff = floorX + scaleFaktX - 1.0;
      if(ff>0) {  // stepping into new dstPixel reached: share srcPixel between old and new dstPixel
         dstLine[ dstX ] += ( scaleFaktX - ff )*srcLine[ srcX ];
         dstX++;
         floorX-=1.0;
         dstLine[ dstX ] = ff*srcLine[ srcX ];
      }
      else // fully add srcPixel to dstPixel ( weighted )
         dstLine[ dstX ] += scaleFaktX * srcLine[ srcX ];
      floorX += scaleFaktX;
   }
}

//-----------------------------------------------------------


template<class T>
void BasicEnlarger<T>::CreateKernelsFromTab(float **kerList, int kerListLen,
                                         float *kerTab, int kerTabLen, float scaleF)
{
   float invScaleF = 1.0/scaleF;
   float invFineScaleF = (1.0/scaleF) * (1.0/float(1<<kerFineExp)) ;
   int smallPos,bigPos,k,a,tabPos;
   // smallPos:    pos of small pixel in dst
   // bigPos:      pos of corresp. bigPixel in src
   // finePosKer:  fine Pos of fine-kernel-entry
   // bigPosKer:   pos of corresp. bigPixel in src
   for(smallPos=0;smallPos<kerListLen;smallPos++) {
      float *ker = kerList[smallPos];
      for( a=0; a<5; a++) {
         ker[a] = 0.0;
      }
      bigPos = int(float(smallPos)*invScaleF);
      for( k=0; k < kerTabLen; k++ ) {
         int finePosKer,bigPosKer;
         // get bigPixelPos of fine-kernel pixel
         // get tabIndex by comp. to bigPos of center of kernel
         finePosKer = float((smallPos<<kerFineExp) + k - (kerTabLen>>1));
         finePosKer += (1<<kerFineExp)>>1; // add half smallPixel
         bigPosKer = int( float(finePosKer)*invFineScaleF );
         tabPos = 2 + bigPosKer - bigPos;
         ker[tabPos] += kerTab[k];
      }

   }
}

template<class T>
void BasicEnlarger<T>::CreateKernelsFromIntegralTab(float **kerList, int kerListLen,
                                         float *kerITab, int kerTabLen, float scaleF, int *smallToBigTab)
{
   float invScaleF = 1.0/scaleF;
   float invFineScaleF = (1.0/scaleF) * (1.0/float(1<<kerFineExp)) ;
   int smallPos,kernelStartPosFine,bigPos,k,a,tabPos;
   // smallPos:    pos of small pixel in dst
   // bigPos:      pos of corresp. bigPixel in src
   // finePosKer:  fine Pos of fine-kernel-entry
   // bigPosKer:   pos of corresp. bigPixel in src

   for(smallPos=0;smallPos<kerListLen;smallPos++) {
      float *ker = kerList[smallPos];
      for( a=0; a<5; a++) {
         ker[a] = 0.0;
      }
      bigPos = smallToBigTab[ smallPos + smallToBigMargin ];
      kernelStartPosFine = (smallPos<<kerFineExp) + ((1<<kerFineExp)>>1) - (kerTabLen>>1);
      for( a=0;a<5;a++ ) {
         int bigPosKer,finePosKerLeft,finePosKerRight;
         bigPosKer = bigPos-2+a;
         finePosKerLeft  = int( float(  bigPosKer    <<kerFineExp)*scaleF ) - kernelStartPosFine;
         finePosKerRight = int( float(( bigPosKer+1 )<<kerFineExp)*scaleF ) - kernelStartPosFine;
         if(      finePosKerLeft  <  0         ) finePosKerLeft  = 0;
         else if( finePosKerLeft  >= kerTabLen ) finePosKerLeft  = kerTabLen-1;
         if(      finePosKerRight <  0         ) finePosKerRight = 0;
         else if( finePosKerRight >= kerTabLen ) finePosKerRight = kerTabLen-1;
         ker[a] = kerITab[ finePosKerRight ] - kerITab[ finePosKerLeft ];
      }
  }
}

template<class T>
void BasicEnlarger<T>::CreateKernels(void) {
   const float enlargeKernelRad = 1.8;
   const float selectKernelRad  = 1.9;
   int kernelLen;
   float *fineKernelIntegralTab;

   // kernels for enlarge in x-dir
   kernelLen = 2*int( float(1<<kerFineExp) * enlargeKernelRad * scaleFaktX ) + 1;
   fineKernelIntegralTab = CreateSmoothEnlargeKernelIntegralTab(kernelLen);
   CreateKernelsFromIntegralTab( enlargeKernelX , sizeXDst, fineKernelIntegralTab,
                                 kernelLen, scaleFaktX, smallToBigTabX );
   delete fineKernelIntegralTab;

   // kernels for enlarge in y-dir
   kernelLen = 2*int( float(1<<kerFineExp) * enlargeKernelRad * scaleFaktY ) + 1;
   fineKernelIntegralTab = CreateSmoothEnlargeKernelIntegralTab(kernelLen);
   CreateKernelsFromIntegralTab( enlargeKernelY , sizeYDst, fineKernelIntegralTab,
                                 kernelLen, scaleFaktY, smallToBigTabY );
   delete fineKernelIntegralTab;

   // kernels for selection in x-dir
   kernelLen = 2*int( float(1<<kerFineExp) * selectKernelRad * scaleFaktX ) + 1;
   fineKernelIntegralTab = CreateSelectKernelIntegralTab(kernelLen);
   CreateKernelsFromIntegralTab( selectKernelX ,  sizeXDst, fineKernelIntegralTab,
                                 kernelLen, scaleFaktX, smallToBigTabX  );
   delete fineKernelIntegralTab;

   // kernels for selection in y-dir
   kernelLen = 2*int( float(1<<kerFineExp) * selectKernelRad * scaleFaktY ) + 1;
   fineKernelIntegralTab = CreateSelectKernelIntegralTab(kernelLen);
   CreateKernelsFromIntegralTab( selectKernelY ,  sizeYDst, fineKernelIntegralTab,
                                 kernelLen, scaleFaktY, smallToBigTabY  );
   delete fineKernelIntegralTab;
}

template<class T>
float * BasicEnlarger<T>::CreateSmoothEnlargeKernelTab(int len) {
   int n;
   float x,y;
   float sum , normF;

   float *tab = new float[len];
   sum=0.0;
   for( n=0 ; n<len; n++ ) {
      x = 2.0 * float(n) / float(len-1) - 1;
      y = 2.0 - 1.0/(1+x+0.00000001) - 1.0/(1-x+0.00000001);
      y = exp(y);
      tab[n] = y;//*y*y;
      sum += tab[n];
   }

   normF = 1.0/sum;
   for( n=0; n<len; n++ ) {
      tab[n] *= normF;
   }
   return tab;
}

template<class T>
float *BasicEnlarger<T>::CreateSelectKernelTab(int len) {
   int n;
   float x,y;
   float sum , normF;

   float *tab = new float[len];
   sum=0.0;
   for( n=0 ; n<len; n++ ) {

      x = 2.0 * float(n) / float(len-1) - 1.0;
      y = 1.0 - x*x*x*x;
      tab[n] = y;
      sum += tab[n];

   }

   normF = 1.0/sum;
   for( n=0; n<len; n++ ) {
      tab[n] *= normF;
   }
   return tab;
}

template<class T>
float *BasicEnlarger<T>::CreateSmoothEnlargeKernelIntegralTab(int len) {
   int n;
   float x,y;
   float sum , normF;

   float *tab = new float[len];
   sum=0.0;
   for( n=0 ; n<len; n++ ) {
      x = 2.0 * float(n) / float(len-1) - 1;
      y = 2.0 - 1.0/(1+x+0.00000001) - 1.0/(1-x+0.00000001);
      y = exp(y);
      sum += y;
      tab[n] = sum;
   }

   normF = 1.0/sum;
   for( n=0; n<len; n++ ) {
      tab[n] *= normF;
   }
   return tab;
}

template<class T>
float *BasicEnlarger<T>::CreateSelectKernelIntegralTab(int len) {
   int n;
   float x,y;
   float sum , normF;

   float *tab = new float[len];
   sum=0.0;
   for( n=0 ; n<len; n++ ) {

      x = 2.0 * float(n) / float(len-1) - 1.0;
      y = 1.0 - x*x*x*x;
      sum += y;
      tab[n] = sum;

   }

   normF = 1.0/sum;
   for( n=0; n<len; n++ ) {
      tab[n] *= normF;
   }
   return tab;
}

template<class T>
void BasicEnlarger<T>::CreateDiffTabs(void) {
   int a;

   if( OnlyShrinking() )
      return;

   for( a=0; a<diffTabLen; a++ ) {
      float w,w0 = float(a)/float(diffTabLen);
      //
      // 1. SelectWeights
      //
      w = 1.0 - w0;
      if(w<0.0) w=0.0;
      w = w*w*(3.0 - 2.0*w);
      w = pow_f(w,selectPeakExp);
      selectDiffTab[a] = w;

      //
      // 2. CenterWeights
      //
      w = w0;
      w = pow_f(w,centerWExp);
      w = 1.0 + centerWeightF*w;

      centerWeightTab[a] = w;
   }

}


 // for a BigPixel ( srcBX, srcBY ) read surrounding 5x5
template<class T>
void BasicEnlarger<T>::ReadBigPixelNeighs( int srcBX, int srcBY ) {
   int x,y,xx,yy,pos;
   for(y=0;y<5;y++) {
      for(x=0;x<5;x++) {
         pos = MatPos(x,y);
         xx = srcBX - 2 + x;
         yy = srcBY - 2 + y;
         bigPixelColor [pos] =  srcBlock->Get(xx,yy);
         bigPixelWeight[pos] =  baseWeights->GetF(xx,yy);
         bigPixelDX    [pos] =  dX->Get(xx,yy);
         bigPixelDY    [pos] =  dY->Get(xx,yy);
         bigPixelD2X   [pos] =  d2X->Get(xx,yy);
         bigPixelD2Y   [pos] =  d2Y->Get(xx,yy);
         bigPixelDXY   [pos] =  dXY->Get(xx,yy);
         bigPixelD2L   [pos] =  d2L->Get(xx,yy);
         bigPixelIntensity [pos] =  baseIntensity->GetF(xx,yy);

         float ff = bigPixelD2L[pos].Norm1()*bigPixelIntensity[pos]*30.0;
         if(ff>1.0)ff=1.0;
         ff*=ff;
         ff*=ff;
         ff*=ff;
         bigPixelCenterW[pos] = ff;  // increased weight near pixel-center

         // for FractNoise: select random center of deform kernel within fractTab
         if( fractTab!=0 && fractNoiseF!=0.0 ) {
            bigPixelFractCX [ pos ] = xx ;
            bigPixelFractCY [ pos ] = yy;
            fractTab->GetKerCenter( bigPixelFractCX [ pos ], bigPixelFractCY [ pos ], bigPixelFractCVal [ pos ] );
         }
      }
   }
}

template<class T>
void BasicEnlarger<T>::ReadDerivatives(void)   {
   int x,y;

   for(y=1;y<sizeSrcBlockY-1;y++) {
      for(x=1;x<sizeSrcBlockX-1;x++) {
         T      s00 = srcBlock->Get( x-1, y-1 );
         T      s10 = srcBlock->Get( x  , y-1 );
         T      s20 = srcBlock->Get( x+1, y-1 );
         T      s01 = srcBlock->Get( x-1, y   );
         T      s11 = srcBlock->Get( x  , y   );
         T      s21 = srcBlock->Get( x+1, y   );
         T      s02 = srcBlock->Get( x-1, y+1 );
         T      s12 = srcBlock->Get( x  , y+1 );
         T      s22 = srcBlock->Get( x+1, y+1 );

         T      dx,dy,d2x,d2y,dxy, d2;

         dx  = 0.5*( s21 - s01 );//0.25.. + 0.125*( s22 - s02 + s20 - s00 );
         dy  = 0.5*( s12 - s10 );//0.25.. + 0.125*( s22 - s20 + s02 - s00 );

         d2x =  s21 + s01 - 2.0*s11 ;

         d2y =  s12 + s10 - 2.0*s11 ;

         dxy = 0.25*(s22 - s20 - s02 + s00);

         dX->Set(x,y,dx);   dY->Set(x,y,dy);
         d2X->Set(x,y,d2x);  d2Y->Set(x,y,d2y);
         dXY->Set(x,y,dxy);

         d2  =   s00 + s02 + s20 + s22;
         d2 += ( s10 + s12 + s01 + s21 )*2.0  ;
         d2 = s11 - (1.0/12.0)*d2;
         d2L->Set(x,y,d2);
      }
   }

   for(y=3;y<sizeSrcBlockY-3;y++) {
      for(x=3;x<sizeSrcBlockX-3;x++) {
         float sum=0.0;
         T      c = srcBlock->Get(x,y);
         for(int ay=0;ay<7;ay++) {
            for(int ax=0;ax<7;ax++) {
               sum += (srcBlock->Get(x-3+ax,y-3+ay) - c).Norm1();
            }
         }
         sum = 1.0/(sum*0.5 + 0.05);
         baseIntensity->Set(x,y,sum);
      }
   }

   MyArray *bI = new MyArray(*baseIntensity);

   for(y=1;y<sizeSrcBlockY-1;y++) {
      for(x=1;x<sizeSrcBlockX-1;x++) {
         float iMin = 1000.0;
         T  c = srcBlock->Get(x,y);
         for(int ay=0;ay<3;ay++) {
            for(int ax=0;ax<3;ax++) {
               float v = bI->GetF(x-1+ax,y-1+ay);
               if(iMin>v) iMin = v;
            }
         }
         baseIntensity->Set(x,y,iMin);
      }
   }

   delete bI;

   MyArray *intensityS = baseIntensity->Smooth();
   delete baseIntensity;
   baseIntensity = intensityS;
   intensityS = baseIntensity->Smooth();
   delete baseIntensity;
   baseIntensity = intensityS;
}


// calc baseWeights for BigPixels
template<class T>
void BasicEnlarger<T>::CalcBaseWeights(void)   {
   int x,y;

   ReadDerivatives();
   for(y=1;y<sizeSrcBlockY-1;y++) {
      for(x=1;x<sizeSrcBlockX-1;x++) {
         float dd,intensityFakt;
         float gradNorm,laplaceNorm;
         float dWork;   // workMask-Fakt

         intensityFakt = baseIntensity->GetF(x,y);
         gradNorm = (srcBlock->DX(x,y).Norm1() + srcBlock->DY(x,y).Norm1());
         laplaceNorm = d2L->Get(x,y).Norm1();

         dd = 1.0 - 12.0*gradNorm*intensityFakt;

         if(dd<0.0) dd=0.0; else if(dd>1.0) dd=1.0;
         dd += 0.0001;

         dWork = (20.0*gradNorm )*(intensityFakt + 0.9);
         dWork *= dWork;
         dWork -= 0.7;
         if(dWork < 0.0) dWork=0.0; else if(dWork>1.0) dWork=1.0;

         baseWeights->Set(x,y,dd);
         workMask->Set(x,y,dWork);

      }
   }

   MyArray bW(*baseWeights);

   for(y=1;y<sizeSrcBlockY-1;y++) {
      for(x=1;x<sizeSrcBlockX-1;x++) {
         float dd,cc,intensityFakt;
         float gradNorm,laplaceNorm;

         intensityFakt = baseIntensity->GetF(x,y);
         gradNorm = (srcBlock->DX(x,y).Norm1() + srcBlock->DY(x,y).Norm1());
         laplaceNorm = d2L->Get(x,y).Norm1();
         float v = bW.GetF(x,y);

         dd =  ModVal2( v - bW.GetF(x-1,y-1), v - bW.GetF(x+1,y+1) );
         dd += ModVal2( v - bW.GetF(x-1,y+1), v - bW.GetF(x+1,y-1) );
         dd *= 0.5;
         dd += ModVal2( v - bW.GetF(x  ,y+1), v - bW.GetF(x  ,y-1) );
         dd += ModVal2( v - bW.GetF(x-1,y  ), v - bW.GetF(x+1,y  ) );
         dd*=(1.0/3.0);

         dd = v - lineNegF*dd;  //0.8
         if(dd<0.01) {
            dd = dd*100.0;
            dd = 1.0/(2.0 - dd);
            dd*= 0.01;
         }
         dd = pow_f(dd,sharpExp);  // Sharpness!

         cc = 1.0 - 12.0*gradNorm*intensityFakt;
         if(cc<0.0) cc=0.0; else if(cc>1.0) dd=1.0;
         cc = 10.0*laplaceNorm*intensityFakt*( 0.4 + cc );
         if(cc>1.0)cc=1.0;
         cc = pow_f(cc,2.0);
         dd+= linePosF*cc;
         if(dd<0.0) dd=0.0;

         baseWeights->Set(x,y,dd);
      }
   }
   workMask->Smoothen();
}

//
//-----------------------------------------------------------------------
//

#endif
