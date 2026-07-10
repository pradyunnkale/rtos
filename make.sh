#!/usr/bin/env bash

rm -rf build
cmake --preset host
cmake --build build/host
