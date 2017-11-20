/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    EnlargeParam.h: data structure containing the sharp,flat,... parameters

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

#ifndef ENLARGEPARAM_H
#define ENLARGEPARAM_H

class EnlargeParameter {
public:
    float flat;
    float sharp;
    float deNoise;
    float preSharp;
    float dither;
    float fractNoise;
};

class EnlargeParamInt {
public:
   int sharp;
   int flat;
   int dither;
   int deNoise;
   int preSharp;
   int fractNoise;

public:
   EnlargeParameter FloatParam(void) {
      EnlargeParameter p;
	  p.sharp    = float(sharp  + 1) * 0.01;
	  p.flat     = float(flat      ) * 0.01;
	  p.dither   = float(dither    ) * 0.01;
	  p.deNoise  = float(deNoise   ) * 0.02;
	  p.preSharp = float(preSharp  ) * 0.01;
	  float    f = float(fractNoise) * 0.01;
	  p.fractNoise = 0.5 * f * (3.0 - f);
      return p;
   }
};

class EnlargeFormat {
public:
   int srcWidth;
   int srcHeight;
   float scaleX;
   float scaleY;
   int clipX0, clipY0;
   int clipX1, clipY1;

public:
   void SetSrcSize(int w, int h) { srcWidth = w; srcHeight = h; }
   void SetScaleFact(float f) { scaleX = scaleY = f; SetFullClip(); }
   void SetScaleFact(float fx, float fy) { scaleX = fx; scaleY = fy; SetFullClip(); }
   int  DstWidth (void) const { return int(float(srcWidth) *scaleX + 0.5); }
   int  DstHeight(void) const { return int(float(srcHeight)*scaleY + 0.5); }
   void SetDstClip(int cx0, int cy0, int cx1, int cy1) { clipX0=cx0; clipY0=cy0; clipX1=cx1; clipY1=cy1; }
   void SetSrcClip(float sx0, float sy0, float sx1, float sy1) {
	  clipX0 = int(scaleX*sx0),  clipY0 = int(scaleY*sy0);
	  clipX1 = clipX0 + int((sx1-sx0)*scaleX + 0.5);
	  clipY1 = clipY0 + int((sy1-sy0)*scaleY + 0.5);
   }
   void GetSrcClip(float & sx0, float & sy0, float & sx1, float & sy1) {
      sx0 = float(clipX0)/scaleX;
      sy0 = float(clipY0)/scaleY;
      sx1 = float(clipX1)/scaleX;
      sy1 = float(clipY1)/scaleY;
   }
   void SetFullClip(void) { clipX0 = 0; clipY0 = 0; clipX1 = DstWidth(); clipY1 = DstHeight(); }
   int ClipW(void) const { return clipX1 - clipX0; }
   int ClipH(void) const { return clipY1 - clipY0; }
};

#endif // ENLARGEPARAM_H
