#!/bin/bash
neko-gcc -std=gnu++11 -fno-exceptions Mouse.cpp Compositor.cpp DirtyRegionManager.cpp main.cpp -lgfx -lgui -lpng -o ../../sysroot/bin/sakura