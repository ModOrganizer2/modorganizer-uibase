#pragma once

#include <QColor>
#include <QFlag>
#include <QFlags>
#include <QRect>
#include <QSize>
#include <QString>
#include <QVariant>

namespace MOBase::details
{
template <class CharT>
inline std::basic_string<CharT> toStdBasicString(QString const& qstring);

template <>
inline std::basic_string<char> toStdBasicString(QString const& qstring)
{
  return qstring.toStdString();
}
template <>
inline std::basic_string<wchar_t> toStdBasicString(QString const& qstring)
{
  return qstring.toStdWString();
}
template <>
inline std::basic_string<char16_t> toStdBasicString(QString const& qstring)
{
  return qstring.toStdU16String();
}
template <>
inline std::basic_string<char32_t> toStdBasicString(QString const& qstring)
{
  return qstring.toStdU32String();
}

inline QString fromStdBasicString(std::string const& value)
{
  return QString::fromStdString(value);
}
inline QString fromStdBasicString(std::wstring const& value)
{
  return QString::fromStdWString(value);
}
inline QString fromStdBasicString(std::u16string const& value)
{
  return QString::fromStdU16String(value);
}
inline QString fromStdBasicString(std::u32string const& value)
{
  return QString::fromStdU32String(value);
}
}  // namespace MOBase::details

template <class CharT>
struct std::formatter<QString, CharT> : std::formatter<std::basic_string<CharT>, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(QString s, FmtContext& ctx) const
  {
    return std::formatter<std::basic_string<CharT>, CharT>::format(
        MOBase::details::toStdBasicString<CharT>(s), ctx);
  }
};

template <class CharT1, class CharT2>
  requires(!std::is_same_v<CharT1, CharT2>)
struct std::formatter<std::basic_string<CharT1>, CharT2>
    : std::formatter<std::basic_string<CharT2>, CharT2>
{
  template <class FmtContext>
  FmtContext::iterator format(std::basic_string<CharT1> s, FmtContext& ctx) const
  {
    return std::formatter<std::basic_string<CharT2>, CharT2>::format(
        MOBase::details::toStdBasicString<CharT2>(
            MOBase::details::fromStdBasicString(s)),
        ctx);
  }
};

template <class CharT>
struct std::formatter<QStringView, CharT> : std::formatter<QString, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(QStringView s, FmtContext& ctx) const
  {
    return std::formatter<QString, CharT>::format(s.toString(), ctx);
  }
};

template <class CharT>
struct std::formatter<QSize, CharT> : std::formatter<std::basic_string<CharT>, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(QSize s, FmtContext& ctx) const
  {
    return std::format_to(ctx.out(), "QSize({}, {})", s.width(), s.height());
  }
};

template <class CharT>
struct std::formatter<QRect, CharT> : std::formatter<std::basic_string<CharT>, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(QRect r, FmtContext& ctx) const
  {
    return std::format_to(ctx.out(), "QRect({},{}-{},{})", r.left(), r.top(), r.right(),
                          r.bottom());
  }
};

template <class CharT>
struct std::formatter<QColor, CharT> : std::formatter<std::basic_string<CharT>, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(QColor c, FmtContext& ctx) const
  {
    return std::format_to(ctx.out(), "QColor({}, {}, {}, {})", c.red(), c.green(),
                          c.blue(), c.alpha());
  }
};

template <class CharT>
struct std::formatter<QByteArray, CharT>
    : std::formatter<std::basic_string<CharT>, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(QByteArray v, FmtContext& ctx) const
  {
    return std::format_to(ctx.out(), "QByteArray({} bytes)", v.size());
  }
};

template <class CharT>
struct std::formatter<QVariant, CharT> : std::formatter<std::basic_string<CharT>, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(QVariant v, FmtContext& ctx) const
  {
    return std::format_to(
        ctx.out(), "QVariant(type={}, value={})", v.typeName(),
        (v.typeId() == QMetaType::Type::QByteArray ? "(binary)" : v.toString()));
  }
};

template <class CharT>
struct std::formatter<QFlag, CharT> : std::formatter<int, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(QFlag v, FmtContext& ctx) const
  {
    return std::formatter<int, CharT>::format(static_cast<int>(v), ctx);
  }
};

template <class T, class CharT>
struct std::formatter<QFlags<T>, CharT> : std::formatter<int, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(QFlags<T> v, FmtContext& ctx) const
  {
    // TODO: display flags has aa | bb | cc?
    return std::formatter<int, CharT>::format(v.toInt(), ctx);
  }
};

template <class Enum, class CharT>
  requires std::is_enum_v<Enum>
struct std::formatter<Enum, CharT> : std::formatter<std::underlying_type_t<Enum>, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(Enum v, FmtContext& ctx) const
  {
    return std::formatter<std::underlying_type_t<Enum>, CharT>::format(
        static_cast<std::underlying_type_t<Enum>>(v), ctx);
  }
};

// random access container formatter (vector, QStringList, ...)

namespace MOBase::details
{
template <typename Container>
concept random_access_container = requires(Container const& c) {
  {
    c.begin()
  } -> std::random_access_iterator;
  {
    c.end()
  } -> std::random_access_iterator;
};
}  // namespace MOBase::details

// formatter specialization for random access container, the default format is
// [ a, b, c ] up to 3 elements, then [ a, b, ..., c ]
//
// it is possible to modify the output by changing the format string, check the test
// file for examples
//
template <typename Container, class CharT>
  requires MOBase::details::random_access_container<Container>
struct std::formatter<Container, CharT>
{
  static constexpr CharT field_delimiter = '$';

  // brackets to use
  CharT open_bracket  = static_cast<CharT>('[');
  bool space_bracket  = true;
  CharT close_bracket = static_cast<CharT>(']');

  // delimiter to use
  basic_string_view<CharT> delimiter = ", ";

  // maximum number of element to show
  std::size_t max_show = 3;

  template <class ParseContext>
  constexpr CharT next_or_throw(ParseContext const& ctx,
                                typename ParseContext::iterator it)
  {
    if (it == ctx.end()) {
      throw std::format_error("invalid format args for random access container.");
    }

    return *it;
  }

  template <class ParseContext>
  constexpr auto end_or_next(ParseContext const& ctx,
                             typename ParseContext::iterator& it)
  {
    if (it == ctx.end() || *it == '}') {
      return it;
    }

    if (*it == field_delimiter) {
      return ++it;
    }

    return it;
  }

  template <class ParseContext>
  constexpr auto parse_size(ParseContext& ctx, typename ParseContext::iterator it,
                            std::size_t& value)
  {
    if (it == ctx.end() || *it == '}' || *it == field_delimiter) {
      return it;
    }
    value = value * 10 + (*it++ - static_cast<CharT>('0'));
    return parse_size(ctx, it, value);
  }

  template <class ParseContext>
  constexpr auto find_end(ParseContext& ctx, typename ParseContext::iterator it)
  {
    if (it == ctx.end() || *it == '}' || *it == field_delimiter) {
      return it;
    }
    return find_end(ctx, ++it);
  }

  template <class ParseContext>
  constexpr ParseContext::iterator parseImpl(ParseContext& ctx,
                                             typename ParseContext::iterator it)
  {
    if (it == ctx.end() || *it == '}') {
      return it;
    }

    const auto c = *it++;
    switch (c) {
    case 'b':
      open_bracket  = next_or_throw(ctx, it++);
      close_bracket = next_or_throw(ctx, it++);

      if (close_bracket == ' ') {
        space_bracket = true;
        close_bracket = next_or_throw(ctx, it++);
      } else {
        space_bracket = false;
      }

      it = end_or_next(ctx, it);
      break;

    case 'n': {
      max_show = 0;
      it       = parse_size(ctx, it, max_show);

      if (*it == field_delimiter) {
        it++;
      }

    } break;

    case 'd': {
      const auto start = it;
      it               = find_end(ctx, it);
      delimiter        = std::basic_string_view<CharT>(start, it);

      if (*it == field_delimiter) {
        it++;
      }

    } break;

    default:
      throw std::format_error("invalid format args for random access container.");
    }

    return parseImpl(ctx, it);
  }

  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext& ctx)
  {
    return parseImpl(ctx, ctx.begin());
  }

  template <typename FmtIt, typename It>
  FmtIt formatRange(FmtIt& out, It cur, It end) const
  {
    if (cur == end) {
      return out;
    }

    out = std::format_to(out, "{}", *cur);
    ++cur;

    if (cur != end) {
      out = std::format_to(out, "{}", delimiter);
    }

    return formatRange(out, cur, end);
  }

  template <typename FmtContext>
  FmtContext::iterator format(Container v, FmtContext& ctx) const
  {
    const auto n = v.end() - v.begin();

    if (n == 0) {
      return std::format_to(ctx.out(), "{}{}", open_bracket, close_bracket);
    }

    auto out =
        std::format_to(ctx.out(), "{}{}", open_bracket, space_bracket ? " " : "");

    if (n <= max_show) {
      out = formatRange(out, v.begin(), v.end());
    } else {
      const auto n_first = (max_show % 2 == 1) ? max_show / 2 + 1 : max_show / 2;
      out                = formatRange(out, v.begin(), v.begin() + n_first);
      out                = std::format_to(out, "{}...{}", delimiter, delimiter);
      out                = formatRange(out, v.end() - (max_show - n_first), v.end());
    }

    return std::format_to(ctx.out(), "{}{}", space_bracket ? " " : "", close_bracket);
  }
};
