#ifndef DBCPARSER_H
#define DBCPARSER_H

#include <QFile>
#include <QRegExp>

class DbcParser
{

public:
    DbcParser();
    void parseFile(QFile *file);

private:
    QString _str;
    int _pos;

    bool isAtSectionEnding();
    void skipWhiteSpace();
    void skipSectionEnding();
    bool skipChar(QChar ch);
    bool skipColon();

    QString readRegExp(const QRegExp &re);
    QString readSectionName();
    QString readIdentifier();
    QString readString();
    qlonglong readInteger();
    double readDouble();

    void parse();
    void parseSection();
    void parseSectionVersion();
    void parseSectionNs();
    void parseSectionBs();
    void parseSectionBu();
    void parseSectionBo();
    void parseSectionBoSg();

};

#endif // DBCPARSER_H
