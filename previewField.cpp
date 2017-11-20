/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    previewField.cpp: the preview-widget within the EnlargerDialog

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


#include "previewField.h"
#include <QtGui>

PreviewField::PreviewField(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    selectX = 0.0;
    selectY = 0.0;
    previewCalculated = false;
    zoomX = zoomY = 5.0;
    selectW = preWidth/zoomX;
    selectH = preHeight/zoomY;

	srcImage = QImage(200, 200, QImage::Format_ARGB32);
	srcImage.fill(qRgb(0,0,255));
    baseClipX0 = 0.0;                      baseClipY0 = 0.0;
    baseClipX1 = float(srcImage.width());  baseClipY1 = float(srcImage.height());
}

PreviewField::~PreviewField(void) {
}

void PreviewField::setZoom(float newZoomX, float newZoomY) {
    previewCalculated = false;
    zoomX = newZoomX;
    zoomY = newZoomY;
    float cx = CenterX();
    float cy = CenterY();
    selectW = preWidth/zoomX;
    selectH = preHeight/zoomY;
    selectX = cx - 0.5*selectW;
    selectY = cy - 0.5*selectH;
    CorrectSelect();
	emit selectionChanged(CenterX(), CenterY());
    update();
}

void PreviewField::setTheImage(const QImage & newImage) {
	if(srcImage != newImage) {
        previewCalculated = false;
        srcImage = newImage; //.convertToFormat(QImage::Format_ARGB32);
        update();
    }
}

void PreviewField::setPreview(const QImage & preImage) {
    previewCalculated = true;
    previewImage = preImage;
    update();
}

QSize PreviewField::sizeHint(void) const {
	return QSize(int(preWidth), int(preHeight));
}


QSize PreviewField::minimumSizeHint(void) const {
	return QSize(int(preWidth), int(preHeight));
}

// set the baseClipRect, which selects the used and displayed part of the source
void PreviewField::setClipRect(float cx0, float cy0, float cx1, float cy1) {
    baseClipX0 = cx0;    baseClipY0 = cy0;
    baseClipX1 = cx1;    baseClipY1 = cy1;
    float xOld = selectX, yOld = selectY;
    CorrectSelect();   // have we left the bounds? move us back
	if(selectX != xOld || selectY != yOld) {  // real change ?
        previewCalculated = false;
		emit selectionChanged(CenterX(), CenterY());
    }
    update();
}

void PreviewField::moveSelection(float nX, float nY) {
    bool doUpdate = false;
	if(nX != CenterX() || nY != CenterY()) {
        float xOld = selectX, yOld = selectY;
        selectX = nX - 0.5*selectW;
        selectY = nY - 0.5*selectH;
        CorrectSelect();   // have we left the bounds? move us back
		if(selectX != xOld || selectY != yOld) {  // real change ?
            previewCalculated = false;
            doUpdate = true;
			emit selectionChanged(CenterX(), CenterY());
        }
    }
	if(doUpdate)  // crop rect is being changed (maybe in other view) -> update
       update();
}

QRect PreviewField::DstRect(void) {
    float x0,y0,x1,y1;
    x0 = selectX;
    y0 = selectY;
    x1 = selectX + selectW;
    y1 = selectY + selectH;
	if(x1 > baseClipX1)
        x1 = baseClipX1;
	if(y1 > baseClipY1)
        y1 = baseClipY1;
   return QRect(int(x0*zoomX), int(y0*zoomY), int((x1-x0)*zoomX), int((y1-y0)*zoomY));
}

void PreviewField::mousePressEvent(QMouseEvent *event) {
   if(event->button() == Qt::LeftButton) {
      mouseXGrabStart = event->pos().x();
      mouseYGrabStart = event->pos().y();

      screenT.edgeX  = selectX;
      screenT.edgeY  = selectY;
      screenT.scaleX = zoomX;
      screenT.scaleY = zoomY;

	  setCursor(Qt::ClosedHandCursor);
      sXGrabStart = CenterX();
      sYGrabStart = CenterY();
   }
}


void PreviewField::mouseMoveEvent(QMouseEvent *event) {
   float newx, newy;
   int px = event->pos().x();
   int py = event->pos().y();
   screenT.edgeX  = selectX;
   screenT.edgeY  = selectY;
   screenT.scaleX = zoomX;
   screenT.scaleY = zoomY;

   if(event->buttons() & Qt::LeftButton) {
	  newx = sXGrabStart - float(px - mouseXGrabStart)/zoomX;
	  newy = sYGrabStart - float(py - mouseYGrabStart)/zoomY;
	  moveSelection(newx, newy);
   }
   else {   // left button not down - change cursor depending on position
	  setCursor(Qt::OpenHandCursor);
   }
}

void PreviewField::mouseReleaseEvent(QMouseEvent *event) {
   if(event->button() == Qt::LeftButton) {   // finish moving select-box
	  if(cursor().shape() == Qt::ClosedHandCursor) {
		 setCursor(Qt::OpenHandCursor);
      }
      update();
   }
}


void PreviewField::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
	if(previewCalculated) {
         int tWidth = preWidth, tHeight=preHeight;
		 if(previewImage.width() < tWidth)
                tWidth = previewImage.width();
		 if(previewImage.height() < tHeight)
                tHeight = previewImage.height();
		 QRectF targetR(0.0, 0.0, float(tWidth), float(tHeight));
		 painter.drawImage(targetR,  previewImage);
    }
    else {
        int sx = int(selectX)-1,   sy = int(selectY)-1;
        int sw = int(selectW)+3, sh = int(selectH)+3;
		if(sw > int(baseClipX1) - sx)
			sw = int(baseClipX1) - sx;
		if(sh > int(baseClipY1) - sy)
			sh = int(baseClipY1) - sy;
		int tx = int((float(sx) - selectX)*zoomX);
		int ty = int((float(sy) - selectY)*zoomY);
		int tw = int(float(sw)*zoomX);
		int th = int(float(sh)*zoomY);
        QRect sourceR, targetR;
		sourceR = QRect(sx, sy, sw, sh);
		targetR = QRect(tx, ty, tw, th);
		painter.drawImage(targetR,  srcImage, sourceR);
    }
}

void PreviewField::CorrectSelect(void) {
	if(selectX > baseClipX1 - selectW)
        selectX = baseClipX1 - selectW ;
	if(selectY > baseClipY1 - selectH)
        selectY = baseClipY1 - selectH ;
	if(selectX < baseClipX0)
        selectX = baseClipX0;
	if(selectY < baseClipY0)
        selectY = baseClipY0;
}


