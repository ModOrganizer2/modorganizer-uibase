#include "strings.h"

#include <algorithm>
#include <locale>

namespace MOBase
{

// this is strongly inspired from boost
class is_iequal
{
  std::locale m_loc;

public:
  is_iequal(const std::locale& loc = std::locale()) : m_loc{loc} {}

  template <typename T1, typename T2>
  bool operator()(const T1& Arg1, const T2& Arg2) const
  {
    return std::toupper<T1>(Arg1, m_loc) == std::toupper<T2>(Arg2, m_loc);
  }
};

bool iequals(std::string_view lhs, std::string_view rhs)
{
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), is_iequal());
}

void ireplace_all(std::string& input, std::string_view search,
                  std::string_view replace) noexcept
{
  const auto search_length  = static_cast<std::string::difference_type>(search.size());
  const auto replace_length = replace.size();
  auto it                   = input.begin();
  while (input.end() - it >= search_length) {
    const auto search_end = it + search_length;
    if (iequals(std::string_view(it, search_end), search)) {
      input.replace(it, search_end, replace);
      it += static_cast<std::string::difference_type>(replace_length);
    } else {
      ++it;
    }
  }
}

}  // namespace MOBase
