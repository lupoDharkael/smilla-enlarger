#include "ImageEnlargerCode/EnlargeParam.h"
#include "formatterclass.h"
#include <iostream>
using namespace std;


void FixWidthFormatter::CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format ) {
   ClipCheck( srcWidth, srcHeight );
   format.SetSrcSize( srcWidth, srcHeight );
   float zoomX =  float( dstWidth ) / ClipW() ;
   format.SetScaleFact( zoomX, zoomX/stretchXY );
   format.SetSrcClip( SrcX0(), SrcY0(), SrcX1(), SrcY1() );
}

void FixHeightFormatter::CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format ) {
   ClipCheck( srcWidth, srcHeight );
   format.SetSrcSize( srcWidth, srcHeight );
   float zoomY =  float( dstHeight ) / ClipH() ;
   format.SetScaleFact( zoomY*stretchXY, zoomY );
   format.SetSrcClip( SrcX0(), SrcY0(), SrcX1(), SrcY1() );
}

void FixZoomFormatter::CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format ) {
   ClipCheck( srcWidth, srcHeight );
   format.SetSrcSize( srcWidth, srcHeight );
   format.SetScaleFact( zoomX, zoomY );
   format.SetSrcClip( SrcX0(), SrcY0(), SrcX1(), SrcY1() );
}

void FixOutStretchFormatter::CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format ) {
   ClipCheck( srcWidth, srcHeight );
   format.SetSrcSize( srcWidth, srcHeight );
   float zoomX =  float( dstWidth )  / ClipW() ;
   float zoomY =  float( dstHeight ) / ClipH() ;
   format.SetScaleFact( zoomX, zoomY );
   format.SetSrcClip( SrcX0(), SrcY0(), SrcX1(), SrcY1() );
}

void CropFormatter::CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format ) {
   ClipCheck( srcWidth, srcHeight );
   format.SetSrcSize( srcWidth, srcHeight );

   float zoomX =  float( dstWidth )  / ( ClipW()*stretchXY ) ;
   float zoomY =  float( dstHeight ) / ClipH() ;
   if( zoomX < zoomY ) {
      float clipW = dstWidth / ( zoomY * stretchXY );
      float srcXMid = 0.5*( SrcX1() + SrcX0() );
      SetSrcX0( srcXMid - 0.5*clipW );
      SetSrcX1( srcXMid + 0.5*clipW );
      format.SetScaleFact( zoomY*stretchXY, zoomY );
   }
   else {
      float clipH = dstHeight /  zoomX;
      float srcYMid = 0.5*( SrcY1() + SrcY0() );
      SetSrcY0( srcYMid - 0.5*clipH );
      SetSrcY1( srcYMid + 0.5*clipH );
      format.SetScaleFact( zoomX*stretchXY, zoomX );

   }
/*
   float srcRatio = stretchXY * ClipW() / ClipH();
   float dstRatio = float( dstWidth ) / float( dstHeight );
   if( srcRatio > dstRatio ) {     // crop sides
      float clipW = dstRatio * ClipW() / stretchXY;
      float srcXMid = 0.5*( SrcX1() + SrcX0() );
      SetSrcX0( srcXMid - 0.5*clipW );
      SetSrcX1( srcXMid + 0.5*clipW );
   }
   else {                          // else crop top & bottom
      float clipH = stretchXY * ClipW() / dstRatio;
      float srcYMid = 0.5*( SrcY1() + SrcY0() );
      SetSrcY0( srcYMid - 0.5*clipH );
      SetSrcY1( srcYMid + 0.5*clipH );
   }

   float zoomX =  float( dstWidth )  / ClipW() ;
   float zoomY =  float( dstHeight ) / ClipH() ;
   format.SetScaleFact( zoomX, zoomY );
*/
   format.SetSrcClip( SrcX0(), SrcY0(), SrcX1(), SrcY1() );
}

void MaxBoundFormatter::CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format ) {
   ClipCheck( srcWidth, srcHeight );
   format.SetSrcSize( srcWidth, srcHeight );
   float zoomX =  float( maxWidth )  / ClipW() ;
   float zoomY =  float( maxHeight ) / ClipH() ;
   if( zoomX < zoomY ) {
      format.SetScaleFact( zoomX );
   }
   else {
      format.SetScaleFact( zoomY );
   }
   format.SetSrcClip( SrcX0(), SrcY0(), SrcX1(), SrcY1() );
}

void MaxBoundBarFormatter::CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format ) {
   ClipCheck( srcWidth, srcHeight );
   format.SetSrcSize( srcWidth, srcHeight );
   float zoomX =  float( maxWidth )  / ClipW() ;
   float zoomY =  float( maxHeight ) / ClipH() ;
   if( zoomX < zoomY ) {
      float clipH = maxHeight /  zoomX;
      float srcYMid = 0.5*( SrcY1() + SrcY0() );
      SetSrcY0( srcYMid - 0.5*clipH );
      SetSrcY1( srcYMid + 0.5*clipH );
      format.SetScaleFact( zoomX );
   }
   else {
      float clipW = maxWidth / zoomY;
      float srcXMid = 0.5*( SrcX1() + SrcX0() );
      SetSrcX0( srcXMid - 0.5*clipW );
      SetSrcX1( srcXMid + 0.5*clipW );
      format.SetScaleFact( zoomY );
   }
   format.SetSrcClip( SrcX0(), SrcY0(), SrcX1(), SrcY1() );
}
