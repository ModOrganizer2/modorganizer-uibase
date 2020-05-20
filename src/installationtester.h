/*
Mod Organizer shared UI functionality

Copyright (C) 2012 Sebastian Herbord. All rights reserved.

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


#ifndef INSTALLATIONTESTER_H
#define INSTALLATIONTESTER_H


#include "dllimport.h"
#include <QString>

namespace MOBase {


/**
 * @brief Various convenience functions used to determine if a mod directory fulfills certain criteria
 *
 * @todo Right now, this is class is used like a namespace and the tests done here are somewhat rudimentary.
 **/
class InstallationTester
{
public:

  /**
   * test if the specified directory qualifies as a top-level directory. A top-level directory
   * is one that contains data used by the game and goes directly below the "data" directory.
   * The most common examples are "textures" and "meshes"
   *
   * @param dirName the directory name to test
   * @return true if the specified directory is a top-level directory
   **/
  QDLLEXPORT static bool isTopLevelDirectory(const QString &dirName);

  /**
   * test if the specified file qualifies as a top-level file. A top-level file is one
   * that will be used by the game if it's directly inside the "data" directory.
   * These are ".esp", ".esm", ".esl" and ".bsa"
   *
   * @param fileName name of the file to test. It doesn't matter if this is an absolute file name or relative since only the extension is interpreted
   * @return bool
   **/
  QDLLEXPORT static bool isTopLevelSuffix(const QString&fileName);

private:

  InstallationTester();

};

} // namespace MOBase

#endif // INSTALLATIONTESTER_H
