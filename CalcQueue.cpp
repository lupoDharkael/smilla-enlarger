/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    CalcQueue.cpp: queue for managing more than one enlarging jobs

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

#include <iostream>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QImage>
#include <QTimer>
#include <QAbstractListModel>
#include "EnlargerThread.h"
#include "formatterclass.h"
#include "CalcQueue.h"
using namespace std;

void JobListModel::setJobList( const QStringList & list ) {
   emit layoutAboutToBeChanged();
   beginRemoveRows( QModelIndex(), 0, jobList.size()-1 );
   jobList.clear();
   endRemoveRows();
   beginInsertRows( QModelIndex(), 0, list.size()-1 );
   jobList = list;
   endInsertRows();
   emit layoutChanged();
}

QVariant JobListModel::data( const QModelIndex & index, int role ) const {
   if ( !index.isValid() )
      return QVariant();

   if ( index.row() >= jobList.size() )
      return QVariant();

   if ( role == Qt::DisplayRole )
      return jobList.at( index.row() );
   else
      return QVariant();
}

Qt::ItemFlags JobListModel::flags( const QModelIndex & index ) const {
   if ( !index.isValid() )
      return Qt::ItemIsEnabled;

   return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

bool JobListModel::setData( const QModelIndex & index, const QVariant & value, int role ) {
   if ( index.isValid()  && role == Qt::EditRole ) {
      jobList.replace( index.row(), value.toString() );
      emit dataChanged( index, index );
      return true;
   }
   return false;
}

bool JobListModel::insertRows(int position, int rows, const QModelIndex & parent ) {
   beginInsertRows( QModelIndex(), position, position+rows-1 );
   for (int row = 0; row < rows; ++row) {
      jobList.insert( position, "" );
   }
   endInsertRows();
   return true;
}

bool JobListModel::removeRows(int position, int rows, const QModelIndex & parent ) {
   beginRemoveRows(QModelIndex(), position, position+rows-1 );
   for (int row = 0; row < rows; ++row) {
      jobList.removeAt( position );
   }
   endRemoveRows();
   return true;
}

//----------------------------------------------------------------------

SingleCalcJob::SingleCalcJob( FormatterClass *formatter ) {
   myThread = 0;
   progress = 0;
   srcImg   = 0;    // no image attached - use srcPath
   myFormatter = formatter->Clone();  // create own clone, delete it at end
}

SingleCalcJob::SingleCalcJob( FormatterClass *formatter, bool useClipping ) {
   myThread = 0;
   progress = 0;
   srcImg   = 0;    // no image attached - use srcPath
   myFormatter = formatter->Clone();  // create own clone, delete it at end
   if( !useClipping ) {
      myFormatter->NoClipping();
   }
}

SingleCalcJob::~SingleCalcJob( void ) {
   if( myThread != 0 )
      delete myThread;
   if( srcImg != 0 )
      delete srcImg;
   if( !Ended() )
      emit EndReached();
   delete myFormatter;    // formatter is in possession of the job ( cloned at beginning)
}

void SingleCalcJob::AttachImage( const QImage & srcI ) {
   if( srcImg != 0 )
      delete srcImg;
   srcImg = new QImage( srcI );
}

void SingleCalcJob::SetActivity( CalcJobActivity  act ) {

   if( myThread == 0 ) {
      if( Activity() != null ) {
         cout<<"SingleCalcJob: Activity!=null without thread.\n"<<flush;
         return;
      }
      if( Activity() == null && act != null  ) {
         StartEnlarge();
      }
      else
         return;
   }

   if( myThread == 0 ) {

      cout<<"SingleCalcJob: thread==0.\n"<<flush;
      return;
   }

   if( act == null ) {
      myThread->setPriority( QThread::IdlePriority );
   }
   else if( act == middle ) {
      myThread->setPriority( QThread::LowestPriority );
   }
   else if( act == high ) {
      myThread->setPriority( QThread::LowPriority );
   }
   CalcJob::SetActivity( act );
}

QString SingleCalcJob::StatusString( void ) {
   if( Status() == notStarted )
      return "";
   else if( Status() == failed )
      return " [ failed ] ";
   else if( Status() == success )
      return " [ finished ] ";
   else {
      return " [ " + QString::number( progress ) + "% ] ";
   }
}

QString SingleCalcJob::DetailedStatusString( void ) {
   if( Status() == notStarted )
      return "Job not started yet. ";
   else if( Status() == success )
      return "Job finished successfully. ";
   else if( Status() == running )
      return "Job is running ( " + QString::number( progress ) + "% done ). ";
   else if( Status() != failed )
      return "Job in unkown condition. ";

   // failed: output of error
   if( Error() == allocFailed )
      return "Job failed: Could not allocate enough memory. ";
   else if( Error() == srcNotFound )
      return "Job failed: Source file not found. ";
   else if( Error() == srcOpenFailed )
      return "Job failed: Could not open source image. ";
   else if( Error() == dstSaveFailed )
      return "Job failed: Could not save result. ";
   else
      return "Job failed. Unknown error. ";
}

QString SingleCalcJob::InfoString( void ) {
   QString infoStr;

   infoStr =  "Source:       '" + srcPath + "'\n";
   infoStr += "Destination:  '" + dstPath + "'\n";
   return infoStr;
}

// create thread, give parameters, start enlarging
void SingleCalcJob::StartEnlarge( void ) {
    QImage srcImage;

    if( srcImg != 0 ) {
       srcImage = *srcImg;
    }
    else {
       if( !QFile::exists(  srcPath ) )  {
          emit ErrorMessage( "<b>ERROR</b> calculating '"+dstName+"'. File '" + srcPath + "' does not exist." );
          SetStatus( failed );
          SetError( srcNotFound );
          return;
       }
       if( !srcImage.load( srcPath ) ) {
          emit ErrorMessage( "<b>ERROR</b> calculating '"+dstName+"'. Could not open image '" + srcPath + "'." );
          cout<<"CalcJob: Could not open image"<<srcPath.toStdString()<<" .\n"<<flush;
          SetStatus( failed );
          SetError( srcOpenFailed );
          return;
       }
       if( srcImage.hasAlphaChannel() )
          srcImage = srcImage.convertToFormat( QImage::Format_ARGB32 );
       else
          srcImage = srcImage.convertToFormat( QImage::Format_RGB32 );
   }

   if( myThread == 0 ) {
      myThread = new EnlargerThread();
      connect( myThread, SIGNAL(imageSaved(int,int)),      this, SLOT(slot_imageSaved(int,int)) );
      connect( myThread, SIGNAL(imageNotSaved()),   this, SLOT(slot_imageNotSaved()) );
      connect( myThread, SIGNAL(badAlloc()),        this, SLOT(slot_badAlloc()) );
      connect( myThread, SIGNAL(tellProgress(int)), this, SLOT(slot_getProgress(int)) );
   }

   EnlargeFormat format;
   myFormatter->CalculateFormat( srcImage.width(), srcImage.height(), format );

   QString msg;
   msg = "Started '" + dstName +"'. ";
   msg +="Zoom: ( " + QString::number(format.scaleX) + " , " + QString::number(format.scaleY) + " ). ";
   msg +="Result: " + QString::number( format.ClipW() ) + "x" + QString::number( format.ClipH() ) + ".";
   emit StatusMessage( msg );
   myThread->EnlargeAndSave( srcImage, format, param.FloatParam(), dstPath, resultQuality );
   myThread->setPriority( QThread::IdlePriority );
   SetStatus( running );
}

 void SingleCalcJob::EndEnlarge( void ) {
    if( myThread != 0 ) {
       delete myThread;
       myThread = 0;
       CalcJob::SetActivity( null );
       emit EndReached();
    }
 }

 void SingleCalcJob::slot_badAlloc( void )     {
   emit ErrorMessage( "<b>ERROR</b> calculating '"+dstName+"'. Image too big ?! Could not allocate enough memory." );
   emit BadAlloc( dstName );
   SetStatus( failed );
   SetError( allocFailed );
   EndEnlarge();
}

void SingleCalcJob::slot_imageNotSaved( void ){
   emit ErrorMessage( "<b>ERROR</b> calculating '"+dstName+"'. Could not save '" + dstPath + "'." );
   SetStatus( failed );
   SetError( dstSaveFailed );
   EndEnlarge();
}

void SingleCalcJob::slot_imageSaved( int w, int h )   {
   QString msg;
   msg = "<b>Finished</b> '"+dstName+"'. Saved to '" + dstPath + "'.<br />";
   msg += "( Size: " + QString::number( w ) + "x" + QString::number( h ) + " )";
   emit StatusMessage( msg );
   SetStatus( success );
   EndEnlarge();
}

//------------------------------------------------------------------------

DirCalcJob::DirCalcJob( FormatterClass *formatter, const QString & sPath,  const QString & dPath ) {
   QStringList filters;

   myFormatter = formatter->Clone();

   //Deactivate Clipping
   myFormatter->NoClipping();

   srcPath = sPath; dstPath = dPath;
   srcDir.setPath( srcPath );
   dstDir.setPath( dstPath );
   srcName = srcDir.dirName();
   dstName = dstDir.dirName();

   if( !dstDir.exists() ) {
      dstDir.mkpath( "./" );
   }

   filters << "*.jpg" << "*.jpeg" << "*.bmp" << "*.png" << "*.tif" << "*.tiff" << "*.gif" << "*.ppm";
   srcDir.setNameFilters(filters);
   srcDir.setSorting ( QDir::Name | QDir::IgnoreCase );
   entries = srcDir.entryList();

   numTotal    = entries.size();
   maxActive   = 0;
   numActive   = 0;
   numFinished = 0;
   numError    = 0;
   manageJobsRecursionBlock = false;
}

DirCalcJob::~DirCalcJob( void ) {
   if( !Ended() )
      emit EndReached();
   delete myFormatter;
}

void DirCalcJob::SetActivity( CalcJobActivity  act ) {
   if( act == null ) {
      maxActive = 0;
   }
   else if( act == middle ) {
      maxActive = 1;
   }
   else if( act == high ) {
      maxActive = 3;
   }
   ManageJobs();       // if there are free active places: create new jobs
   CalcJob::SetActivity( act );
}

QString DirCalcJob::StatusString( void ) {
   QString statusTxt;
   if( Status() == notStarted )
      statusTxt = "";
   else if( Status() == failed )
      statusTxt = " [ failed ] ";
   else if( Status() == success ) {
      statusTxt = " [ " + QString::number( numFinished ) + "/" + QString::number( numTotal ) + " finished";
      if( numError == 0 )
         statusTxt += " ] ";
      else if( numError == 1 )
         statusTxt += ", one error ] ";
      else if( numError>0 )
         statusTxt += ", " + QString::number( numError ) + " errors ] ";
   }
   else {
      statusTxt = " [ " + QString::number( numFinished ) + "/" + QString::number( numTotal );
      if( numError == 0 )
         statusTxt += " ] ";
      else if( numError == 1 )
         statusTxt += " , one error ] ";
      else if( numError>0 )
         statusTxt += " , " + QString::number( numError ) + " errors ] ";
   }
   return statusTxt;
}

QString DirCalcJob::DetailedStatusString( void ) {
   if( Status() == notStarted )
      return "Job not started yet. ";
   else if( Status() == success )
      return "Job finished. ";
   else if( Status() == running )
      return "Job is running ( " + QString::number( numFinished ) + " / " + QString::number( numTotal ) +"done ). ";
   else if( Status() != failed )
      return "Job in unkown condition. ";

   // failed: output of error
   if( Error() == allocFailed )
      return "Job failed: Could not allocate enough memory. ";
   else if( Error() == srcNotFound )
      return "Job failed: Source file not found. ";
   else if( Error() == srcOpenFailed )
      return "Job failed: Could not open source image. ";
   else if( Error() == dstSaveFailed )
      return "Job failed: Could not save result. ";
   else
      return "Job failed. Unknown error. ";
}

QString DirCalcJob::InfoString( void ) {
   QString infoStr;

   infoStr =  "Source:       '" + srcPath + "'\n";
   infoStr += "Destination:  '" + dstPath + "'\n";
   return infoStr;
}

void DirCalcJob::ChildJobEnded( void ) {
   numActive--;
   numFinished++;
   if( numActive < 0 ) {
      cout<<"DirCalcJob::ChildJobEnded: numActive <= 0. \n"<<flush;
   }
   ManageJobs();
   emit StatusChanged( this );
}

void DirCalcJob::ManageJobs( void ) {
   // ManageJobs might be called indirectly recursive ( by SetActivity )
   // this is avoided by the boolean variable manageJobsRecursionBlock.
   if( manageJobsRecursionBlock )
      return;
   manageJobsRecursionBlock = true;
   if( numFinished >= numTotal ) {
      if( entries.size() > 0 ) {
         cout<<"DirCalcJob::ManageJobs: All finished, but entries left.\n"<<flush;
      }
      if( !Ended() ) {
         emit StatusMessage( "<b>Finished Folder</b> '"+dstName+"'. Saved to '" + dstPath + "'." );
         emit EndReached();
         SetStatus( success );
         maxActive = 0;
         CalcJob::SetActivity( null );
      }
      return;
   }
   while( numActive < maxActive && entries.size() > 0 ) {
      NewChildJob();
   }
   manageJobsRecursionBlock = false;
}

void DirCalcJob::NewChildJob( void ) {
   if( numActive >= maxActive || entries.size() == 0 )
      return;

   QFileInfo fi;
   QString childName, childDstName;
   childName = entries.takeFirst();

   SingleCalcJob *newJob;
   newJob = new SingleCalcJob( myFormatter );

   newJob->srcName = childName;
   newJob->srcPath = srcDir.absoluteFilePath( childName );
   fi.setFile( childName );
   if( fi.suffix().compare( "gif", Qt::CaseInsensitive ) == 0 )
      childDstName = fi.completeBaseName() + "_e.png";
   else
      childDstName = fi.completeBaseName() + "_e." + fi.suffix();

   newJob->dstName = "    " + dstName + "/" + childDstName;
   newJob->dstPath = dstDir.absoluteFilePath( childDstName );

   newJob->param  = param;
   newJob->resultQuality = resultQuality;

   newJob->SetRemoveAtEnd( true );   // for tidy queue: remove finished child jobs

   connect( newJob, SIGNAL(StatusMessage(QString)),  this, SLOT(GetChildStatusMessage(QString)) );
   connect( newJob, SIGNAL(ErrorMessage(QString)),   this, SLOT(GetChildErrorMessage(QString)) );
   connect( newJob, SIGNAL(EndReached())         ,   this, SLOT(ChildJobEnded()) );
   Queue()->AddJob( newJob );
   numActive++;

   if( Status() == notStarted ) {
      SetStatus( running );
   }
   else
      emit StatusChanged( this );
}


//------------------------------------------------------------------------

CalcQueue::CalcQueue( void ) {
   finishedCount = 0;
   unfinishedCount = 0;
   UpdateProgress();
   updateTimer = new QTimer(this);
   connect( updateTimer, SIGNAL(timeout()), this, SLOT(slot_TimerUpdate()) );
   updateTimer->start(1000);
}

CalcQueue::~CalcQueue( void ) {
    delete updateTimer;
    while( jobList.size() > 0 ) {
      CalcJob *job = jobList.takeLast();
      if( job != 0 )
         delete job;
   }
}

void CalcQueue::AddJob( CalcJob *newJob ) {
   jobList.append( newJob );
   newJob->SetQueue( this );
   newJob->SetPosition( jobList.size() - 1 );
   queueDisplayModel.insertRow(  jobList.size() - 1,  QModelIndex()  );
   connect( newJob, SIGNAL(StatusChanged(CalcJob*)), this, SLOT(slot_JobStatusChanged(CalcJob*)) );
   UpdateQueue();
   UpdateProgress();
   newJob->Update();
}

bool CalcQueue::IsInQueue( const QString & dstPath ) {
   for( int a=0; a<jobList.size(); a++ ) {
      if( jobList.at( a )->DstPath() == dstPath )
         return true;
   }

   return false;
}


// queue changed: update job positions&activity, rebuild the queueDisplayModel
void CalcQueue::UpdateQueue( void ) {
   QStringList namesInQueue;

   int priorityCounter = NumCalcJobs - 1;
   for( int a=0; a<jobList.size(); a++ ) {
      CalcJob *currentJob = jobList.at( a );
      if( currentJob!=0 ) {
         currentJob->SetPosition( a );
         if( !currentJob->Ended() ) {
            if( priorityCounter == NumCalcJobs - 1 )
               currentJob->SetActivity( high );
            else if( priorityCounter == NumCalcJobs - 2 )
               currentJob->SetActivity( middle );
            else if( priorityCounter >= 0  )
               currentJob->SetActivity( low );
            else
               currentJob->SetActivity( null );
            priorityCounter -= currentJob->ThreadsUsed();  // for each running job decrease priority
         }
      }
   }

   //for( int a=0; a<jobList.size(); a++ ) {
   //   namesInQueue.append(jobList[ a ]->Name() + jobList[ a ]->StatusString() );
   //}
   //queueDisplayModel.setJobList( namesInQueue );
   //for( int a=0; a<jobList.size(); a++ ) {
   //    PrintJob( jobList.at( a ) );
   //}
   PrintAll();
}

// calculate and emit the current progress
// ( progress since last reset )
// if reset==true: forget all finished jobs
void CalcQueue::UpdateProgress( void ) {
   float unfinProgress = 0.0;
   int totalCount;

   totalCount = 0;
   unfinishedCount = 0;
   for( int a=0; a<jobList.size(); a++ ) {
      CalcJob *currentJob = jobList.at( a );
      if( currentJob!=0 ) {
         totalCount      += currentJob->Total();
         unfinishedCount += currentJob->Unfinished();
         if( !currentJob->Ended() ) {
            unfinProgress += float( currentJob->Progress() );
         }
      }
   }
   // finished count contains all finished since last reset

   float progress;
   if( finishedCount + unfinishedCount > 0 ) {
      progress = ( unfinProgress + float( finishedCount ) ) /float( finishedCount + unfinishedCount );
   }
   else {
      progress = 1.0;
   }

   emit tellProgress( int( progress*100.0 ) );
   emit tellJobCount( totalCount - unfinishedCount, totalCount );

}

void CalcQueue::PrintJob( CalcJob *job ) {
   int jobPos = job->Position();
   QModelIndex jobIdx = queueDisplayModel.index( jobPos, 0 );
   if( !jobIdx.isValid() ) {
      cout<<" CalcQueue: StatusChanged: jobIdx not valid. ( "<<jobPos<<") \n"<<flush;
      return;
   }

   //queueDisplayModel.setData( jobIdx, job->Name() + job->StatusString(), Qt::DisplayRole );
   queueDisplayModel.setData( jobIdx, job->Name() + job->StatusString(), Qt::EditRole );
}

void CalcQueue::PrintAll( void ) {
   for( int a=0; a<jobList.size(); a++ ) {
      CalcJob *job = jobList.at(a);
      QModelIndex jobIdx = queueDisplayModel.index( a, 0 );
      if( jobIdx.isValid() && job!=0 ) {
         //queueDisplayModel.setData( jobIdx, job->Name() + job->StatusString(), Qt::DisplayRole );
         queueDisplayModel.setData( jobIdx, job->Name() + job->StatusString(), Qt::EditRole );
      }
   }
}



void CalcQueue::slot_JobStatusChanged( CalcJob *job ) {
   PrintJob( job );
   if( job->Ended() ) {
      finishedCount++;
   }
   if( job->Status() != running ) {
      UpdateQueue();  // set new activities
   }
   UpdateProgress();
}

void CalcQueue::RemoveJob( CalcJob *job ) {
   if( job == 0 ) {
      cout<<"CalcQueue: RemoveJob: job == 0.\n"<<flush;
      return;
   }
   int jobPos = job->Position();
   if( jobPos<0 || jobPos>=jobList.size() ) {
      cout<<"CalcQueue: RemoveJob: job with invalid position.\n"<<flush;
      return;
   }
   queueDisplayModel.removeRow( jobPos, QModelIndex() );

   CalcJob *job1 = jobList.takeAt( jobPos );
   if( job1 != job ) {
      cout<<"CalcQueue: RemoveJob: Inconsistence in list.\n"<<flush;
   }

   UpdateQueue();
   UpdateProgress();
   delete job;
}

void CalcQueue::RemoveJob( QModelIndex jobIdx ) {
   if( !jobIdx.isValid() )
      return;
   int jobPos = jobIdx.row();
   if( jobPos<0 || jobPos>=jobList.size() ) {
      cout<<"CalcQueue: RemoveJob 2: job with invalid position.\n"<<flush;
      return;
   }
   queueDisplayModel.removeRow( jobPos, QModelIndex()  );
   CalcJob *job = jobList.takeAt( jobPos );
   UpdateQueue();
   UpdateProgress();
   if( job != 0 ) {
      delete job;
   }
}

void CalcQueue::Clear( void ) {
   while( jobList.size() > 0 ) {
      RemoveJob( jobList.last() );
   }
}

void CalcQueue::RemoveEnded( void ) {
   for( int a=jobList.size() - 1; a>=0; a--) {
      if( jobList.at(a)->Ended() ) {
         CalcJob *job = jobList.takeAt( a );
         queueDisplayModel.removeRow( a );
         if( job != 0 ) {
            delete job;
         }
      }
   }
   UpdateQueue();
   UpdateProgress();
}

// some jobs want to be removed when ended
void CalcQueue::SearchRemoveAtEnd( void ) {
   bool found = false;
   for( int a=jobList.size() - 1; a>=0; a--) {
      if( jobList.at(a)->Ended() && jobList.at(a)->RemoveAtEnd() ) {
         found = true;
         CalcJob *job = jobList.takeAt( a );
         queueDisplayModel.removeRow( a );
         if( job != 0 ) {
            delete job;
         }
      }
   }
   UpdateQueue();
}
