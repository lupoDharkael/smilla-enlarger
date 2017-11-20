# ----------------------------------------------------------------
# SmillaEnlarger - resize, especially magnify bitmaps in high quality
# ImageEnlarger.pro: the project file
# Copyright (C) 2009 Mischa Lusteck
# Copyright (C) 2017 Alejandro Sirgo
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

VERSION = 0.9.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SmillaEnlarger
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += src/main.cpp \
    src/selectField.cpp \
    src/previewField.cpp \
    src/EnlargerDialog.cpp \
    src/ImageEnlargerCode/Array.cpp \
    src/EnlargerThread.cpp \
    src/TemplateInst.cpp \
    src/ImageEnlargerCode/FractTab.cpp \
    src/CalcQueue.cpp \
    src/ArgumentParser.cpp \
    src/ConsoleManager.cpp \
    src/ED_LoadSave.cpp \
    src/ED_Parameter.cpp \
    src/Preferences.cpp \
    src/ClipRect.cpp \
    src/formatterclass.cpp \
    src/thumbField.cpp
HEADERS += src/selectField.h \
    src/previewField.h \
    src/EnlargerDialog.h \
    src/ImageEnlargerCode/timing.h \
    src/ImageEnlargerCode/PointClass.h \
    src/ImageEnlargerCode/ConstDefs.h \
    src/ImageEnlargerCode/ArraysTemplateDefs.h \
    src/ImageEnlargerCode/ArraysTemplate.h \
    src/ImageEnlargerCode/Array.h \
    src/EnlargerThread.h \
    src/ImageEnlargerCode/EnlargerTemplateDefs.h \
    src/ImageEnlargerCode/FractTab.h \
    src/CalcQueue.h \
    src/ImageEnlargerCode/EnlargeParam.h \
    src/ArgumentParser.h \
    src/ImageEnlargerCode/EnlargerTemplate.h \
    src/ConsoleManager.h \
    src/Preferences.h \
    src/ClipRect.h \
    src/formatterclass.h \
    src/thumbField.h
FORMS += src/enlargerdialog.ui \
    src/preferences.ui
RESOURCES += ressources.qrc
CONFIG += console
