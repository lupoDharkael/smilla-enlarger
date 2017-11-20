/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    ClipRect.cpp: for managing cropping selection

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

#include <QPainter>
#include "ClipRect.h"

CropSelectRect::CropSelectRect( int w, int h ) {
   SetSrc( w, h );
   srcFormatRatio = -1.0;
   isOpen = false;
   isDragged = false;
}

void CropSelectRect::DrawClipRect( QPainter & painter,  const STransform & sTrans ) {
   if( !isOpen )
      return;

    QRect screenRect = painter.viewport();
    // draw clip-rect
    int cx0 = sTrans.ScreenX( clipX0 ), cy0 = sTrans.ScreenY( clipY0 );
    int cx1 = sTrans.ScreenX( clipX1 ), cy1 = sTrans.ScreenY( clipY1 );
    int cx0_o = cx0 - dFrameOut, cy0_o = cy0 - dFrameOut;
    int cx1_o = cx1 + dFrameOut, cy1_o = cy1 + dFrameOut;

    int sWidth  = screenRect.width();
    int sHeight = screenRect.height();
    painter.fillRect( 0    ,  0    , sWidth        , cy0_o          , QColor(0,0,0,100) );
    painter.fillRect( 0    ,  cy0_o, cx0_o         , cy1_o-cy0_o    , QColor(0,0,0,100) );
    painter.fillRect( cx1_o,  cy0_o, sWidth-cx1_o  , cy1_o-cy0_o    , QColor(0,0,0,100) );
    painter.fillRect( 0    ,  cy1_o, sWidth        , sHeight - cy1_o, QColor(0,0,0,100) );

    painter.fillRect( cx0_o, cy0_o, dFrameOut, dFrameOut, QColor(255,255,255,150) );
    painter.fillRect( cx0_o, cy1  , dFrameOut, dFrameOut, QColor(255,255,255,150) );
    painter.fillRect( cx1  , cy0_o, dFrameOut, dFrameOut, QColor(255,255,255,150) );
    painter.fillRect( cx1  , cy1  , dFrameOut, dFrameOut, QColor(255,255,255,150) );

    painter.fillRect( cx0  , cy0_o, cx1 - cx0, dFrameOut, QColor( 0, 0, 0,180) );
    painter.fillRect( cx0  , cy1  , cx1 - cx0, dFrameOut, QColor( 0, 0, 0,180) );
    painter.fillRect( cx0_o, cy0  , dFrameOut, cy1 - cy0, QColor( 0, 0, 0,180) );
    painter.fillRect( cx1  , cy0  , dFrameOut, cy1 - cy0, QColor( 0, 0, 0,180) );

    painter.setPen( QColor( 255,255,255, 150) );
    painter.drawRect( cx0-1, cy0-1, cx1-cx0+1 ,cy1-cy0+1 );

    painter.setPen( QColor( 255,255,255, 40) );
    painter.drawRect( cx0_o, cy0_o, cx1_o-cx0_o-1, cy1_o-cy0_o-1 );
 }

void CropSelectRect::OpenClip( float x0, float y0, float x1, float y1 ) {
   clipX0 =  x0; clipY0 = y0;
   clipX1 =  x1; clipY1 = y1;
   CorrectClip();
   AdjustClipFormat();
   isOpen = true;
   isDragged = false;
}

void CropSelectRect::SetSrc( int w, int h ) {
   srcWidth = float( w );
   srcHeight = float( h );
   clipX0 = clipY0 = clipX1 = clipY1 = 0.0;
   isOpen = false;
   isDragged = false;
}

// check position of screen ( mouse ) coordinates in CropRect
void CropSelectRect::CheckPosition( int x, int y, const STransform & sTrans, PosInRect & posX, PosInRect & posY ) {
   int rx0,ry0,rx1,ry1;
   rx0 = sTrans.ScreenX( clipX0 );    ry0 = sTrans.ScreenY( clipY0 );
   rx1 = sTrans.ScreenX( clipX1 );    ry1 = sTrans.ScreenY( clipY1 );

   if( x < rx0-dFrameOut || x >= rx1+dFrameOut )
       posX = posOutside;
   else if(  x < rx0+dFrameIn )
      posX = posLeft;
   else if(  x >= rx1-dFrameIn )
      posX = posRight;
   else
      posX = posInside;

   if( y < ry0-dFrameOut || y >= ry1+dFrameOut )
       posY = posOutside;
   else if(  y < ry0+dFrameIn )
      posY = posTop;
   else if(  y >= ry1-dFrameIn )
      posY = posBottom;
   else
      posY = posInside;
}

void CropSelectRect::DragStart( float x, float y, PosInRect modeX, PosInRect modeY,  const STransform & sTrans  ) {
   currentSTrans = sTrans;
   dragModeX = modeX;
   dragModeY = modeY;
   if( modeX == posRight )
      grabDX = x - clipX1;
   else
      grabDX = x - clipX0;
   if( modeY == posBottom )
      grabDY = y - clipY1;
   else
      grabDY = y - clipY0;
   isDragged = true;
}

void CropSelectRect::DragMove ( float x, float y ) {
   float snapDX = snapWidth / currentSTrans.scaleX;
   float snapDY = snapWidth / currentSTrans.scaleY;
   float dx,dy;

   if( dragModeX == posRight )
      dx = x - clipX1 - grabDX;
   else
      dx = x - clipX0 - grabDX;

   if( dragModeY == posBottom )
      dy = y - clipY1 - grabDY;
   else
      dy = y - clipY0 - grabDY;

   if( dragModeX == posInside && dragModeY == posInside ) {  // drag the clip-rect around - with snap-effect
      if( clipX0 >= 0.0 && clipX0 + dx < 0.0 && clipX0 + dx > 0.0 - snapDX )
         dx = 0.0 - clipX0;
      else if( clipX1 <= srcWidth && clipX1 + dx > srcWidth && clipX1 + dx < srcWidth + snapDX )
         dx = srcWidth - clipX1;
      if( clipY0 >= 0.0 && clipY0 + dy < 0.0 && clipY0 + dy > 0.0 - snapDY )
         dy = 0.0 - clipY0;
      else if( clipY1 <= srcHeight && clipY1 + dy > srcHeight && clipY1 + dy < srcHeight + snapDY )
         dy = srcHeight - clipY1;

      clipX0 += dx; clipY0 += dy;
      clipX1 += dx; clipY1 += dy;
      return;
   }

   // change the frame in x direction
   if( dragModeX == posLeft ) {
      float cOld = clipX0;
      clipX0 += dx;
      if( cOld>= 0.0 && clipX0 < 0.0 && clipX0 > 0.0 - snapDX )
         clipX0 = 0.0;
      else if( clipX0 > srcWidth )
         clipX0 = srcWidth;

      if( clipX0 > clipX1 ) {
         float h = clipX0;
         clipX0 = clipX1;
         clipX1 = h;
         dragModeX = posRight;
      }
   }
   else if( dragModeX == posRight ) {
      float cOld = clipX1;
      clipX1 += dx;
      if( cOld <= srcWidth && clipX1 > srcWidth && clipX1 < srcWidth + snapDX )
         clipX1 = srcWidth;
      else if( clipX1 < 0.0 )
         clipX1 = 0.0;

      if( clipX0 > clipX1 ) {
         float h = clipX0;
         clipX0 = clipX1;
         clipX1 = h;
         dragModeX = posLeft;
      }
   }

   // change the frame in y direction
   if( dragModeY == posTop ) {
      float cOld = clipY0;
      clipY0 += dy;
      if( cOld>= 0.0 && clipY0 < 0.0 && clipY0 > 0.0 - snapDY )
         clipY0 = 0.0;
      else if( clipY0 > srcHeight )
         clipY0 = srcHeight;

      if( clipY0 > clipY1 ) {
         float h = clipY0;
         clipY0 = clipY1;
         clipY1 = h;
         dragModeY = posBottom;
      }
   }
   else if( dragModeY == posBottom ) {
      float cOld = clipY1;
      clipY1 += dy;
      if( cOld <= srcHeight && clipY1 > srcHeight && clipY1 < srcHeight + snapDY )
         clipY1 = srcHeight;
      else if( clipY1 < 0.0 )
         clipY1 = 0.0;
      if( clipY0 > clipY1 ) {
         float h = clipY0;
         clipY0 = clipY1;
         clipY1 = h;
         dragModeY = posTop;
      }
   }

   // format correction during dragging
   if( srcFormatRatio > 0.0 && ( dragModeX != posInside || dragModeY != posInside ) ) {
      if( dragModeX == posInside ) {
         AdjustClipFormatX();
      }
      else if( dragModeY == posInside ) {
         AdjustClipFormatY();
      }
      else { // corner
         AdjustClipFormatXY( dragModeX, dragModeY );
      }
   }
}

void CropSelectRect::CorrectClip( void ) {
   if( clipX0 < 0.0 )
      clipX0 = 0.0;
   if( clipX1 > srcWidth )
      clipX1 = srcWidth;
   if( clipY0 < 0.0 )
      clipY0 = 0.0;
   if( clipY1 > srcHeight )
      clipY1 = srcHeight;

   if( clipX0 > clipX1 )
      clipX0 = clipX1;
   if( clipY0 > clipY1 )
      clipY0 = clipY1;
}

void CropSelectRect::AdjustClipFormat( void ) {
   if( srcFormatRatio <= 0.0 )
      return;
   if( !IsOpen() || ClipW()==0.0 || ClipH()==0.0 )
      return;

   float f = ClipW() / ClipH();
   if( f != srcFormatRatio ) {  // always only change width of clipRect
      float xM = 0.5*( clipX0 + clipX1 );
      float w  = ClipW() * srcFormatRatio / f;
      clipX0 = xM - 0.5*w;
      clipX1 = xM + 0.5*w;
   }
   /*
   if( f < srcFormatRatio ) {
      float xM = 0.5*( clipX0 + clipX1 );
      float w  = ClipW() * srcFormatRatio / f;
      clipX0 = xM - 0.5*w;
      clipX1 = xM + 0.5*w;
   }
   else if( f > srcFormatRatio )  {
      float yM = 0.5*( clipY0 + clipY1 );
      float h  = ClipH() * f / srcFormatRatio;
      clipY0 = yM - 0.5*h;
      clipY1 = yM + 0.5*h;
   }
   */
}

void CropSelectRect::AdjustClipFormatXY( PosInRect modeX, PosInRect modeY ) {
//   float snapDX = snapWidth / currentSTrans.scaleX;
//   float snapDY = snapWidth / currentSTrans.scaleY;

   if( srcFormatRatio <= 0.0 )
      return;
   if( !IsOpen() || ClipW()==0.0 || ClipH()==0.0 )
      return;

   float f = ClipW() / ClipH();
   if( f < srcFormatRatio ) {
      float w  = ClipW() * srcFormatRatio / f;
      if( modeX == posLeft ) {
         clipX0 = clipX1 - w;
      }
      else {
         clipX1 = clipX0 + w;
      }
   }
   else if( f > srcFormatRatio )  {
      float h  = ClipH() * f / srcFormatRatio;
      if( modeY == posTop )
         clipY0 = clipY1 - h;
      else
         clipY1 = clipY0 + h;
   }

}
void CropSelectRect::AdjustClipFormatX( void ) {
   if( srcFormatRatio <= 0.0 )
      return;
   if( !IsOpen() || ClipW()==0.0 || ClipH()==0.0 )
      return;
   float xM = 0.5*( clipX0 + clipX1 );
   float w  = ClipH() * srcFormatRatio ;
   clipX0 = xM - 0.5*w;
   clipX1 = xM + 0.5*w;
}

void CropSelectRect::AdjustClipFormatY( void ) {
   if( srcFormatRatio <= 0.0 )
      return;
   if( !IsOpen() || ClipW()==0.0 || ClipH()==0.0 )
      return;
   float yM = 0.5*( clipY0 + clipY1 );
   float h  = ClipW() / srcFormatRatio;
   clipY0 = yM - 0.5*h;
   clipY1 = yM + 0.5*h;
}

