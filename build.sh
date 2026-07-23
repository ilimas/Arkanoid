#!/bin/bash
rm -rf build && cmake -G Ninja -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build
