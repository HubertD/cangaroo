#ifndef DBCTOKENS_H
#define DBCTOKENS_H

#include <QFile>
#include <QChar>
#include <QRegExp>

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
    dbc_tok_semicolon = 8192
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
};

class DbcRegExpToken : public DbcToken {
public:
    DbcRegExpToken(int line, int column, dbc_token_type_t type, QRegExp re);
    virtual bool acceptsChar(QChar ch);
private:
    QRegExp _re;
};

class DbcSingleCharToken : public DbcToken {
public:
    DbcSingleCharToken(int line, int column, dbc_token_type_t type, QChar ch);
    virtual bool acceptsChar(QChar ch);
private:
    QChar _ch;
};

#endif // DBCTOKENS_H
