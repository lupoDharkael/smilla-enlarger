/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    ArgumentParser.cpp: parsing of command line arguments

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
#include <QString>
#include <QRegExp>
#include <iostream>
#include "ArgumentParser.h"

using namespace std;


ArgumentParser::ArgumentParser(void) {
   optionsFound = 0;
}

ArgumentParser::~ArgumentParser(void) {
}

void ArgumentParser::AddOption(BasicOption *newOption) {
   optionList.append(newOption);
}

bool ArgumentParser::Parse(int argc, char *argv[]) {
   int argN;
   for(argN = 1; argN < argc; argN++) {
      QString currentArg, nextArg;
      currentArg = argv[argN];
	  if(argN<argc) {
         nextArg = argv[argN+1];
      }
      else {
         nextArg = "";
      }
	  if(!currentArg.isEmpty() && currentArg.at(0) == '-') { // option found
         bool optionIsValid = false;
		 for(int optNr=0; optNr<optionList.size(); optNr++) {
			BasicOption & option = *optionList.at(optNr);
            bool error = false;
			if(option.Parse(currentArg, nextArg, error)) {
			   if(error)
                  return false;
               optionIsValid = true;
               optionsFound++;
			   if(option.HasParameter())   // step over option parameter
                  argN++;
               break;                        // option found, don't look further
            }
         }
		 if(!optionIsValid) {   // option not in optionList -> error
            cout<<"Error: Unknown option '"<<currentArg.toStdString()<<"'.\n"<<flush;
            return false;
         }
      }
      else {  // Non-Option-Argument found
		 otherArgs.append(currentArg);
      }
   }
   return true;   // parsing successful
}

//---------------------------------------------------------------------------

void BasicOption::Set(ArgumentParser *theParser, const QString & optStr, const QString & optStr2) {
   myParser = theParser;
   optFound = false;
   AddOptString(optStr);
   if(!optStr2.isEmpty())
	  AddOptString(optStr2);
   theParser->AddOption(this);
}

void BasicOption::AddOptString(QString str) {
   str = str.trimmed();
   if(str.isEmpty())
      return;

   if(str.at(0) != '-') {
	  optStrings.append("-" + str);
   }
   else {
	  optStrings.append(str);
   }
}

// optArg: "-option" nextArg: param of opt? use if needed, "", if none there
bool BasicOption::Parse(const QString & optArg, const QString & nextArg, bool & error) {
   error = false;
   for(int a=0; a<optStrings.size(); a++) {
	  if(optArg == optStrings.at(a)) {
         optFound = true;
         return true;
      }
   }
   return false;
}

//-----------------------------------------------------------------

void IntegerOption::Set(ArgumentParser *theParser, const QString & optStr, const QString & optStr2, int defaultVal)
{
   BasicOption::Set(theParser, optStr, optStr2);
   value = defaultVal;
   hasRange = false;
}

bool IntegerOption::Parse(const QString & optArg, const QString & nextArg, bool & error) {
   error = false;
   for(int a=0; a<OptStrings().size(); a++) {
	  if(optArg == OptStrings().at(a)) {
		 SetFound(true);
         bool ok;
		 value = nextArg.toInt(&ok);
		 if(!ok) {
            cout<<"Option '"<<optArg.toStdString()<<"': wrong parameter '"<<nextArg.toStdString()<<"'. \n";
            cout<<"   Integer number expected.\n"<<flush;
            error = true;
            return true;
         }
		 if(hasRange && (value<minVal || value>maxVal)) {
            cout<<"Option '"<<optArg.toStdString()<<"': parameter "<<nextArg.toStdString()<<" out of range. \n";
            cout<<"   Should be between "<<minVal<<" and "<<maxVal<<" .\n"<<flush;
            error = true;
            return true;
         }
         return true;
      }
   }
   return false;
}

bool StringOption::Parse(const QString & optArg, const QString & nextArg, bool & error) {
   error = false;
   for(int a=0; a<OptStrings().size(); a++) {
	  if(optArg == OptStrings().at(a)) {
		 SetFound(true);
         value = nextArg;
         return true;
      }
   }
   return false;
}
