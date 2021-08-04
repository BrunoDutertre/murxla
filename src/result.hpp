#ifndef __MURXLA__RESULT_H
#define __MURXLA__RESULT_H

#include <iomanip>

namespace murxla {

/* -------------------------------------------------------------------------- */

enum Result
{
  RESULT_ERROR,
  RESULT_ERROR_CONFIG,
  RESULT_OK,
  RESULT_TIMEOUT,
  RESULT_UNKNOWN,
};

std::ostream& operator<<(std::ostream& out, const Result& res);

/* -------------------------------------------------------------------------- */

}  // namespace murxla

#endif