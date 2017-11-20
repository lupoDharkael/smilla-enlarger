/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    Part of EnlargerDialog: Settings and parameter tab

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

#include <QFileDialog>
#include <QDesktopServices>    // for getting std. picture-directory
#include <QDir>
#include <QSettings>
#include <QtGui>

#include <iostream>
#include "EnlargerDialog.h"
#include "ui_enlargerdialog.h"
#include "previewField.h"
#include "EnlargerThread.h"
#include "ImageEnlargerCode/FractTab.h"
#include "CalcQueue.h"

// Part of EnlargeDialog: Settings & Parameter Tab
//
// ReadSettings
// WriteSettings
// FindSettingsFile
// ReadParameters
// AddParamSet
// DelParamSet
// ChangeParamSet
// GetParamSliders
// SetParamSliders
// slot_AddParam
// slot_DelParam
// slot_ParamCheckBoxChanged
// slot_SetParamSlidersFromCombo
//
// ------------------------------------------------------------

// search for smillaenlarger.ini
// in ~/.smillacenlarger/, in current dir, and in MacOS-bundle Contents dir
// create *theSettings,
// read the settings
void EnlargerDialog::ReadSettings( void ) {
   QString iniFilePath;

   iniFilePath = FindSettingsFile();
   //cout<<"Settings Path: "<<iniFilePath.toStdString()<<" \n"<<flush;
   if( theSettings != 0) {   // settings have been read before, delete them
      delete theSettings;
   }
   theSettings = new QSettings( iniFilePath, QSettings::IniFormat );


   //--- 1.Parameter Settings ---
   // clear the array of parameter settings
   ui->paramCombo->clear();
   ui->paramCombo2->clear();
   paramSetVal.clear();

   // fill with data from settings file
   int size = theSettings->beginReadArray("parameters/array");
   for (int i = 0; i < size; ++i) {
      theSettings->setArrayIndex(i);
      QString settingName;
      EnlargeParamInt p;

      settingName = theSettings->value("name").toString();
      p.sharp      = theSettings->value("sharp",      80 ).toInt();
      p.flat       = theSettings->value("flat",       80 ).toInt();
      p.dither     = theSettings->value("dither",      80 ).toInt();
      p.deNoise    = theSettings->value("deNoise",    80 ).toInt();
      p.preSharp   = theSettings->value("preSharp",   80 ).toInt();
      p.fractNoise = theSettings->value("fractNoise", 80 ).toInt();
      ui->paramCombo->addItem( settingName );
      ui->paramCombo2->addItem( settingName );
      paramSetVal.append( p );
   }
   theSettings->endArray();

   currentParamIdx = theSettings->value( "parameters/currentIndex", 0 ).toInt();
   if( currentParamIdx<0 || currentParamIdx>=paramSetVal.size() )
      currentParamIdx = 0;
   if( size == 0 ) {
      EnlargeParamInt p;
      p.sharp    = 80;
      p.flat     = 10;
      p.dither   = 20;
      p.preSharp = 0;
      p.deNoise  = 10;
      p.fractNoise = 0;
      AddParamSet( "default", p );
      currentParamIdx = 0;
   }
   SetParamSliders( paramSetVal.at( currentParamIdx ) );

   ui->paramCombo->setCurrentIndex( currentParamIdx );
   ui->paramCombo2->setCurrentIndex( currentParamIdx );
   ui->paramChangeCheckBox->setChecked( false );
   slot_ParamCheckBoxChanged( Qt::Unchecked );

   //--- 2.Other Settings ---

   // Destination folder: if empty: Use Source Folder
   QString dstDirPath = theSettings->value( "DestinationFolder", "" ).toString();
   if( !dstDirPath.isEmpty() ) {
      QDir dDir( dstDirPath );
      if( !dDir.exists() ) {
         slot_MessageToLog(" Startup: Destination folder '" + dstDirPath + "' does not exist." );
      }
      else {
         if( ui->srcCheckBox->isChecked() ) {
            ui->srcCheckBox->setChecked( false );
         }
         SetDestDir( dstDirPath );
         AdjustDestName();   // if dstName exists already: add or inc number at end of name
      }
   }

   //--- Formatter Settings ---
   theSettings->beginGroup( "Formatters" );
   if( theSettings->contains("currentIndex") ) {  // there are formatter data
      ui->formatterCombo->setCurrentIndex( theSettings->value("currentIndex").toInt() );
      ui->mainZoomBox->setValue(           theSettings->value("zoom").toInt() );
      ui->mainWidthBox->setValue(          theSettings->value("width").toInt() );
      ui->mainHeightBox->setValue(         theSettings->value("height").toInt() );
      ui->mainStretchWBox->setValue(       theSettings->value("stretchW").toInt() );
      ui->mainStretchHBox->setValue(       theSettings->value("stretchH").toInt() );
      ui->mainMaxWBox->setValue(           theSettings->value("maxW").toInt() );
      ui->mainMaxHBox->setValue(           theSettings->value("maxH").toInt() );
      ui->mainCropWBox->setValue(          theSettings->value("cropW").toInt() );
      ui->mainCropHBox->setValue(          theSettings->value("cropH").toInt() );
      ui->mainCropStretchBox->setValue(    theSettings->value("cropStretch").toDouble() );
      ui->mainAddBarWBox->setValue(        theSettings->value("addBarW").toInt() );
      ui->mainAddBarHBox->setValue(        theSettings->value("addBarH").toInt() );
   }
   theSettings->endGroup();

   //--- Cropping ---
   theSettings->beginGroup( "Cropping" );
   if( theSettings->contains("currentIndex") ) {  // there are cropping data
      ui->clipFormatCombo->setCurrentIndex( theSettings->value("currentIndex").toInt() );
      ui->cropFormatXBox->setValue(         theSettings->value("customX").toInt() );
      ui->cropFormatYBox->setValue(         theSettings->value("customY").toInt() );
      slot_clipFormatComboChanged( ui->clipFormatCombo->currentText() );
   }
   theSettings->endGroup();

}

void EnlargerDialog::WriteSettings( void ) {

   //--- 1.Parameter Settings ---
   EnlargeParamInt p;
   if( ui->paramChangeCheckBox->isChecked() ) {
      GetParamSliders( p );
      ChangeParamSet( currentParamIdx, p );
   }
   theSettings->remove("parameters/array");
   theSettings->beginWriteArray("parameters/array");
   for (int i = 0; i < paramSetVal.size(); i++) {
      theSettings->setArrayIndex(i);
      p = paramSetVal.at( i );

      theSettings->setValue( "name"      , ui->paramCombo->itemText( i ) );
      theSettings->setValue( "sharp"     , p.sharp );
      theSettings->setValue( "flat"      , p.flat );
      theSettings->setValue( "dither"    , p.dither );
      theSettings->setValue( "deNoise"   , p.deNoise );
      theSettings->setValue( "preSharp"  , p.preSharp );
      theSettings->setValue( "fractNoise", p.fractNoise );
   }
   theSettings->endArray();
   theSettings->setValue( "parameters/currentIndex", currentParamIdx );

   //--- 2.Other Settings ---
   QString dstDirPath;
   if( ui->srcCheckBox->isChecked() ) {
      theSettings->setValue( "DestinationFolder", "" );  // empty -> Use Source Folder
   }
   else {
      theSettings->setValue( "DestinationFolder", dstDir.absolutePath() );
   }

   //--- Formatter Settings ---
   theSettings->beginGroup( "Formatters" );
   theSettings->setValue("currentIndex", ui->formatterCombo->currentIndex() ) ;
   theSettings->setValue("zoom",         ui->mainZoomBox->value() );
   theSettings->setValue("width",        ui->mainWidthBox->value() ) ;
   theSettings->setValue("height",       ui->mainHeightBox->value()) ;
   theSettings->setValue("stretchW",     ui->mainStretchWBox->value()) ;
   theSettings->setValue("stretchH",     ui->mainStretchHBox->value()) ;
   theSettings->setValue("maxW",         ui->mainMaxWBox->value()) ;
   theSettings->setValue("maxH",         ui->mainMaxHBox->value()) ;
   theSettings->setValue("cropW",        ui->mainCropWBox->value()) ;
   theSettings->setValue("cropH",        ui->mainCropHBox->value()) ;
   theSettings->setValue("cropStretch",  ui->mainCropStretchBox->value()) ;
   theSettings->setValue("addBarW",      ui->mainAddBarWBox->value()) ;
   theSettings->setValue("addBarH",      ui->mainAddBarHBox->value()) ;

   theSettings->endGroup();

   //--- Cropping ---
   theSettings->beginGroup( "Cropping" );
   theSettings->setValue("currentIndex", ui->clipFormatCombo->currentIndex() ) ;
   theSettings->setValue("customX",      ui->cropFormatXBox->value()) ;
   theSettings->setValue("customY",      ui->cropFormatYBox->value()) ;
   theSettings->endGroup();

}

QString EnlargerDialog::FindSettingsFile( void ) {
   QDir settingsDir;
   QString iniFilePath;

   settingsDir = QDir::home();
   iniFilePath = settingsDir.absoluteFilePath( ".smillaenlarger/smillaenlarger.ini" );
   //cout<<" Home: "<<iniFilePath.toStdString()<<" \n";
   if( QFile::exists( iniFilePath ) ) {
      return iniFilePath;
   }

   settingsDir = QDir::currentPath();
   if( settingsDir.dirName() == "MacOS" ) {  // maybe we are in a MacOS-app-bundle?
      settingsDir.cdUp();
      if( settingsDir.dirName() == "Contents" ) { // look in "Contents" of the bundle for smillaenlarger.ini
         iniFilePath = settingsDir.absoluteFilePath( "smillaenlarger.ini" );
         //cout<<" MacOS: "<<iniFilePath.toStdString()<<" \n";
         if( QFile::exists( iniFilePath ) ) {
            return iniFilePath;
         }
      }
   }

   // maybe we are near a MacOS-app-bundle?
   iniFilePath = settingsDir.absoluteFilePath( "SmillaEnlarger.app/Contents/smillaenlarger.ini" );
   //cout<<" MacOS b: "<<iniFilePath.toStdString()<<" \n";
   if( QFile::exists( iniFilePath ) ) {
      return iniFilePath;
   }

   // no settings file found elsewhere: use directory of binary
   settingsDir = QDir::currentPath();
   iniFilePath = settingsDir.absoluteFilePath( "smillaenlarger.ini" );
   //cout<<" Current: "<<iniFilePath.toStdString()<<" \n";
   return iniFilePath;
}

void EnlargerDialog::ReadParameters( EnlargeParamInt & param ) {
    param.sharp      = ui->sharpSlider->value();
    param.flat       = ui->flatSlider->value();
    param.dither     = ui->ditherSlider->value();
    param.deNoise    = ui->deNoiseSlider->value();
    param.preSharp   = ui->preSharpSlider->value();
    param.fractNoise = ui->fractNoiseSlider->value();
}

//--------- list of Parameter Settings --------
int  EnlargerDialog::AddParamSet( const QString & name, const EnlargeParamInt & p ) {
   ui->paramCombo->addItem( name );
   ui->paramCombo2->addItem( name );
   paramSetVal.append( p );
   return paramSetVal.size() - 1;
}

void EnlargerDialog::DelParamSet( int idx ) {
   if( idx>=0 && idx<paramSetVal.size() ) {
      ui->paramCombo->removeItem( idx );
      ui->paramCombo2->removeItem( idx );
      paramSetVal.removeAt( idx );
   }
}

void EnlargerDialog::ChangeParamSet( int idx, const EnlargeParamInt & p ) {
   if( idx>=0 && idx<paramSetVal.size() ) {
      paramSetVal.replace( idx, p );
   }
}

void EnlargerDialog::GetParamSliders( EnlargeParamInt & p ) {
   p.sharp      = ui->sharpSlider     ->value();
   p.flat       = ui->flatSlider      ->value();
   p.dither     = ui->ditherSlider    ->value();
   p.deNoise    = ui->deNoiseSlider   ->value();
   p.preSharp   = ui->preSharpSlider  ->value();
   p.fractNoise = ui->fractNoiseSlider->value();
}

void EnlargerDialog::SetParamSliders( const EnlargeParamInt & p ) {
   ui->sharpSlider     ->setValue( p.sharp );
   ui->flatSlider      ->setValue( p.flat );
   ui->ditherSlider    ->setValue( p.dither );
   ui->deNoiseSlider   ->setValue( p.deNoise );
   ui->preSharpSlider  ->setValue( p.preSharp );
   ui->fractNoiseSlider->setValue( p.fractNoise );
}

void EnlargerDialog::slot_AddParam( void ) {
   int idx;
   EnlargeParamInt p;

   GetParamSliders( p );

   // if changes allowed: write slider settings before switching to new
   if( ui->paramChangeCheckBox->isChecked() ) {
      ChangeParamSet( currentParamIdx, p );
   }

   idx = AddParamSet( "new", p );
   ui->paramCombo->setCurrentIndex( idx );
   ui->paramCombo2->setCurrentIndex( idx );
   currentParamIdx = idx;
   ui->paramChangeCheckBox->setCheckState( Qt::Checked );  // allow editing of new
   ui->paramCombo->setFocus();   // for typing of new name: set focus

   idx = ui->paramCombo->currentIndex();
}

void EnlargerDialog::slot_DelParam( void ) {
   int idx;

   if( ui->paramCombo->count() > 1 ) {
      idx = ui->paramCombo->currentIndex();
      DelParamSet( idx );
      ui->paramChangeCheckBox->setCheckState( Qt::Unchecked );  // switch -> disable editing

      idx = ui->paramCombo->currentIndex();
      currentParamIdx = idx;
   }
}

void EnlargerDialog::slot_ParamCheckBoxChanged( int state ) {
   EnlargeParamInt p;
   GetParamSliders( p );
   ChangeParamSet( currentParamIdx, p );

   if( ui->paramChangeCheckBox->isChecked() ) {
      ui->paramDelButton->setEnabled( true );
      ui->paramCombo->setFocusPolicy( Qt::StrongFocus );
      ui->sharpSlider     ->setEnabled( true ); ui->sharpSpinBox     ->setEnabled( true );
      ui->flatSlider      ->setEnabled( true ); ui->flatSpinBox      ->setEnabled( true );
      ui->ditherSlider    ->setEnabled( true ); ui->ditherSpinBox    ->setEnabled( true );
      ui->preSharpSlider  ->setEnabled( true ); ui->preSharpSpinBox  ->setEnabled( true );
      ui->deNoiseSlider   ->setEnabled( true ); ui->deNoiseSpinBox   ->setEnabled( true );
      ui->fractNoiseSlider->setEnabled( true ); ui->fractNoiseSpinBox->setEnabled( true );
   }
   else {
      ui->paramDelButton->setEnabled( false );
      ui->paramCombo->setFocusPolicy( Qt::NoFocus );
      ui->paramCombo->clearFocus();
      ui->sharpSlider     ->setEnabled( false ); ui->sharpSpinBox     ->setEnabled( false );
      ui->flatSlider      ->setEnabled( false ); ui->flatSpinBox      ->setEnabled( false );
      ui->ditherSlider    ->setEnabled( false ); ui->ditherSpinBox    ->setEnabled( false );
      ui->preSharpSlider  ->setEnabled( false ); ui->preSharpSpinBox  ->setEnabled( false );
      ui->deNoiseSlider   ->setEnabled( false ); ui->deNoiseSpinBox   ->setEnabled( false );
      ui->fractNoiseSlider->setEnabled( false ); ui->fractNoiseSpinBox->setEnabled( false );
   }
}

void EnlargerDialog::slot_SetParamSlidersFromCombo( int idx ) {
   //ui->paramCombo->setEditable( !ui->paramCombo->isEditable() );
   EnlargeParamInt p;
   GetParamSliders( p );

   // if changes allowed: write slider settings before switching to new
   if( ui->paramChangeCheckBox->isChecked() ) {
      ChangeParamSet( currentParamIdx, p );
   }

   if( idx != currentParamIdx && idx >=0 && idx < paramSetVal.size() ) {
      SetParamSliders( paramSetVal.at( idx ) );
      currentParamIdx = idx;
      ui->paramChangeCheckBox->setCheckState( Qt::Unchecked );  // switch -> disable editing
   }
   ui->paramCombo->setCurrentIndex( idx );
   ui->paramCombo2->setCurrentIndex( idx );
}
