#pragma once

#include "HierarchyGuard.h"
#include <string>
#include <map>

using DatabaseGuard = StrIDGuard;
using DDLGuards = std::map<std::string, DatabaseGuard>;
