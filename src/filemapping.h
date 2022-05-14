#ifndef FILEMAPPING_H
#define FILEMAPPING_H

#include <vector>

#include <QString>

struct Mapping
{
  QString source;
  QString destination;
  bool isDirectory;
  bool createTarget;
};

typedef std::vector<Mapping> MappingType;

#endif  // FILEMAPPING_H
