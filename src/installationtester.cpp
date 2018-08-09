/*
Copyright (C) 2012 Sebastian Herbord. All rights reserved.

This file is part of Mod Organizer.

Mod Organizer is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Mod Organizer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Mod Organizer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "installationtester.h"
#include "filenamestring.h"

#include <QFileInfo>
#include <set>

namespace MOBase {


InstallationTester::InstallationTester()
{
}


bool InstallationTester::isTopLevelDirectory(const FileNameString &dirName)
{
  static std::set<FileNameString> tlDirectoryNames = {
    "fonts", "interface", "menus", "meshes", "music", "scripts", "shaders",
    "sound", "strings", "textures", "trees", "video", "facegen", "materials",
    "skse", "obse", "mwse", "nvse", "fose", "f4se", "distantlod", "asi",
    "SkyProc Patchers", "Tools", "MCM", "icons", "bookart", "distantland",
    "mits", "splash", "dllplugins", "CalienteTools"
  };

  return tlDirectoryNames.count(dirName) != 0;
}


bool InstallationTester::isTopLevelDirectoryBain(const FileNameString &dirName)
{
  static std::set<FileNameString> tlDirectoryNames = {
    "fonts", "interface", "menus", "meshes", "music", "scripts", "shaders",
    "sound", "strings", "textures", "trees", "video", "facegen", "materials",
    "skse", "obse", "mwse", "nvse", "fose", "f4se", "distantlod", "asi",
    "SkyProc Patchers", "Tools", "MCM", "icons", "bookart", "distantland",
    "mits", "splash", "dllplugins", "Docs", "INITweaks", "CalienteTools"
  };

  return tlDirectoryNames.count(dirName) != 0;
}


bool InstallationTester::isTopLevelSuffix(const FileNameString &fileName)
{
  static std::set<FileNameString> tlSuffixes = { "esp", "esm", "esl", "bsa", "ba2" };
  return tlSuffixes.count(QFileInfo(fileName.toQString()).suffix()) != 0;
}

} // namespace MOBase
