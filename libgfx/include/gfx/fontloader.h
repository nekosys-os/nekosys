#ifndef _FONT_LOADER_H
#define _FONT_LOADER_H

#include "font.h"
#include <nk/string.h>

class FontLoader
{
public:
    static Font *LoadFont(const nk::String &path);

};

#endif