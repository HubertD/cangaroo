#include "DbcParser.h"
#include <QTextStream>
#include <stdint.h>
#include <iostream>

DbcParser::DbcParser()
  : _pos(0)
{

}

void DbcParser::parseFile(QFile *file)
{
    if (!file->open(QIODevice::ReadOnly)) {
        // TODO raise cannot open file exception
    }

    QTextStream in(file);

    _str = in.readAll();
    parse();
    file->close();
}

void DbcParser::parse()
{
    _pos = 0;
    while (_pos<_str.length()) {
        parseSection();
    }
}

void DbcParser::skipWhiteSpace()
{
    while ((_pos<_str.length()) && (_str[_pos].isSpace())) {
        _pos++;
    }
}

bool DbcParser::isAtSectionEnding()
{
    int mypos = _pos;
    while (true) {
        if (mypos >= _str.length()) {
            return true;
        } else if (_str[mypos]==';') {
            return true;
        } else if (_str.midRef(mypos,4)=="\r\n\r\n") {
            return true;
        } else if (_str[mypos].isSpace()) {
            mypos++;
            continue;
        } else {
            return false;
        }
    }
}

void DbcParser::skipSectionEnding()
{
    while ((_pos<_str.length())) {

        if (_str[_pos]==';') {
            _pos++;
            return;
        } else if (_str.midRef(_pos,4)=="\r\n\r\n") {
            _pos+=4;
            return;
        } else if (_str[_pos].isSpace()) {
            _pos++;
            continue;
        } else {
            // TODO throw no section ending
            return;
        }

    }

}

void DbcParser::skipColon()
{
    skipWhiteSpace();
    if (_pos<_str.length()) {
        if (_str[_pos] == ':') {
            _pos++;
        } else {
            // TODO throw colon expected
        }
    }
}

QString DbcParser::readRegExp(const QRegExp &re)
{
    if (re.indexIn(_str, _pos)==_pos) {
        QString result = re.cap(1);
        _pos += result.length();
        return result;
    } else {
        return "";
    }
}

QString DbcParser::readString()
{
    QRegExp rx("(\".*\")");
    rx.setMinimal(true);
    skipWhiteSpace();
    QString s = readRegExp(rx);
    if (s.length()>1) {
        return s.mid(1, s.length()-2);
    } else {
        // TODO throw could not read string exception
        return "";
    }
}

QString DbcParser::readSectionName()
{
    static const QRegExp rx("([A-Z_]+)");
    skipWhiteSpace();
    return readRegExp(rx);
}

QString DbcParser::readIdentifier()
{
    static const QRegExp rx("([A-Za-z_][A-Za-z0-9_]+)");
    skipWhiteSpace();
    return readRegExp(rx);
}

qlonglong DbcParser::readInteger()
{
    static const QRegExp rx("([0-9]+)");
    skipWhiteSpace();
    QString s = readRegExp(rx);

    bool ok;
    qlonglong result = s.toLongLong(&ok, 10);
    if (ok) {
        return result;
    } else {
        // TODO throw could not read integer exception
        return 0;
    }
}


void DbcParser::parseSection() {

    QString sectionName = readSectionName();

    if (sectionName == "VERSION") {
        parseSectionVersion();
    } else if (sectionName == "NS_") {
        parseSectionNs();
    } else if (sectionName == "BS_") {
        parseSectionBs();
    } else if (sectionName == "BU_") {
        parseSectionBu();
    } else if (sectionName == "BO_") {
        parseSectionBo();
    } else {
        // skip till next section...
        while (!isAtSectionEnding()) { _pos++; }
    }

    /*
    if (sectionName == "CM_")             { return tokSectionCm; }
    if (sectionName == "BA_")             { return tokSectionBa; }
    if (sectionName == "BA_REL_")         { return tokSectionBaRel; }
    if (sectionName == "BA_DEF_")         { return tokSectionBaDef; }
    if (sectionName == "BA_DEF_DEF_")     { return tokSectionBaDefDef; }
    if (sectionName == "BA_DEF_DEF_REL_") { return tokSectionBaDefDefRel; }
    if (sectionName == "VAL_")            { return tokSectionVal; }
*/
}


void DbcParser::parseSectionVersion()
{
    /*_dbcVersion =*/ readString();
    skipSectionEnding();
}

void DbcParser::parseSectionNs()
{
    skipColon();
    while (!isAtSectionEnding()) {
        readIdentifier();
    }
}

void DbcParser::parseSectionBs()
{
    skipColon();
    while (!isAtSectionEnding()) {
        _pos++;
    }
}

void DbcParser::parseSectionBu()
{
    skipColon();
    while (!isAtSectionEnding()) {
        readIdentifier();
    }
}

void DbcParser::parseSectionBo()
{
    skipWhiteSpace();
    qlonglong can_id = readInteger();
    QString msg_name = readIdentifier();
    skipColon();
    qlonglong dlc = readInteger();
    QString sender = readIdentifier();

    while (!isAtSectionEnding()) {
        _pos++;
    }
}

