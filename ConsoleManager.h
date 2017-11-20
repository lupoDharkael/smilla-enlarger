/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    ConsoleManager.h: command line functionality

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
#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <QString>
#include <QObject>
#include <QImage>
#include <iostream>

#include "ArgumentParser.h"
#include "ImageEnlargerCode/EnlargeParam.h"

using namespace std;

class EnlargerThread;
class EnlargerDialog;

// QObject for console output
class EnlargerOut : public QObject {
   Q_OBJECT

   QString dstName;
   bool ended;
public:
   EnlargerOut() : QObject(), ended(false) {}
   ~EnlargerOut() {}
   void SetName(const QString &name) { dstName = name; }
   void StartMessage() {
	   cout << "Calculating '" << dstName.toStdString() << "' - " << flush;
   }

public slots:
	void PrintProgress(int  p) {
	   if(!ended) {
		   cout << "\rCalculating '" << dstName.toStdString() << "' - [ "<<p<<"% ]   "
				<<flush;
	   }
	}
	void badAlloc() {
		cout << "\n[ ERROR ]\nCould not allocate enough memory for '" <<
				dstName.toStdString() << "'.\n" << flush;
		ended=true;
	}
	void imageNotSaved() {
		cout << " \n[ ERROR ] - Could not save image '" << dstName.toStdString()<<"'.\n"<<flush; ended=true;  }
	void imageSaved(int w, int h) {
		cout << " OK.\n" << flush;
		ended=true;
	}
 };

class ConsoleManager : public QObject {
   Q_OBJECT

   bool parseError;
   ArgumentParser myParser;
   IntegerOption oZoom;
   IntegerOption oWidth,   oHeight;

   IntegerOption oSharp,   oFlat;
   IntegerOption oDeNoise, oPreSharp;
   IntegerOption oDither,  oFNoise;

   IntegerOption oQuality;

   StringOption oOutput;
   StringOption oOutputFolder;
   BasicOption  oHelp, oInteractive;
   BasicOption  oFormatCover, oFormatFit;
   BasicOption  oFormatCrop, oFormatBars;

   EnlargerOut myEnOut;

   QString dstName;

public:
   ConsoleManager( int argc, char *argv[] );
   ~ConsoleManager( void ) {}
   bool UseGUI( void );
   void SetupEnlargerDialog ( EnlargerDialog & theDialog );
   bool StartConsoleEnlarge  ( EnlargerThread & myThread );
   bool TryOpenSource( QString filename, QImage & srcImage );
   void IncDestName( QString & dstName ,  const QString & dstDirPath  );
   void PrintHelp( void );

};


#endif // CONSOLEMANAGER_H
