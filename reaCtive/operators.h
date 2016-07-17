
#pragma once
#if ! defined(REAcTIVE_OPERATORS_H)
#define REAcTIVE_OPERATORS_H

#include "core.h"

/* COUNT: generates an endless (until overflow) stream of ints */
Observable *reaC_new_count();

/* LIMIT: stops a sequence after a number of results */
Observable *reaC_op_limit(Observable *producer, int max);

#endif
