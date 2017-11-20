/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    CalcQueue.h: queue for managing more than one enlarging jobs

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

#ifndef CALCQUEUE_H
#define CALCQUEUE_H

#include <QObject>
#include <QStringList>
#include <QDir>
#include <QAbstractListModel>
#include "ImageEnlargerCode/ConstDefs.h"
#include "ImageEnlargerCode/EnlargeParam.h"

const int NumCalcJobs = 3;   // places in the queue with activity != null

class QImage;
class QTimer;
class EnlargerThread;
class CalcQueue;
class FormatterClass;

enum CalcJobStatus   { notStarted, running, failed, success };
enum CalcJobActivity { null, low, middle, high };
enum CalcJobError    { none, allocFailed, srcNotFound, srcOpenFailed, dstSaveFailed };

class JobListModel : public QAbstractListModel {
   Q_OBJECT
private:
   QStringList jobList;

public:
   JobListModel( void ) : QAbstractListModel() {}
   void setJobList( const QStringList & list );
   int rowCount( const QModelIndex & parent = QModelIndex() ) const { return jobList.size(); }
   QVariant data( const QModelIndex & index, int role ) const;
   Qt::ItemFlags flags( const QModelIndex & index ) const;
   bool setData( const QModelIndex & index, const QVariant &value, int role = Qt::EditRole );
   bool insertRows( int position, int rows, const QModelIndex & parent = QModelIndex() );
   bool removeRows( int position, int rows, const QModelIndex & parent = QModelIndex() );
};

class CalcJob :public QObject {
   Q_OBJECT
private:
   CalcJobStatus   status;
   CalcJobActivity activity;
   CalcJobError    error;
   CalcQueue  *myQueue;
   int posInQueue;
   bool removeAtEnd;            // for child jobs of dir-calc
public:
   CalcJob( void ) : status( notStarted ), activity( null ), error( none ), posInQueue( -1 ), removeAtEnd( false ) { }
   virtual void SetActivity( CalcJobActivity act ) { activity = act; }
   virtual QString StatusString( void ) { return ""; }
   virtual QString DetailedStatusString( void ) { return ""; }
   virtual QString DstPath( void )    { return ""; }
   virtual QString Name( void )       { return "< none >"; }
   virtual QString InfoString( void ) { return ""; }

   // unfinished and total count
   // for single-calc jobs 0 or 1,
   // necessary for dir-calc jobs
   virtual int Unfinished( void ) { return 1; }
   virtual int Total( void )      { return 1; }
   virtual float   Progress( void ) { return 0.0; }
   virtual int ThreadsUsed( void )  { return 1; }

   CalcJobStatus Status( void ) { return status; }
   CalcJobError  Error ( void ) { return error; }
   bool Ended( void )  { return status == failed || status == success; }
   CalcJobActivity Activity( void ) { return activity; }
   int Position( void ) { return posInQueue; }
   void SetQueue( CalcQueue *q ) { myQueue = q; }
   void SetPosition( int p ) { posInQueue = p; }
   void Update( void ) { emit StatusChanged( this ); }
   bool IsInQueue( void ) { return myQueue!=0; }
   CalcQueue *Queue( void ) { return myQueue; }

   void SetRemoveAtEnd( bool r ) { removeAtEnd = r; }
   bool RemoveAtEnd( void ) { return removeAtEnd; }

protected:
   void SetStatus( CalcJobStatus s ) { status = s; emit StatusChanged( this ); }
   void SetError ( CalcJobError e )  { error = e; }

signals:
   void StatusChanged( CalcJob *myPtr );
   void StatusMessage( const QString & message );
   void ErrorMessage( const QString & message );
   void EndReached( void );

private slots:
private:
};

class SingleCalcJob : public CalcJob {
   Q_OBJECT

public:
   QString srcName;
   QString dstName;
   
   EnlargeParamInt param;
   QString srcPath;
   QString dstPath;

   int resultQuality;

private:
   FormatterClass *myFormatter;
   QImage *srcImg;     // a source image can be attached directly
                       // if srcImg==0, srcPath is used
   int progress;
   EnlargerThread *myThread;

public:
   SingleCalcJob( FormatterClass *formatter );
   SingleCalcJob( FormatterClass *formatter, bool useClipping );
   ~SingleCalcJob( void );
   void AttachImage( const QImage & srcI );
   void SetActivity( CalcJobActivity  act );
   QString StatusString( void );
   QString DetailedStatusString( void );
   QString InfoString( void );
   QString DstPath( void )  { return dstPath; }
   QString Name( void )     { return dstName; }

   int Unfinished( void ) { if( !Ended() ) return 1; return 0; }
   int Total( void )      { if( Ended() && RemoveAtEnd() ) return 0; return 1; }
   int ThreadsUsed( void )  { return 1; }
   float   Progress( void ) { return float( progress )*0.01; }

signals:
   void BadAlloc( const QString & dstName );

private slots:
   void slot_getProgress( int p ) { progress = p; emit StatusChanged( this ); }
   void slot_badAlloc( void );
   void slot_imageNotSaved( void );
   void slot_imageSaved( int w, int h );

private:
   void StartEnlarge( void ); // create thread, give parameters, start enlarging
   void EndEnlarge( void ); //   delete thread
   void CalculateFormat( const QImage & srcImg, EnlargeFormat & format );
};

class DirCalcJob : public CalcJob {
   Q_OBJECT

public:
   EnlargeParamInt param;

   int resultQuality;

private:
   FormatterClass *myFormatter;

   QString srcPath;
   QString dstPath;
   QString srcName;
   QString dstName;

   QDir srcDir, dstDir;
   QStringList entries;
   int maxActive;            // maximum of child jobs ( depends on activity )
   int numActive;
   int numTotal, numFinished, numError;
   bool manageJobsRecursionBlock;

public:
   DirCalcJob( FormatterClass *formatter, const QString & sPath,  const QString & dPath );
   ~DirCalcJob( void );
   void SetActivity( CalcJobActivity  act );
   QString StatusString( void );
   QString DetailedStatusString( void );
   QString InfoString( void );
   QString DstPath( void )  { return dstPath; }
   QString Name( void )     { return dstName; }
   float   Progress( void ) { return 0.0; }   // no progress in this job, active jobs are put out into queue
   int Unfinished( void )   { return entries.size(); }
   int Total( void )        { return entries.size() + numFinished; }
   int ThreadsUsed( void )  { return 0; }     // no calculations within this job, calcjobs external

signals:
   void BadAlloc( const QString & dstName );

private slots:
   void GetChildStatusMessage( const QString & message ) { emit StatusMessage( "   " + message ); }
   void GetChildErrorMessage( const QString & message )  { emit ErrorMessage ( "   " + message ); numError++; }
   void ChildJobEnded( void );

private:
   void ManageJobs( void );  // push new jobs if possible, check if end reached
   void NewChildJob( void ); // create child job from an entry, put it into the queue
};


class CalcQueue  : public QObject {
   Q_OBJECT
private:
   QList < CalcJob* > jobList;
   JobListModel queueDisplayModel;         // used for displaying the queue in a ListView
   int finishedCount;                      // all jobs finished since last progress reset
   int unfinishedCount;                    // all jobs unfinished at the moment
   QTimer *updateTimer;                    // for  clean-up and printing

public:
   CalcQueue( void );
   ~CalcQueue( void );
   void AddJob( CalcJob *newJob );
   JobListModel *DisplayModel( void ) { return &queueDisplayModel; }
   bool IsInQueue( const QString & dstPath );
   int UnfinishedJobs( void ) { return unfinishedCount; }
   void Clear( void );
   void RemoveEnded( void );
   void RemoveJob( QModelIndex jobIdx );
   void ResetProgress(void ) { finishedCount = 0; }

signals:
   void tellProgress( int p );
   void tellJobCount( int endedJobs, int totalJobs );

private slots:
   void slot_JobStatusChanged( CalcJob *job );
   void slot_TimerUpdate( void ) { SearchRemoveAtEnd(); PrintAll(); }

private:
   void RemoveJob( CalcJob *job );
   void UpdateQueue( void );    // queue changed: update job positions&activity, rebuild the queueDisplayModel
   void UpdateProgress( void );
   void PrintJob( CalcJob *job );
   void PrintAll( void );
   void SearchRemoveAtEnd( void );  // some jobs ( childs of dir-calc ) want to be removed at end
};

#endif // CALCQUEUE_H
