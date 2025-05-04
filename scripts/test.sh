#!/bin/bash
cd builddir || exit $?
meson install -C meson-src --skip-subprojects --quiet || exit $?
meson test -C meson-src || exit $?