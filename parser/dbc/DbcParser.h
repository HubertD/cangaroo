#ifndef DBCPARSER_H
#define DBCPARSER_H

#include <QFile>
#include <QRegExp>
#include <QList>
#include "DbcTokens.h"

class CanDb;
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
    bool parseFile(QFile *file, CanDb *candb);

private:
    int _errorLine;
    int _errorColumn;

    QString _dbcVersion;
    QStringList _nsEntries;
    QStringList _buEntries;

    DbcToken *createNewToken(QChar ch, int line, int column);
    error_t tokenize(QFile *file, DbcTokenList &tokens);

    bool isSectionEnding(DbcToken *token);
    bool expectSectionEnding(DbcTokenList &tokens);
    bool expectLineBreak(DbcTokenList &tokens);
    bool expectAndSkipToken(DbcTokenList &tokens, dbc_token_type_t type, bool skipWhitespace=true, bool skipSectionEnding=false);

    bool expectData(DbcTokenList &tokens, dbc_token_type_t type, QString *data, bool skipWhitespace=true, bool skipSectionEnding=false);
    bool expectIdentifier(DbcTokenList &tokens, QString *id, bool skipWhitespace=true, bool skipSectionEnding=false);
    bool expectString(DbcTokenList &tokens, QString *id, bool skipWhitespace=true);
    bool expectInt(DbcTokenList &tokens, int *i, int base=10, bool skipWhitespace=true);
    bool expectLongLong(DbcTokenList &tokens, long long *i, int base=10, bool skipWhitespace=true);
    bool expectDouble(DbcTokenList &tokens, double *df, bool skipWhitespace=true);
    void skipUntilSectionEnding(DbcTokenList &tokens);

    DbcToken *readToken(DbcTokenList &tokens, int typeMask, bool skipWhitespace=true, bool skipSectionEnding=false);

    bool parse(CanDb *candb, DbcTokenList &tokens);
    bool parseIdentifierList(DbcTokenList &tokens, QStringList *list);

    bool parseSection(CanDb *candb, DbcTokenList &tokens);
    bool parseSectionVersion(CanDb *candb, DbcTokenList &tokens);
    bool parseSectionBs(DbcTokenList &tokens);
    bool parseSectionBo(CanDb *candb, DbcTokenList &tokens);
    bool parseSectionBoSg(CanDb *candb, CanDbMessage *msg, DbcTokenList &tokens);

};

#endif // DBCPARSER_H
