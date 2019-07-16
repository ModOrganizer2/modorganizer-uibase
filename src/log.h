#pragma once

#include <string>
#include <filesystem>
#include <fmt/format.h>

#pragma warning(push)
#pragma warning(disable: 4365)
#define SPDLOG_WCHAR_FILENAMES 1
#include <spdlog/logger.h>
#pragma warning(pop)

#include "dllimport.h"

namespace MOBase::log::details
{

void QDLLEXPORT doLogImpl(
  spdlog::logger& lg, spdlog::level::level_enum lv, const std::string& s);

template <class F, class... Args>
void doLog(
  spdlog::logger& logger, spdlog::level::level_enum lv,
  F&& format, Args&&... args) noexcept
{
  try
  {
    const auto s = fmt::format(
      std::forward<F>(format), std::forward<Args>(args)...);

    doLogImpl(logger, lv, s);
  }
  catch(std::exception& e)
  {
    fprintf(stderr, "uncaugh exception while logging, %s\n", e.what());
  }
  catch(...)
  {
    fprintf(stderr, "uncaugh exception while logging\n");
  }
}

} // namespace


namespace MOBase::log
{

enum Levels
{
  Debug   = spdlog::level::debug,
  Info    = spdlog::level::info,
  Warning = spdlog::level::warn,
  Error   = spdlog::level::err
};

class QDLLEXPORT Logger
{
public:
  Logger(std::string name, Levels maxLevel, std::string pattern);

  void setLevel(Levels lv);
  void setPattern(const std::string& pattern);

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
      m_logger, static_cast<spdlog::level::level_enum>(lv),
      std::forward<F>(format), std::forward<Args>(args)...);
  }

private:
  spdlog::logger m_logger;

  static spdlog::logger createLogger(const std::string& name);
};


struct Entry
{
  std::chrono::system_clock::time_point time;
  Levels level;
  std::string message;
  std::string formattedMessage;
};


class QDLLEXPORT File
{
public:
  enum Types
  {
    None = 0,
    Daily,
    Rotating
  };

  File();

  static File daily(std::filesystem::path file, int hour, int minute);

  static File rotating(
    std::filesystem::path file, std::size_t maxSize, std::size_t maxFiles);

  spdlog::sink_ptr make_sink() const;

private:
  Types m_type;
  std::filesystem::path m_file;
  std::size_t m_maxSize, m_maxFiles;
  int m_dailyHour, m_dailyMinute;
};


using Callback = void (Entry);

QDLLEXPORT void init(
  bool console, const File& file,
  Levels maxLevel, const std::string& pattern,
  Callback* callback=nullptr);

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

} // namespace
