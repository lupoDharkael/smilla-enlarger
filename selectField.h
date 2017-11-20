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

#ifndef SELECTFIELD_H
#define SELECTFIELD_H

#include <QWidget>
#include "ClipRect.h"

const float MAXHEIGHT = 400.0;
const float MAXWIDTH  = 400.0;

class QImage;

class SelectField : public QWidget {
    Q_OBJECT
    Q_PROPERTY( QImage theImage READ theImage WRITE setTheImage)

private:
    QImage myImage;
    QSize  mySize;

    // transform between screen and source
    STransform screenT;
    float formatRatio;  // clip format ( free if -1.0 )
    // shown part of the sourceImage
    float baseClipX0, baseClipY0,baseClipX1,baseClipY1;

    // for clipping-select
    CropSelectRect *cropRect;
    int dragStartX, dragStartY;

    // instead of drawing the source image into the field, which may be slow for big sizes,
    // use a screenImage, into which the needed clip of the source is drawn once at every clipping-change
    QImage screenImage;

public:
    SelectField( QWidget *parent=0 );
    ~SelectField( void );
    void SetCropRect( CropSelectRect *cR ) { cropRect = cR; }
    void setTheImage( const QImage & newImage );
    QImage theImage( void ) const { return myImage; }
    void AdjustView( void );
    void SetFormatRatio( float f ) {
       formatRatio = f ;
       if(  cropRect!=0 ) {
          cropRect->SetFormat( f );
          AdjustViewAroundClip();
          emit clippingChanged();
          Update();
          //SimpleAdjust();
       }
    }

signals:
    void selectionChanged( float x, float y );
    void clippingChanged ( void );

public slots:
    void Update( void ) { update(); }

protected:
    void mousePressEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void paintEvent( QPaintEvent *event );
    void resizeEvent ( QResizeEvent * event );

private:
    void SimpleAdjust( void );          // adjust view if clip-rect has moved outside
    void AdjustViewAroundClip( void );  // center view around clip-rect
    void UpdateScreenImage( void );     // for changed souce or clipping: redraw screenImage
};

#endif // SELECTFIELD_H
