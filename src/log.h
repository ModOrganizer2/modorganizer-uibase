#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <QList>
#include <QString>
#include <QStringView>
#include <QSize>
#include <QRect>
#include <QColor>

#pragma warning(push)
#pragma warning(disable : 4061 4459 4574 4582)
#include <fmt/format.h>
#include <fmt/xchar.h>
#pragma warning(pop)

#include "dllimport.h"

namespace spdlog { class logger; }
namespace spdlog::sinks { class sink; }

namespace MOBase::log
{

enum Levels
{
  Debug   = 0,
  Info    = 1,
  Warning = 2,
  Error   = 3
};

struct BlacklistEntry
{
    std::string filter;
    std::string replacement;
};

} // namespace


namespace MOBase::log::details
{

// T to std::string converters
//
// those are kept in this namespace so they don't leak all over the place;
// they're used directly by doLog() below

template <class T, class = void>
struct converter
{
  static const T& convert(const T& t)
  {
    return t;
  }
};

template <>
struct QDLLEXPORT converter<std::wstring>
{
  static std::string convert(const std::wstring& s);
};

template <>
struct QDLLEXPORT converter<QString>
{
  static std::string convert(const QString& s);
};

template <>
struct QDLLEXPORT converter<QStringView>
{
  static std::string convert(const QStringView& s);
};


template <>
struct QDLLEXPORT converter<QSize>
{
  static std::string convert(const QSize& s);
};

template <>
struct QDLLEXPORT converter<QRect>
{
  static std::string convert(const QRect& s);
};

template <>
struct QDLLEXPORT converter<QColor>
{
  static std::string convert(const QColor& c);
};

template <>
struct QDLLEXPORT converter<QByteArray>
{
  static std::string convert(const QByteArray& v);
};

template <>
struct QDLLEXPORT converter<QVariant>
{
  static std::string convert(const QVariant& v);
};

// custom converter for enum and enum class that seems to not work with latest fmt
template <typename T>
struct QDLLEXPORT converter<T, std::enable_if_t<std::is_enum_v<T>>>
{
  static auto convert(const T& v)
  {
    return static_cast<std::underlying_type_t<T>>(v);
  }
};

void QDLLEXPORT doLogImpl(
  spdlog::logger& lg, Levels lv, const std::string& s) noexcept;

void QDLLEXPORT ireplace_all(std::string& input, std::string const& search, std::string const& replace) noexcept;

template <class F, class... Args>
void doLog(
  spdlog::logger& logger, Levels lv, const std::vector<MOBase::log::BlacklistEntry> bl, F&& format, Args&&... args) noexcept
{
  std::string s;

  // format errors are logged without much information to avoid throwing again

  try
  {
    if constexpr (sizeof... (Args) == 0) {
      s = fmt::format("{}", std::forward<F>(format));
    }
    else {
      s = fmt::vformat(
        std::forward<F>(format),
        fmt::make_format_args(
          converter<std::decay_t<Args>>::convert(std::forward<Args>(args))...));
    }

    // check the blacklist
    for (const BlacklistEntry& entry : bl) {
      ireplace_all(s, entry.filter, entry.replacement);
    }
  }
  catch(fmt::format_error&)
  {
    s = "format error while logging";
    lv = Levels::Error;
  }
  catch(std::exception&)
  {
    s = "exception while formatting for logging";
    lv = Levels::Error;
  }
  catch(...)
  {
    s = "unknown exception while formatting for logging";
    lv = Levels::Error;
  }

  doLogImpl(logger, lv, s);
}

} // namespace


namespace MOBase::log
{

struct QDLLEXPORT File
{
public:
  enum Types
  {
    None = 0,
    Daily,
    Rotating,
    Single
  };

  File();

  static File daily(std::filesystem::path file, int hour, int minute);

  static File rotating(
    std::filesystem::path file, std::size_t maxSize, std::size_t maxFiles);

  static File single(std::filesystem::path file);

  Types type;
  std::filesystem::path file;
  std::size_t maxSize, maxFiles;
  int dailyHour, dailyMinute;
};


struct Entry
{
  std::chrono::system_clock::time_point time;
  Levels level;
  std::string message;
  std::string formattedMessage;
};

using Callback = void (Entry);

struct LoggerConfiguration
{
  std::string name;
  Levels maxLevel = Levels::Info;
  std::string pattern;
  bool utc = false;
  std::vector<BlacklistEntry> blacklist;
};


class QDLLEXPORT Logger
{
public:
  Logger(LoggerConfiguration conf);
  ~Logger();

  Levels level() const;
  void setLevel(Levels lv);

  void setPattern(const std::string& pattern);
  void setFile(const File& f);
  void setCallback(Callback* f);

  void addToBlacklist(const std::string& filter, const std::string& replacement);
  void removeFromBlacklist(const std::string& filter);
  void resetBlacklist();

  template <class F, class... Args>
  void debug(F&& format, Args&&... args) noexcept
  {
    log(Debug, std::forward<F>(format), std::forward<Args>(args)...);
  }

  template <class F, class... Args>
  void info(F&& format, Args&&... args) noexcept
  {
    log(Info, std::forward<F>(format), std::forward<Args>(args)...);
  }

  template <class F, class... Args>
  void warn(F&& format, Args&&... args) noexcept
  {
    log(Warning, std::forward<F>(format), std::forward<Args>(args)...);
  }

  template <class F, class... Args>
  void error(F&& format, Args&&... args) noexcept
  {
    log(Error, std::forward<F>(format), std::forward<Args>(args)...);
  }

  template <class F, class... Args>
  void log(Levels lv, F&& format, Args&&... args) noexcept
  {
    details::doLog(
      *m_logger, lv, m_conf.blacklist, std::forward<F>(format), std::forward<Args>(args)...);
  }

private:
  LoggerConfiguration m_conf;
  std::unique_ptr<spdlog::logger> m_logger;
  std::shared_ptr<spdlog::sinks::sink> m_sinks;
  std::shared_ptr<spdlog::sinks::sink> m_console, m_callback, m_file;

  void createLogger(const std::string& name);
  void addSink(std::shared_ptr<spdlog::sinks::sink> sink);
};

QDLLEXPORT void createDefault(LoggerConfiguration conf);
QDLLEXPORT Logger& getDefault();


template <class F, class... Args>
void debug(F&& format, Args&&... args) noexcept
{
  getDefault().debug(
    std::forward<F>(format), std::forward<Args>(args)...);
}

template <class F, class... Args>
void info(F&& format, Args&&... args) noexcept
{
  getDefault().info(
    std::forward<F>(format), std::forward<Args>(args)...);
}

template <class F, class... Args>
void warn(F&& format, Args&&... args) noexcept
{
  getDefault().warn(
    std::forward<F>(format), std::forward<Args>(args)...);
}

template <class F, class... Args>
void error(F&& format, Args&&... args) noexcept
{
  getDefault().error(
    std::forward<F>(format), std::forward<Args>(args)...);
}

template <class F, class... Args>
void log(Levels lv, F&& format, Args&&... args) noexcept
{
  getDefault().log(
    lv, std::forward<F>(format), std::forward<Args>(args)...);
}


//
QDLLEXPORT QString levelToString(Levels level);

} // namespace
