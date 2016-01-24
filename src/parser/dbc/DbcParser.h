/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include <QFile>
#include <QRegExp>
#include <QList>
#include <qstringlist.h>

#include <core/CanDb.h>

#include "DbcTokens.h"

class CanDbMessage;

class DbcParser
{

public:
    typedef QList<DbcToken *> DbcTokenList;

    typedef enum {
        err_ok,
        err_cannot_open_file,
        err_tokenize_error,
    } error_t;

public:
    DbcParser();
    bool parseFile(QFile *file, CanDb &candb);

private:
    int _errorLine;
    int _errorColumn;

    QString _dbcVersion;
    QStringList _nsEntries;
    QStringList _buEntries;

    DbcToken *createNewToken(QChar ch, int line, int column);
    error_t tokenize(QFile *file, DbcTokenList &tokens);

    bool isSectionEnding(DbcToken *token, bool newLineIsSectionEnding=false);
    bool expectSectionEnding(DbcTokenList &tokens, bool newLineIsSectionEnding=false);
    bool expectLineBreak(DbcTokenList &tokens);
    bool expectAndSkipToken(DbcTokenList &tokens, dbc_token_type_t type, bool skipWhitespace=true, bool skipSectionEnding=false);

    bool expectData(DbcTokenList &tokens, dbc_token_type_t type, QString *data, bool skipWhitespace=true, bool skipSectionEnding=false, bool newLineIsSectionEnding=false);
    bool expectIdentifier(DbcTokenList &tokens, QString *id, bool skipWhitespace=true, bool skipSectionEnding=false, bool newLineIsSectionEnding=false);
    bool expectString(DbcTokenList &tokens, QString *str, bool skipWhitespace=true);

    bool expectNumber(DbcTokenList &tokens, QString *str, bool skipWhitespace=true);

    bool expectInt(DbcTokenList &tokens, int *i, int base=10, bool skipWhitespace=true);
    bool expectLongLong(DbcTokenList &tokens, long long *i, int base=10, bool skipWhitespace=true);
    bool expectDouble(DbcTokenList &tokens, double *df, bool skipWhitespace=true);
    void skipUntilSectionEnding(DbcTokenList &tokens);

    DbcToken *readToken(DbcTokenList &tokens, int typeMask, bool skipWhitespace=true, bool skipSectionEnding=false, bool newLineIsSectionEnding=false);

    bool parse(CanDb &candb, DbcTokenList &tokens);
    bool parseIdentifierList(DbcTokenList &tokens, QStringList *list, bool newLineIsSectionEnding=false);

    bool parseSection(CanDb &candb, DbcTokenList &tokens);
    bool parseSectionVersion(CanDb &candb, DbcTokenList &tokens);
    bool parseSectionBs(DbcTokenList &tokens);
    bool parseSectionBu(CanDb &candb, DbcTokenList &tokens);
    bool parseSectionBo(CanDb &candb, DbcTokenList &tokens);
    bool parseSectionBoSg(CanDb &candb, CanDbMessage *msg, DbcTokenList &tokens);
    bool parseSectionCm(CanDb &candb, DbcTokenList &tokens);
    bool parseSectionVal(CanDb &candb, DbcTokenList &tokens);

};
