#pragma once

#include <QString>
#include <QVariant>

namespace MOBase
{

// class representing a group of settings
//
class SettingGroup
{
public:
  SettingGroup(QString const& name, QString const& title, QString const& description)
      : m_Name{name}, m_Title{title}, m_Description{description}
  {}

  // return the (internal) name of this group, localization independent
  //
  const auto& name() const { return m_Name; }

  // retrieve the title of this group, can be localized
  //
  const auto& title() const { return m_Title; }

  // retrieve the description of this group, can be localized
  //
  const auto& description() const { return m_Description; }

private:
  QString m_Name, m_Title, m_Description;
};

// class that represents an extension or a plugin setting
//
class Setting
{
public:
  // deprecated constructor that was previously available as PluginSettin
  //
  [[deprecated]] Setting(const QString& name, const QString& description,
                         const QVariant& defaultValue)
      : m_Name{name}, m_Title{name}, m_Description{description}, m_Group{},
        m_DefaultValue{defaultValue}
  {}

  Setting(const QString& name, const QString& title, const QString& description,
          const QVariant& defaultValue)
      : m_Name{name}, m_Title{title}, m_Description{description}, m_Group{},
        m_DefaultValue{defaultValue}
  {}

  Setting(const QString& name, const QString& title, const QString& description,
          const QString& group, const QVariant& defaultValue)
      : m_Name{name}, m_Title{title}, m_Description{description}, m_Group{group},
        m_DefaultValue{defaultValue}
  {}

public:
  // return the (internal) name of this setting, localization independent
  //
  const auto& name() const { return m_Name; }

  // retrieve the title of this setting, can be localized
  //
  const auto& title() const { return m_Title; }

  // retrieve the description of this setting, can be localized
  //
  const auto& description() const { return m_Description; }

  // retrieve the name of the group this settings belongs to or an empty string if there
  // is none
  //
  const auto& group() const { return m_Group; }

  // retrieve the default value of this setting
  //
  const auto& defaultValue() const { return m_DefaultValue; }

private:
  QString m_Name;
  QString m_Title;
  QString m_Description;
  QString m_Group;
  QVariant m_DefaultValue;
};

}  // namespace MOBase
