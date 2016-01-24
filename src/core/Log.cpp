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

#include "Log.h"
#include <core/Backend.h>

void log_msg(const QDateTime dt, const log_level_t level, const QString msg)
{
    Backend::instance().logMessage(dt, level, msg);
}

void log_msg(const log_level_t level, const QString msg)
{
    Backend::instance().logMessage(QDateTime::currentDateTime(), level, msg);
}

void log_debug(const QString msg)
{
    log_msg(log_level_debug, msg);
}

void log_info(const QString msg)
{
    log_msg(log_level_info, msg);
}

void log_warning(const QString msg)
{
    log_msg(log_level_warning, msg);
}

void log_error(const QString msg)
{
    log_msg(log_level_error, msg);
}

void log_critical(const QString msg)
{
    log_msg(log_level_critical, msg);
}

void log_fatal(const QString msg)
{
    log_msg(log_level_fatal, msg);
}
