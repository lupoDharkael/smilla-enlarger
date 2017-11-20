/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    thumbField.cpp: widget with thumbnail of output + prewiew-selection

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

#include <QtGui>
#include <iostream>

#include "thumbField.h"
#include "ClipRect.h"
#include "previewField.h"
#include "ImageEnlargerCode/EnlargeParam.h"

using namespace std;

const int selectBorderWidth = 14;
ThumbField::ThumbField( QWidget *parent ) {
   setAttribute(Qt::WA_StaticContents);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   myImage = QImage( 200, 200, QImage::Format_ARGB32 );
   myImage.fill( qRgb(0,0,255) );
   stretchY = 1.0;
   baseClipX0 = 0.0;
   baseClipY0 = 0.0;
   baseClipX1 = 200.0;
   baseClipY1 = 200.0;
   selectW = selectH = 0.0;
   selectCrossVisible = false;
   UpdateSizeAndZoom();
   UpdateScreenImage();
}

ThumbField::~ThumbField( void ) {
}


void ThumbField::setTheImage( const QImage & newImage ) {
   if( newImage != myImage ) {
      myImage = newImage;
   }

   stretchY = 1.0;
   baseClipX0 = baseClipY0 = 0.0;
   baseClipX1 = float( myImage.width() );
   baseClipY1 = float( myImage.height() );
   selectW = selectH = 0.0;
   UpdateSizeAndZoom();
   UpdateScreenImage();
   update();
   CenterSelect();
}

void ThumbField::setFormat( const EnlargeFormat & format ) {
   float invScaleX = 1.0/format.scaleX;
   float invScaleY = 1.0/format.scaleY;
   stretchY = format.scaleY * invScaleX;
   baseClipX0 = float( format.clipX0 )*invScaleX;
   baseClipY0 = float( format.clipY0 )*invScaleY;
   baseClipX1 = float( format.clipX1 )*invScaleX;
   baseClipY1 = float( format.clipY1 )*invScaleY;

   UpdateSizeAndZoom();
   UpdateScreenImage();
   selectW = preWidth*invScaleX;
   selectH = preHeight*invScaleY;

   float oldX = selectX;
   float oldY = selectY;
   CorrectSelect();
   if( oldX!=selectX || oldY!=selectY ) {
      emit selectionChanged( selectX, selectY );
   }
   update();
}


void ThumbField::mousePressEvent( QMouseEvent *event ) {
   if( event->button() == Qt::LeftButton ) {
      selectX = screenT.SrcX( event->pos().x() );
      selectY = screenT.SrcY( event->pos().y() );
      CorrectSelect();
      emit selectionChanged( selectX, selectY );
      update();
   }
}


void ThumbField::mouseMoveEvent( QMouseEvent *event ) {
   if( event->buttons() & Qt::LeftButton ) {
      float newX = screenT.SrcX( event->pos().x() );
      float newY = screenT.SrcY( event->pos().y() );
      if( newX != selectX || newY != selectY ) {
         selectX = newX;
         selectY = newY;
         CorrectSelect();
         emit selectionChanged( selectX, selectY );
         update();
      }
   }
}

void ThumbField::mouseReleaseEvent( QMouseEvent *event ) {
   if( event->button() == Qt::LeftButton ) {   // finish moving select-box
      float newX = screenT.SrcX( event->pos().x() );
      float newY = screenT.SrcY( event->pos().y() );
      if( newX != selectX || newY != selectY ) {
         selectX = newX;
         selectY = newY;
         CorrectSelect();
         emit selectionChanged( selectX, selectY );
         update();
      }
   }
}

void ThumbField::paintEvent( QPaintEvent *event ) {
    QPainter painter(this);
    painter.setClipping( true );
    painter.setClipRect( 0, 0, thumbSize.width(), thumbSize.height() );
    painter.drawImage( 0, 0, screenImage );
    int cx = screenT.ScreenX( selectX );
    int cy = screenT.ScreenY( selectY );
    if( selectCrossVisible ) {
       painter.fillRect( cx - 2, cy - 6,  5, 13, QColor(  0,  0,  0,150 ) );
       painter.fillRect( cx - 6, cy - 2, 13,  5, QColor(  0,  0,  0,150) );
       painter.fillRect( cx - 5, cy   ,  11,  1, QColor(255,255,255,255 ) );
       painter.fillRect( cx    , cy - 5,  1, 11, QColor(255,255,255,255 ) );
    }
}

void ThumbField::resizeEvent ( QResizeEvent * event ) {
   UpdateSizeAndZoom();
   UpdateScreenImage();
   update();
}

void ThumbField::moveSelection( float newX, float newY ) {
   float oldX = selectX;
   float oldY = selectY;
   selectX = newX;
   selectY = newY;
   CorrectSelect();
   if( oldX == selectX && oldY == selectY ) {
      return;
   }

   emit selectionChanged( selectX, selectY );
   update();
}

// ---- ------------- ----

void ThumbField::UpdateSizeAndZoom( void ) {

    float sizeX = float( size().width() );
    float sizeY = float( size().height() );
    float newW,newH,zoomX, zoomY;
    zoomX = sizeX / ( baseClipX1 - baseClipX0 );
    zoomY = zoomX * stretchY;
    newW = sizeX;
    newH = ( baseClipY1 - baseClipY0 ) * zoomY;
    if( newH > sizeY ) {
        zoomX = zoomX * sizeY / newH;
        zoomY = zoomX * stretchY;
        newH = sizeY;
        newW = ( baseClipX1 - baseClipX0 ) * zoomX;
    }
    screenT.edgeX  = baseClipX0;
    screenT.edgeY  = baseClipY0;
    screenT.scaleX = zoomX;
    screenT.scaleY = zoomY;

    thumbSize = QSize( int( newW ), int( newH ) );
}

// instead of drawing the source image into the field, which may be slow for big sizes,
// use a screenImage, into which the needed clip of the source is drawn once at every clipping-change
void ThumbField::UpdateScreenImage( void ) {
    screenImage = QImage( thumbSize, QImage::Format_ARGB32 );
    screenImage.fill( qRgb(0,0,0) );

    QPainter painter( &screenImage );
    painter.setRenderHint(QPainter::Antialiasing, true );

    int sx = int( baseClipX0 )-1,   sy = int( baseClipY0 )-1;
    int sw = int( baseClipX1 - baseClipX0 ) + 3;
    int sh = int( baseClipY1 - baseClipY0 ) + 3;

    int tx = screenT.ScreenX( float(sx) );
    int ty = screenT.ScreenY( float(sy) );
    int tw = int( float(sw)*screenT.scaleX );
    int th = int( float(sh)*screenT.scaleY );
    QRect sourceR, targetR;
    sourceR = QRect( sx, sy, sw, sh );
    targetR = QRect( tx, ty, tw, th );
    painter.drawImage( targetR,  myImage, sourceR );

}

void ThumbField::CorrectSelect( void ) {
    if( selectX > baseClipX1 - 0.5*selectW )
        selectX = baseClipX1 - 0.5*selectW;
    if( selectY > baseClipY1 - 0.5*selectH )
        selectY = baseClipY1 - 0.5*selectH ;
    if( selectX < baseClipX0 + 0.5*selectW )
        selectX = baseClipX0 + 0.5*selectW;
    if( selectY < baseClipY0 + 0.5*selectH  )
        selectY = baseClipY0 + 0.5*selectH ;
}

