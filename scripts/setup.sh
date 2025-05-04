#!/bin/bash
mkdir -p builddir || exit $?
conan install . --output-folder=builddir --build=missing || exit $?
#conan install . --output-folder=builddir --build=missing -vvv
#conan install . --output-folder=builddir --build=missing -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True
cd builddir || exit $?
meson setup --wipe --warnlevel 3 --native-file conan_meson_native.ini .. meson-src || exit $?