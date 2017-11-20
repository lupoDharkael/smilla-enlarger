/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    EnlargerDialog.h: the Qt Dialog

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

#ifndef ENLARGERDIALOG_H
#define ENLARGERDIALOG_H

#include <QtGui/QMainWindow>
#include <QDir>
#include "ImageEnlargerCode/FractTab.h"
#include "ImageEnlargerCode/EnlargeParam.h"
#include "formatterclass.h"

class QMimeData;
class QSettings;
class CropSelectRect;
class EnlargerThread;
class CalcQueue;
class PreferencesDialog;

enum DialogMode { noEnlarge, previewEnlarge, fullEnlarge };

// indices of the formatters and their widgets in the StackedWidget
const int idxZoomF    = 0;
const int idxWidthF   = 1;
const int idxHeightF  = 2;
const int idxMaxF     = 3;
const int idxStretchF = 4;
const int idxCropF    = 5;
const int idxAddBarF  = 6;

namespace Ui
{
    class EnlargerDialog;
}

class EnlargerDialog : public QMainWindow
{
   Q_OBJECT
private:
   Ui::EnlargerDialog *ui;
   EnlargerThread *previewThread;
   PreferencesDialog *preferencesD;
   DialogMode currentDialogMode;
   QString calcResultName;     // memorize the absolute path of result at start of calc.
   QString calcSrc, calcDst;   // filename of src, dst of current calc

   QDir srcDir, dstDir;
   QString currentSrcName;
   QString currentSrcPath;

   // for cropping
   CropSelectRect *cropRect;

   // for managing the various zoom-setting-possibilities
   float zoomFact;
   float aspectX,aspectY;  // zoomX = zoomFact*aspectX, zoomY = zoomFact*aspectY, aspectX*aspectY=1.0
   bool  freeAspectRatio;  // aspect ratio mode: true allows aspect ratio changes

   int newWidth,newHeight;
   int srcWidth,srcHeight;

   // formatters for the different ways of setting the output dimensions
   FixZoomFormatter   *mainZoomFormatter;
   FixWidthFormatter  *mainWidthFormatter;
   FixHeightFormatter *mainHeightFormatter;
   FixOutStretchFormatter *mainStretchFormatter;
   MaxBoundFormatter   *mainMaxFormatter;
   CropFormatter       *mainCropFormatter;
   MaxBoundBarFormatter *mainAddBarFormatter;

   FormatterClass *currentMainFormatter;  // the currently active ( selected ) formatter

   // Storing and retrieving Settings ( enlarging parameters )
   QSettings *theSettings;

   // List of Enlarge-Parameter-Settings ( names in ui->paramCOmbo )
   QList< EnlargeParamInt > paramSetVal;
   int currentParamIdx;

   // the calculation queue
   CalcQueue *theCalcQueue;

   // mini mode action
   QAction *miniModeA;


public:
   EnlargerDialog(QWidget *parent = 0);
   ~EnlargerDialog();
   bool TryOpenSource( QString fileName );
   void ZoomUseWidth ( void );
   void ZoomUseHeight( void );
   void ZoomUseWidthAndHeight( void );
   void ZoomUseZoom  ( void );
   float ZoomX( void ) { return aspectX * zoomFact; }
   float ZoomY( void ) { return aspectY * zoomFact; }
   int  AddParamSet( const QString & name, const EnlargeParamInt & p );

public slots:
   void DoPreview( void );

protected:
   void closeEvent( QCloseEvent *event );
   void dragEnterEvent( QDragEnterEvent *event );
   void dropEvent( QDropEvent *event );

private slots:
   void slot_load1( void );
   void slot_load2( void );
   void slot_comboChanged( const QString & txt );
   void slot_checkBoxChanged( int state );
   void slot_clipFormatComboChanged( const QString & txt );
   void slot_customFormatChanged( void );
   void slot_mainFormatterSelected( int idx );

   void slot_mainZoomFormatterSet ( int zI );
   void slot_mainWidthFormatterSet( int w );
   void slot_mainHeightFormatterSet( int h );
   void slot_mainStretchFormatterSetW( int w );
   void slot_mainStretchFormatterSetH( int w );
   void slot_mainMaxFormatterSetW( int w );
   void slot_mainMaxFormatterSetH( int w );
   void slot_mainCropFormatterSetW( int w );
   void slot_mainCropFormatterSetH( int w );
   void slot_mainCropFormatterSetStretch( double v );
   void slot_mainAddBarFormatterSetW( int w );
   void slot_mainAddBarFormatterSetH( int w );

   void slot_queueCalc( void );
   void slot_showPreview( const QImage & result );
   void slot_showPreviewProgress( int val );
   void slot_paste( void );
   void stopPreview( void );
   void slot_applyClipping( void );
   void slot_clippingChanged( void );
   void slot_ShowHelp( void );
   void slot_ShowPreviewTab( void );
   void slot_TabChanged( int idx );
   void InfoMessage( void );
   void HelpMessageMore( void );
   void BadAllocMessage( const QString & dstName );
   void slot_AddParam( void );
   void slot_DelParam( void );
   void slot_ParamCheckBoxChanged( int state );
   void slot_SetParamSlidersFromCombo( int idx );
   void slot_ShowJobCount( int endedJobs, int totalJobs );
   void slot_MessageToLog( const QString & message );
   void slot_QueueRemoveSelected( void );
   void slot_QueueClear( void );
   void slot_QueueRemoveEnded( void );
   void slot_MiniMode( bool b);
   void slot_ShowPref( void );
   void slot_PrefChanged( void );

private:
   void MenuBarSetup( void );

   void ReadSettings( void );
   void WriteSettings( void );
   QString FindSettingsFile( void );

   void MainFormatterUpdate( void );
   void ResetDialog( void );
   void SetSource(const  QImage & src );
   void ReadParameters( EnlargeParamInt & param );
   void FillComboBox( void );
   void SourceFromMimeData( const QMimeData *mimeData ); // for dropping, clipboard-paste
   void SetSourceDir( const QString & srcName );
   void SetDestDir( const QString & dstName );
   bool FileExistsMessage( const QString & fileName, const QString & dirName );
   bool AbortCalcMessage( int unfinishedJobs  );
   void AdjustDestName( void );
   void IncDestName( QString & dstName , const QDir & dDir );
   bool GetPaths( QString filePath, QString & srcPath, QString & dstPath, bool & isDir ); // check and modify filePath
   void SetToolTip( QWidget *widget, const QString & text );
   void SetToolTip( QWidget *w1,  QWidget *w2, const QString & text );
   void PrintStatusText( QString statusText );

 //int  AddParamSet( const QString & name, const EnlargeParamInt & p ); now public
   void DelParamSet( int idx );
   void ChangeParamSet( int idx, const EnlargeParamInt & p );
   void GetParamSliders( EnlargeParamInt & p );
   void SetParamSliders( const EnlargeParamInt & p );

   void BatchAutomaticOpen( QString filePath );
   void BatchAddJob( const QString & srcPath,  const QString & dstPath, const QString & dstName );
   void BatchProcessDir( const QString & srcPath,  const QString & dstPath );

   QString DefaultType( void );
   int     ResultQuality( void );
};

#endif // ENLARGERDIALOG_H
