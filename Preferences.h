/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    Preferences Dialog

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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtGui/QDialog>
#include <QString>

namespace Ui
{
    class PreferencesDialog;
}
class QSettings;
class QButtonGroup;

class PreferencesDialog : public QDialog
{
   Q_OBJECT
private:
   Ui::PreferencesDialog *uiPref;
   QButtonGroup *typeGroup;

public:
   PreferencesDialog(QWidget *parent = 0);
   ~PreferencesDialog();
   void ReadSettings( QSettings *mySettings );
   void WriteSettings( QSettings *mySettings );

private slots:
   void ComboChanged( void );
};

#endif // PREFERENCES_H
