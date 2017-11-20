/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    Part of EnlargerDialog: Loading & Saving

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

#include <QFileDialog>
#include <QDesktopServices>    // for getting std. picture-directory
#include <QDir>
#include <QSettings>
#include <QtGui>
#include <QImageReader>

#include <iostream>

#include "EnlargerDialog.h"
#include "ui_enlargerdialog.h"
#include "previewField.h"
#include "EnlargerThread.h"
#include "ImageEnlargerCode/FractTab.h"
#include "CalcQueue.h"
#include "formatterclass.h"

using namespace std;

//
// Part of EnlargerDialog: Load & Save
//
// GetPaths
// TryOpenSource
// BatchAutomaticOpen
// SourceFromMimeData
// SetSourceDir
// SetDestDir
// AdjustDestName
// IncDestName
// BatchProcessDir
// BatchAddJob

//----------------------------------------------------------------------------

// check and possibly modify src and dst path
bool EnlargerDialog::GetPaths( QString filePath, QString & srcPath, QString & dstPath, bool & isDir ) {
   QString body,type;
   QString dstName, symLinkTarget;
   QString defaultType = DefaultType();
   QFileInfo fi;
   QDir dDir;

   dDir = dstDir;
   fi.setFile( filePath );
   if( ui->srcCheckBox->isChecked() ) {
      // use source dir as dest dir,
      // if source itself is directory use containing dir.
      dDir = fi.absoluteDir();
      if( fi.isDir() && fi.fileName().isEmpty()  )
         dDir.cdUp();
   }
   else {
      dDir = dstDir;
   }

   // test if symbolic link, if true: use link target ( but use dir of link as dstDir )
   symLinkTarget = QFile::symLinkTarget( filePath );
   if( !symLinkTarget.isEmpty() ) {  // is symLink
      filePath = symLinkTarget;  // switch to the target
   }

   if( !QFile::exists( filePath ) ) {
      PrintStatusText( "File '" + filePath + "' does not exist." );
      return false;
   }

   // switch to absolute file path
   fi.setFile( filePath );
   filePath = fi.absoluteFilePath();
   srcPath = filePath;

   if( fi.isDir() ) {
      isDir = true;
      if( fi.fileName().isEmpty() )
         dstName = fi.absoluteDir().dirName() + "_e";
      else
         dstName = fi.fileName() + "_e";
   }
   else {
      isDir = false;
      body     = fi.completeBaseName();
      type     = fi.suffix();
      QStringList typeList;
      typeList << "jpg" << "jpeg" << "bmp" << "png" << "tif" << "tiff" << "ppm" << "gif";
      if( !typeList.contains( type, Qt::CaseInsensitive ) ) {
         PrintStatusText( "File '" + filePath + "' of unsupported type < " + type + " > ." );
         return false;
      }
      if( type.toLower() == QString("gif") )
         type = QString("png");

      if( defaultType.isEmpty() ) {
         dstName =  body+"_e."+type;
      }
      else {
         dstName =  body+"_e."+defaultType;
      }

   }
   IncDestName( dstName, dDir );
   dstPath = dDir.absoluteFilePath( dstName );
   return true;
}

bool EnlargerDialog::TryOpenSource( QString filePath ) {
   QString srcPath,dstPath;
   QString srcName, dstName;
   QFileInfo fiSrc, fiDst;
   bool isDir;

   if( !GetPaths( filePath, srcPath, dstPath, isDir ) ) // check and possibly modify src and dst path
      return false;
   fiSrc.setFile( srcPath );
   fiDst.setFile( dstPath );
   srcName = fiSrc.fileName();
   dstName = fiDst.fileName();

   if( isDir ) {
      PrintStatusText( "<b>Batch:</b> Processing Folder " + srcPath + " --> " + dstPath );
      BatchProcessDir( srcPath, dstPath );
      return true;
   }
   PrintStatusText( "Loading image '" + srcName + "' ." );
   QImage sIm;
   if( !sIm.load( srcPath ) ) {
      PrintStatusText( "Could not open image '" + srcName + "' ." );
      return false;
   }

   if( sIm.hasAlphaChannel() )
      sIm = sIm.convertToFormat( QImage::Format_ARGB32 );
   else
      sIm = sIm.convertToFormat( QImage::Format_RGB32 );

   SetSource( sIm );
   SetSourceDir( fiSrc.absolutePath() );
   SetDestDir( fiDst.absolutePath() );

   FillComboBox();
   int cIdx = ui->comboBox->findText( srcName );
   if( cIdx >= 0 )
      ui->comboBox->setCurrentIndex( cIdx );

   ui->destFileEdit->setText( dstName );
   currentSrcName = srcName;
   currentSrcPath = srcPath;
   stopPreview();
   PrintStatusText( "Source image '" + srcName + "' loaded." );

   return true;
}

void EnlargerDialog::BatchAutomaticOpen( QString filePath ) {
   QString srcPath,dstPath;
   QString dstName;
   QFileInfo fiDst;
   QString messageTxt;
   bool isDir;

   if( !GetPaths( filePath, srcPath, dstPath, isDir ) ) // check and possibly modify src and dst path
      return;
   if( isDir ) {
      messageTxt = "<b>Batch:</b> Processing Folder.<br />";
      messageTxt += "Source is '"+ srcPath + "'.<br />";
      messageTxt += "Result will be '" + dstPath + "'.";
      slot_MessageToLog( messageTxt );
      BatchProcessDir( srcPath, dstPath );
      return;
   }
   fiDst.setFile( dstPath );
   messageTxt =  "<b>Batch:</b> Added Job '" + fiDst.fileName() + "'.<br />";
   messageTxt += "Source is '" + srcPath + "'.<br />";
   messageTxt += "Result will be saved as '" + dstPath + "'.";
   slot_MessageToLog( messageTxt );
   BatchAddJob( srcPath, dstPath, fiDst.fileName() );
}

// for dropping, clipboard-paste
void EnlargerDialog::SourceFromMimeData( const QMimeData *mimeData ) {
   QString statusText;

   if( mimeData->hasUrls() ) {
      QList<QUrl>  uriList;
      uriList = mimeData->urls();
      if( uriList.size() == 1 ) {
         if( TryOpenSource( uriList.at(0).toLocalFile() ) ) {
            return;
         }
      }
      else if( uriList.size() > 1 ) {
         PrintStatusText( "<b>Batch:</b> Automatically processing " + QString::number( uriList.size() ) + " Files." );
         for (int i = 0; i < uriList.size(); i++) {
            BatchAutomaticOpen( uriList.at(i).toLocalFile() );
         }
         return;
      }
   }

   if ( mimeData->hasImage() ){
      QImage image = qvariant_cast<QImage>( mimeData->imageData() );
      if ( !image.isNull() ) {   // set the image, clear comboBox
         SetSource( image );
         ui->comboBox->clear();
         currentSrcName = " < dropped / pasted > ";
         currentSrcPath = "";   // no path -> image itself has to be stored in calcJob
         ui->comboBox->addItem( currentSrcName );
         QString picDir = QDesktopServices::storageLocation ( QDesktopServices::PicturesLocation );
         srcDir.setPath( picDir );
         if( ui->srcCheckBox->isChecked() )
            SetDestDir( picDir );

         if( DefaultType().isEmpty() )
            ui->destFileEdit->setText( "dropped.jpg" );
         else
            ui->destFileEdit->setText( "dropped." + DefaultType() );

         AdjustDestName();
     }
   }
}

void EnlargerDialog::SetSourceDir( const QString & srcDirPath ) {
    srcDir.setPath( srcDirPath );
    FillComboBox();
}

void EnlargerDialog::SetDestDir( const QString & dstDirPath ) {
    dstDir.setPath( dstDirPath );
    ui->destDirLabel->setText( dstDir.dirName() );
}

// if the current dst-filename exists, then
// increment a number at end of filename
void EnlargerDialog::AdjustDestName( void ) {
   QString dstName = ui->destFileEdit->text();
   QString body, type, dstPath;
   QString defaultType = DefaultType();
   QFileInfo fi;

   if( dstName.isEmpty() ) {
      if( defaultType.isEmpty() )
         dstName = QString("enlarged.jpg");
      else
         dstName = "enlarged."+defaultType;
   }

   fi.setFile( dstName );
   type = fi.suffix();
   body = fi.completeBaseName();

   QStringList typeList;
   typeList << "jpg" << "jpeg" << "bmp" << "png" << "tif" << "tiff" <<"ppm";
   if( !typeList.contains( type, Qt::CaseInsensitive ) ) {
       type = QString("PNG");
       dstName = body+ "." + type;
       ui->destFileEdit->setText( dstName );
   }

   IncDestName( dstName, dstDir );
   dstPath = dstDir.absoluteFilePath( dstName );

   ui->destFileEdit->setText( dstName );
}

// if the current dst-filename exists, then
// increment a number at end of filename
void EnlargerDialog::IncDestName( QString & dstName , const QDir & dDir ) {
   QString body, type, path, dstPath;

   dstPath = dDir.absoluteFilePath( dstName );
   if( !dDir.exists( dstName ) && !theCalcQueue->IsInQueue( dstPath ) )
       return;

   QFileInfo fi( dstName );
   type = fi.suffix();
   if( !type.isEmpty() )
      type = "."+type;
   body = fi.completeBaseName();

   int num=0;
   QRegExp rx("[0-9]*$");   // search the number before end
   int numPos = rx.indexIn(body);
   if( numPos == body.size() )  // no number: begin with 0
      num=0;
   else {
      // find the number and increment it, cut body
      bool isOk;
      num = body.right( body.size() - numPos ).toInt( &isOk, 10 );
      if( num<0 )
          num=0;
      num++;
      body = body.left( numPos );
   }

   // count up until name is found, which does not exist in dir nor queue
   while( num < 1000 ) {
      dstName = body + QString::number(num) + type;
      dstPath = dDir.absoluteFilePath( dstName );
      if(  !dDir.exists( dstName )  && !theCalcQueue->IsInQueue( dstPath ) )
          break;
      num++;
   }
}

void EnlargerDialog::BatchProcessDir( const QString & srcPath,  const QString & dstPath ) {
   DirCalcJob *newJob;

   newJob = new DirCalcJob( currentMainFormatter, srcPath, dstPath );  // newFormatter is cloned, original can be deleted

   newJob->resultQuality = ResultQuality();
   ReadParameters( newJob->param );

   connect( newJob, SIGNAL(StatusMessage(QString)), this, SLOT(slot_MessageToLog(QString)) );
   connect( newJob, SIGNAL(ErrorMessage(QString)), this, SLOT(slot_MessageToLog(QString)) );

   theCalcQueue->ResetProgress();
   theCalcQueue->AddJob( newJob );

   ui->queueListView->clearFocus();
   ui->queueListView->setFocus();
}

void EnlargerDialog::BatchAddJob( const QString & srcPath,  const QString & dstPath, const QString & dstName ) {
   SingleCalcJob *newJob;

   newJob = new SingleCalcJob( currentMainFormatter, false );  // dont use clipping of the mainFormatter

   QFileInfo fi;
   fi.setFile( srcPath );
   newJob->srcName = fi.fileName();
   newJob->srcPath = srcPath;

   newJob->dstName = dstName;
   newJob->dstPath = dstPath;
   newJob->resultQuality = ResultQuality();

   ReadParameters( newJob->param );

   connect( newJob, SIGNAL(StatusMessage(QString)), this, SLOT(slot_MessageToLog(QString)) );
   connect( newJob, SIGNAL(ErrorMessage(QString)), this, SLOT(slot_MessageToLog(QString)) );
   connect( newJob, SIGNAL(BadAlloc(QString)), this, SLOT(BadAllocMessage(QString)) );

   theCalcQueue->ResetProgress();
   theCalcQueue->AddJob( newJob );
}
