/*
Mod Organizer shared UI functionality

Copyright (C) 2013 Sebastian Herbord. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef IPLUGINDIAGNOSE_H
#define IPLUGINDIAGNOSE_H


#include <vector>
#include <functional>
#include <QObject>
#include <QString>
#pragma warning(push)
#pragma warning(disable: 4100)
#include <boost/signals2.hpp>
#pragma warning(pop)

namespace MOBase {


/**
 * brief A plugin that creates problem reports to be displayed in the UI.
 * This can be used to report problems related to the same plugin (which implements further
 * interfaces) or as a stand-alone diagnosis tool.
 * This does not derive from IPlugin to prevent multiple inheritance issues. For stand-alone
 * diagnosis plugins, derive from IPlugin and IPluginDiagnose
 */
class IPluginDiagnose {
public:

  /// signal to be emitted when the diagnosis information of the plugin is invalidated
  typedef boost::signals2::signal<void (void)> SignalInvalidated;

public:

  /**
   * @return a list of keys of active problems
   * @note this is not expected to be called if isActive returns false
   */
  virtual std::vector<unsigned int> activeProblems() const = 0;

  /**
   * @brief retrieve a short description for the specified problem for the overview page. HTML syntax is supported.
   * @param key the identifier of the problem as reported by activeProblems()
   * @return a (short) description text
   * @throw should throw an exception if the key is not valid
   */
  virtual QString shortDescription(unsigned int key) const = 0;

  /**
   * @brief retrieve the full description for the specified problem. HTML syntax is supported.
   * @param key the identifier of the problem as reported by activeProblems()
   * @return a (long) description text
   * @throw should throw an exception if the key is not valid
   */
  virtual QString fullDescription(unsigned int key) const = 0;

  /**
   * @param key the identifier of the problem as reported by activeProblems()
   * @return true if this plugin provides a guide to fix the issue
   * @throw should throw an exception if the key is not valid
   */
  virtual bool hasGuidedFix(unsigned int key) const = 0;

  /**
   * @brief start the guided fix for the specified problem
   * @param key the identifier of the problem as reported by activeProblems()
   * @throw should throw an exception if the key is not valid or if there is no guided fix for the issue
   */
  virtual void startGuidedFix(unsigned int key) const = 0;

  /**
   * @brief the application will use this to register callbacks to be called when
   *        the diagnosis information needs to be re-evaluated
   */
  virtual boost::signals2::connection onInvalidated(std::function<void()> callback) {
    return m_OnInvalidated.connect(callback);
  }


protected:

  void invalidate() {
    m_OnInvalidated();
  }

private:

  SignalInvalidated m_OnInvalidated;

};

} // namespace MOBase

Q_DECLARE_INTERFACE(MOBase::IPluginDiagnose, "com.tannin.ModOrganizer.PluginDiagnose/1.1")

#endif // IPLUGINDIAGNOSE_H
