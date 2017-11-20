/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    EnlargerTemplate.h: the enlarging algorithm

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

#ifndef ENLARGER_TEMPLATE_H
#define ENLARGER_TEMPLATE_H

#include <iostream>
#include "ConstDefs.h"
#include "Array.h"
#include "PointClass.h"
#include "FractTab.h"
#include "EnlargeParam.h"
#include "timing.h"

using namespace std;

const long smallToBigMargin = blockLen + 40;     // need values<0,>size in smallToBigTabs

// BasicEnlarger contains the algorithm, applied on srcBlock and dstBlock
// a derived real enlarger has to implement
//      void ReadCurrentBlock(int dstXEdge,int dstYEdge);
//      for reading a block from the source
// and
//      void WriteDstBlock( void );
//      for writing the enlarged block to the dest.
// Thus BasicEnlarger is independent from format, data type of source, destination
// It needs only the size and scaleFactor,
// and clipping, parameters
// There is no global enlarge-method, this has to be written in the derived enlarger-classes,
// using EnlargeBlock

template<class T>
class BasicEnlarger {

   BasicArray<T> *srcBlock, *dstBlock;  // srcBlock contains additional margins of borderPixels

   float scaleFaktX,invScaleFaktX;
   float scaleFaktY,invScaleFaktY;
   int sizeX,sizeY;
   int sizeXDst,sizeYDst;
   int outputWidth, outputHeight;          // size of resulting image
   int offsetX, offsetY;                   // pos of cliprect within image ( generate black margins )
   int  clipX0,clipX1,clipY0,clipY1;       // clip-rect
   bool onlyShrinking;                     // if scaleF < 1.0: don't allocate blocks&tabs, only call ShrinkClip()

   //
   //--------- Parameters -----------
   //
   float sharpExp;
   float centerWeightF;
   float centerWExp;
   float derivF;
   float selectPeakExp;
   float derivDiffF;
   float linePosF;
   float lineNegF;
   float ditherF;
   float dirDiffF;
   float preSharpenF;
   float deNoiseF;
   float fractNoiseF;

   //
   //--------- Helper-Objects -----------
   //

   // Enlarging is done blockwise
   int sizeSrcBlockX, sizeSrcBlockY;
   int sizeDstBlock;
   int dstBlockEdgeX,dstBlockEdgeY;                   // smallPos of upper left edge of DstBlock
   int srcBlockEdgeX,srcBlockEdgeY;                   // bigPos of upper left edge of   SrcBlock
   int dstMinBX, dstMinBY;
   int dstMaxBX, dstMaxBY;                            // clipped part of the current block

   RandGen  *randGen;
   FractTab *fractTab;      // used for deforming kernels
   float *selectDiffTab;
   float *centerWeightTab;  // weight multiplied with factor increasing near center of bigPixel
   float *invTab;           // table for x -> 1/x ( for inner loop )
   int   *smallToBigTabX;   // tables for mapping small/dst-pixels to big/src-pixels
   int   *smallToBigTabY;   //

   // Before Enlarging, calculate the importance of each BigPixel in Block
   //
   // similWeights: big weight, if similar to neighbours
   // border pixels are less important
   //
   // indieWeights: important are also pixels, which are totally
   // independent from most neighbours, e.g single specs or thin lines in front of
   // background of different color
   MyArray *baseWeights;
   MyArray *workMask;     // decides, if anything is to be done within bigPixel (smoothed -> multiply)
   MyArray *workMaskDst;  // workMaskDst: smooth-enlarged
   MyArray *baseParams;
   BasicArray<T>  *dX, *dY, *d2X, *d2Y, *dXY, *d2L;   // get modified Gradient, 2nd Deriv, Laplace
   MyArray *baseIntensity;

   // for each smallPixelPos calc. kernels for smooth-enlarging
   // and for selecting of neigh. BigPixels
   float **enlargeKernelX;
   float **enlargeKernelY;
   float **selectKernelX;    // 5-Kernel , multiply kx*ky
   float **selectKernelY;

   // Helper-Matrices containing values and weights of the current 5x5 BigPixels
   T      bigPixelColor [5*5];
   float  bigPixelWeight[5*5];
   float  bigPixelIntensity[5*5];
   float  bigPixelCenterW[5*5];  // increased weight near pixel-center dep. of d2L

   T      bigPixelDX    [5*5];
   T      bigPixelDY    [5*5];
   T      bigPixelD2X   [5*5];
   T      bigPixelD2Y   [5*5];
   T      bigPixelDXY   [5*5];

   T      bigPixelD2L   [5*5];

   // for FractNoise: random kernel center pos & center val for fract deform kernels
   int    bigPixelFractCX  [ 5*5 ];
   int    bigPixelFractCY  [ 5*5 ];
   float  bigPixelFractCVal[ 5*5 ];

public:
   BasicEnlarger( const EnlargeFormat & format, const EnlargeParameter & param );
   virtual ~BasicEnlarger(void);

   // test this in own enlarger before calling any enlarger-methods!
   // ( blocks etc. are only created if not shrinking )
   bool OnlyShrinking( void ) { return onlyShrinking; }

   void Enlarge( void );   // example enlarge
   void SetParameter( const EnlargeParameter & p);
   void SetParameter(float sharpness, float flatness);
   void SetDeNoise ( float dF )    { deNoiseF = dF;    }
   void SetPreSharpen ( float pF ) { preSharpenF = pF; }
   void SetDither( float pD )      { ditherF = pD;     }
   void SetFractNoise( float fN )  { fractNoiseF = fN; }
   void SetFractTab( FractTab *fT ){ fractTab = fT; }

   int SizeDstX( void ) const { return sizeXDst; }
   int SizeDstY( void ) const { return sizeYDst; }
   int SizeSrcX( void ) const { return sizeX; }
   int SizeSrcY( void ) const { return sizeY; }

   float Dither( void )     { return ditherF;     };
   float FractNoise( void ) { return fractNoiseF; };

protected:
    // the methods for Enlarge, protected for use in class with calc-thread-design

   // the read & write methods,
   // normally only ReadSrcPixel & WriteDstPixel have to be implemented in real enlarger
   // these are used by the predefined Block & Line Read/Write methods
   virtual void ReadSrcPixel( int srcX, int srcY, T & dstP ) {}
   virtual void WriteDstPixel( T p, int dstCX, int dstCY )   {}
   virtual void ReadSrcBlock( void );
   virtual void WriteDstBlock( void );
   virtual void ReadSrcLine ( int srcY, T *srcLine );  // read & write line: for case of shrinking
   virtual void WriteDstLine( int dstY, T *dstLine );

   void BlockBegin(int dstXEdge,int dstYEdge);  // calculate positions, clipping
   void CalcBaseWeights(void);             // calc indie & simil Weights for BigPixels
   void BlockEnlargeSmooth(void);
   void AddRandom( void );
   void EnlargeBlock(void);
   void EnlargeBlockPart( int syStart, int syEnd );  // used for splitting up EnlargeBlock ( -> calc thread )
   void MaskBlockEnlargeSmooth(void);     // Smooth-Enlarging Mask-Field

   // Shrinking ( scaleF < 1.0 )
   void ShrinkClip( void );
   float ScaleFaktX( void ) { return scaleFaktX; }
   float ScaleFaktY( void ) { return scaleFaktY; }

   // needed for implementing  Read&Write
   int ClipX0 ( void ) const { return clipX0; }
   int ClipY0 ( void ) const { return clipY0; }
   int ClipX1 ( void ) const { return clipX1; }
   int ClipY1 ( void ) const { return clipY1; }
   int OutputWidth ( void ) const { return outputWidth; }
   int OutputHeight( void ) const { return outputHeight; }
   int DstMinBX( void ) const { return dstMinBX; }
   int DstMaxBX( void ) const { return dstMaxBX; }
   int DstMinBY( void ) const { return dstMinBY; }
   int DstMaxBY( void ) const { return dstMaxBY; }
   int SrcBlockEdgeX( void ) const { return srcBlockEdgeX; }
   int SrcBlockEdgeY( void ) const { return srcBlockEdgeY; }
   int DstBlockEdgeX( void ) const { return dstBlockEdgeX; }
   int DstBlockEdgeY( void ) const { return dstBlockEdgeY; }
   int SizeSrcBlockX( void ) const { return sizeSrcBlockX; }
   int SizeSrcBlockY( void ) const { return sizeSrcBlockY; }
   int SizeDstBlock ( void ) const { return sizeDstBlock;  }

   void SrcBlockReduceNoise( void ) { srcBlock->ReduceNoise( deNoiseF ); }
   void SrcBlockSharpen( void )     { srcBlock->Sharpen( preSharpenF );  }
   BasicArray<T> *CurrentSrcBlock( void ) { return srcBlock; }
   BasicArray<T> *CurrentDstBlock( void ) { return dstBlock; }

   float RandF( void ) { return randGen->RandF(); }
   FractTab *MyFractTab( void ) { return fractTab; }

private:
   // format.clip allows exceeding bounds ( for black margins )
   // this is converted to new cliprect within bounds and additional offset
   void CalculateClipAndOffset( const EnlargeFormat & format );
   // Shrinking ( scaleF < 1.0 )
   void ShrinkLineClip( T *srcLine, T *dstLine );
   // general kernelList-Creation
   void CreateKernels(void);            // Create the Smooth-Enlarger- and Select-Kernels
   float *CreateSmoothEnlargeKernelTab(int len);
   float *CreateSelectKernelTab(int len);
   float *CreateSmoothEnlargeKernelIntegralTab(int len);
   float *CreateSelectKernelIntegralTab(int len);
   void CreateKernelsFromTab(float **kerList, int kerListLen,
                             float *kerTab, int kerTabLen, float scaleF);
   void CreateKernelsFromIntegralTab(float **kerList, int kerListLen,
                             float *kerITab, int kerTabLen, float scaleF, int *smallToBigTab);

   void CreateDiffTabs(void);

   void BlockReadLineSmooth( int srcY, T  *line );
   void MaskBlockReadLineSmooth( int srcY, float *line );

   // bigPos of smallPixel / smallPixel in current block ( margin: need values<0,etc )
   int BigSrcPosX(int smallPos) { return smallToBigTabX[ smallPos + smallToBigMargin ]; }
   int BigSrcPosY(int smallPos) { return smallToBigTabY[ smallPos + smallToBigMargin ]; }
   int SrcX( int dstBX )  { return BigSrcPosX( dstBX + dstBlockEdgeX ); }
   int SrcY( int dstBY )  { return BigSrcPosY( dstBY + dstBlockEdgeY ); }

   // bigPos in current srcBlock of  smallPos in current block
   int CurrentSrcBlockX( int dstBX )    { return SrcX( dstBX ) - srcBlockEdgeX; }
   int CurrentSrcBlockY( int dstBY )    { return SrcY( dstBY ) - srcBlockEdgeY; }

   void ReadDerivatives(void);
   void CalcBaseWeights0(void);             // calc indie & simil Weights for BigPixels
   void CalcBaseWeights1(void);             // calc indie & simil Weights for BigPixels
   void ReadBigPixelNeighs(int srcBX, int srcBY ); // for a BigPixel (srcBX,srcBY) read surrounding 5x5

   // Selection-WeightFact: used when selecting BigPixel for smallPixel
   float SelectWeight( float pointDiff ) {
      if(pointDiff>=1.0)
         return selectDiffTab[diffTabLen-1];
      return selectDiffTab[ int( pointDiff * float(diffTabLen-1) ) ];
   }

   // give bigPixel add. Weigt near center
   float CenterWeight( float dd ) {
      dd = 1.0 - dd*(1.0/1.5);
      if(dd<0.0)
         return centerWeightTab[0];
      return centerWeightTab[ int( dd * float(diffTabLen-1) ) ];
   }

   int MatPos ( int x,int y ) { return x + 5*y; }
   inline float Inverse ( float x );
   inline T     LinModColor  ( float fx, float fy, int a );
   inline T     LinModColorB ( float fx, float fy, int a );
   inline void  QuadricCalc  ( float fx, float fy, int a, float deltaX, T  & quad, T  & quadD, T  & quadD2 );
   inline float ModVal  ( float f );
   inline float ModVal2 ( float f, float f2 );
};

template<class T>
inline float BasicEnlarger<T>::Inverse( float x ) {
   if(x<0.001) {
      x*=1000.0*float(invTabLen-1);
      return 1000.0*invTab[int(x+0.5)];
   }
   else if(x>=1.0) {
      x*=0.01;
      if(x>=1.0) {
         x*=0.01;
         if(x>1.0)
             return 0.0001/x;  // we give up
         return 0.0001*invTab[int( x*float(invTabLen-1) + 0.5 )];
      }
      return 0.01*invTab[int( x*float(invTabLen-1) + 0.5 )];
   }
   return invTab[int(x*float(invTabLen-1)+0.5)];
}

template<class T>
inline  T  BasicEnlarger<T>::LinModColor( float fx, float fy, int a ) {
   T  modC;
   const float faktD=1.0, faktD2=0.7;  //!!! 0.7;
   modC = bigPixelColor[a] + faktD*( fx*bigPixelDX[a] + fy*bigPixelDY[a]);
   T  modC2;
   modC2 =  0.5*( fx*fx*bigPixelD2X[a] + fy*fy*bigPixelD2Y[a] ) + fx*fy*bigPixelDXY[a] ;

   modC2 *= faktD2;
      //float ff = (4.0 - fx*fx - fy*fy)*(1.0/4.0);
      //if(ff<0.0) ff=0.0;
      //ff = ff*ff + faktD2;
      //modC2 *= ff;

   modC += modC2;
   modC.Clip();
   return modC;
}

template<class T>
   inline T  BasicEnlarger<T>::LinModColorB( float fx, float fy, int a ) {
   T  modC;
   const float faktD=1.0, faktD2=0.7;  //!!! 1.0 /  0.7;

   modC = fx*bigPixelDX[a] + fy*bigPixelDY[a];
   modC +=  (0.5*faktD2*fx*fx)*bigPixelD2X[a] + (0.5*faktD2*fy*fy)*bigPixelD2Y[a];
   modC += (faktD2*fx*fy)*bigPixelDXY[a] ;
   return modC;
}

template<class T>
inline void BasicEnlarger<T>::QuadricCalc( float fx, float fy, int a, float deltaX, T  & quad, T  & quadD, T  & quadD2 ) {
   quad = fx*bigPixelDX[a] + fy*bigPixelDY[a];
   quad +=  (0.35*fx*fx)*bigPixelD2X[a] + (0.35*fy*fy)*bigPixelD2Y[a];
   quad += (0.7*fx*fy)*bigPixelDXY[a] ;
   quad *= derivF;

   quadD = deltaX*bigPixelDX[a];
   quadD += ( 0.35*deltaX*(2.0*fx + deltaX) )*bigPixelD2X[a];
   quadD += (0.7*deltaX*fy)*bigPixelDXY[a] ;
   quadD *= derivF;

   quadD2 = ( 0.7*derivF*deltaX*deltaX )*bigPixelD2X[a];
}

template<class T>
inline float BasicEnlarger<T>::ModVal( float f ) {
   if(f<0.0)
      return 0.0;
   return 6.0*pow_f((double)f,3.0);
}

template<class T>
inline float BasicEnlarger<T>::ModVal2( float f, float f2 ) {
   if(f>f2)
      f=f2;
   if(f<0.0)
      return 0.0;
   return 6.0*pow_f((double)f,2.0);
}

//
//-----------------------------------------------------------------------
//

#endif
