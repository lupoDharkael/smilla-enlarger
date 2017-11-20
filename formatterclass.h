/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
	selectField.h: the select-widget within the EnlargerDialog

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

#ifndef FORMATTERCLASS_H
#define FORMATTERCLASS_H

#include "ImageEnlargerCode/EnlargeParam.h"

class FormatterClass
{
   bool hasClipping;
   float srcX0,srcY0,srcX1,srcY1;
public:
   FormatterClass() { hasClipping = false; }
   virtual ~FormatterClass( void ) {}
   void SetSrcClip( float x0, float y0, float x1, float y1 ) { hasClipping=true; srcX0 = x0; srcY0 = y0; srcX1 = x1; srcY1 = y1; }
   void NoClipping( void ) { hasClipping = false; }
   void ClipCheck( int srcWidth, int srcHeight ) {
      if(!hasClipping) {
         srcX0 = srcY0 = 0.0;
         srcX1 = float( srcWidth );
         srcY1 = float( srcHeight );
      }
   }

   float SrcX0(void) { return srcX0; }
   float SrcY0(void) { return srcY0; }
   float SrcX1(void) { return srcX1; }
   float SrcY1(void) { return srcY1; }
   float ClipW(void) { return srcX1 - srcX0; }
   float ClipH(void) { return srcY1 - srcY0; }
   void SetSrcX0( float s) { srcX0 = s; }
   void SetSrcY0( float s) { srcY0 = s; }
   void SetSrcX1( float s) { srcX1 = s; }
   void SetSrcY1( float s) { srcY1 = s; }

   virtual FormatterClass *Clone( void ) { return new FormatterClass( *this ); }
   virtual void CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format ) {}
};

class FixWidthFormatter : public FormatterClass {
   int dstWidth;
   float stretchXY;

public:
   FixWidthFormatter( int w ) : FormatterClass(), dstWidth( w ), stretchXY( 1.0 ) {}
   void SetWidth( int w ) { dstWidth = w; }
   void SetStretch( float s ) { stretchXY = s; }  // scaleX/scaleY
   FormatterClass *Clone( void ) { return new FixWidthFormatter( *this ); }
   void CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format );
};

class FixHeightFormatter : public FormatterClass {
   int dstHeight;
   float stretchXY;

public:
   FixHeightFormatter( int h ) : FormatterClass(), dstHeight( h ), stretchXY( 1.0 ) {}
   void SetHeight( int h ) { dstHeight = h; }
   void SetStretch( float s ) { stretchXY = s; }  // scaleX/scaleY
   FormatterClass *Clone( void ) { return new FixHeightFormatter( *this ); }
   void CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format );
};

class FixZoomFormatter : public FormatterClass {
   float zoomX, zoomY;

public:
   FixZoomFormatter( float z ) : FormatterClass(), zoomX( z ), zoomY( z ) {}
   FixZoomFormatter( float zx, float zy ) : FormatterClass(), zoomX( zx ), zoomY( zy ) {}
   void SetZoom( float z ) { zoomX = zoomY = z; }
   void SetZoom( float zx, float zy ) { zoomX = zx; zoomY = zy; }
   FormatterClass *Clone( void ) { return new FixZoomFormatter( *this ); }
   void CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format );
};

class FixOutStretchFormatter : public FormatterClass {
   int dstWidth, dstHeight;

public:
   FixOutStretchFormatter( int w, int h ) : FormatterClass(), dstWidth( w ), dstHeight( h ) {}
   void SetWidth( int w ) { dstWidth = w; }
   void SetHeight( int h ) { dstHeight = h; }
   FormatterClass *Clone( void ) { return new FixOutStretchFormatter( *this ); }
   void CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format );
};

class CropFormatter : public FormatterClass {
   int dstWidth, dstHeight;
   float stretchXY;

public:
   CropFormatter( int w, int h ) : FormatterClass(), dstWidth( w ), dstHeight( h ), stretchXY( 1.0 ) {}
   void SetWidth( int w ) { dstWidth = w; }
   void SetHeight( int h ) { dstHeight = h; }
   void SetStretch( float s ) { stretchXY = s; }  // scaleX/scaleY
   FormatterClass *Clone( void ) { return new CropFormatter( *this ); }
   void CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format );
};

class MaxBoundFormatter : public FormatterClass {
   int maxWidth, maxHeight;

public:
   MaxBoundFormatter( int w, int h ) : FormatterClass(), maxWidth( w ), maxHeight( h ) {}
   void SetWidth( int w ) { maxWidth = w; }
   void SetHeight( int h ) { maxHeight = h; }
   FormatterClass *Clone( void ) { return new MaxBoundFormatter( *this ); }
   void CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format );
};

class MaxBoundBarFormatter : public FormatterClass {
   int maxWidth, maxHeight;

public:
   MaxBoundBarFormatter( int w, int h ) : FormatterClass(), maxWidth( w ), maxHeight( h ) {}
   void SetWidth( int w ) { maxWidth = w; }
   void SetHeight( int h ) { maxHeight = h; }
   FormatterClass *Clone( void ) { return new MaxBoundBarFormatter( *this ); }
   void CalculateFormat( int srcWidth, int srcHeight, EnlargeFormat & format );
};

#endif // FORMATTERCLASS_H
