/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    EnlargerThread.h: things necessary for putting the enlarging into own Qt-Thread

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

#ifndef ENLARGERTHREAD_H
#define ENLARGERTHREAD_H


#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>

#include "ImageEnlargerCode/EnlargerTemplate.h"
#include "ImageEnlargerCode/EnlargeParam.h"

class EnlargerThread;
class FractTab;

class ThColorEnlarger : public BasicEnlarger<Point> {

   EnlargerThread *myThread;

   QImage srcImg, *dstImg;

   Timer t1,t2,tTotal;


public:
   ThColorEnlarger( const QImage & srcI, const EnlargeFormat & format, const EnlargeParameter & param, EnlargerThread *thread)
	  :  BasicEnlarger<Point> (format, param), myThread(thread)
   {
      srcImg = srcI;
   }

   // Enlarge can be stopped by thread, gives progress to thread
   bool Enlarge(QImage *dstI);

   // those have to be implemented for communication between real src/dst and BasicEnlarger
   void ReadSrcPixel(int srcX, int srcY, Point & dstP);
   void WriteDstPixel(Point p, int dstCX, int dstCY);
   void AddRandomNew(void);
   void FractModify(void);
   void ColorToPoint(QRgb c, Point & p) {
	  p.x = float(qRed  (c))*(1.0/255.0);
	  p.y = float(qGreen(c))*(1.0/255.0);
	  p.z = float(qBlue (c))*(1.0/255.0);
   }
};

class ThColorEnlargerAlpha : public BasicEnlarger<Point4> {

   EnlargerThread *myThread;

   QImage srcImg, *dstImg;

   Timer t1,t2,tTotal;


public:
   ThColorEnlargerAlpha( const QImage & srcI, const EnlargeFormat & format, const EnlargeParameter & param, EnlargerThread *thread)
	  :  BasicEnlarger<Point4> (format, param), myThread(thread)
   {
      srcImg = srcI;
   }

   // Enlarge can be stopped by thread, gives progress to thread
   bool Enlarge(QImage *dstI);

   // those have to be implemented for communication between real src/dst and BasicEnlarger
   void ReadSrcPixel(int srcX, int srcY, Point4 & dstP);
   void WriteDstPixel(Point4 p, int dstCX, int dstCY);
   void AddRandomNew(void);
   void FractModify(void);
   void ColorToPoint(QRgb c, Point4 & p) {
	  p.x = float(qRed  (c))*(1.0/255.0);
	  p.y = float(qGreen(c))*(1.0/255.0);
	  p.z = float(qBlue (c))*(1.0/255.0);
	  p.w = float(qAlpha(c))*(1.0/255.0);
   }
};



class EnlargerThread : public QThread {
    Q_OBJECT
private:
    QMutex mutex;  // protects the following data
    QImage sourceImage;
    float scaleF;
    EnlargeFormat    format;
    EnlargeParameter param;
    int quality;             // result image quality for QImage::save
    float progress;

    bool stopEnlarge;
    bool restartEnlarge;
    bool abort;
    bool saveAtEnd;
    QString dstFileName;

    QWaitCondition waiter;

    int threadId;   // ID which may be given at thread-creation,
                    // used in queue-management, returned in enlargeEnd signal

public:
	EnlargerThread(QObject *parent = 0, int id=0);
	~EnlargerThread(void);

	void StopEnlarge(void) { QMutexLocker locker(&mutex); stopEnlarge = true; restartEnlarge = false; }
	void Enlarge(const QImage & src, const EnlargeFormat & f, const EnlargeParameter & p);
	void EnlargeAndSave(const QImage & src, const EnlargeFormat & f, const EnlargeParameter & p,
						 const QString & dstName, int resultQuality);
	void SetParameter(const EnlargeParameter & p) { QMutexLocker locker(&mutex); param = p; }

	bool AddProgress(float pAdd) {
		QMutexLocker locker(&mutex);
        progress += pAdd;
		if(progress>1.0)
            progress=1.0;
		emit tellProgress(int(progress*100.0));

        return true;
    }

	float Progress(void) {
		QMutexLocker locker(&mutex);
        return progress;
    }

	bool CheckStop(void) { QMutexLocker locker(&mutex); return abort || stopEnlarge; }

protected:
	void run(void);

signals:
	void tellProgress(int  p);
	void enlargedImage(const QImage & result);
	void badAlloc(void);
	void imageNotSaved(void);
	void imageSaved(int w, int h);
	void enlargeEnd(int myId);

private:
	void waitForRestart(void);
	bool ExecEnlarge(QImage *dstImg, FractTab *fractTab);
};

#endif // ENLARGERTHREAD_H
