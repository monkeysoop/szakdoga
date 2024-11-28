import os

from conan import ConanFile
from conan.tools.files import copy
from conan.tools.gnu import PkgConfigDeps
from conan.tools.meson import MesonToolchain

class App(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "PkgConfigDeps", "MesonToolchain"

    def requirements(self):
        self.requires("opengl/system")
        self.requires("glew/2.2.0", override=True)
        self.requires("glu/system")
        self.requires("imgui/1.91.2")
        self.requires("sdl/2.28.3")
        self.requires("sdl_image/2.6.3")
        self.requires("gtest/1.15.0")

    def generate(self):
        imgui = self.dependencies["imgui"]

        imgui_bindings_folder = os.path.join(self.source_folder, "subprojects", "imgui_bindings")

        copy(self, "imgui_impl_sdl2.h", src=imgui.cpp_info.srcdirs[0], dst=os.path.join(imgui_bindings_folder, "include"))
        copy(self, "imgui_impl_opengl3.h", src=imgui.cpp_info.srcdirs[0], dst=os.path.join(imgui_bindings_folder, "include"))
        copy(self, "imgui_impl_opengl3_loader.h", src=imgui.cpp_info.srcdirs[0], dst=os.path.join(imgui_bindings_folder, "include"))

        copy(self, "imgui_impl_sdl2.cpp", src=imgui.cpp_info.srcdirs[0], dst=os.path.join(imgui_bindings_folder, "src"))
        copy(self, "imgui_impl_opengl3.cpp", src=imgui.cpp_info.srcdirs[0], dst=os.path.join(imgui_bindings_folder, "src"))
