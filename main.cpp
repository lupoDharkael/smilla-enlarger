/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    main.cpp: just exec()

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

#include <QtGui/QApplication>
#include <QIcon>
#include "ConsoleManager.h"
#include "EnlargerDialog.h"
#include "EnlargerThread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QIcon myIcon(":/smilla.png");
    qApp->setWindowIcon( myIcon );

    // in the ConsoleManager the command line options are defined
    // the ConsoleManager will parse them, decides via UseGUI() which mode to use
    // and sets up the Dialog or starts calc in console according to given args
    ConsoleManager myConsoleManager( argc, argv );

    if( myConsoleManager.UseGUI() ) {
       EnlargerDialog w;
       myConsoleManager.SetupEnlargerDialog( w );
       w.show();
       return a.exec();
    }
    else {
       EnlargerThread myThread;
       if( myConsoleManager.StartConsoleEnlarge( myThread ) ) {
          return a.exec();
       }
    }
    return 0;
}
