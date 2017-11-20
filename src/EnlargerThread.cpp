/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    EnlargerThread.cpp: things necessary for putting the enlarging into own Qt-Thread

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

#include <exception>

#include "EnlargerThread.h"
#include <QThread>
#include "ImageEnlargerCode/timing.h"
#include "ImageEnlargerCode/FractTab.h"
#include "ImageEnlargerCode/EnlargerTemplate.h"
#include "ImageEnlargerCode/EnlargerTemplateDefs.h"

using namespace std;

template class BasicEnlarger <Point>;  // explicit instantiations
template class BasicEnlarger <Point4>;

bool ThColorEnlarger::Enlarge(QImage *dstI) {
   const int dstStepBY = 50;
   Timer timer0;
   int dstX, dstY;

   dstImg = dstI;
   if(dstImg->width() != OutputWidth() || dstImg->height() != OutputHeight()) {  // image not correctly allocated
      cout<<" ClipError: "<<dstImg->width()<<" "<< OutputWidth() <<" \n";
      cout<<"          : "<<dstImg->height()<<" "<< OutputHeight() <<" \n"<<flush;
      return false;
   }

   dstImg->fill(qRgb(0,0,0));

   if(OnlyShrinking()) {  // shrinking
      ShrinkClip();
      return true;
   }

   timer0.Clear();
   timer0.Start();

   long totalSteps;
   float progressStep=0.0;
   totalSteps  = (ClipX1() - ClipX0()) / blockLen + 1;
   totalSteps *= (ClipY1() - ClipY0()) / blockLen + 1;
   totalSteps *= blockLen;
   if(totalSteps>0)
	   progressStep = 1.0/float(totalSteps);

   for(dstY=ClipY0(); dstY<ClipY1(); dstY+=blockLen) {
	  for(dstX=ClipX0(); dstX<ClipX1(); dstX+=blockLen) {
		 if(myThread->CheckStop())
            { return false; }
		 BlockBegin(dstX, dstY);
         ReadSrcBlock();

         SrcBlockReduceNoise();
         SrcBlockSharpen();
         CalcBaseWeights();

		 if(myThread->CheckStop())
            { return false; }

         BlockEnlargeSmooth();
         MaskBlockEnlargeSmooth();

         int dstStartBY;
         int progressOld = 0;
		 for(dstStartBY = DstMinBY(); dstStartBY + dstStepBY < DstMaxBY(); dstStartBY+=dstStepBY) {
			 if(myThread->CheckStop())
                { return false; }
			 EnlargeBlockPart(dstStartBY, dstStartBY+dstStepBY);
			 myThread->AddProgress(progressStep*float(dstStartBY+dstStepBY-progressOld));
             progressOld = dstStartBY + dstStepBY;
         }
		 if(myThread->CheckStop())
            { return false; }

		 EnlargeBlockPart(dstStartBY, DstMaxBY());
         AddRandomNew ();
		 if(FractNoise() > 0.0)
            FractModify();
         CurrentDstBlock()->Clamp01();
         WriteDstBlock();
		 myThread->AddProgress(progressStep*float(blockLen - progressOld));
      }
   }
   return true;
}

void ThColorEnlarger::ReadSrcPixel(int srcX, int srcY, Point & dstP) {
   ColorToPoint(srcImg.pixel(srcX, srcY) , dstP);
}

void ThColorEnlarger::WriteDstPixel(Point p, int dstCX, int dstCY) {
   QRgb c = qRgb(int(p.x*255.0 + 0.5), int(p.y*255.0 + 0.5),  int(p.z*255.0 + 0.5));
   dstImg->setPixel(dstCX, dstCY, c);
}

void ThColorEnlarger::AddRandomNew(void) {
   int dstBX,dstBY;
   Point p;

   if(OnlyShrinking())
      return;

   BasicArray< Point > *dstBlock = CurrentDstBlock();
   for(dstBY = DstMinBY(); dstBY<DstMaxBY() ; dstBY++) {
	  for(dstBX = DstMinBX(); dstBX<DstMaxBX() ; dstBX++) {
         float maxW;
         float w = (2.0 * RandF() - 1.0);
         w *= RandF();
		 p = dstBlock->Get( dstBX, dstBY);

         maxW = 0.5*Dither();
		 if(p.x  < maxW)
            maxW = p.x;
		 if(1.0 - p.x  < maxW)
            maxW = 1.0 - p.x;
         p.x += w*maxW*p.x;

         maxW = 0.5*Dither();
		 if(p.y  < maxW)
            maxW = p.y;
		 if(1.0 - p.y  < maxW)
            maxW = 1.0 - p.y;
         p.y += w*maxW*p.y;

         maxW = 0.5*Dither();
		 if(p.z  < maxW)
            maxW = p.z;
		 if(1.0 - p.z  < maxW)
            maxW = 1.0 - p.z;
         p.z += w*maxW*p.z;

		 dstBlock->Set(dstBX, dstBY, p);
      }
   }
}

void ThColorEnlarger::FractModify(void) {
   int dstBX,dstBY;
   Point p;

   if(OnlyShrinking() || MyFractTab()==0 || FractNoise()==0.0)
      return;

   BasicArray< Point > *dstBlock = CurrentDstBlock();
   for(dstBY = DstMinBY(); dstBY<DstMaxBY() ; dstBY++) {
	  for(dstBX = DstMinBX(); dstBX<DstMaxBX() ; dstBX++) {
         const float fractW = 0.2*FractNoise();
         float maxW;
         int dstX = dstBX + DstBlockEdgeX();
         int dstY = dstBY + DstBlockEdgeY();
		 float w = 0.03*MyFractTab()->GetT(dstX, dstY);

		 p = dstBlock->Get( dstBX, dstBY);

         maxW = fractW;
		 if(p.x  < maxW)
            maxW = p.x;
		 if(1.0 - p.x  < maxW)
            maxW = 1.0 - p.x;
         p.x += w*maxW*p.x;

         maxW = fractW;
		 if(p.y  < maxW)
            maxW = p.y;
		 if(1.0 - p.y  < maxW)
            maxW = 1.0 - p.y;
         p.y += w*maxW*p.y;

         maxW = fractW;
		 if(p.z  < maxW)
            maxW = p.z;
		 if(1.0 - p.z  < maxW)
            maxW = 1.0 - p.z;
         p.z += w*maxW*p.z;

		 dstBlock->Set(dstBX, dstBY, p);
      }
   }
}

//--------------------------------------------------------------------

bool ThColorEnlargerAlpha::Enlarge(QImage *dstI) {
   const int dstStepBY = 50;
   Timer timer0;
   int dstX, dstY;

   dstImg = dstI;
   if(dstImg->width() != OutputWidth() || dstImg->height() != OutputHeight()) {  // image not correctly allocated
      cout<<" ClipError: "<<dstImg->width()<<" "<< OutputWidth() <<" \n";
      cout<<"          : "<<dstImg->height()<<" "<< OutputHeight() <<" \n"<<flush;
      return false;
   }
   dstImg->fill(qRgba(0,0,0,0));

   if(OnlyShrinking()) {  // shrinking
      ShrinkClip();
      return true;
   }

   timer0.Clear();
   timer0.Start();

   long totalSteps;
   float progressStep=0.0;
   totalSteps  = (ClipX1() - ClipX0()) / blockLen + 1;
   totalSteps *= (ClipY1() - ClipY0()) / blockLen + 1;
   totalSteps *= blockLen;
   if(totalSteps>0)
	   progressStep = 1.0/float(totalSteps);

   for(dstY=ClipY0(); dstY<ClipY1(); dstY+=blockLen) {
	  for(dstX=ClipX0(); dstX<ClipX1(); dstX+=blockLen) {
		 if(myThread->CheckStop())
            { return false; }
		 BlockBegin(dstX, dstY);
         ReadSrcBlock();

         SrcBlockReduceNoise();
         SrcBlockSharpen();
         CalcBaseWeights();

		 if(myThread->CheckStop())
            { return false; }

         BlockEnlargeSmooth();
         MaskBlockEnlargeSmooth();

         int dstStartBY;
         int progressOld = 0;
		 for(dstStartBY = DstMinBY(); dstStartBY + dstStepBY < DstMaxBY(); dstStartBY+=dstStepBY) {
			 if(myThread->CheckStop())
                { return false; }
			 EnlargeBlockPart(dstStartBY, dstStartBY+dstStepBY);
			 myThread->AddProgress(progressStep*float(dstStartBY+dstStepBY-progressOld));
             progressOld = dstStartBY + dstStepBY;
         }
		 if(myThread->CheckStop())
            { return false; }

		 EnlargeBlockPart(dstStartBY, DstMaxBY());
         AddRandomNew ();
		 if(FractNoise() > 0.0)
            FractModify();
         CurrentDstBlock()->Clamp01();
         WriteDstBlock();
		 myThread->AddProgress(progressStep*float(blockLen - progressOld));
      }
   }
   //timer0.Stop();
   //cout<<"EnlargeTime: "<<timer0.Get()<<" \n"<<flush;
   return true;
}

void ThColorEnlargerAlpha::ReadSrcPixel(int srcX, int srcY, Point4 & dstP) {
   ColorToPoint(srcImg.pixel(srcX, srcY) , dstP);
}

void ThColorEnlargerAlpha::WriteDstPixel(Point4 p, int dstCX, int dstCY) {
   QRgb c = qRgba(int(p.x*255.0 + 0.5), int(p.y*255.0 + 0.5),  int(p.z*255.0 + 0.5), int(p.w*255.0 + 0.5));
   dstImg->setPixel(dstCX, dstCY, c);
}

void ThColorEnlargerAlpha::AddRandomNew(void) {
   int dstBX,dstBY;
   Point4 p;

   if(OnlyShrinking())
      return;

   BasicArray< Point4 > *dstBlock = CurrentDstBlock();
   for(dstBY = DstMinBY(); dstBY<DstMaxBY() ; dstBY++) {
	  for(dstBX = DstMinBX(); dstBX<DstMaxBX() ; dstBX++) {
         float maxW;
         float w = (2.0 * RandF() - 1.0);
         w *= RandF();
		 p = dstBlock->Get( dstBX, dstBY);

         maxW = 0.5*Dither();
		 if(p.x  < maxW)
            maxW = p.x;
		 if(1.0 - p.x  < maxW)
            maxW = 1.0 - p.x;
         p.x += w*maxW*p.x;

         maxW = 0.5*Dither();
		 if(p.y  < maxW)
            maxW = p.y;
		 if(1.0 - p.y  < maxW)
            maxW = 1.0 - p.y;
         p.y += w*maxW*p.y;

         maxW = 0.5*Dither();
		 if(p.z  < maxW)
            maxW = p.z;
		 if(1.0 - p.z  < maxW)
            maxW = 1.0 - p.z;
         p.z += w*maxW*p.z;

		 dstBlock->Set(dstBX, dstBY, p);
      }
   }
}

void ThColorEnlargerAlpha::FractModify(void) {
   int dstBX,dstBY;
   Point4 p;

   if(OnlyShrinking() || MyFractTab()==0 || FractNoise()==0.0)
      return;

   BasicArray< Point4 > *dstBlock = CurrentDstBlock();
   for(dstBY = DstMinBY(); dstBY<DstMaxBY() ; dstBY++) {
	  for(dstBX = DstMinBX(); dstBX<DstMaxBX() ; dstBX++) {
         const float fractW = 0.2*FractNoise();
         float maxW;
         int dstX = dstBX + DstBlockEdgeX();
         int dstY = dstBY + DstBlockEdgeY();
		 float w = 0.03*MyFractTab()->GetT(dstX, dstY);

		 p = dstBlock->Get( dstBX, dstBY);

         maxW = fractW;
		 if(p.x  < maxW)
            maxW = p.x;
		 if(1.0 - p.x  < maxW)
            maxW = 1.0 - p.x;
         p.x += w*maxW*p.x;

         maxW = fractW;
		 if(p.y  < maxW)
            maxW = p.y;
		 if(1.0 - p.y  < maxW)
            maxW = 1.0 - p.y;
         p.y += w*maxW*p.y;

         maxW = fractW;
		 if(p.z  < maxW)
            maxW = p.z;
		 if(1.0 - p.z  < maxW)
            maxW = 1.0 - p.z;
         p.z += w*maxW*p.z;

		 dstBlock->Set(dstBX, dstBY, p);
      }
   }
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------


EnlargerThread::EnlargerThread(QObject *parent, int id) {
    restartEnlarge = false;
    stopEnlarge = false;
    abort = false;
    threadId = id;
}

EnlargerThread::~EnlargerThread(void) {
    mutex.lock();
    abort = true;
    waiter.wakeAll();
    mutex.unlock();
    wait();
}


void EnlargerThread::Enlarge(const QImage & src, const EnlargeFormat & f, const EnlargeParameter & p) {
	QMutexLocker locker(&mutex);
    sourceImage = src;
    format = f;
    param  = p;
	if(f.srcWidth != src.width() || f.srcHeight != src.height()) {
       cout<<"EnlargerThread:  Enlarge: source does not fit to format.\n"<<flush;
    }

    saveAtEnd = false;
    restartEnlarge = true;

	if(!isRunning()) {
		start(QThread::LowPriority);
    }
    else {
        stopEnlarge = true;
        waiter.wakeOne();
    }
}

void EnlargerThread::EnlargeAndSave(const QImage & src, const EnlargeFormat & f, const EnlargeParameter & p,
									 const QString & dstName, int resultQuality )
{
	QMutexLocker locker(&mutex);
    sourceImage = src;
    format = f;
    param  = p;
    quality = resultQuality;

	if(f.srcWidth != src.width() || f.srcHeight != src.height()) {
       cout<<"EnlargerThread:  EnlargeAndSave: source does not fit to format.\n"<<flush;
    }

    saveAtEnd = true;
    dstFileName = dstName;
    restartEnlarge = true;

	if(!isRunning()) {
		start(QThread::LowPriority);
    }
    else {
        stopEnlarge = true;
        waiter.wakeOne();
    }
}

void EnlargerThread::run(void) {
   bool sourceHasAlpha;
   QImage *dstImg=0;
   long *dstBuffer=0;        // the data of dstImg are created in dstBuffer
   FractTab *fractTab=0;       // the plasma fractal; tab is deleted/reconstructed when scaleF changes
   float    fractTScaleF = 1.0;


   for(;;) {
      waitForRestart();
	  if(abort) {
		 if(fractTab != 0)
            delete fractTab;
         return;
      }
      mutex.lock();
      restartEnlarge = false;
      stopEnlarge = false;
      sourceHasAlpha = sourceImage.hasAlphaChannel();
      progress = 0.0;
	  emit tellProgress(0);
      mutex.unlock();

      try {
         mutex.lock();
		 if(fractTab == 0) {
			fractTab = new FractTab(format.scaleX);
            fractTScaleF =  format.scaleX ;
         }
		 else if( format.scaleX != fractTScaleF) {  // not the scaleFactor for which the fractTab was constructed
            delete fractTab;
			fractTab = new FractTab(format.scaleX);
            fractTScaleF = format.scaleX;
         }
         mutex.unlock();

         dstBuffer = new long[ (format.ClipW()+1) * (format.ClipH()+1) ];
      }
      catch (bad_alloc&)
      {
         stopEnlarge = true;
         dstBuffer = 0;
         emit badAlloc();
      }

	  if(!stopEnlarge && dstBuffer!=0) {
		 if(sourceHasAlpha)
			dstImg = new QImage((uchar*)dstBuffer, format.ClipW(), format.ClipH(), QImage::Format_ARGB32);
         else
			dstImg = new QImage((uchar*)dstBuffer, format.ClipW(), format.ClipH(), QImage::Format_RGB32);

         // Enlarge with stop/restart/abort-check and progress
		 if(!ExecEnlarge(dstImg, fractTab)) {
			if(!abort && !stopEnlarge) {  // enlarged was not aborted by user
               stopEnlarge = true;
               emit badAlloc();
			   emit enlargeEnd(threadId);
            }
         }
      }

	  if(abort) {
		 if(dstBuffer != 0) {
            delete[] dstBuffer;
			if(dstImg!=0)
               delete dstImg;
         }
		 if(fractTab != 0)
            delete fractTab;
		 emit enlargeEnd(threadId);
         return;
      }
	  if(!stopEnlarge) {     // enlarge finished, no restart/abort
		 if(saveAtEnd) {
			if(!dstImg->save(dstFileName, 0, quality)) {
               emit imageNotSaved();
            }
            else {
			   emit imageSaved(dstImg->width(), dstImg->height());
            }
         }
         else {
            QImage emitImg = *dstImg;
            emitImg.detach();  // detach from the dstBuffer created above, this is destroyed below
			emit enlargedImage(emitImg);
         }
      }
	  if(dstBuffer != 0) {
         delete[] dstBuffer;
		 if(dstImg!=0)
            delete dstImg;
      }
      dstBuffer = 0;
      dstImg    = 0;
	  emit tellProgress(100);
	  emit enlargeEnd(threadId);
   }

   if(dstBuffer != 0)
      delete[] dstBuffer;
   if(dstImg!=0)
      delete dstImg;
   if(fractTab != 0)
      delete fractTab;

}

void EnlargerThread::waitForRestart(void) {
	QMutexLocker locker(&mutex);
	while(!abort && !restartEnlarge) {
		waiter.wait(&mutex);
    }
}

bool EnlargerThread::ExecEnlarge(QImage *dstImg,  FractTab *fractTab) {
   bool resultFlag;

   if(dstImg == 0)
      return false;

   mutex.lock();
   QImage srcImg = sourceImage;
   EnlargeFormat eFormat = format;
   EnlargeParameter eParam = param;
   mutex.unlock();

   if(srcImg.hasAlphaChannel()) {
      //cout<<"Enlarge WITH ALPHA.\n"<<flush;
      ThColorEnlargerAlpha *theEnlarger=0;
      try {
		 theEnlarger = new ThColorEnlargerAlpha (srcImg, eFormat, eParam, this);
         mutex.lock();
		 if(fractTab != 0) {
			theEnlarger->SetFractTab(fractTab);
         }
         mutex.unlock();
      }
      catch (bad_alloc&)
      {
         return false;
      }

	  resultFlag = theEnlarger->Enlarge(dstImg);
      delete theEnlarger;
   }
   else {   // no alpha channel
      /*
      cout<<"Enlarge without alpha.\n"<<flush;
	  if(dstImg->hasAlphaChannel())
         cout<<"DstImg has Alpha.\n"<<flush;
      else
         cout<<"DstImg has No Alpha.\n"<<flush;
     */
      ThColorEnlarger *theEnlarger=0;
      try {
		 theEnlarger = new ThColorEnlarger (srcImg, eFormat, eParam, this);
         mutex.lock();
		 if(fractTab != 0) {
			theEnlarger->SetFractTab(fractTab);
         }
         mutex.unlock();
      }
      catch (bad_alloc&)
      {
         return false;
      }

	  resultFlag = theEnlarger->Enlarge(dstImg);
      delete theEnlarger;
   }
  return resultFlag;
}



