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
    typedef enum {
        tokInvalid,

        tokSectionVersion,
        tokSectionNs,
        tokSectionBs,
        tokSectionBu,
        tokSectionBo,
        tokSectionSg,
        tokSectionBoTxBu,
        tokSectionCm,
        tokSectionBaDef,
        tokSectionBaDefDef,
        tokSectionBaDefDefRel,
        tokSectionBa,
        tokSectionBaRel,
        tokSectionVal,

        tokIdentifier,
        tokInteger,
        tokFloat,
        tokString,
        tokColon,
        tokPipe,
        tokAt,
        tokPlus,
        tokComma,
        tokParentOpen,
        tokParentClose,
        tokBracketOpen,
        tokBracketClose,
    } token_t;

    QString _str;
    int _pos;

    bool isAtSectionEnding();
    void skipWhiteSpace();
    void skipSectionEnding();
    void skipColon();

    QString readRegExp(const QRegExp &re);
    QString readSectionName();
    QString readIdentifier();
    QString readString();
    qlonglong readInteger();

    void parse();
    void parseSection();
    void parseSectionVersion();
    void parseSectionNs();
    void parseSectionBs();
    void parseSectionBu();
    void parseSectionBo();

};

#endif // DBCPARSER_H
