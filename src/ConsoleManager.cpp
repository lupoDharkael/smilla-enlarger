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


#include <QString>
#include <QObject>
#include <QFile>
#include <QImage>
#include <QApplication>
#include <iostream>

#include "ConsoleManager.h"
#include "ArgumentParser.h"
#include "EnlargerDialog.h"
#include "EnlargerThread.h"
#include "formatterclass.h"

using namespace std;

ConsoleManager::ConsoleManager(int argc, char *argv[]) : QObject() {
   oZoom.Set    (&myParser, "-z", "-zoom"); oZoom.SetRange (1, 100000);   oZoom.SetDefault(200);
   oWidth.Set   (&myParser, "-width"     ); oWidth.SetRange(1, 1000000);
   oHeight.Set  (&myParser, "-height"    ); oHeight.SetRange(1, 1000000);

   oSharp.Set   (&myParser, "-sharp"     ); oSharp.SetRange(0, 100);      oSharp.SetDefault   (80);
   oFlat.Set    (&myParser, "-flat"      ); oFlat.SetRange(0, 100);       oFlat.SetDefault    (20);
   oDeNoise.Set (&myParser, "-deNoise"   ); oDeNoise.SetRange(0, 100);    oDeNoise.SetDefault (20);
   oPreSharp.Set(&myParser, "-preSharp"  ); oPreSharp.SetRange(0, 100);   oPreSharp.SetDefault( 0);
   oDither.Set  (&myParser, "-dither"    ); oDither.SetRange(0, 100);     oDither.SetDefault  (10);
   oFNoise.Set  (&myParser, "-fNoise"    ); oFNoise.SetRange(0, 100);     oFNoise.SetDefault  ( 0);

   oQuality.Set (&myParser, "-quality"   ); oQuality.SetRange(0, 100);    oQuality.SetDefault  (90);
   oOutput.Set  (&myParser, "-o");
   oOutputFolder.Set  (&myParser, "-saveto");

   oHelp.Set(&myParser, "-h" , "-help");
   oInteractive.Set(&myParser, "-i");
   oFormatCover.Set(&myParser, "-cover");
   oFormatFit. Set(&myParser, "-fit");
   oFormatCrop.Set(&myParser, "-coverandcrop");
   oFormatBars.Set(&myParser, "-fitandbars");
   parseError = false;
   if(!myParser.Parse(argc, argv)) {
      cout<<"Error parsing command line arguments. \n"<<flush;
      PrintHelp();
      parseError = true;
      return;
   }
   if(oHelp.IsThere())
      PrintHelp();

   // informations are now saved in option objects and argumentParser (otherArguments)
   // can be used in SetupEnlargerDialog, RunConsoleEnlarge
}

// decide if GUI or Console mode is used
bool ConsoleManager::UseGUI(void) {
   if(parseError) {
      return false;
   }
   if(oInteractive.IsThere()) {
      return true;
   }
   if(myParser.NonOptionArguments().isEmpty()) {    // no file given for processing -> GUI mode
      return true;
   }
   else if(!oZoom.IsThere() && !oWidth.IsThere() && !oHeight.IsThere()) { // no output dimensions
      cout<<"No output dimensions given. Starting in interactive mode.\n"<<flush;
      return true;
   }
   else if(myParser.OptionsFound()) {               // options & file -> Console mode
      return false;
   }
   return true;
}

void ConsoleManager::SetupEnlargerDialog (EnlargerDialog & theDialog) {
   EnlargeParamInt param;

   if(oSharp.IsThere()   || oFlat.IsThere()     || oDither.IsThere() ||
	   oDeNoise.IsThere() || oPreSharp.IsThere() || oFNoise.IsThere()   ) {
      param.sharp =      oSharp.Value();
      param.flat  =      oFlat.Value();
      param.dither =     oDither.Value();
      param.deNoise =    oDeNoise.Value();
      param.preSharp =   oPreSharp.Value();
      param.fractNoise = oFNoise.Value();

	  theDialog.AddParamSet("console", param);
   }

   if(!myParser.NonOptionArguments().isEmpty()) {
	  theDialog.TryOpenSource(myParser.NonOptionArguments().at(0));
   }
   theDialog.DoPreview();
}

bool ConsoleManager::StartConsoleEnlarge  (EnlargerThread & myThread) {
	if(parseError) {
       cout<<"Parse error, aborting.\n"<<flush;
       return false;
    }
	if(myParser.NonOptionArguments().isEmpty()) {
       cout<<"No filename given, aborting.\n"<<flush;
       return false;
    }

    QImage srcImage;
	if(!TryOpenSource(myParser.NonOptionArguments().at(0), srcImage)) {
       return false;
    }
	if(oOutput.IsThere()) {
       dstName = oOutput.Value();
    }
	myEnOut.SetName(dstName);

	connect(&myThread, SIGNAL(enlargeEnd(int)),   qApp,     SLOT(quit()));
	connect(&myThread, SIGNAL(imageNotSaved()),   &myEnOut, SLOT(imageNotSaved()));
	connect(&myThread, SIGNAL(imageSaved(int,int)),      &myEnOut, SLOT(imageSaved(int,int)));
	connect(&myThread, SIGNAL(tellProgress(int)), &myEnOut, SLOT(PrintProgress(int)));
	connect(&myThread, SIGNAL(badAlloc()),        &myEnOut, SLOT(badAlloc()));

    EnlargeFormat format;
    EnlargeParamInt param;

    param.sharp =      oSharp.Value();
    param.flat  =      oFlat.Value();
    param.dither =     oDither.Value();
    param.deNoise =    oDeNoise.Value();
    param.preSharp =   oPreSharp.Value();
    param.fractNoise = oFNoise.Value();

    format.srcWidth  = srcImage.width();
    format.srcHeight = srcImage.height();

	if(oZoom.IsThere()) {
	   format.SetScaleFact(float(oZoom.Value())*0.01);
    }

	float sx =  float(oWidth.Value() ) / float(srcImage.width());
	float sy =  float(oHeight.Value()) / float(srcImage.height());
	if(oWidth.IsThere() && !oHeight.IsThere()) {
	   format.SetScaleFact(sx);
    }
	else if(!oWidth.IsThere() && oHeight.IsThere()) {
	   format.SetScaleFact(sy);
    }
	else if(oWidth.IsThere() && oHeight.IsThere()) {
	   if(oFormatFit.IsThere()) {
           float s = sx;
		   if(sy < s)
              s =sy;
		   format.SetScaleFact(s);
       }
	   else if(oFormatCover.IsThere()) {
          float s = sx;
		  if(sy > s)
             s =sy;
		  format.SetScaleFact(s);
       }
	   else if(oFormatCrop.IsThere()) {
		  CropFormatter myFormatter(oWidth.Value(), oHeight.Value());
		  myFormatter.CalculateFormat(srcImage.width(), srcImage.height(), format);
       }
	   else if(oFormatBars.IsThere()) {
		  MaxBoundBarFormatter myFormatter(oWidth.Value(), oHeight.Value());
		  myFormatter.CalculateFormat(srcImage.width(), srcImage.height(), format);
       }
       else {
		  format.SetScaleFact(sx, sy);
       }
    }

    myEnOut.StartMessage();
	myThread.EnlargeAndSave(srcImage, format, param.FloatParam(), dstName, oQuality.Value());
    return true;
}


bool ConsoleManager::TryOpenSource(QString fileName, QImage & srcImage) {
   QString dstDirPath,body,type,typeL;
   QString symLinkTarget, symLinkPath;
   bool isSymLink = false;

   // test if symbolic link, if true: use link target (but use dir of link as dstDir)
   symLinkTarget = QFile::symLinkTarget(fileName);
   if(!symLinkTarget.isEmpty()) {  // is symLink
      isSymLink = true;
	  QFileInfo fi(fileName);
      symLinkPath = fi.absolutePath();
      fileName = symLinkTarget;  // switch to the target
   }

   if(!QFile::exists(fileName)) {
      cout<<"Source file '" + fileName.toStdString() + "' does not exist.\n"<<flush;
      return false;
   }

   // switch to absolute file path
   QFileInfo fi(fileName);
   fileName = fi.absoluteFilePath();
   type = fi.suffix();
   body = fi.completeBaseName();
   if(oOutputFolder.IsThere()) {
      dstDirPath = oOutputFolder.Value();
   }
   else {
      dstDirPath = fi.absolutePath();
   }
   QStringList typeList;
   typeList << "jpg" << "jpeg" << "bmp" << "png" << "tif" << "tiff" << "ppm" << "gif";
   if(!typeList.contains(type, Qt::CaseInsensitive)) {
      cout<<"Source file '" + fileName.toStdString() + "' of unsupported type < " + type.toStdString() + " >.\n"<<flush;
      return false;
   }

   if(!srcImage.load(fileName)) {
      cout<<"Could not open image '" + fileName.toStdString() + "'.\n"<<flush;
      return false;
   }
   if(srcImage.hasAlphaChannel())
	  srcImage = srcImage.convertToFormat(QImage::Format_ARGB32);
   else
	  srcImage = srcImage.convertToFormat(QImage::Format_RGB32);

   if(type.toLower() == QString("gif"))
      type = QString("png");
   dstName = body+"_e."+type;
   IncDestName(dstName, dstDirPath);
   QDir dDir(dstDirPath);
   dstName = dDir.absoluteFilePath(dstName);

   return true;
}

// if the current dst-filename exists, then
// increment a number at end of filename
void ConsoleManager::IncDestName(QString & dstName ,  const QString & dstDirPath ) {
   QString body, type, path, dstPath;
   QDir dDir(dstDirPath);

   dstPath = dDir.absoluteFilePath(dstName);
   if(!dDir.exists(dstName))
       return;

   QFileInfo fi(dstName);
   type = fi.suffix();
   if(!type.isEmpty())
      type = "."+type;
   body = fi.completeBaseName();

   int num=0;
   QRegExp rx("[0-9]*$");   // search the number before end
   int numPos = rx.indexIn(body);
   if(numPos == body.size())  // no number: begin with 0
      num=0;
   else {
      // find the number and increment it, cut body
      bool isOk;
	  num = body.right(body.size() - numPos).toInt(&isOk, 10);
	  if(num<0)
          num=0;
      num++;
	  body = body.left(numPos);
   }

   // count up until name is found, which does not exist in dir nor queue
   while(num < 1000) {
      dstName = body + QString::number(num) + type;
	  dstPath = dDir.absoluteFilePath(dstName);
	  if( !dDir.exists(dstName))
          break;
      num++;
   }
}

void ConsoleManager::PrintHelp(void) {
   cout<<"\n";
   cout<<"Usage:\n\n";
   cout<<"SmillaEnlarger [ < sourcename > ] [ -options... ]\n";
   cout<<"   with options \n";
   cout<<"   -z <number>  / -zoom <number> \n";
   cout<<"       Set zoom-factor to <number> percent (integer value).\n";
   cout<<"   -o <filename>   \n";
   cout<<"       Write result to file <filename> .\n";
   cout<<"   -saveto <foldername>   \n";
   cout<<"       Write results into folder <foldername> .\n";
   cout<<"\n";
   cout<<"Output Dimensions: \n";
   cout<<"   -width < sizex > and -height < sizey >   \n";
   cout<<"       set size of resulting image.\n";
   cout<<"   If both width and height are given, aspect ratio is changed by default.\n";
   cout<<"   Additionally, if you have set -width AND -height ,\n";
   cout<<"   you can set one of the following options: \n";
   cout<<"\n";
   cout<<"   -fit \n";
   cout<<"       Fit output inside the given rectangle.\n";
   cout<<"   -fitandbars \n";
   cout<<"       Fit output inside the given rectangle, \n";
   cout<<"       fill up with black margins.\n";
   cout<<"   -cover \n";
   cout<<"       Cover the given rectangle.\n";
   cout<<"   -coverandcrop \n";
   cout<<"       Cover the given rectangle, cut away the overlapping parts.\n";
   cout<<"\n";
   cout<<"Enlarge Parameters: \n";
   cout<<"   -sharp < n >,   -flat < n >,     -dither < n > \n";
   cout<<"   -deNoise < n >, -preSharp < n >, -fNoise < n > \n";
   cout<<"       Set the enlarge parameters with integer numbers < n > between 0 and 100.\n";
   cout<<"    \n";
   cout<<"\n";
   cout<<"   -quality <number>   \n";
   cout<<"       Set image quality of the result.\n";
   cout<<"   -h / -help \n";
   cout<<"       Print this help.\n";
   cout<<"   -i \n";
   cout<<"       Start in interactive mode.\n";
   cout<<flush;
}

