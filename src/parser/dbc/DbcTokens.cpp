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

#include "DbcTokens.h"

DbcToken::DbcToken(int line, int column, dbc_token_type_t type)
  : _line(line), _col(column), _type(type), _numLineBreaks(0)
{

}

bool DbcToken::appendChar(QChar ch)
{
    if (acceptsChar(ch)) {
        if (ch=='\n') {
            _numLineBreaks++;
        }
        _data.append(ch);
        return true;
    } else {
        return false;
    }
}

dbc_token_type_t DbcToken::getType()
{
    return _type;
}

QString DbcToken::getData()
{
    return _data;
}

int DbcToken::countLineBreaks()
{
    return _numLineBreaks;
}

int DbcToken::getLine()
{
    return _line;
}

int DbcToken::getColumn()
{
    return _col;
}


DbcIdentifierToken::DbcIdentifierToken(int line, int column)
  : DbcToken(line, column, dbc_tok_identifier)
{
}

bool DbcIdentifierToken::acceptsChar(QChar ch)
{
    static const QString acceptableStartChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_");
    static const QString acceptableChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_");
    if (_data.isEmpty()) {
        return acceptableStartChars.contains(ch);
    } else {
        return acceptableChars.contains(ch);
    }
}


DbcStringToken::DbcStringToken(int line, int column)
  : DbcToken(line, column, dbc_tok_string),
   _done(false),
   _escape(false)
{
}

/// Accept strings surrounded by '"", including '\'-escaped characters
bool DbcStringToken::acceptsChar(QChar ch)
{
    if (_done) {
        return false;
    }

    if(_data.isEmpty()) {
        // Start of string must be '"'
        return (ch == '"');
    } else {
        // Accept anything until an unescaped '"'
        if (_escape) {
            _escape = false;
        } else {
            if (ch == '\\') {
                _escape = true;
            }
            if (ch == '"') {
                _done = true;
            }
        }
        return true;
    }
}



DbcWhitespaceToken::DbcWhitespaceToken(int line, int column)
  : DbcToken(line, column, dbc_tok_whitespace)
{
}

bool DbcWhitespaceToken::acceptsChar(QChar ch)
{
    return ch.isSpace();
}

DbcRegExpToken::DbcRegExpToken(int line, int column, dbc_token_type_t type, QRegularExpression re)
  : DbcToken(line, column, type),
    _re(re)
{
}

bool DbcRegExpToken::acceptsChar(QChar ch)
{
    QString s = _data + ch;

    QRegularExpressionMatch match = _re.match(s);
    return (match.hasMatch());

    //return (_re.indexIn(s)==0);
}

DbcSingleCharToken::DbcSingleCharToken(int line, int column, dbc_token_type_t type, QChar ch)
  : DbcToken(line, column, type),
    _ch(ch)
{
}

bool DbcSingleCharToken::acceptsChar(QChar ch)
{
    return (ch==_ch) && _data.isEmpty();
}

