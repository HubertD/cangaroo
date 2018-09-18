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

#include "DbcParser.h"
#include <QTextStream>
#include <stdint.h>
#include <iostream>
#include <core/Backend.h>
#include <core/CanDb.h>

#include "DbcTokens.h"

DbcParser::DbcParser()
  : _errorLine(0), _errorColumn(0)
{
}

bool DbcParser::parseFile(QFile *file, CanDb &candb)
{
    DbcTokenList tokens;
    if (tokenize(file, tokens) != err_ok) {
        QString msg = QString("error parsing dbc file %1").arg(file->fileName());
        if (_errorLine) {
            msg += QString(" at line %1, column %2").arg(_errorLine).arg(_errorColumn);
        }
        log_error(msg);
        return false;
    }

    candb.setPath(file->fileName());
    return parse(candb, tokens);
}

DbcToken *DbcParser::createNewToken(QChar ch, int line, int column)
{
    static const QString acceptableIdStartChars("ABCDEFGHIKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_");
    static const QRegExp numberRegExp("^(\\d+(\\.\\d*)?(E[-+]?\\d*)?)$");

    if (ch.isSpace()) {
        return new DbcWhitespaceToken(line, column);
    } else if (ch.isDigit()) {
        return new DbcRegExpToken(line, column, dbc_tok_number, numberRegExp);
    } else if (ch == '"') {
        return new DbcStringToken(line, column);
    } else if (ch == ':') {
        return new DbcSingleCharToken(line, column, dbc_tok_colon, ':');
    } else if (ch == '|') {
        return new DbcSingleCharToken(line, column, dbc_tok_pipe, '|');
    } else if (ch == '@') {
        return new DbcSingleCharToken(line, column, dbc_tok_at, '@');
    } else if (ch == '+') {
        return new DbcSingleCharToken(line, column, dbc_tok_plus, '+');
    } else if (ch == '-') {
        return new DbcSingleCharToken(line, column, dbc_tok_minus, '-');
    } else if (ch == '(') {
        return new DbcSingleCharToken(line, column, dbc_tok_parenth_open, '(');
    } else if (ch == ')') {
        return new DbcSingleCharToken(line, column, dbc_tok_parenth_close, ')');
    } else if (ch == '[') {
        return new DbcSingleCharToken(line, column, dbc_tok_bracket_open, '[');
    } else if (ch == ']') {
        return new DbcSingleCharToken(line, column, dbc_tok_bracket_close, ']');
    } else if (ch == ',') {
        return new DbcSingleCharToken(line, column, dbc_tok_comma, ',');
    } else if (ch == ';') {
        return new DbcSingleCharToken(line, column, dbc_tok_semicolon, ';');
    } else if (acceptableIdStartChars.contains(ch)) {
        return new DbcIdentifierToken(line, column);
    } else {
        return 0;
    }
}

DbcParser::error_t DbcParser::tokenize(QFile *file, DbcParser::DbcTokenList &tokens)
{

    if (!file->open(QIODevice::ReadOnly)) {
        // TODO raise cannot open file exception
        return err_cannot_open_file;
    }

    DbcToken *currentToken = 0;
    int line = 1;
    int column = 0;

    error_t retval = err_ok;

    QTextStream in(file);
    in.setCodec("ISO 8859-1");

    while (true) {
        QString s = in.read(1);
        if (s.isEmpty()) { break; }

        QChar ch = s[0];

        if (ch=='\n') {
            line++;
            column=1;
        } else {
            column++;
        }

        if (currentToken) {
            if (!currentToken->appendChar(ch)) {
                tokens.append(currentToken);
                currentToken = 0;
            }
        }

        if (!currentToken) {
            currentToken = createNewToken(ch, line, column);
            if (currentToken) {
                currentToken->appendChar(ch);
            } else {
                retval = err_tokenize_error;
                _errorColumn = column;
                _errorLine = line;
                break;
            }
        }

    }

    file->close();
    return retval;
}

bool DbcParser::isSectionEnding(DbcToken *token, bool newLineIsSectionEnding)
{
    if (!token) {
        return true;
    } else {
        int numNewLinesForEnding = newLineIsSectionEnding ? 1 : 2;
        dbc_token_type_t type = token->getType();
        return ( (type==dbc_tok_semicolon) || ( (type==dbc_tok_whitespace) && (token->countLineBreaks()>=numNewLinesForEnding)));
    }
}

DbcToken *DbcParser::readToken(DbcParser::DbcTokenList &tokens, int typeMask, bool skipWhitespace, bool skipSectionEnding, bool newLineIsSectionEnding)
{
    while (true) {
        if (tokens.isEmpty()) { return 0; }

        DbcToken *token = tokens.first();
        dbc_token_type_t type = token->getType();

        if (type & typeMask) {

            tokens.pop_front();
            return token;

        } else if (isSectionEnding(token, newLineIsSectionEnding)) {

            if (skipSectionEnding) {
                tokens.pop_front();
                free(token);
                continue;
            } else {
                return 0;
            }

        } else if (skipWhitespace && (type==dbc_tok_whitespace)) {

            tokens.pop_front();
            free(token);
            continue;

        } else {

            return 0;

        }
    }

    return 0;
}

bool DbcParser::expectSectionEnding(DbcTokenList &tokens, bool newLineIsSectionEnding)
{
    if (tokens.isEmpty()) {
        return true;
    }

    DbcToken *token = readToken(tokens, dbc_tok_whitespace|dbc_tok_semicolon);
    if (!token) {
        return false;
    }

    if (!isSectionEnding(token, newLineIsSectionEnding)) {
        free(token);
        return false;
    } else {
        free(token);
        return true;
    }

}

bool DbcParser::expectLineBreak(DbcParser::DbcTokenList &tokens)
{
    bool found_line_break;

    DbcToken *token = readToken(tokens, dbc_tok_whitespace);
    if (token) {
        found_line_break = token->countLineBreaks()>0;
        free(token);
    } else {
        found_line_break = false;
    }

    return found_line_break;
}

bool DbcParser::expectAndSkipToken(DbcTokenList &tokens, dbc_token_type_t type, bool skipWhitespace, bool skipSectionEnding)
{
    DbcToken *token = readToken(tokens, type, skipWhitespace, skipSectionEnding);
    if (!token) {
        return false;
    } else {
        free(token);
        return true;
    }
}

bool DbcParser::expectData(DbcParser::DbcTokenList &tokens, dbc_token_type_t type, QString *data, bool skipWhitespace, bool skipSectionEnding, bool newLineIsSectionEnding)
{
    DbcToken *token;
    if (!(token = readToken(tokens, type, skipWhitespace, skipSectionEnding, newLineIsSectionEnding))) {
        return false;
    }

    if (data) {
        data->clear();
        data->append(token->getData());
    }

    free(token);
    return true;
}

bool DbcParser::expectIdentifier(DbcParser::DbcTokenList &tokens, QString *id, bool skipWhitespace, bool skipSectionEnding, bool newLineIsSectionEnding)
{
    return expectData(tokens, dbc_tok_identifier, id, skipWhitespace, skipSectionEnding, newLineIsSectionEnding);
}

bool DbcParser::expectString(DbcParser::DbcTokenList &tokens, QString *str, bool skipWhitespace)
{
    QString quotedStr;
    bool ok = expectData(tokens, dbc_tok_string, &quotedStr, skipWhitespace);
    if (ok && quotedStr.length()>=2) {
        *str = quotedStr.mid(1, quotedStr.length()-2);
        return true;
    } else {
        return false;
    }
}

bool DbcParser::expectNumber(DbcParser::DbcTokenList &tokens, QString *str, bool skipWhitespace)
{
    QString data;
    if  (expectAndSkipToken(tokens, dbc_tok_minus, skipWhitespace)) {
        *str = "-";
    } else if (expectAndSkipToken(tokens, dbc_tok_plus, skipWhitespace)) {
        *str = "+";
    }

    if (expectData(tokens, dbc_tok_number, &data, skipWhitespace)) {
        *str += data;
        return true;
    } else {
        return false;
    }

}

bool DbcParser::expectInt(DbcParser::DbcTokenList &tokens, int *i, int base, bool skipWhitespace)
{
    QString data;
    if (!expectNumber(tokens, &data, skipWhitespace)) {
        return false;
    }

    bool convert_ok;
    *i = data.toInt(&convert_ok, base);
    return convert_ok;
}

bool DbcParser::expectLongLong(DbcTokenList &tokens, long long *i, int base, bool skipWhitespace)
{
    QString data;
    if (!expectNumber(tokens, &data, skipWhitespace)) {
        return false;
    }

    bool convert_ok;
    *i = data.toLongLong(&convert_ok, base);
    return convert_ok;
}

bool DbcParser::expectDouble(DbcTokenList &tokens, double *df, bool skipWhitespace)
{
    QString data;
    if (!expectNumber(tokens, &data, skipWhitespace)) {
        return false;
    }

    bool convert_ok;
    *df = data.toDouble(&convert_ok);
    return convert_ok;
}

void DbcParser::skipUntilSectionEnding(DbcTokenList &tokens)
{
    while (!tokens.isEmpty()) {
        DbcToken *token = readToken(tokens, dbc_tok_ALL, false, false);
        if (!token) { return; }
        if (isSectionEnding(token)) {
            free(token);
            return;
        } else {
            free(token);
        }
    }
}

bool DbcParser::parseIdentifierList(DbcTokenList &tokens, QStringList *list, bool newLineIsSectionEnding)
{
    if (!expectAndSkipToken(tokens, dbc_tok_colon)) {
        return false;
    }

    QString id;
    while (expectIdentifier(tokens, &id, true, false, newLineIsSectionEnding)) {
        if (list) {
            list->append(id);
        }
    }

    return expectSectionEnding(tokens, newLineIsSectionEnding);
}

bool DbcParser::parse(CanDb &candb, DbcTokenList &tokens)
{
    _dbcVersion.clear();
    _nsEntries.clear();
    _buEntries.clear();

    while (!tokens.isEmpty()) {
        if (!parseSection(candb, tokens)) {
            return false;
        }
    }

    return true;
}

bool DbcParser::parseSection(CanDb &candb, DbcTokenList &tokens) {
    bool retval = true;

    QString sectionName;
    QStringList strings;

    while (retval) {

        if (tokens.isEmpty()) {
            break;
        }

        if (expectIdentifier(tokens, &sectionName, true, true)) {
            if (sectionName == "VERSION") {
                retval &= parseSectionVersion(candb, tokens);
            } else if (sectionName == "NS_") {
                strings.clear();
                retval &= parseIdentifierList(tokens, &strings);
            } else if (sectionName == "BS_") {
                retval &= parseSectionBs(tokens);
            } else if (sectionName == "BU_") {
                retval &= parseSectionBu(candb, tokens);
            } else if (sectionName == "BO_") {
                retval &= parseSectionBo(candb, tokens);
            } else if (sectionName == "CM_") {
                retval &= parseSectionCm(candb, tokens);
            } else if (sectionName == "VAL_") {
                retval &= parseSectionVal(candb, tokens);
            } else {
                skipUntilSectionEnding(tokens);
            }

        } else {
            retval = false;
        }

    }

    if (!retval) {
        log_error("dbc parse error");
    }
    return retval;

    /*
    if (sectionName == "BA_")             { return tokSectionBa; }
    if (sectionName == "BA_REL_")         { return tokSectionBaRel; }
    if (sectionName == "BA_DEF_")         { return tokSectionBaDef; }
    if (sectionName == "BA_DEF_DEF_")     { return tokSectionBaDefDef; }
    if (sectionName == "BA_DEF_DEF_REL_") { return tokSectionBaDefDefRel; }
*/
}

bool DbcParser::parseSectionVersion(CanDb &candb, DbcTokenList &tokens)
{
    QString version;
    if (!expectString(tokens, &version)) { return false; }
    candb.setVersion(version);
    return expectSectionEnding(tokens);
}

bool DbcParser::parseSectionBs(DbcParser::DbcTokenList &tokens)
{
    if (!expectAndSkipToken(tokens, dbc_tok_colon)) {
        return false;
    }

    return expectSectionEnding(tokens);
}

bool DbcParser::parseSectionBu(CanDb &candb, DbcParser::DbcTokenList &tokens)
{
    QStringList strings;
    QString s;

    if (!parseIdentifierList(tokens, &strings, true)) {
        return false;
    }

    foreach(s, strings) {
        candb.getOrCreateNode(s);
    }

    return true;
}


bool DbcParser::parseSectionBo(CanDb &candb, DbcTokenList &tokens)
{
    long long can_id;
    int dlc;
    QString msg_name;
    QString sender;

    if (!expectLongLong(tokens, &can_id)) { return false; }
    if (!expectIdentifier(tokens, &msg_name)) { return false; }
    if (!expectAndSkipToken(tokens, dbc_tok_colon)) { return false; }
    if (!expectInt(tokens, &dlc)) { return false; }
    if (!expectIdentifier(tokens, &sender)) { return false; }

    CanDbMessage *msg = new CanDbMessage(&candb);
    msg->setRaw_id(can_id);
    msg->setName(msg_name);
    msg->setDlc(dlc);
    msg->setSender(candb.getOrCreateNode(sender));
    candb.addMessage(msg);

    QString subsect;
    while (true) {
        if (expectSectionEnding(tokens)) {
            return true;
         } else {
            if (!expectIdentifier(tokens, &subsect)) {
                return false;
            }

            if (subsect!="SG_") {
                return false;
            }

            if (!parseSectionBoSg(candb, msg, tokens)) {
                return false;
            }
        }
    }


}

bool DbcParser::parseSectionBoSg(CanDb &candb, CanDbMessage *msg, DbcTokenList &tokens)
{
    (void)candb;

    QString signal_name;
    QString mux_indicator;
    int start_bit = 0;
    int length = 0;
    int byte_order = 0;
    double factor = 1;
    double offset = 0;
    double minimum = 0;
    double maximum = 0;
    QString unit;
    QString receiver;
    QStringList receivers;

    CanDbSignal *signal = new CanDbSignal(msg);
    msg->addSignal(signal);

    if (!expectIdentifier(tokens, &signal_name)) { return false; }
    signal->setName(signal_name);


    if (expectIdentifier(tokens, &mux_indicator)) {
        if (mux_indicator=="M") {
            signal->setIsMuxer(true);
            msg->setMuxer(signal);
        } else if (mux_indicator.startsWith('m')) {
            signal->setIsMuxed(true);
            bool ok;
            signal->setMuxValue(mux_indicator.mid(1).toUInt(&ok));
            if (!ok) { return false; }
        } else {
            return false;
        }
    }

    if (!expectAndSkipToken(tokens, dbc_tok_colon)) { return false; }
    if (!expectInt(tokens, &start_bit)) { return false; }

    signal->setStartBit(start_bit);

    if (!expectAndSkipToken(tokens, dbc_tok_pipe)) { return false; }
    if (!expectInt(tokens, &length)) { return false; }
    signal->setLength(length);

    if (!expectAndSkipToken(tokens, dbc_tok_at)) { return false; }
    if (!expectInt(tokens, &byte_order)) { return false; }
    signal->setIsBigEndian(byte_order==0);

    // If the signal is big endian, convert the start bit to the Intel-style start bit for further parsing
    if(signal->isBigEndian())
    {
        // This will be the number of 8-bit rows above the message
        uint8_t row_position = signal->startBit() >> 3;

        // Bit position in current row (0-7)
        uint8_t column_position = signal->startBit() & 0b111;

        // Calcualte the normalized start bit position (bit index starting at 0)
        uint8_t normalized_position = (row_position * 8) + (7 - column_position);

        signal->setStartBit(normalized_position);
    }

    if (expectAndSkipToken(tokens, dbc_tok_plus)) {
        signal->setUnsigned(true);
    } else {
        if (expectAndSkipToken(tokens, dbc_tok_minus)) {
            signal->setUnsigned(false);
        } else {
            return false;
        }
    }

    if (!expectAndSkipToken(tokens, dbc_tok_parenth_open)) { return false; }
    if (!expectDouble(tokens, &factor)) { return false; }
    signal->setFactor(factor);
    if (!expectAndSkipToken(tokens, dbc_tok_comma)) { return false; }
    if (!expectDouble(tokens, &offset)) { return false; }
    signal->setOffset(offset);
    if (!expectAndSkipToken(tokens, dbc_tok_parenth_close)) { return false; }

    if (!expectAndSkipToken(tokens, dbc_tok_bracket_open)) { return false; }
    if (!expectDouble(tokens, &minimum)) { return false; }
    signal->setMinimumValue(minimum);
    if (!expectAndSkipToken(tokens, dbc_tok_pipe)) { return false; }
    if (!expectDouble(tokens, &maximum)) { return false; }
    signal->setMaximumValue(maximum);
    if (!expectAndSkipToken(tokens, dbc_tok_bracket_close)) { return false; }

    if (!expectString(tokens, &unit)) { return false; }
    signal->setUnit(unit);

    if (!expectIdentifier(tokens, &receiver)) { return false; }
    receivers.append(receiver);

    while (expectAndSkipToken(tokens, dbc_tok_comma, false, false)) {
        if (!expectIdentifier(tokens, &receiver)) { return false; }
        receivers.append(receiver);
    }


    return true;
}

bool DbcParser::parseSectionCm(CanDb &candb, DbcParser::DbcTokenList &tokens)
{
    QString s;
    QString idtype;
    QString id;
    long long ll;

    if (expectString(tokens, &s)) { // DBC file comment
        candb.setComment(s);
        return true;
    }

    if (!expectIdentifier(tokens, &idtype)) { return false; }

    if (idtype=="BU_") {

        if (!expectIdentifier(tokens, &id)) { return false; }
        if (!expectString(tokens, &s)) { return false; }
        candb.getOrCreateNode(id)->setComment(s);
        return expectSectionEnding(tokens);

    } else if (idtype=="BO_") {

        if (!expectLongLong(tokens, &ll)) { return false; }
        if (!expectString(tokens, &s)) { return false; }
        CanDbMessage *msg = candb.getMessageById(ll);
        if (!msg) { return false; }
        msg->setComment(s);
        return expectSectionEnding(tokens);

    } else if (idtype=="SG_") {

        if (!expectLongLong(tokens, &ll)) { return false; }
        CanDbMessage *msg = candb.getMessageById(ll);
        if (!msg) { return false; }

        if (!expectIdentifier(tokens, &id)) { return false; }
        CanDbSignal *signal = msg->getSignalByName(id);
        if (!signal) { return false; }

        if (!expectString(tokens, &s)) { return false; }
        signal->setComment(s);

        return expectSectionEnding(tokens);

    } else {

        return false;

    }


}

bool DbcParser::parseSectionVal(CanDb &candb, DbcParser::DbcTokenList &tokens)
{
    long long can_id;
    QString signal_id;
    long long value;
    QString name;

    if (!expectLongLong(tokens, &can_id)) { return false; }
    CanDbMessage *msg = candb.getMessageById(can_id);
    if (!msg) { return false; }

    if (!expectIdentifier(tokens, &signal_id)) { return false; }
    CanDbSignal *signal = msg->getSignalByName(signal_id);
    if (!signal) { return false; }

    while (!expectAndSkipToken(tokens, dbc_tok_semicolon)) {
        if (!expectLongLong(tokens, &value)) { return false; }
        if (!expectString(tokens, &name)) { return false; }
        signal->setValueName(value, name);
    }

    return true;
}

