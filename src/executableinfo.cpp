#include "executableinfo.h"

using namespace MOBase;


MOBase::ExecutableInfo::ExecutableInfo(const QString &title, const QFileInfo &binary)
  : m_Title(title)
  , m_Binary(binary)
  , m_WorkingDirectory(binary.exists() ? binary.absoluteDir() : QString())
  , m_SteamAppID()
{
}

ExecutableInfo &ExecutableInfo::withArgument(const QString &argument)
{
  m_Arguments.append(argument);
  return *this;
}

ExecutableInfo &ExecutableInfo::withWorkingDirectory(const QDir &workingDirectory)
{
  m_WorkingDirectory = workingDirectory;
  return *this;
}

ExecutableInfo &MOBase::ExecutableInfo::withSteamAppId(const QString &appId)
{
  m_SteamAppID = appId;
  return *this;
}

ExecutableInfo &ExecutableInfo::asCustom()
{
  m_Custom = true;
  return *this;
}

bool ExecutableInfo::isValid() const
{
  return m_Binary.exists();
}

QString ExecutableInfo::title() const
{
  return m_Title;
}

QFileInfo ExecutableInfo::binary() const
{
  return m_Binary;
}

QStringList ExecutableInfo::arguments() const
{
  return m_Arguments;
}

QDir ExecutableInfo::workingDirectory() const
{
  return m_WorkingDirectory;
}

QString ExecutableInfo::steamAppID() const
{
  return m_SteamAppID;
}

bool ExecutableInfo::isCustom() const
{
  return m_Custom;
}
