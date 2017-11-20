/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
	EnlargerThread.cpp: things necessary for putting the enlarging into own Qt-Thread

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

#include "ImageEnlargerCode/EnlargeParam.h"
#include "formatterclass.h"
#include <iostream>

using namespace std;

void FixWidthFormatter::CalculateFormat(int srcWidth, int srcHeight, EnlargeFormat & format) {
   ClipCheck(srcWidth, srcHeight);
   format.SetSrcSize(srcWidth, srcHeight);
   float zoomX =  float(dstWidth) / ClipW() ;
   format.SetScaleFact(zoomX, zoomX/stretchXY);
   format.SetSrcClip(SrcX0(), SrcY0(), SrcX1(), SrcY1());
}

void FixHeightFormatter::CalculateFormat(int srcWidth, int srcHeight, EnlargeFormat & format) {
   ClipCheck(srcWidth, srcHeight);
   format.SetSrcSize(srcWidth, srcHeight);
   float zoomY =  float(dstHeight) / ClipH() ;
   format.SetScaleFact(zoomY*stretchXY, zoomY);
   format.SetSrcClip(SrcX0(), SrcY0(), SrcX1(), SrcY1());
}

void FixZoomFormatter::CalculateFormat(int srcWidth, int srcHeight, EnlargeFormat & format) {
   ClipCheck(srcWidth, srcHeight);
   format.SetSrcSize(srcWidth, srcHeight);
   format.SetScaleFact(zoomX, zoomY);
   format.SetSrcClip(SrcX0(), SrcY0(), SrcX1(), SrcY1());
}

void FixOutStretchFormatter::CalculateFormat(int srcWidth, int srcHeight, EnlargeFormat & format) {
   ClipCheck(srcWidth, srcHeight);
   format.SetSrcSize(srcWidth, srcHeight);
   float zoomX =  float(dstWidth)  / ClipW() ;
   float zoomY =  float(dstHeight) / ClipH() ;
   format.SetScaleFact(zoomX, zoomY);
   format.SetSrcClip(SrcX0(), SrcY0(), SrcX1(), SrcY1());
}

void CropFormatter::CalculateFormat(int srcWidth, int srcHeight, EnlargeFormat & format) {
   ClipCheck(srcWidth, srcHeight);
   format.SetSrcSize(srcWidth, srcHeight);

   float zoomX =  float(dstWidth)  / (ClipW()*stretchXY) ;
   float zoomY =  float(dstHeight) / ClipH() ;
   if(zoomX < zoomY) {
	  float clipW = dstWidth / (zoomY * stretchXY);
	  float srcXMid = 0.5*(SrcX1() + SrcX0());
	  SetSrcX0(srcXMid - 0.5*clipW);
	  SetSrcX1(srcXMid + 0.5*clipW);
	  format.SetScaleFact(zoomY*stretchXY, zoomY);
   }
   else {
      float clipH = dstHeight /  zoomX;
	  float srcYMid = 0.5*(SrcY1() + SrcY0());
	  SetSrcY0(srcYMid - 0.5*clipH);
	  SetSrcY1(srcYMid + 0.5*clipH);
	  format.SetScaleFact(zoomX*stretchXY, zoomX);

   }
/*
   float srcRatio = stretchXY * ClipW() / ClipH();
   float dstRatio = float(dstWidth) / float(dstHeight);
   if(srcRatio > dstRatio) {     // crop sides
      float clipW = dstRatio * ClipW() / stretchXY;
	  float srcXMid = 0.5*(SrcX1() + SrcX0());
	  SetSrcX0(srcXMid - 0.5*clipW);
	  SetSrcX1(srcXMid + 0.5*clipW);
   }
   else {                          // else crop top & bottom
      float clipH = stretchXY * ClipW() / dstRatio;
	  float srcYMid = 0.5*(SrcY1() + SrcY0());
	  SetSrcY0(srcYMid - 0.5*clipH);
	  SetSrcY1(srcYMid + 0.5*clipH);
   }

   float zoomX =  float(dstWidth)  / ClipW() ;
   float zoomY =  float(dstHeight) / ClipH() ;
   format.SetScaleFact(zoomX, zoomY);
*/
   format.SetSrcClip(SrcX0(), SrcY0(), SrcX1(), SrcY1());
}

void MaxBoundFormatter::CalculateFormat(int srcWidth, int srcHeight, EnlargeFormat & format) {
   ClipCheck(srcWidth, srcHeight);
   format.SetSrcSize(srcWidth, srcHeight);
   float zoomX =  float(maxWidth)  / ClipW() ;
   float zoomY =  float(maxHeight) / ClipH() ;
   if(zoomX < zoomY) {
	  format.SetScaleFact(zoomX);
   }
   else {
	  format.SetScaleFact(zoomY);
   }
   format.SetSrcClip(SrcX0(), SrcY0(), SrcX1(), SrcY1());
}

void MaxBoundBarFormatter::CalculateFormat(int srcWidth, int srcHeight, EnlargeFormat & format) {
   ClipCheck(srcWidth, srcHeight);
   format.SetSrcSize(srcWidth, srcHeight);
   float zoomX =  float(maxWidth)  / ClipW() ;
   float zoomY =  float(maxHeight) / ClipH() ;
   if(zoomX < zoomY) {
      float clipH = maxHeight /  zoomX;
	  float srcYMid = 0.5*(SrcY1() + SrcY0());
	  SetSrcY0(srcYMid - 0.5*clipH);
	  SetSrcY1(srcYMid + 0.5*clipH);
	  format.SetScaleFact(zoomX);
   }
   else {
      float clipW = maxWidth / zoomY;
	  float srcXMid = 0.5*(SrcX1() + SrcX0());
	  SetSrcX0(srcXMid - 0.5*clipW);
	  SetSrcX1(srcXMid + 0.5*clipW);
	  format.SetScaleFact(zoomY);
   }
   format.SetSrcClip(SrcX0(), SrcY0(), SrcX1(), SrcY1());
}
