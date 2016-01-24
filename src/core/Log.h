/*

  Copyright (c) 2016 Hubert Denkmair <hubert@denkmair.de>

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

#include <QDateTime>

typedef enum log_level {
    log_level_debug,
    log_level_info,
    log_level_warning,
    log_level_error,
    log_level_critical,
    log_level_fatal
} log_level_t;

void log_msg(const QDateTime dt, const log_level_t level, const QString msg);
void log_msg(const log_level_t level, const QString msg);

void log_debug(const QString msg);
void log_info(const QString msg);
void log_warning(const QString msg);
void log_error(const QString msg);
void log_critical(const QString msg);
void log_fatal(const QString msg);
