#ifndef UIBASE_MOASSERT_INCLUDED
#define UIBASE_MOASSERT_INCLUDED

#include "log.h"

namespace MOBase
{

template <class T>
inline void mo_assert(
  T&& t, const char* exp, const char* file, int line, const char* func)
{
  if (!t)
  {
    log::error("assertion failed: {}:{} {}: '{}'", file, line, func, exp);
    DebugBreak();
  }
}

} // namespace


#define MO_ASSERT(v) mo_assert(v, #v, __FILE__, __LINE__, __FUNCSIG__);

#endif // UIBASE_MOASSERT_INCLUDED
