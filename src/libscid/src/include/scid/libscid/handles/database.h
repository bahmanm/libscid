#ifndef SCID_LIBSCID_HANDLES_DATABASE_H
#define SCID_LIBSCID_HANDLES_DATABASE_H

#include "scid/database/scidbase.h"

#include <string>

struct scid_database
{
        scid::database::scidBaseT value;
        std::string               type;
};

#endif
