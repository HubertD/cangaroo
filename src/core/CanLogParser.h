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

#pragma once

#include <QString>
#include <QVector>
#include "CanMessage.h"

class CanLogParser
{
public:
    static bool parseCanDump(const QString& filename, QVector<CanMessage>& outMessages);
    static bool parseVectorAsc(const QString& filename, QVector<CanMessage>& outMessages);

private:
    static CanMessage parseCanDumpLine(const QString& line, bool& ok);
    static CanMessage parseVectorAscLine(const QString& line, double baseTime, bool& ok);
};
