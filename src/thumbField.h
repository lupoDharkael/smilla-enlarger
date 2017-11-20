/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    thumbField.h: widget with thumbnail of output + prewiew-selection

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

#ifndef THUMBFIELD_H
#define THUMBFIELD_H

#include <QWidget>
#include "ClipRect.h"
#include "ImageEnlargerCode/EnlargeParam.h"

class QImage;

class ThumbField : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QImage theImage READ theImage WRITE setTheImage)

private:
    QImage myImage;
    QSize  thumbSize;

    // transform between screen and source
    STransform screenT;

    // selected position in source
    float selectX, selectY;
    float selectW, selectH;
    bool selectCrossVisible;

    // shown part of the sourceImage
    float baseClipX0, baseClipY0,baseClipX1,baseClipY1;
    float stretchY;   // zoomY/zoomX

    // instead of drawing the source image into the field, which may be slow for big sizes,
    // use a screenImage, into which the needed clip of the source is drawn once at every clipping-change
    QImage screenImage;

public:
    ThumbField(QWidget *parent=0);
    ~ThumbField(void);

    void setTheImage(const QImage & newImage);
    void setFormat(const EnlargeFormat & format);
    QImage theImage(void) const { return myImage; }
    void ShowCross(void) { selectCrossVisible = true; update(); }
    void HideCross(void) { selectCrossVisible = false; update(); }

signals:
    void selectionChanged(float x, float y);

public slots:
    void Update(void) { update(); }
    void moveSelection(float x, float y);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent (QResizeEvent * event);

private:
    void SimpleAdjust(void);          // adjust view if clip-rect has moved outside
    void AdjustViewAroundClip(void);  // center view around clip-rect
    void UpdateSizeAndZoom(void);     // for myImage calculate zoom,size to fit into MAX-W,H
    void UpdateScreenImage(void);     // for changed souce or clipping: redraw screenImage
    void CorrectSelect(void);
    void CenterSelect(void) {
       moveSelection(0.5*(baseClipX0 + baseClipX1), 0.5*(baseClipY0 + baseClipY1));
    }
};

#endif // THUMBFIELD_H
