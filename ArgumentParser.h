/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    ArgumentParser.h: parsing of command line arguments

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
#include <QStringList>

#ifndef ARGUMENTPARSER_H
#define ARGUMENTPARSER_H

class BasicOption;

class ArgumentParser {
   QList< BasicOption* > optionList;
   QStringList otherArgs;
   int optionsFound;

public:
   ArgumentParser(void);
   ~ArgumentParser(void);
   void AddOption(BasicOption *newOption);
   bool Parse(int argc, char *argv[]);  // return false -> error
   QStringList NonOptionArguments(void) { return otherArgs; }
   bool OptionsFound(void) { return optionsFound; }

private:

};

class BasicOption {
   friend class ArgumentParser;

   ArgumentParser *myParser;
   QStringList optStrings;
   bool optFound;

public:
   BasicOption(void) : myParser(0), optFound(false) {}
   virtual ~BasicOption(void) {}
   bool Found(void) { return optFound; }
   bool IsThere(void) { return optFound; }
   void AddOptString(QString str);
   void Set( ArgumentParser *theParser, const QString & optStr, const QString & optStr2 = "");

protected:
   // optArg: "-option" nextArg: param of opt? use if needed, "", if none there
   virtual bool Parse(const QString & optArg, const QString & nextArg, bool & error);
   virtual bool HasParameter(void) { return false; }
   QStringList OptStrings(void) { return optStrings; }
   void SetFound(bool f) { optFound = f; }

};

class IntegerOption : public BasicOption {
   int value;
   bool hasRange;
   int minVal, maxVal;

public:
   IntegerOption(void) : BasicOption() {}
   void Set(ArgumentParser *theParser, const QString & optStr, const QString & optStr2 = "", int defaultVal=0);
   int Value(void) { return value; }
   void SetRange(int min, int max) { minVal=min; maxVal=max; hasRange=true; }
   void SetDefault(int defaultV) { value = defaultV; }

protected:
   bool Parse(const QString & optArg, const QString & nextArg, bool & error);
   bool HasParameter(void) { return true; }
};

class StringOption : public BasicOption {
   QString value;

public:
   StringOption(void) : BasicOption() {}
   ~StringOption(void) {}
   void Set(ArgumentParser *theParser, const QString & optStr, const QString & optStr2 = "")
	  { BasicOption::Set(theParser, optStr, optStr2); }
   QString Value(void) { return value; }
   void SetDefault(const QString & defaultV) { value = defaultV; }

protected:
   bool Parse(const QString & optArg, const QString & nextArg, bool & error);
   bool HasParameter(void) { return true; }
};

#endif // ARGUMENTPARSER_H
