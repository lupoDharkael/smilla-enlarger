/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    EnlargerDialog.cpp: the Qt Dialog

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

#include <iostream>
#include "EnlargerDialog.h"
#include "ui_enlargerdialog.h"
#include "previewField.h"
#include "ClipRect.h"
#include "EnlargerThread.h"
#include "ImageEnlargerCode/FractTab.h"
#include "CalcQueue.h"
#include "Preferences.h"
#include "formatterclass.h"

using namespace std;

EnlargerDialog::EnlargerDialog(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::EnlargerDialog),
      previewThread( new EnlargerThread ),
      preferencesD( new PreferencesDialog() )
{
    setUpdatesEnabled( false );
    ui->setupUi(this);
    preferencesD->hide();
    MenuBarSetup();
    setCentralWidget( ui->centralwidget );
    zoomFact = 1.0;
    aspectX = aspectY = 1.0;
    freeAspectRatio = false;

    mainZoomFormatter    = new FixZoomFormatter( 5.0 );
    mainWidthFormatter   = new FixWidthFormatter ( 1000 );
    mainHeightFormatter  = new FixHeightFormatter( 1000 );
    mainStretchFormatter = new FixOutStretchFormatter( 1000, 1000 );
    mainMaxFormatter     = new MaxBoundFormatter( 1000, 1000 );
    mainCropFormatter    = new CropFormatter( 1000, 1000 );
    mainAddBarFormatter  = new MaxBoundBarFormatter( 1000, 1000 );
    currentMainFormatter = mainZoomFormatter;

    cropRect = new CropSelectRect( 100, 100 );
    ui->selectField->SetCropRect( cropRect );
    ui->selectField->setMouseTracking( true );
    ui->previewField->setMouseTracking( true );

    // create the calculation queue
    theCalcQueue = new CalcQueue();
    ui->queueListView->setModel( theCalcQueue->DisplayModel() );

    QString picDir = QDesktopServices::storageLocation ( QDesktopServices::PicturesLocation );
    srcDir.setPath( picDir );
    SetDestDir( picDir );

    ui->srcCheckBox->setChecked( true );
    ui->load2Button->setEnabled( false );

    // only visible if custom is selected
    ui->cropFormatXBox->setVisible( false );
    ui->cropFormatYBox->setVisible( false );
    ui->cropFormatLabel->setVisible( false );

    connect( ui->load1Button,   SIGNAL(clicked()), this, SLOT(slot_load1()) );
    connect( ui->load2Button,   SIGNAL(clicked()), this, SLOT(slot_load2()) );
    connect( ui->comboBox,      SIGNAL(activated ( const QString & )),
             this, SLOT(slot_comboChanged( const QString & )) );
    connect( ui->clipFormatCombo,   SIGNAL(activated ( const QString & )),
             this, SLOT(slot_clipFormatComboChanged( const QString & )) );
    connect( ui->formatterCombo,   SIGNAL(currentIndexChanged (int)),
             ui->formatterStackedWidget, SLOT(setCurrentIndex(int)) );
    connect( ui->formatterStackedWidget,   SIGNAL(currentChanged (int)),
             this, SLOT(slot_mainFormatterSelected(int)) );
    connect( ui->srcCheckBox,   SIGNAL(stateChanged( int ) ), this, SLOT(slot_checkBoxChanged( int ) ) );
    connect( ui->previewButton, SIGNAL(clicked()), this, SLOT(DoPreview()) );
    connect( ui->testButton, SIGNAL(clicked()), this, SLOT(slot_ShowPreviewTab() ) );
    connect( ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_TabChanged(int)) );

    connect( ui->calcButton,    SIGNAL(clicked()), this, SLOT(slot_queueCalc()) );

    connect( ui->thumbField,   SIGNAL(selectionChanged(float,float)),
             ui->previewField,  SLOT(moveSelection( float, float ) ) );
    connect( ui->previewField,   SIGNAL(selectionChanged(float,float)),
             ui->thumbField,  SLOT(moveSelection( float, float ) ) );
    connect( ui->previewField,   SIGNAL(selectionChanged(float,float)),
             this,  SLOT(stopPreview() ) );
    connect( ui->cropButton, SIGNAL(clicked()), this, SLOT(slot_applyClipping()) );
    connect( ui->selectField,   SIGNAL(clippingChanged()),
             this, SLOT(slot_clippingChanged()) );

    //--------- ToolTips ---------------------------------
    QString tipStr;
    tipStr =  "Draw or change a cropping frame with left mouse button. \n";
    tipStr += "If a cropping frame is open, its contents will be enlarged.\n";
    tipStr += "Otherwise, if no region is selected, the complete input image will be enlarged.\n";
    SetToolTip( ui->selectField, tipStr );

    tipStr =  QString(" Preview - shows a ( small ) part of the result \n");
    tipStr += QString(" in the desired magnification. \n" );
    tipStr += QString(" Move it with left mouse-button ");
    SetToolTip( ui->previewField, tipStr );

    //--------- End of ToolTips ---------------------------------

    // calc & preview Threads
    connect( theCalcQueue,      SIGNAL( tellProgress(int) ), ui->progressBar, SLOT( setValue(int) ) );
    connect( theCalcQueue,      SIGNAL(tellJobCount(int,int)), this, SLOT(slot_ShowJobCount(int,int)) );

    connect( previewThread,        SIGNAL( tellProgress(int) ), this, SLOT( slot_showPreviewProgress(int) ) );
    connect( previewThread,        SIGNAL( enlargedImage(QImage) ), this, SLOT(slot_showPreview(QImage)) );

    // Sliders & Boxes
    const int zoomMin     = 1,   zoomMax      = 3000;
    const int sharpMin    = 0,   sharpMax     = 100;
    const int flatMin     = 0,   flatMax      = 100;
    const int ditherMin   = 0,   ditherMax    = 100;
    const int deNoiseMin  = 0,   deNoiseMax   = 100;
    const int preSharpMin = 0,   preSharpMax  = 100;
    const int fractNoiseMin=0,   fractNoiseMax= 100;
    ui->mainWidthBox ->setRange  ( 1, 100000 );
    ui->mainHeightBox->setRange  ( 1, 100000 );
    ui->mainStretchWBox->setRange( 1, 100000 );
    ui->mainStretchHBox->setRange( 1, 100000 );
    ui->mainMaxWBox->setRange( 1, 100000 );
    ui->mainMaxHBox->setRange( 1, 100000 );
    ui->mainCropWBox->setRange( 1, 100000 );
    ui->mainCropHBox->setRange( 1, 100000 );
    ui->mainAddBarWBox->setRange( 1, 100000 );
    ui->mainAddBarHBox->setRange( 1, 100000 );

    ui->mainZoomBox->setRange( zoomMin, zoomMax );
    ui->mainZoomSlider->setRange( zoomMin, zoomMax );
    ui->mainZoomBox->setSuffix( "%" );
    connect( ui->mainZoomSlider,  SIGNAL(valueChanged(int)), ui->mainZoomBox, SLOT(setValue(int)) );
    connect( ui->mainZoomBox, SIGNAL(valueChanged(int)), ui->mainZoomSlider,  SLOT(setValue(int)) );
    connect( ui->mainZoomBox,     SIGNAL(valueChanged(int)), this,  SLOT(slot_mainZoomFormatterSet(int)) );
    connect( ui->mainWidthBox,    SIGNAL(valueChanged(int)), this,  SLOT(slot_mainWidthFormatterSet(int)) );
    connect( ui->mainHeightBox,   SIGNAL(valueChanged(int)), this,  SLOT(slot_mainHeightFormatterSet(int)) );
    connect( ui->mainStretchWBox, SIGNAL(valueChanged(int)), this,  SLOT(slot_mainStretchFormatterSetW(int)) );
    connect( ui->mainStretchHBox, SIGNAL(valueChanged(int)), this,  SLOT(slot_mainStretchFormatterSetH(int)) );
    connect( ui->mainMaxWBox,     SIGNAL(valueChanged(int)), this,  SLOT(slot_mainMaxFormatterSetW(int)) );
    connect( ui->mainMaxHBox,     SIGNAL(valueChanged(int)), this,  SLOT(slot_mainMaxFormatterSetH(int)) );
    connect( ui->mainCropWBox,    SIGNAL(valueChanged(int)), this,  SLOT(slot_mainCropFormatterSetW(int)) );
    connect( ui->mainCropHBox,    SIGNAL(valueChanged(int)), this,  SLOT(slot_mainCropFormatterSetH(int)) );
    connect( ui->mainCropStretchBox,SIGNAL(valueChanged(double)), this,  SLOT(slot_mainCropFormatterSetStretch(double)) );
    connect( ui->mainAddBarWBox,  SIGNAL(valueChanged(int)), this,  SLOT(slot_mainAddBarFormatterSetW(int)) );
    connect( ui->mainAddBarHBox,  SIGNAL(valueChanged(int)), this,  SLOT(slot_mainAddBarFormatterSetH(int)) );

    connect( ui->cropFormatXBox, SIGNAL(valueChanged(int)), this,  SLOT(slot_customFormatChanged()) );
    connect( ui->cropFormatYBox, SIGNAL(valueChanged(int)), this,  SLOT(slot_customFormatChanged()) );

    ui->sharpSpinBox->setRange( sharpMin, sharpMax );
    ui->sharpSlider ->setRange( sharpMin, sharpMax );
    connect( ui->sharpSlider,  SIGNAL(valueChanged(int)), ui->sharpSpinBox, SLOT(setValue(int)) );
    connect( ui->sharpSpinBox, SIGNAL(valueChanged(int)), ui->sharpSlider,  SLOT(setValue(int)) );
    ui->sharpSlider->setValue( 80 );

    ui->flatSpinBox->setRange( flatMin, flatMax );
    ui->flatSlider ->setRange( flatMin, flatMax );
    connect( ui->flatSlider,  SIGNAL(valueChanged(int)), ui->flatSpinBox, SLOT(setValue(int)) );
    connect( ui->flatSpinBox, SIGNAL(valueChanged(int)), ui->flatSlider,  SLOT(setValue(int)) );
    ui->flatSlider->setValue( 20 );

    ui->ditherSpinBox->setRange( ditherMin, ditherMax );
    ui->ditherSlider ->setRange( ditherMin, ditherMax );
    connect( ui->ditherSlider,  SIGNAL(valueChanged(int)), ui->ditherSpinBox, SLOT(setValue(int)) );
    connect( ui->ditherSpinBox, SIGNAL(valueChanged(int)), ui->ditherSlider,  SLOT(setValue(int)) );
    ui->ditherSlider->setValue( 20 );

    ui->deNoiseSpinBox->setRange( deNoiseMin, deNoiseMax );
    ui->deNoiseSlider ->setRange( deNoiseMin, deNoiseMax );
    connect( ui->deNoiseSlider,  SIGNAL(valueChanged(int)), ui->deNoiseSpinBox, SLOT(setValue(int)) );
    connect( ui->deNoiseSpinBox, SIGNAL(valueChanged(int)), ui->deNoiseSlider,  SLOT(setValue(int)) );
    ui->deNoiseSlider->setValue( 50 );

    ui->preSharpSpinBox->setRange( preSharpMin, preSharpMax );
    ui->preSharpSlider ->setRange( preSharpMin, preSharpMax );
    connect( ui->preSharpSlider,  SIGNAL(valueChanged(int)), ui->preSharpSpinBox, SLOT(setValue(int)) );
    connect( ui->preSharpSpinBox, SIGNAL(valueChanged(int)), ui->preSharpSlider,  SLOT(setValue(int)) );
    ui->preSharpSlider->setValue( 0 );

    ui->fractNoiseSpinBox->setRange( fractNoiseMin, fractNoiseMax );
    ui->fractNoiseSlider ->setRange( fractNoiseMin, fractNoiseMax );
    connect( ui->fractNoiseSlider,  SIGNAL(valueChanged(int)), ui->fractNoiseSpinBox, SLOT(setValue(int)) );
    connect( ui->fractNoiseSpinBox, SIGNAL(valueChanged(int)), ui->fractNoiseSlider,  SLOT(setValue(int)) );
    ui->fractNoiseSlider->setValue( 0 );

    SetSource( QImage(":/smilla.bmp") );

    ui->progressBar->setRange( 0, 100 );
    ui->progressBar->setValue( 100 );

    ui->comboBox->setSizeAdjustPolicy( QComboBox::AdjustToContentsOnFirstShow );
    ui->comboBox->setMinimumContentsLength( 30 );
    ui->comboBox->clear();
    currentSrcName = " < Smilla > ";
    currentSrcPath = ":/smilla.bmp";

    ui->comboBox->addItem( currentSrcName );

    //----- Parameter Edit -----
    ui->paramCombo->setEditable( true );
    ui->paramCombo->setSizeAdjustPolicy( QComboBox::AdjustToContentsOnFirstShow );
    ui->paramCombo->setMinimumContentsLength( 15 );
    ui->paramCombo->clear();
    ui->paramCombo->setCompleter( 0 );
    ui->paramCombo->setInsertPolicy( QComboBox::InsertAtCurrent );

    ui->paramCombo2->setSizeAdjustPolicy( QComboBox::AdjustToContentsOnFirstShow );
    ui->paramCombo2->setMinimumContentsLength( 15 );
    ui->paramCombo2->clear();
    ui->paramCombo2->setCompleter( 0 );
    ui->paramCombo2->setInsertPolicy( QComboBox::InsertAtCurrent );

    connect( ui->paramAddButton,   SIGNAL( clicked() ), this, SLOT(slot_AddParam()) );
    connect( ui->paramDelButton,   SIGNAL( clicked() ), this, SLOT(slot_DelParam()) );
    connect( ui->paramCombo,       SIGNAL( activated(int) ), this, SLOT(slot_SetParamSlidersFromCombo(int)) );
    connect( ui->paramCombo2,      SIGNAL( activated(int) ), this, SLOT(slot_SetParamSlidersFromCombo(int)) );
    connect( ui->paramChangeCheckBox, SIGNAL(stateChanged( int ) ), this, SLOT(slot_ParamCheckBoxChanged( int ) ) );

    //----- CalcQueue -----
    connect( ui->clearQueueButton,  SIGNAL( clicked() ), this, SLOT(slot_QueueClear()) );
    connect( ui->removeJobButton,   SIGNAL( clicked() ), this, SLOT(slot_QueueRemoveSelected()) );
    connect( ui->cleanUpQueueButton,SIGNAL( clicked() ), this, SLOT(slot_QueueRemoveEnded()) );

    //----- help browser -----
    ui->helpBrowser->setSource( QUrl("qrc:/help/helpMain.html") );
    connect( ui->helpBackButton,    SIGNAL(clicked()), ui->helpBrowser, SLOT(backward()) );
    connect( ui->helpForwardButton, SIGNAL(clicked()), ui->helpBrowser, SLOT(forward()) );
    connect( ui->helpHomeButton,    SIGNAL(clicked()), ui->helpBrowser, SLOT(home()) );

    //----- Preferences ------
    connect( preferencesD, SIGNAL(accepted()), this, SLOT(slot_PrefChanged()) );


    //----- Formatter Settings
    ui->mainZoomBox->setValue( 500 );
    ui->mainWidthBox->setValue( 500 );
    ui->mainHeightBox->setValue( 500 );
    ui->mainStretchWBox->setValue( 500 );
    ui->mainStretchHBox->setValue( 500 );
    ui->mainMaxWBox->setValue( 500 );
    ui->mainMaxHBox->setValue( 500 );
    ui->mainCropWBox->setValue( 500 );
    ui->mainCropHBox->setValue( 500 );
    ui->mainAddBarWBox->setValue( 500 );
    ui->mainAddBarHBox->setValue( 500 );

    ui->clipFormatCombo->setCurrentIndex( 2 );
    ui->selectField->SetFormatRatio( 1.0 );
    ui->formatterCombo->setCurrentIndex( idxStretchF );
    ui->formatterCombo->setCurrentIndex( idxZoomF );

    //----------------------------


    theSettings = 0;    // Pointer to settings, none loaded yet
    ReadSettings();

    ResetDialog();

    setAcceptDrops(true);  // Drag and Drop: accept drops ( of source files )

    PrintStatusText(" Welcome to SmillaEnlarger 0.8.9.  Drop a file onto the window to open it." );

    if( DefaultType().isEmpty() )
       ui->destFileEdit->setText( "enlarged.jpg" );
    else
       ui->destFileEdit->setText( "enlarged." + DefaultType() );

    DoPreview();
    AdjustDestName();   // if dstName exists already: add or inc number at end of name

    resize( minimumSize() );
    setUpdatesEnabled( true );

}

EnlargerDialog::~EnlargerDialog()
{
    delete previewThread;
    if( theSettings != 0 )
       delete theSettings;
    delete theCalcQueue;
    delete preferencesD;
    delete ui;
    delete cropRect;

    delete mainZoomFormatter;
    delete mainWidthFormatter;
    delete mainHeightFormatter;
    delete mainStretchFormatter;
    delete mainMaxFormatter;
    delete mainCropFormatter;
    delete mainAddBarFormatter;
}

void EnlargerDialog::MenuBarSetup( void ) {
   QMenu *fileMenu, *helpMenu;
   QAction *aboutA, *quitA, *loadA, *saveA, *pasteA, *prefA;
   QAction *helpShowA, *helpMoreA;

   fileMenu = menuBar()->addMenu(tr("File"));
   helpMenu = menuBar()->addMenu(tr("Help"));

   loadA  = fileMenu->addAction(tr("&Open..."));
   saveA  = fileMenu->addAction(tr("Enlarge && Save"));
   pasteA = fileMenu->addAction(tr("Paste"));
   prefA  = fileMenu->addAction(tr("Preferences..."));

   // miniModeA is EnlargerDialog  class member
   miniModeA  = fileMenu->addAction(tr("Mini Mode"));
   miniModeA->setCheckable( true );
   miniModeA->setChecked( false );

   quitA  = fileMenu->addAction(tr("Quit"));

   aboutA      = helpMenu->addAction(tr("About"));
   helpShowA   = helpMenu->addAction(tr("Show Help"));
   helpMoreA   = helpMenu->addAction(tr("more Help..."));

   loadA->setShortcut( QKeySequence::Open );
   saveA->setShortcut( QKeySequence::Save );
   pasteA->setShortcut( QKeySequence::Paste );

   connect( aboutA, SIGNAL(triggered()), this, SLOT( InfoMessage()) );
   connect( loadA,  SIGNAL(triggered()), this, SLOT( slot_load1())  );
   connect( saveA,  SIGNAL(triggered()), this, SLOT( slot_queueCalc())   );
   connect( pasteA, SIGNAL(triggered()), this, SLOT( slot_paste())   );
   connect( miniModeA, SIGNAL(toggled(bool)), this, SLOT( slot_MiniMode( bool ))   );
   connect( prefA,  SIGNAL(triggered()), this, SLOT( slot_ShowPref())   );
   connect( quitA,  SIGNAL(triggered()), this, SLOT( close())        );

   connect( helpShowA,  SIGNAL(triggered()), this, SLOT(slot_ShowHelp()) );
   connect( helpMoreA,   SIGNAL(triggered()), this, SLOT( HelpMessageMore()) );
}

void EnlargerDialog::slot_load1( void ) {
   QString fileName = QFileDialog::getOpenFileName(this,
       tr("Open File"), srcDir.absolutePath(),
       tr("Image Files (*.bmp *.jpg *.png *.jpeg *.ppm *.tif *.tiff *.gif )") );
   if (!fileName.isEmpty()) {
      TryOpenSource( fileName );
   }
}

void EnlargerDialog::slot_load2( void ) {
    QString fileName = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dstDir.absolutePath() );
    if (!fileName.isEmpty()) {
        SetDestDir( fileName );
    }
    AdjustDestName();   // if dstName exists already: add or inc number at end of name
}

void EnlargerDialog::slot_comboChanged( const QString & txt ) {
   if( txt == currentSrcName )
       return;
   if( srcDir.exists( txt ) ) {
      TryOpenSource( srcDir.absoluteFilePath( txt ) );
      stopPreview();
   }
}

void EnlargerDialog::slot_checkBoxChanged( int state ) {
    if( ui->srcCheckBox->isChecked() ) {
        ui->load2Button->setEnabled( false );
        SetDestDir( srcDir.absolutePath() );
        AdjustDestName();   // if dstName exists already: add or inc number at end of name
    }
    else {
        ui->load2Button->setEnabled( true );
    }
}

void EnlargerDialog::stopPreview( void ) {
    previewThread->StopEnlarge();
    ui->previewProgressLabel->setText("  " );
}

void EnlargerDialog::DoPreview( void ) {
    if( ZoomX() < 1.0 && ZoomY() < 1.0 )
        return;

    QRect dstRect;
    QImage srcImage;
    QImage preImage;
    EnlargeFormat format;
    EnlargeParamInt enlargeParam;

    dstRect = ui->previewField->DstRect();
    srcImage = ui->previewField->theImage();

    format.srcWidth  = srcImage.width();
    format.srcHeight = srcImage.height();
    format.SetScaleFact( ui->previewField->ZoomX(), ui->previewField->ZoomY() );
    format.SetDstClip( dstRect.x(), dstRect.y(), dstRect.x() + dstRect.width(), dstRect.y() + dstRect.height() );

    ReadParameters( enlargeParam );
    previewThread->Enlarge( srcImage, format, enlargeParam.FloatParam() );
    previewThread->setPriority( QThread::NormalPriority );

}

void EnlargerDialog::slot_queueCalc( void ) {
   if( dstDir.exists( ui->destFileEdit->text() ) )  {
      if( !FileExistsMessage( ui->destFileEdit->text(), dstDir.dirName() ) )
         return;
   }

   // if there is a crop rect, use it
   // else use complete source
   //float srcX0,srcY0,srcX1,srcY1;
   //cropRect->GetMarkedClipRect( srcX0, srcY0, srcX1, srcY1 );

   SingleCalcJob *newJob;
   newJob = new SingleCalcJob( currentMainFormatter );

   newJob->srcName = currentSrcName;
   newJob->srcPath = currentSrcPath;
   if( currentSrcPath.isEmpty() ) {   // indicates, that the image itself has to be attached to the job
      newJob->AttachImage( ui->previewField->theImage() );
   }
   newJob->dstName = ui->destFileEdit->text();
   newJob->dstPath = dstDir.absoluteFilePath( ui->destFileEdit->text() );
   newJob->resultQuality = ResultQuality();

   ReadParameters( newJob->param );

   connect( newJob, SIGNAL(StatusMessage(QString)), this, SLOT(slot_MessageToLog(QString)) );
   connect( newJob, SIGNAL(ErrorMessage(QString)), this, SLOT(slot_MessageToLog(QString)) );
   connect( newJob, SIGNAL(BadAlloc(QString)), this, SLOT(BadAllocMessage(QString)) );

   QString statusText;
   statusText =  "Calculating '" + ui->destFileEdit->text() + "' . ";
   statusText += " Source was '" + ui->comboBox->currentText() + "' . ";
   statusText += " Destination directory is '" + ui->destDirLabel->text() + "' . ";
   PrintStatusText( statusText );

   theCalcQueue->ResetProgress();
   theCalcQueue->AddJob( newJob );
   AdjustDestName();
   ui->queueListView->clearFocus();
   ui->queueListView->setFocus();
}

void EnlargerDialog::slot_showPreview( const QImage & result ) {
    ui->previewField->setPreview( result );
}

void EnlargerDialog::slot_showPreviewProgress( int val ) {
   if( val<100 ) {
      ui->previewProgressLabel->setText(" [ " + QString::number( val ) + "% ]   " );
   }
   else {
      ui->previewProgressLabel->setText("  " );
   }
}

void EnlargerDialog::slot_clippingChanged( void ) {
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainFormatterSelected( int idx ) {

   if( idx == idxZoomF ){
      currentMainFormatter = mainZoomFormatter;
   }
   else if( idx == idxWidthF ) {
      currentMainFormatter = mainWidthFormatter;
   }
   else if( idx == idxHeightF ) {
      currentMainFormatter = mainHeightFormatter;
   }
   else if( idx == idxMaxF ) {
      currentMainFormatter = mainMaxFormatter;
   }
   else if( idx == idxStretchF ) {
      currentMainFormatter = mainStretchFormatter;
   }
   else if( idx == idxCropF ) {
      currentMainFormatter = mainCropFormatter;
   }
   else if( idx == idxAddBarF ) {
      currentMainFormatter = mainAddBarFormatter;
   }
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainZoomFormatterSet ( int zI ) {
   mainZoomFormatter->SetZoom( float(zI) * 0.01 );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainWidthFormatterSet( int w ) {
   mainWidthFormatter->SetWidth( w );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainHeightFormatterSet( int h ) {
   mainHeightFormatter->SetHeight( h );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainStretchFormatterSetW( int w ) {
   mainStretchFormatter->SetWidth( w );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainStretchFormatterSetH( int h ) {
   mainStretchFormatter->SetHeight( h );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainMaxFormatterSetW( int w ) {
   mainMaxFormatter->SetWidth( w );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainMaxFormatterSetH( int h ) {
   mainMaxFormatter->SetHeight( h );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainCropFormatterSetW( int w ) {
   mainCropFormatter->SetWidth( w );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainCropFormatterSetH( int h ) {
   mainCropFormatter->SetHeight( h );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainCropFormatterSetStretch( double v ) {
   mainCropFormatter->SetStretch( float(v) );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainAddBarFormatterSetW( int w ) {
   mainAddBarFormatter->SetWidth( w );
   MainFormatterUpdate();
}

void EnlargerDialog::slot_mainAddBarFormatterSetH( int h ) {
   mainAddBarFormatter->SetHeight( h );
   MainFormatterUpdate();
}

void EnlargerDialog::MainFormatterUpdate( void ) {
   EnlargeFormat format;

   // if there is a crop rect, use it
   // else use complete source
   float srcX0,srcY0,srcX1,srcY1;
   if( cropRect->IsOpen() ) {
      cropRect->GetMarkedClipRect( srcX0, srcY0, srcX1, srcY1 );
      currentMainFormatter->SetSrcClip( srcX0, srcY0, srcX1, srcY1 );
   }
   else {
      currentMainFormatter->NoClipping();
   }

   currentMainFormatter->CalculateFormat( srcWidth, srcHeight, format );

   format.GetSrcClip( srcX0, srcY0, srcX1, srcY1 );
   ui->previewField->setClipRect(  srcX0, srcY0, srcX1, srcY1  );
   ui->previewField->setZoom( format.scaleX, format. scaleY );

   ui->dimensionsLabel->setText( QString::number( format.ClipW() ) + "x" + QString::number( format.ClipH() ) );
   ui->thumbField->setFormat( format );
}

void EnlargerDialog::slot_applyClipping( void ) {
    ui->selectField ->AdjustView();
}

void EnlargerDialog::slot_paste( void )
{
   const QClipboard *clipboard = QApplication::clipboard();
   SourceFromMimeData( clipboard->mimeData() );
}

void EnlargerDialog::slot_ShowJobCount( int endedJobs, int totalJobs ) {
   int tabIndex;
   tabIndex = ui->tabWidget->indexOf( ui->jobTab );
   QString jobStr = "Jobs: ( " + QString::number( endedJobs ) + "/" + QString::number( totalJobs ) + " )";
   if( tabIndex != -1 ) {
      ui->tabWidget->setTabText( tabIndex, jobStr );
   }
   SetToolTip( ui->progressBar, jobStr );
}

void EnlargerDialog::slot_MessageToLog( const QString & message ) {
   ui->logDisplay->append( message );
   ui->logDisplay->append( " " );
   ui->logDisplay->moveCursor ( QTextCursor::End );
   ui->logDisplay->ensureCursorVisible();
}

void EnlargerDialog::slot_MiniMode( bool switchToMini ) {
   setUpdatesEnabled( false );
   if( switchToMini ) {
      ui->tabWidget->hide();
      ui->statusTextLabel->hide();
      // workaround ?!?: force correct resize ?!?
      setUpdatesEnabled( true );
      ui->centralwidget->setParent( 0 );
      setCentralWidget( ui->centralwidget );
      ui->centralwidget->show();
   }
   else {
      ui->tabWidget->show();
      ui->statusTextLabel->show();
   }
   resize( minimumSize() );
   setUpdatesEnabled( true );

}

void EnlargerDialog::slot_ShowPref( void ) {
   preferencesD->ReadSettings( theSettings );
   preferencesD->show();
}

void EnlargerDialog::slot_PrefChanged( void ) {
   preferencesD->WriteSettings( theSettings );
}

void EnlargerDialog::slot_customFormatChanged( void ) {
   float f=float( ui->cropFormatXBox->value()) / float( ui->cropFormatYBox->value());
   ui->selectField->SetFormatRatio( f );
}

void EnlargerDialog::slot_clipFormatComboChanged( const QString & txt ) {

   bool cropVisible = ( txt =="custom..." );
   // only visible if custom is selected
   ui->cropFormatXBox->setVisible( cropVisible );
   ui->cropFormatYBox->setVisible( cropVisible );
   ui->cropFormatLabel->setVisible( cropVisible );

   if( txt == "free" ) {
      ui->selectField->SetFormatRatio( -1.0 );
   }
   else if( txt =="custom..." ) {
      float f=float( ui->cropFormatXBox->value()) / float( ui->cropFormatYBox->value());
      ui->selectField->SetFormatRatio( f );
   }
   else if( txt =="3:5" ) {
      ui->selectField->SetFormatRatio( 3.0/5.0 );
   }
   else if( txt =="3:4" ) {
      ui->selectField->SetFormatRatio( 3.0/4.0 );
   }
   else if( txt =="4:3" ) {
      ui->selectField->SetFormatRatio( 4.0/3.0 );
   }
   else if( txt =="16:9" ) {
      ui->selectField->SetFormatRatio( 16.0/9.0 );
   }
   else if( txt =="5:3" ) {
      ui->selectField->SetFormatRatio( 5.0/3.0 );
   }
   else if( txt =="sqrt(2) : 1" ) {
      ui->selectField->SetFormatRatio( sqrt( 2.0 ) );
   }
   else if( txt =="1 : sqrt(2)" ) {
      ui->selectField->SetFormatRatio( 0.5*sqrt( 2.0 ) );
   }
   else if( txt =="Golden Cut (p.)" ) {
      ui->selectField->SetFormatRatio( 2.0/(sqrt( 5.0 ) + 1 ) );
   }
   else if( txt =="Golden Cut (l.)" ) {
      ui->selectField->SetFormatRatio( 0.5*(sqrt( 5.0 ) + 1 ) );
   }
   else {
      ui->selectField->SetFormatRatio( 1.0 );
   }
}

void EnlargerDialog::slot_TabChanged( int idx ) {
   if( idx==1 ) {
      ui->thumbField->ShowCross();
   }
   else {
      ui->thumbField->HideCross();
   }
}

//----------------------------------------------------

void EnlargerDialog::closeEvent(QCloseEvent *event)
{
   WriteSettings();
   if ( theCalcQueue->UnfinishedJobs() == 0 ) {
      theCalcQueue->Clear();
      event->accept();
      return;
   }
   if( AbortCalcMessage( theCalcQueue->UnfinishedJobs() ) ) {
      theCalcQueue->Clear();
      event->accept();
   } else {
      event->ignore();
   }
 }

// ---- Drag and Drop ----

void EnlargerDialog::dragEnterEvent(QDragEnterEvent *event)
 {
 /*
    QStringList formatsList;

    cout<<" ED DragEnter "<<" \n"<<flush;
    formatsList = event->mimeData()->formats();
    for (int i = 0; i < formatsList.size(); i++) {
       cout << "MimeType ("<<i<<"): "<<formatsList.at(i).toLocal8Bit().constData()<<" \n"<<flush;
    }
    if (event->mimeData()->hasFormat("text/uri-list")) {
       cout<<"    text/uri-list "<<" \n"<<flush;
       QList<QUrl>  uriList;
       uriList = event->mimeData()->urls();
       for (int i = 0; i < uriList.size(); i++) {
          cout << "URI ("<<i<<"): "<<uriList.at(i).toString().toLocal8Bit().constData()<<" \n"<<flush;
          cout << "   Path: "<<uriList.at(i).path().toLocal8Bit().constData()<<" \n"<<flush;
       }
    }
    cout << "Text: "<<event->mimeData()->text().toLocal8Bit().constData()<<" \n"<<flush;
    event->acceptProposedAction();

    if( event->mimeData()->hasImage() )
       cout << "Has Image!\n"<<flush;
    else
       cout << "No Image.\n"<<flush;
*/
    event->acceptProposedAction();

}

void EnlargerDialog::dropEvent(QDropEvent *event)
{
   SourceFromMimeData( event->mimeData() );
   event->acceptProposedAction();
}

//-----------------------------------------------------------

void EnlargerDialog::ResetDialog( void ) {
    ui->previewButton->setFocus();
}

void EnlargerDialog::SetSource(const  QImage & src ) {
    srcWidth  = src.width();
    srcHeight = src.height();

    cropRect->SetSrc( src.width(), src.height() );

    ui->selectField->setTheImage( src );

    ui->previewField->setTheImage( src );
    ui->previewField->setClipRect( 0.0, 0.0, float( srcWidth ), float( srcHeight ) );

    ui->thumbField->setTheImage( src );

    //newWidth  = int( float( srcWidth  )*zoomFact );
    //newHeight = int( float( srcHeight )*zoomFact );
    //ui->newWidthBox ->setValue( newWidth  );
    //ui->newHeightBox->setValue( newHeight );
    stopPreview();

    MainFormatterUpdate();
}

void EnlargerDialog::FillComboBox( void ) {
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.bmp" << "*.png" << "*.tif" << "*.tiff" << "*.gif" << "*.ppm";
    srcDir.setNameFilters(filters);
    srcDir.setSorting ( QDir::Name | QDir::IgnoreCase );
    QStringList entries;
    entries = srcDir.entryList();
    ui->comboBox->clear();
    ui->comboBox->addItems( entries );
}

void EnlargerDialog::PrintStatusText( QString statusText ) {
   ui->logDisplay->append( statusText );
   ui->logDisplay->append( " " );
   ui->logDisplay->moveCursor ( QTextCursor::End );
   ui->logDisplay->ensureCursorVisible();

   SetToolTip( ui->statusTextLabel, statusText );
   statusText.truncate( 180 );
   ui->statusTextLabel->setText ( statusText );
}

void EnlargerDialog::slot_ShowHelp( void ) {
   // to show help, deactivate mini mode
   if( miniModeA->isChecked() )
      miniModeA->toggle();
   ui->tabWidget->setCurrentIndex( 4 );
}

void EnlargerDialog::slot_ShowPreviewTab( void ) {
   // to show preview, deactivate mini mode
   if( miniModeA->isChecked() )
      miniModeA->toggle();
   ui->tabWidget->setCurrentIndex( 1 );
   DoPreview();
}

void EnlargerDialog::InfoMessage( void ) {
    QMessageBox msgBox;
    QString mainText, infoText;
    msgBox.setTextFormat( Qt::RichText );

    mainText =  "<p><b>SmillaEnlarger version 0.9.0</b></p>";
    infoText = "<p>Copyright (C) 2009 Mischa Lusteck</p>";
    infoText += "<p>Project site: <a href=\"http://sourceforge.net/projects/imageenlarger/\">";
    infoText += "http://sourceforge.net/projects/imageenlarger/</a></p>";

    infoText +=  "<p></p>";
    infoText += "<p>This program is free software; </p>";
    infoText += "<p> you can redistribute it and/or modify it under the terms of the ";
    infoText += " GNU General Public License as published by the Free Software Foundation; ";
    infoText += " either version 3 of the License, or (at your option) any later version. ";
    infoText += "</p>";
    infoText += "<p>This program is distributed in the hope that it will be useful, ";
    infoText += " but WITHOUT ANY WARRANTY; ";
    infoText += " without even the implied warranty of MERCHANTABILITY ";
    infoText += " or FITNESS FOR A PARTICULAR PURPOSE. ";
    infoText += " See the GNU General Public License for more details. ";
    infoText += "</p>";
    infoText += "<p>You should have received a copy of the GNU General Public License ";
    infoText += " along with this program; if not, see ";
    infoText += " <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>. </p>";

    msgBox.setText( mainText );
    msgBox.setInformativeText( infoText );
    msgBox.exec();
}

void EnlargerDialog::HelpMessageMore( void ) {
    QMessageBox msgBox;
    msgBox.setText("<b>More Help:</b>");
    msgBox.setTextFormat( Qt::RichText );
    QString helpText;
    helpText += "<p>Visit the <a href=\"http://sourceforge.net/projects/imageenlarger/\"> ";
    helpText += " Project Page</a> ,</p>";
    helpText += "<p>especially the <a href=\"http://sourceforge.net/forum/forum.php?forum_id=974929\">Support Forum</a>.</p>";
    helpText += "<p>Most interface elemets have tool tips ( they pop up when you don't move the mouse some time ).</p> ";


    msgBox.setInformativeText( helpText );
    msgBox.exec();
}

void EnlargerDialog::BadAllocMessage(  const QString & dstName  ) {
    QMessageBox msgBox;
    msgBox.setTextFormat( Qt::RichText );
    msgBox.setText("<b>Result too big !?</b>");
    QString helpText;
    helpText =  "<p> An error occured:</p>";
    helpText += "<p> SmillaEnlarger was not able to allocate enough memory for '" + dstName + "', calculation was aborted.</p>";

    msgBox.setInformativeText( helpText );
    msgBox.exec();

    ResetDialog();
}

bool EnlargerDialog::FileExistsMessage( const QString & fileName, const QString & dirName ) {
   QMessageBox msgBox;
   msgBox.setText("The file '" + fileName + "' exists. Do you want to replace it? " );
   QString helpText;
   helpText =  "In the directory '" + dirName +"' exists a file with the same name. ";
   helpText += " Click 'OK' to replace it. ";
   msgBox.setInformativeText( helpText );
   msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel);
   int ret = msgBox.exec();

   return ret == QMessageBox::Ok;
}

bool EnlargerDialog::AbortCalcMessage( int unfinishedJobs  ) {
   QMessageBox msgBox;
   QString helpText;

   if( unfinishedJobs == 0 )
      return true;

   if( unfinishedJobs == 1 ) {
      msgBox.setText("Do you want to abort the current calculation? " );
      helpText =  "Currently there is a unfinished job in the calculation queue. ";
      helpText += " Click 'Abort' to stop the calculation. ";
   }
   else {
      msgBox.setText("Do you want to abort the current calculations? " );
      helpText =  "Currently there are " + QString::number( unfinishedJobs ) + " unfinished jobs in the calculation queue. ";
      helpText += " Click 'Abort' to stop the calculations. ";
   }

   msgBox.setInformativeText( helpText );
   msgBox.setStandardButtons( QMessageBox::Abort | QMessageBox::Cancel);
   int ret = msgBox.exec();

   return ret == QMessageBox::Abort;
}

void EnlargerDialog::SetToolTip( QWidget *widget, const QString & tipStr ) {
    widget->setToolTip(   tipStr );
    widget->setWhatsThis( tipStr );
}

void EnlargerDialog::SetToolTip( QWidget *w1,  QWidget *w2, const QString & tipStr ) {
    w1->setToolTip(   tipStr );
    w1->setWhatsThis( tipStr );
    w2->setToolTip(   tipStr );
    w2->setWhatsThis( tipStr );
}

void EnlargerDialog::slot_QueueRemoveSelected( void ) {
   QModelIndex jobIdx = ui->queueListView->currentIndex();
   if( !jobIdx.isValid() )
      return;
   if( !ui->queueListView->selectionModel()->isSelected( jobIdx ) )
      return;
   theCalcQueue->RemoveJob( jobIdx );
   ui->queueListView->clearFocus();
   ui->queueListView->setFocus();
}

void EnlargerDialog::slot_QueueClear( void ) {
   theCalcQueue->Clear();
   ui->queueListView->clearFocus();
   ui->queueListView->setFocus();
}

void EnlargerDialog::slot_QueueRemoveEnded( void ) {
   theCalcQueue->RemoveEnded();
   ui->queueListView->clearFocus();
   ui->queueListView->setFocus();
}

QString EnlargerDialog::DefaultType( void )   {
   return theSettings->value("output/file-format", "" ).toString();
}

int  EnlargerDialog::ResultQuality( void ) {
   return theSettings->value("output/quality", 90 ).toInt();
}

