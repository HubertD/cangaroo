#include "DbcParser.h"
#include <QTextStream>
#include <stdint.h>
#include <iostream>

#include "DbcTokens.h"

DbcParser::DbcParser()
  : _errorLine(0), _errorColumn(0)
{
}

void DbcParser::parseFile(QFile *file)
{
    DbcTokenList tokens;
    if (tokenize(file, tokens) != err_ok) {
        //std::cout << "error parsing file " << file->fileName();
        if (_errorLine) {
            //std::cout << " at line " << _errorLine << ", column " << _errorColumn;
        }
        //std::cout << "." << std::endl;
        return;
    }
    parse(tokens);
}

DbcToken *DbcParser::createNewToken(QChar ch, int line, int column)
{
    static const QString acceptableIdStartChars("ABCDEFGHIKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_");
    static const QRegExp numberRegExp("^([-\\+]?\\d*(\\.\\d*)?(E[-+]?\\d*)?)$");

    if (ch.isSpace()) {
        return new DbcWhitespaceToken(line, column);
    } else if (ch.isDigit() || (ch=='-')) {
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

    char ch;
    DbcToken *currentToken = 0;
    int line = 1;
    int column = 0;

    error_t retval = err_ok;

    while (file->getChar(&ch)) {

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

bool DbcParser::isSectionEnding(DbcToken *token)
{
    if (!token) {
        return true;
    } else {
        dbc_token_type_t type = token->getType();
        return ( (type==dbc_tok_semicolon) || ( (type==dbc_tok_whitespace) && (token->countLineBreaks()>1)));
    }
}

DbcToken *DbcParser::readToken(DbcParser::DbcTokenList &tokens, int typeMask, bool skipWhitespace, bool skipSectionEnding)
{
    while (true) {
        if (tokens.isEmpty()) { return 0; }

        DbcToken *token = tokens.first();
        dbc_token_type_t type = token->getType();

        if (type & typeMask) {

            tokens.pop_front();
            return token;

        } else if (isSectionEnding(token)) {

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

bool DbcParser::expectSectionEnding(DbcTokenList &tokens)
{
    if (tokens.isEmpty()) {
        return true;
    }

    DbcToken *token = readToken(tokens, dbc_tok_whitespace);
    if (!token) {
        return false;
    }

    if (!isSectionEnding(token)) {
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

bool DbcParser::expectData(DbcParser::DbcTokenList &tokens, dbc_token_type_t type, QString *data, bool skipWhitespace, bool skipSectionEnding)
{
    DbcToken *token;
    if (!(token = readToken(tokens, type, skipWhitespace, skipSectionEnding))) {
        return false;
    }

    if (data) {
        data->clear();
        data->append(token->getData());
    }

    free(token);
    return true;
}

bool DbcParser::expectIdentifier(DbcParser::DbcTokenList &tokens, QString *id, bool skipWhitespace, bool skipSectionEnding)
{
    return expectData(tokens, dbc_tok_identifier, id, skipWhitespace, skipSectionEnding);
}

bool DbcParser::expectString(DbcParser::DbcTokenList &tokens, QString *id, bool skipWhitespace)
{
    return expectData(tokens, dbc_tok_string, id, skipWhitespace);
}

bool DbcParser::expectInt(DbcParser::DbcTokenList &tokens, int *i, int base, bool skipWhitespace)
{
    QString data;
    bool convert_ok;

    if (!expectData(tokens, dbc_tok_number, &data, skipWhitespace)) { return false; }
    *i = data.toInt(&convert_ok, base);
    return convert_ok;
}

bool DbcParser::expectLongLong(DbcTokenList &tokens, long long *i, int base, bool skipWhitespace)
{
    QString data;
    bool convert_ok;

    if (!expectData(tokens, dbc_tok_number, &data, skipWhitespace)) { return false; }
    *i = data.toLongLong(&convert_ok, base);
    return convert_ok;
}

bool DbcParser::expectDouble(DbcTokenList &tokens, double *df, bool skipWhitespace)
{
    QString data;
    bool convert_ok;

    if (!expectData(tokens, dbc_tok_number, &data, skipWhitespace)) { return false; }
    *df = data.toDouble(&convert_ok);
    return convert_ok;
}

void DbcParser::skipUntilSectionEnding(DbcTokenList &tokens)
{
    while (!tokens.isEmpty()) {
        if (expectSectionEnding(tokens)) {
            break;
        }
    }
}

bool DbcParser::parseIdentifierList(DbcTokenList &tokens, QStringList *list)
{
    if (!expectAndSkipToken(tokens, dbc_tok_colon)) {
        return false;
    }

    QString id;
    while (expectIdentifier(tokens, &id)) {
        if (list) {
            list->append(id);
        }
    }

    return expectSectionEnding(tokens);
}

void DbcParser::parse(DbcTokenList &tokens)
{
    _dbcVersion.clear();
    _nsEntries.clear();
    _buEntries.clear();

    while (!tokens.isEmpty()) {
        if (!parseSection(tokens)) {
            return; // todo throw error?
        }
    }
}

bool DbcParser::parseSection(DbcTokenList &tokens) {
    bool retval = true;

    QString sectionName;

    while (retval) {

        if (tokens.isEmpty()) {
            break;
        }

        if (expectIdentifier(tokens, &sectionName, true, true)) {
            if (sectionName == "VERSION") {
                retval &= parseSectionVersion(tokens);
            } else if (sectionName == "NS_") {
                retval &= parseIdentifierList(tokens, &_nsEntries);
            } else if (sectionName == "BS_") {
                retval &= parseSectionBs(tokens);
            } else if (sectionName == "BU_") {
                retval &= parseIdentifierList(tokens, &_buEntries);
            } else if (sectionName == "BO_") {
                retval &= parseSectionBo(tokens);
            } else {
                skipUntilSectionEnding(tokens);
            }

        } else {
            retval = false;
        }

    }
    return retval;

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

bool DbcParser::parseSectionVersion(DbcTokenList &tokens)
{
    if (!expectString(tokens, &_dbcVersion)) { return false; }
    return expectSectionEnding(tokens);
}

bool DbcParser::parseSectionBs(DbcParser::DbcTokenList &tokens)
{
    if (!expectAndSkipToken(tokens, dbc_tok_colon)) {
        return false;
    }

    return expectSectionEnding(tokens);
}


bool DbcParser::parseSectionBo(DbcTokenList &tokens)
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

    QString subsect;
    while (expectIdentifier(tokens, &subsect)) {
        if (subsect!="SG_") { return false; }
        if (!parseSectionBoSg(tokens)) { return false; }
    }

    return expectSectionEnding(tokens);
}

bool DbcParser::parseSectionBoSg(DbcTokenList &tokens)
{
    QString signal_name;
    QString mux_indicator;
    bool is_muxer = false;
    bool is_muxed = false;
    int  mux_value = 0;
    int start_bit = 0;
    int length = 0;
    int byte_order = 0;
    bool is_unsigned = false;
    double factor = 1;
    double offset = 0;
    double minimum = 0;
    double maximum = 0;
    QString unit;
    QString receiver;
    QStringList receivers;


    if (!expectIdentifier(tokens, &signal_name)) { return false; }

    if (expectIdentifier(tokens, &mux_indicator)) {
        if (mux_indicator=="M") {
            is_muxer = true;
        } else if (mux_indicator.startsWith('m')) {
            bool ok;
            is_muxed = true;
            mux_value = mux_indicator.mid(1).toInt(&ok);
            if (!ok) { return false; }
        } else {
            return false;
        }
    }

    if (!expectAndSkipToken(tokens, dbc_tok_colon)) { return false; }
    if (!expectInt(tokens, &start_bit)) { return false; }
    if (!expectAndSkipToken(tokens, dbc_tok_pipe)) { return false; }
    if (!expectInt(tokens, &length)) { return false; }
    if (!expectAndSkipToken(tokens, dbc_tok_at)) { return false; }
    if (!expectInt(tokens, &byte_order)) { return false; }

    if (expectAndSkipToken(tokens, dbc_tok_plus)) {
        is_unsigned = true;
    }

    if (!expectAndSkipToken(tokens, dbc_tok_parenth_open)) { return false; }
    if (!expectDouble(tokens, &factor)) { return false; }
    if (!expectAndSkipToken(tokens, dbc_tok_comma)) { return false; }
    if (!expectDouble(tokens, &offset)) { return false; }
    if (!expectAndSkipToken(tokens, dbc_tok_parenth_close)) { return false; }

    if (!expectAndSkipToken(tokens, dbc_tok_bracket_open)) { return false; }
    if (!expectDouble(tokens, &minimum)) { return false; }
    if (!expectAndSkipToken(tokens, dbc_tok_pipe)) { return false; }
    if (!expectDouble(tokens, &maximum)) { return false; }
    if (!expectAndSkipToken(tokens, dbc_tok_bracket_close)) { return false; }

    if (!expectString(tokens, &unit)) { return false; }

    if (!expectIdentifier(tokens, &receiver)) { return false; }
    receivers.append(receiver);

    while (!expectLineBreak(tokens)) {
        if (!expectAndSkipToken(tokens, dbc_tok_comma)) { return false; }
        if (!expectIdentifier(tokens, &receiver)) { return false; }
        receivers.append(receiver);
    }


    return true;
}

