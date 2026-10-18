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
#include <QChar>
#include <QRegularExpression>

typedef enum {
    dbc_tok_whitespace = 1,
    dbc_tok_identifier = 2,
    dbc_tok_string = 4,
    dbc_tok_number = 8,
    dbc_tok_colon = 16,
    dbc_tok_pipe = 32,
    dbc_tok_at = 64,
    dbc_tok_plus = 128,
    dbc_tok_parenth_open = 256,
    dbc_tok_parenth_close = 512,
    dbc_tok_bracket_open = 1024,
    dbc_tok_bracket_close = 2048,
    dbc_tok_comma = 4096,
    dbc_tok_semicolon = 8192,
    dbc_tok_minus = 16384,

    dbc_tok_ALL = 0xFFFFFFFF
} dbc_token_type_t;

class DbcToken {
public:
    DbcToken(int line, int column, dbc_token_type_t type);
    virtual bool acceptsChar(QChar ch) = 0;

    bool appendChar(QChar ch);
    dbc_token_type_t getType();
    QString getData();
    int countLineBreaks();
    int getLine();
    int getColumn();

protected:
    QString _data;
private:
    int _line;
    int _col;
    dbc_token_type_t _type;
    int _numLineBreaks;
};

class DbcWhitespaceToken : public DbcToken {
public:
    DbcWhitespaceToken(int line, int column);
    virtual bool acceptsChar(QChar ch);

};

class DbcIdentifierToken : public DbcToken {
public:
    DbcIdentifierToken(int line, int column);
    virtual bool acceptsChar(QChar ch);
};

class DbcStringToken : public DbcToken {
public:
    DbcStringToken(int line, int column);
    virtual bool acceptsChar(QChar ch);
private:
    bool _done;
    bool _escape;
};

class DbcRegExpToken : public DbcToken {
public:
    DbcRegExpToken(int line, int column, dbc_token_type_t type, QRegularExpression re);
    virtual bool acceptsChar(QChar ch);
private:
    QRegularExpression _re;
};

class DbcSingleCharToken : public DbcToken {
public:
    DbcSingleCharToken(int line, int column, dbc_token_type_t type, QChar ch);
    virtual bool acceptsChar(QChar ch);
private:
    QChar _ch;
};
