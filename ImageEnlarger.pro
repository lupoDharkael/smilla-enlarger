# ----------------------------------------------------------------
# SmillaEnlarger - resize, especially magnify bitmaps in high quality
# ImageEnlarger.pro: the project file
# Copyright (C) 2009 Mischa Lusteck
# This program is free software;
# you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation;
# either version 3 of the License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
# ----------------------------------------------------------------------
# -------------------------------------------------
# Project created by QtCreator 2009-06-16T16:07:28
# -------------------------------------------------
TARGET = SmillaEnlarger
TEMPLATE = app
SOURCES += main.cpp \
    selectField.cpp \
    previewField.cpp \
    EnlargerDialog.cpp \
    ImageEnlargerCode/Array.cpp \
    EnlargerThread.cpp \
    TemplateInst.cpp \
    ImageEnlargerCode/FractTab.cpp \
    CalcQueue.cpp \
    ArgumentParser.cpp \
    ConsoleManager.cpp \
    ED_LoadSave.cpp \
    ED_Parameter.cpp \
    Preferences.cpp \
    ClipRect.cpp \
    formatterclass.cpp \
    thumbField.cpp
HEADERS += selectField.h \
    previewField.h \
    EnlargerDialog.h \
    ImageEnlargerCode/timing.h \
    ImageEnlargerCode/PointClass.h \
    ImageEnlargerCode/ConstDefs.h \
    ImageEnlargerCode/ArraysTemplateDefs.h \
    ImageEnlargerCode/ArraysTemplate.h \
    ImageEnlargerCode/Array.h \
    EnlargerThread.h \
    ImageEnlargerCode/EnlargerTemplateDefs.h \
    ImageEnlargerCode/FractTab.h \
    CalcQueue.h \
    ImageEnlargerCode/EnlargeParam.h \
    ArgumentParser.h \
    ImageEnlargerCode/EnlargerTemplate.h \
    ConsoleManager.h \
    Preferences.h \
    ClipRect.h \
    formatterclass.h \
    thumbField.h
FORMS += enlargerdialog.ui \
    preferences.ui
RESOURCES += ressources.qrc
OTHER_FILES += icon.rc
RC_FILE = icon.rc
CONFIG += console
