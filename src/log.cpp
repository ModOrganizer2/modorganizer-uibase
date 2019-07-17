#include "pch.h"
#include "log.h"
#include <iostream>

#pragma warning(push)
#pragma warning(disable: 4365)
#define SPDLOG_WCHAR_FILENAMES 1
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#pragma warning(pop)


namespace MOBase::log
{

namespace fs = std::filesystem;

static std::unique_ptr<Logger> g_default;
static bool g_console = false;
static File g_file;
static bool g_callbackEnabled = false;
static Callback* g_callback = nullptr;


spdlog::level::level_enum toSpdlog(Levels lv)
{
  switch (lv)
  {
    case Debug:
      return spdlog::level::debug;

    case Warning:
      return spdlog::level::warn;

    case Error:
      return spdlog::level::err;

    case Info:  // fall-through
    default:
      return spdlog::level::info;
  }
}

Levels fromSpdlog(spdlog::level::level_enum lv)
{
  switch (lv)
  {
    case spdlog::level::trace:
    case spdlog::level::debug:
      return Debug;

    case spdlog::level::warn:
      return Warning;

    case spdlog::level::critical: // fall-through
    case spdlog::level::err:
      return Error;

    case spdlog::level::info:  // fall-through
    case spdlog::level::off:
    default:
      return Info;
  }
}


class CallbackSink : public spdlog::sinks::base_sink<std::mutex>
{
public:
  void sink_it_(const spdlog::details::log_msg& m) override
  {
    thread_local bool active = false;

    if (active) {
      // trying to log from a log callback, ignoring
      return;
    }

    if (!g_callbackEnabled || !g_callback) {
      // disabled
      return;
    }

    try
    {
      auto g = Guard([&]{ active = false; });
      active = true;

      Entry e;
      e.time = m.time;
      e.level = fromSpdlog(m.level);
      e.message = fmt::to_string(m.payload);

      fmt::memory_buffer formatted;
      sink::formatter_->format(m, formatted);

      if (formatted.size() >= 2) {
        // remove \r\n
        e.formattedMessage.assign(formatted.begin(), formatted.end() - 2);
      } else {
        e.formattedMessage = fmt::to_string(formatted);
      }

      g_callback(std::move(e));
    }
    catch(std::exception& e)
    {
      fprintf(
        stderr, "uncaugh exception un logging callback, %s\n",
        e.what());
    }
    catch(...)
    {
      fprintf(stderr, "uncaught exception in logging callback\n");
    }
  }

  void flush_() override
  {
    // no-op
  }
};


File::File() :
  type(None),
  maxSize(0), maxFiles(0),
  dailyHour(0), dailyMinute(0)
{
}

File File::daily(fs::path file, int hour, int minute)
{
  File fl;

  fl.type = Daily;
  fl.file = std::move(file);
  fl.dailyHour = hour;
  fl.dailyMinute = minute;

  return fl;
}

File File::rotating(
  fs::path file, std::size_t maxSize, std::size_t maxFiles)
{
  File fl;

  fl.type = Rotating;
  fl.file = std::move(file);
  fl.maxSize = maxSize;
  fl.maxFiles = maxFiles;

  return fl;
}

spdlog::sink_ptr make_sink(const File& f)
{
  try
  {
    switch (f.type)
    {
      case File::Daily:
      {
        return std::make_shared<spdlog::sinks::daily_file_sink_mt>(
          f.file.native(), f.dailyHour, f.dailyMinute);
      }

      case File::Rotating:
      {
        return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
          f.file.native(), f.maxSize, f.maxFiles);
      }

      case File::None:  // fall-through
      default:
        return {};
    }
  }
  catch(spdlog::spdlog_ex& e)
  {
    std::cerr << "failed to create file log, " << e.what() << "\n";
    return {};
  }
}


spdlog::sink_ptr get_stderr_sink()
{
  static auto s = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  return s;
}

spdlog::sink_ptr get_cb_sink()
{
  static auto s = std::make_shared<CallbackSink>();
  return s;
}

spdlog::sink_ptr get_file_sink()
{
  static auto s = make_sink(g_file);
  return s;
}


Logger::Logger(std::string name, Levels maxLevel, std::string pattern)
  : m_logger(createLogger(name))
{
  m_logger->set_level(toSpdlog(maxLevel));
  m_logger->set_pattern(pattern);
  m_logger->flush_on(spdlog::level::trace);
}

Levels Logger::level() const
{
  return fromSpdlog(m_logger->level());
}

void Logger::setLevel(Levels lv)
{
  m_logger->set_level(toSpdlog(lv));
}

void Logger::setPattern(const std::string& s)
{
  m_logger->set_pattern(s);
}

std::unique_ptr<spdlog::logger> Logger::createLogger(const std::string& name)
{
  std::vector<spdlog::sink_ptr> sinks;

  if (auto s=get_stderr_sink()) {
    sinks.push_back(s);
  }

  if (auto s=get_cb_sink()) {
    sinks.push_back(s);
  }

  if (auto s=get_file_sink()) {
    sinks.push_back(std::move(s));
  }

  using sink_type = spdlog::sinks::wincolor_stderr_sink_mt;

  for (auto& s : sinks) {
    if (auto* cs=dynamic_cast<sink_type*>(&*s)) {
      cs->set_color(spdlog::level::info, cs->WHITE);
      cs->set_color(spdlog::level::debug, cs->WHITE);
    }
  }

  return std::make_unique<spdlog::logger>(name, sinks.begin(), sinks.end());
}


void init(
  bool console, const File& file,
  Levels maxLevel, const std::string& pattern,
  Callback* callback)
{
  g_console = console;
  g_file = file;
  g_callback = callback;
  g_callbackEnabled = (callback != nullptr);

  g_default = std::make_unique<Logger>("default", maxLevel, pattern);
}

Logger& getDefault()
{
  return *g_default;
}

} // namespace


namespace MOBase::log::details
{

void doLogImpl(spdlog::logger& lg, Levels lv, const std::string& s)
{
  const char* start = s.c_str();
  const char* p = start;

  for (;;) {
    while (*p && *p != '\n') {
      ++p;
    }

    std::string_view sv(start, static_cast<std::size_t>(p - start));
    lg.log(toSpdlog(lv), "{}", sv);

    if (!*p) {
      break;
    }

    ++p;
    start = p;
  }
}

}	// namespace
