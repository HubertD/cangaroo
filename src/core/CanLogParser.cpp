/*

  Copyright (c) 2026 Jayachandran Dharuman

  This file is part of CANgaroo.

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

#include "CanLogParser.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QRegularExpression>
#include <QDateTime>

bool CanLogParser::parseCanDump(const QString& filename, QVector<CanMessage>& outMessages)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#") || line.startsWith(";"))
            continue;

        bool ok = false;
        CanMessage msg = parseCanDumpLine(line, ok);
        if (ok) {
            outMessages.append(msg);
        }
    }

    return true;
}

CanMessage CanLogParser::parseCanDumpLine(const QString& line, bool& ok)
{
    // candump format: (timestamp) interface ID#DATA
    // e.g. (1710842000.123456) vcan0 123#1122334455667788
    // e.g. (1710842000.123456) vcan0 00000123#1122334455667788
    
    CanMessage msg;
    ok = false;

    QRegularExpression re("^\\((?<timestamp>\\d+\\.\\d+)\\)\\s+(?<interface>\\S+)\\s+(?<id>[0-9A-Fa-f]+)#(?<data>[0-9A-Fa-f]*)$");
    QRegularExpressionMatch match = re.match(line);

    if (match.hasMatch()) {
        double timestamp = match.captured("timestamp").toDouble();
        
        // Split timestamp into seconds and microseconds
        uint64_t seconds = static_cast<uint64_t>(timestamp);
        uint32_t micro_seconds = static_cast<uint32_t>((timestamp - seconds) * 1000000.0);
        msg.setTimestamp(seconds, micro_seconds);

        QString idStr = match.captured("id");
        bool idOk;
        uint32_t id = idStr.toUInt(&idOk, 16);
        if (!idOk) return msg;

        // In candump, standard vs extended is determined implicitly by interface or ID length length, 
        // but typically 3 chars vs 8 chars. We will set extended if ID > 0x7FF or id string length > 3
        if (id > 0x7FF || idStr.length() > 3) {
            msg.setExtended(true);
            msg.setId(id & 0x1FFFFFFF);
        } else {
            msg.setExtended(false);
            msg.setId(id & 0x7FF);
        }

        QString dataStr = match.captured("data");
        uint8_t dlc = dataStr.length() / 2;
        msg.setLength(dlc);
        for (int i = 0; i < dlc && i < 64; ++i) {
            QString byteStr = dataStr.mid(i * 2, 2);
            msg.setByte(i, byteStr.toUInt(nullptr, 16));
        }

        ok = true;
    } else {
        // Try readable date format with DLC: (YYYY-MM-DD HH:MM:SS.uuuuuu) interface ID [DLC] DATA...
        QRegularExpression re2("^\\s*\\((?<date>\\d{4}-\\d{2}-\\d{2})\\s+(?<time>\\d{2}:\\d{2}:\\d{2})\\.(?<us>\\d+)\\)\\s+(?<interface>\\S+)\\s+(?<id>[0-9A-Fa-f]+)\\s+\\[(?<dlc>\\d+)\\]\\s*(?<data>([0-9A-Fa-f]{2}\\s*)*)$");
        QRegularExpressionMatch match2 = re2.match(line);

        if (match2.hasMatch()) {
            QDateTime dt = QDateTime::fromString(match2.captured("date") + " " + match2.captured("time"), "yyyy-MM-dd HH:mm:ss");
            uint64_t seconds = dt.toSecsSinceEpoch();
            
            QString usStr = match2.captured("us");
            while(usStr.length() < 6) usStr += "0";
            uint32_t micro_seconds = usStr.left(6).toUInt();
            
            msg.setTimestamp(seconds, micro_seconds);

            QString idStr = match2.captured("id");
            bool idOk;
            uint32_t id = idStr.toUInt(&idOk, 16);
            if (!idOk) return msg;

            if (id > 0x7FF || idStr.length() > 3) {
                msg.setExtended(true);
                msg.setId(id & 0x1FFFFFFF);
            } else {
                msg.setExtended(false);
                msg.setId(id & 0x7FF);
            }

            QString dataStr = match2.captured("data").remove(" ");
            uint8_t dlcOk = match2.captured("dlc").toUInt();
            msg.setLength(dlcOk);
            
            int datalen = dataStr.length() / 2;
            for (int i = 0; i < dlcOk && i < datalen && i < 64; ++i) {
                QString byteStr = dataStr.mid(i * 2, 2);
                msg.setByte(i, byteStr.toUInt(nullptr, 16));
            }

            ok = true;
        }
    }

    return msg;
}

bool CanLogParser::parseVectorAsc(const QString& filename, QVector<CanMessage>& outMessages)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    double baseTime = 0.0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("//") || line.startsWith("date") || line.startsWith("base") || line.startsWith("internal") || line.startsWith("End "))
            continue;

        if (line.startsWith("Begin Triggerblock")) {
            // Might have "0.000000 Start of measurement" next
            continue;
        }

        if (line.contains("Start of measurement")) {
            // usually "   0.000000 Start of measurement"
            continue;
        }

        bool ok = false;
        CanMessage msg = parseVectorAscLine(line, baseTime, ok);
        if (ok) {
            outMessages.append(msg);
        }
    }

    return true;
}

CanMessage CanLogParser::parseVectorAscLine(const QString& line, double baseTime, bool& ok)
{
    // format from saveVectorAsc:
    // 0.000000 1  000A1234x Rx   d 8 11 22 33 44 55 66 77 88 Length = 0 BitCount = 0 ID = 123
    CanMessage msg;
    ok = false;

    QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.size() < 6) return msg;

    // parts[0] is timestamp
    bool tsOk;
    double offsetTime = parts[0].toDouble(&tsOk);
    if (!tsOk) return msg;

    double timestamp = baseTime + offsetTime;
    uint64_t seconds = static_cast<uint64_t>(timestamp);
    uint32_t micro_seconds = static_cast<uint32_t>((timestamp - seconds) * 1000000.0);
    msg.setTimestamp(seconds, micro_seconds);

    // parts[1] is normally CAN channel (e.g. "1")
    
    // parts[2] is ID (e.g. "123" or "123x")
    QString idStr = parts[2];
    if (idStr.endsWith("x", Qt::CaseInsensitive)) {
        msg.setExtended(true);
        idStr.chop(1);
    } else {
        msg.setExtended(false);
    }
    
    bool idOk;
    msg.setId(idStr.toUInt(&idOk, 16));
    if (!idOk) return msg;

    // parts[3] is dir (Rx/Tx)
    // parts[4] is 'd'
    // parts[5] is DLC
    if (parts[4] != "d") return msg;

    bool dlcOk;
    uint8_t dlc = parts[5].toUInt(&dlcOk);
    if (!dlcOk) return msg;
    msg.setLength(dlc);

    // Data bytes start at index 6
    if (parts.size() >= 6 + dlc) {
        for (int i = 0; i < dlc; ++i) {
            msg.setByte(i, parts[6 + i].toUInt(nullptr, 16));
        }
        ok = true;
    }

    return msg;
}
