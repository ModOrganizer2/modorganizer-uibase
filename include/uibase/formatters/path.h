#pragma once

#include <filesystem>
#include <format>
#include <type_traits>

template <class CharT>
struct std::formatter<std::filesystem::path, CharT>
    : std::formatter<std::filesystem::path::string_type, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(const std::filesystem::path& v, FmtContext& ctx) const
  {
    return std::formatter<std::filesystem::path::string_type, CharT>::format(v.native(),
                                                                             ctx);
  }
};
