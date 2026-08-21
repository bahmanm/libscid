#ifndef SCID_LIBSCID_HANDLES_DATABASE_H
#define SCID_LIBSCID_HANDLES_DATABASE_H

#include "scid/database.h"
#include "scid/database_filter.h"
#include "scid/database/scidbase.h"

#include <string>
#include <utility>
#include <vector>

struct scid_database
{
        scid::database::scidBaseT                           value;
        std::string                                         type;
        scid_filter_id                                      next_filter_id = 1;
        std::vector<std::pair<scid_filter_id, std::string>> filters;
};

#endif
