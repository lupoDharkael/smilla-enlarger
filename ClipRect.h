/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    ClipRect.h: for managing cropping selection

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

#ifndef CLIPRECT_H
#define CLIPRECT_H

class QPainter;

const int dFrameOut = 10;
const int dFrameIn  = 4;
const float snapWidth = 60.0;

// for checking a given (mouse) position
// is it completely outside/inside
// or does it touch the frame at top/bottom/...
enum PosInRect {
   posOutside, posInside,
   posTop, posBottom, posLeft, posRight
};

// Transform screen <-> source image
class STransform {
public:
   float scaleX,scaleY;
   float edgeX, edgeY;

public:
   float SrcX(int screenX) const { return float(screenX)/scaleX  + edgeX; }
   float SrcY(int screenY) const { return float(screenY)/scaleY  + edgeY; }
   int ScreenX(float x) const { return int((x - edgeX)*scaleX); }
   int ScreenY(float y) const { return int((y - edgeY)*scaleY); }
};

class CropSelectRect {
   float srcWidth, srcHeight;     // total size of image
   float clipX0,clipY0,clipX1,clipY1;  // the selected rect
   float srcFormatRatio;               // clipWidth/clipHeight

   bool isOpen;
   bool isDragged;
   float grabDX,grabDY;   // for dragging
   STransform currentSTrans;
   PosInRect dragModeX;
   PosInRect dragModeY;
public:
   CropSelectRect(int w, int h);
   ~CropSelectRect(void) {}
   void DrawClipRect(QPainter & painter,  const STransform & sTrans);
   void SetFormat(float f) { srcFormatRatio = f; AdjustClipFormat(); }
   bool IsOpen(void) { return isOpen; }
   bool IsDragged(void) { return isDragged; }
   void CheckPosition(int x, int y, const STransform & sTrans, PosInRect & posX, PosInRect & posY);
   void DragStart(float x, float y, PosInRect modeX, PosInRect modeY,  const STransform & sTrans);
   void DragMove (float x, float y);
   void DragEnd(void) { isDragged = false; }

   void OpenClip(float x0, float y0, float x1, float y1);
   void CloseClip(void)   { isOpen = false; }
   void GetMarkedClipRect(float & cx0, float & cy0, float & cx1, float & cy1) {
	  if(IsOpen() && ClipW() > 0.0 && ClipH() > 0.0) {
         cx0 = clipX0; cy0 = clipY0; cx1 = clipX1; cy1 = clipY1;
      }
      else {
         cx0 = cy0 = 0.0; cx1 = srcWidth; cy1 = srcHeight;
      }
   }
   void SetSrc(int w, int h);
   float ClipW(void) { return clipX1 - clipX0; }
   float ClipH(void) { return clipY1 - clipY0; }

private:
   void CorrectClip(void);
   void AdjustClipFormat(void);
   void AdjustClipFormatX(void);
   void AdjustClipFormatY(void);
   void AdjustClipFormatXY(PosInRect modeX, PosInRect modeY);
};

#endif // CLIPRECT_H
