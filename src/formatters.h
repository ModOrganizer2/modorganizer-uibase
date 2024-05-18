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
