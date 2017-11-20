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
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QtGui>

#include <iostream>

#include "ui_preferences.h"
#include "Preferences.h"

using namespace std;

PreferencesDialog::PreferencesDialog(QWidget *parent)
   : QDialog(parent), uiPref( new Ui::PreferencesDialog )
{
   uiPref->setupUi(this);
   typeGroup = new QButtonGroup();
   typeGroup->addButton(uiPref->srcTypeRadioButton);
   typeGroup->addButton(uiPref->otherTypeRadioButton);

   uiPref->qualitySlider->setMinimum(1);
   uiPref->qualitySlider->setMaximum(100);
   connect(uiPref->qualitySlider, SIGNAL(valueChanged(int)), uiPref->qualitySpinBox, SLOT(setValue(int)) );
   connect(uiPref->qualitySpinBox,SIGNAL(valueChanged(int)), uiPref->qualitySlider,  SLOT(setValue(int)) );
   connect(uiPref->typeComboBox,SIGNAL(activated(int)), this, SLOT(ComboChanged()) );

}

PreferencesDialog::~PreferencesDialog( void ) {
   delete typeGroup;
   delete uiPref;
}

void PreferencesDialog::ReadSettings( QSettings *mySettings ) {
   QString dstType;
   int quality;

   quality = mySettings->value( "output/quality", 90 ).toInt();
   dstType = mySettings->value( "output/file-format", "" ).toString();
   uiPref->qualitySlider->setValue( quality );
   if( dstType.isEmpty() ) {
      uiPref->srcTypeRadioButton->setChecked( true );
   }
   else {
      int idx = uiPref->typeComboBox->findText ( dstType, Qt::MatchExactly );
      if( idx == -1 ) {
         uiPref->srcTypeRadioButton->setChecked( true );
      }
      else {
         uiPref->otherTypeRadioButton->setChecked( true );
         uiPref->typeComboBox->setCurrentIndex( idx );
      }
   }
}

void PreferencesDialog::WriteSettings( QSettings *mySettings ) {
   QString dstType;

   mySettings->setValue( "output/quality", uiPref->qualitySlider->value() );
   dstType = "";
   if( uiPref->otherTypeRadioButton->isChecked() ) {
      dstType = uiPref->typeComboBox->currentText();
   }
   mySettings->setValue( "output/file-format", dstType );

}

void PreferencesDialog::ComboChanged( void ) {
   uiPref->otherTypeRadioButton->setChecked( true );
}
