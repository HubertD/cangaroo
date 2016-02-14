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


#include "FilterACL.h"

FilterACL::FilterACL()
{
    clear();
}

void FilterACL::clear()
{
    setDefaultAction(acl_action_continue);
    _rules.clear();
}

void FilterACL::setDefaultAction(acl_action_t action)
{
    _defaultAction = action;
}

void FilterACL::addRule(const FilterACLRule &rule)
{
    _rules.append(rule);
}

acl_action_t FilterACL::test(const CanMessage &msg)
{
    foreach (FilterACLRule rule, _rules) {
        acl_action_t action = rule.test(msg);
        if (action != acl_action_continue) {
            return action;
        }
    }
    return _defaultAction;
}
