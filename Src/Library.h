#ifndef LIBRARY_H
#define LIBRARY_H

#include "iTunes.h"

namespace Library
{
    void Update(const std::vector<String>& rootDirs, IiTunes* iTunes);
    void Rebuild(const std::vector<String>& rootDirs, IiTunes* iTunes);
}

#endif // LIBRARY_H