/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    selectField.cpp: the select-widget within the EnlargerDialog

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

#include "selectField.h"
#include "ClipRect.h"

using namespace std;

const int selectBorderWidth = 14;
SelectField::SelectField(QWidget *parent) {
	Q_UNUSED(parent);
	setAttribute(Qt::WA_StaticContents);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	myImage = QImage(200, 200, QImage::Format_ARGB32);
	myImage.fill(qRgb(0,0,255));
	mySize = size();
	cropRect = 0;
	formatRatio = 1.0;
	AdjustViewAroundClip();

}

SelectField::~SelectField() {
}


void SelectField::setTheImage(const QImage & newImage) {
   if(newImage != myImage) {
      myImage = newImage;
	  if(cropRect != 0) {
		 cropRect->SetSrc(myImage.width(), myImage.height());
      }
      AdjustViewAroundClip();
      update();
   }
}



// set the baseClipRect, which selects the used and displayed part of the source
void SelectField::AdjustView(void) {
   AdjustViewAroundClip();
   emit clippingChanged();
   update();
}

void SelectField::mousePressEvent(QMouseEvent *event) {
   if(event->button() == Qt::LeftButton) {
	  this->setFocus(); // only a simple means to remove focus from textedits (and thus allow image paste in the EnlargerDialog)
      dragStartX = event->pos().x();
      dragStartY = event->pos().y();
	  if(cropRect != 0 && cropRect->IsOpen()) {  // there is a clipRect on the screen
         PosInRect modeX, modeY;
         // check if frame or interior was clicked,
         // in this case begin changing the rect
		 cropRect->CheckPosition(dragStartX, dragStartY, screenT, modeX, modeY);
		 if(modeX != posOutside && modeY != posOutside) {
			cropRect->DragStart(screenT.SrcX(dragStartX), screenT.SrcY(dragStartY), modeX, modeY, screenT);
			if(modeX == posInside && modeY == posInside) {
			   setCursor(Qt::ClosedHandCursor);
            }
         }
         else {
            cropRect->CloseClip();
            emit clippingChanged();
         }
      }
   }
   emit selectionChanged(screenT.SrcX(dragStartX), screenT.SrcY(dragStartY));
}


void SelectField::mouseMoveEvent(QMouseEvent *event) {
   int px = event->pos().x();
   int py = event->pos().y();
   if(event->buttons() & Qt::LeftButton) {
      float newx,newy;
	  newx = screenT.SrcX(px);
	  newy = screenT.SrcY(py);

	  if(cropRect == 0) {
		 emit selectionChanged(newx, newy);
         return;
      }

	  if(!cropRect->IsDragged()) {
		 if(px != dragStartX || py != dragStartY) {
            float cx0,cx1,cy0,cy1;
            PosInRect modeX, modeY;

			if(px < dragStartX) {
			   cx0 = screenT.SrcX(px);
			   cx1 = screenT.SrcX(dragStartX);
               modeX = posLeft;
            }
            else {
			   cx0 = screenT.SrcX(dragStartX);
			   cx1 = screenT.SrcX(px);
               modeX = posRight;
            }
			if(py < dragStartY) {
			   cy0 = screenT.SrcY(py);
			   cy1 = screenT.SrcY(dragStartY);
               modeY = posTop;
            }
            else {
			   cy0 = screenT.SrcY(dragStartY);
			   cy1 = screenT.SrcY(py);
               modeY = posBottom;
            }
			cropRect->OpenClip(cx0, cy0, cx1, cy1);
			cropRect->DragStart(newx, newy, modeX, modeY, screenT);
            emit clippingChanged();
         }
      }
      else {
		 cropRect->DragMove(newx, newy);
         emit clippingChanged();
      }
	  emit selectionChanged(newx, newy);
      update();
   }
   else {    // left button not down - change cursor depending on position
	  if(!cropRect->IsOpen()) {
		 setCursor(Qt::CrossCursor);
         return;
      }

      PosInRect posX, posY;
	  cropRect->CheckPosition(px, py, screenT, posX, posY);
	  if(posX == posOutside || posY == posOutside) {
		 setCursor(Qt::CrossCursor);
         return;
      }
	  if(posX == posInside && posY == posInside) {
		 setCursor(Qt::OpenHandCursor);
         return;
      }
	  if(posX == posInside) {
		 setCursor(Qt::SizeVerCursor);
         return;
      }
	  if(posY == posInside) {
		 setCursor(Qt::SizeHorCursor);
         return;
      }
	  if((posX == posLeft  && posY == posTop  ) ||
		  (posX == posRight && posY ==posBottom)   )
      {
		 setCursor(Qt::SizeFDiagCursor);
         return;
      }
	  setCursor(Qt::SizeBDiagCursor);
   }
}

void SelectField::mouseReleaseEvent(QMouseEvent *event) {
   if(event->button() == Qt::LeftButton) {   // finish moving select-box
	  if(cropRect != 0) {
         cropRect->DragEnd();
		 if(cursor().shape() == Qt::ClosedHandCursor) {
			setCursor(Qt::OpenHandCursor);
         }
         SimpleAdjust();
         emit clippingChanged();
      }
      update();
   }
}

void SelectField::paintEvent(QPaintEvent *event) {
	Q_UNUSED(event);
    QPainter painter(this);
	painter.setClipping(true);
	painter.setClipRect(0, 0, mySize.width(), mySize.height());
	painter.drawImage(0, 0, screenImage);

   if(cropRect != 0) {
	  cropRect->DrawClipRect(painter, screenT);
   }
}

void SelectField::resizeEvent (QResizeEvent * event) {
   if(cropRect==0 || !cropRect->IsOpen() || mySize.width() == 0 || mySize.height() == 0) {
      mySize = event->size();
      AdjustViewAroundClip();
      return;
   }
   mySize = event->size();
   baseClipX1 = baseClipX0 + float(mySize.width()) / screenT.scaleX;
   baseClipY1 = baseClipY0 + float(mySize.height()) / screenT.scaleY;
   SimpleAdjust();
}

// ---- ------------- ----

// instead of drawing the source image into the field, which may be slow for big sizes,
// use a screenImage, into which the needed clip of the source is drawn once at every clipping-change
void SelectField::UpdateScreenImage(void) {
	screenImage = QImage(mySize, QImage::Format_ARGB32);
	screenImage.fill(qRgb(0,0,0));
	QPainter painter(&screenImage);
	painter.setRenderHint(QPainter::Antialiasing, true);

	int sx = int(baseClipX0)-1,   sy = int(baseClipY0)-1;
	int sw = int(baseClipX1 - baseClipX0) + 3;
	int sh = int(baseClipY1 - baseClipY0) + 3;

	int tx = screenT.ScreenX(float(sx));
	int ty = screenT.ScreenY(float(sy));
	int tw = int(float(sw)*screenT.scaleX);
	int th = int(float(sh)*screenT.scaleY);
    QRect sourceR, targetR;
	sourceR = QRect(sx, sy, sw, sh);
	targetR = QRect(tx, ty, tw, th);
	painter.drawImage(targetR,  myImage, sourceR);

}

void SelectField::AdjustViewAroundClip() {
   float cx0,cy0,cx1,cy1;

   if(mySize.width() == 0 || mySize.height() == 0)
      return;

   if(cropRect != 0 && cropRect->IsOpen()) {  //if necessary, change baseClip to show the complete clipRect
	  cropRect->GetMarkedClipRect(cx0, cy0, cx1, cy1);
	  float margin = 0.1 * ((cx1 - cx0)  + (cy1 - cy0));
      float mx = margin;
      float my = margin;
      cx0 -= mx; cx1 += mx;
      cy0 -= my; cy1 += my;
   }
   else {
      cx0 = cy0 = 0.0;
      cx1 = float(myImage.width());
      cy1 = float(myImage.height());
   }
   //float cWidth  = cx1 - cx0;
   //float cHeight = cy1 - cy0;


   float srcRatio = (cx1-cx0) / (cy1-cy0);
   float dstRatio = float(mySize.width()) / float(mySize.height());
   if(srcRatio < dstRatio) {   // add at sides
	  float clipW = dstRatio * (cy1 - cy0);
	  float cxMid = 0.5*(cx1 + cx0);
      cx0 = cxMid - 0.5*clipW;
      cx1 = cxMid + 0.5*clipW;
   }
   else {              // add at top & bottom
	  float clipH = (cx1 - cx0) / dstRatio;
	  float cyMid = 0.5*(cy1 + cy0);
      cy0 = cyMid - 0.5*clipH;
      cy1 = cyMid + 0.5*clipH;
   }

   screenT.edgeX = cx0;
   screenT.edgeY = cy0;
   screenT.scaleX = float(mySize.width())  / (cx1-cx0);
   screenT.scaleY = float(mySize.height()) / (cy1-cy0);
   baseClipX0 = cx0;
   baseClipX1 = cx1;
   baseClipY0 = cy0;
   baseClipY1 = cy1;

   UpdateScreenImage();
}

void SelectField::SimpleAdjust(void) {
   float cx0,cy0,cx1,cy1;
   float bx0,by0,bx1,by1;
   //float cWidth, cHeight;

   bx0 = baseClipX0; by0 = baseClipY0;
   bx1 = baseClipX1; by1 = baseClipY1;
   const float margin = 40.0;
   float marginDX = margin / screenT.scaleX;
   float marginDY = margin / screenT.scaleY;
   float d;
   if(cropRect!=0 && cropRect->IsOpen()) {  //if necessary, change baseClip to show the complete clipRect
	  cropRect->GetMarkedClipRect(cx0, cy0, cx1, cy1);
   }
   else {
      cx0 = cy0 = 0.0;
      cx1 = float(myImage.width());
      cy1 = float(myImage.height());
   }
   if(cx0 < baseClipX0) {
      d = baseClipX0 - cx0 + marginDX;
      bx0 -= d; bx1 -= d;
   }
   else if(cx1 > baseClipX1) {
      d = cx1 - baseClipX1 + marginDX;
      bx0 += d; bx1 += d;
   }
   if(cy0 < baseClipY0) {
      d = baseClipY0 - cy0 + marginDY;
      by0 -= d; by1 -= d;
   }
   else if(cy1 > baseClipY1) {
      d = cy1 - baseClipY1 + marginDY;
      by0 += d; by1 += d;
   }
   if(cx0 < bx0 || cx1 > bx1 || cy0 < by0 || cy1 > by1) { // simple adjust does not work -> rescale
      AdjustViewAroundClip();
      return;
   }

   if(bx0 != baseClipX0 || bx1 != baseClipX1 ||
	   by0 != baseClipY0 || by1 != baseClipY1)
   {
      screenT.edgeX = bx0;
      screenT.edgeY = by0;
      baseClipX0 = bx0;
      baseClipX1 = bx1;
      baseClipY0 = by0;
      baseClipY1 = by1;
   }
   UpdateScreenImage();
}


