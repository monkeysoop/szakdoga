#!/bin/bash
cd builddir || exit $?
meson compile -C meson-src || exit $?
meson install -C meson-src --skip-subprojects --quiet || exit $?

#it is important to change directory because std::filesystem uses the current working directory
cd meson-src/app || exit $?

#valgrind ./main || exit $?
./main || exit $?