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

#include <stdint.h>
#include <core/CanMessage.h>
#include "TraceViewTypes.h"

class FilterACLRule
{
public:
    FilterACLRule(uint32_t can_id, acl_action_t action);
    acl_action_t test(const CanMessage &msg);
private:
    uint32_t _can_id;
    acl_action_t _action;
};
