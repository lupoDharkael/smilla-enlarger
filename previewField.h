/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    previewField.h: the preview-widget within the EnlargerDialog

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

#ifndef PREVIEWFIELD_H
#define PREVIEWFIELD_H
#include <QWidget>
#include "ClipRect.h"

class QImage;
const float preWidth  = 400.0;
const float preHeight = 400.0;

class PreviewField : public QWidget {
    Q_OBJECT

private:
    QImage srcImage;
    QImage previewImage;
    bool previewCalculated;
    float selectX,selectY;
    float selectW,selectH;
    float zoomX,zoomY;

    // transform between screen and source
    STransform screenT;

    // shown part of the sourceImage
    float baseClipX0, baseClipY0,baseClipX1,baseClipY1;

    // for grab-moving of selection
    int mouseXGrabStart, mouseYGrabStart;
    float sXGrabStart, sYGrabStart;

public:
    PreviewField( QWidget *parent=0 );
    ~PreviewField( void );
    void setZoom( float newZoomX,  float newZoomY );
    void setZoom( float newZoom  )  { setZoom( newZoom, newZoom ); }
    void setTheImage( const QImage & newImage );
    void setClipRect( float cx0, float cy0, float cx1, float cy1 );
    void setPreview( const QImage & preImage );
    QImage theImage( void ) const { return srcImage; }
    QRect DstRect( void );
    float ZoomX( void ) { return zoomX; };
    float ZoomY( void ) { return zoomY; };
    float CenterX( void ) { return selectX + 0.5*selectW; }
    float CenterY( void ) { return selectY + 0.5*selectH; }
    QSize sizeHint( void ) const;
    QSize minimumSizeHint( void ) const;

signals:
    void selectionChanged( float x, float y );

public slots:
    void moveSelection( float x, float y );

protected:
    void mousePressEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void paintEvent( QPaintEvent *event );

private:
    void CorrectSelect( void );
};

#endif // PREVIEWFIELD_H
