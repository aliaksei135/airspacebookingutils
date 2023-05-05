from conans import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.layout import cmake_layout


class abConan(ConanFile):
    name = "airspacebookingutils"
    version = '0.1'
    author = "Aliaksei Pilko <dev@apilko.me>"
    settings = "os", "compiler", "arch", "build_type"
    license = "Proprietary"
    generators = "cmake_find_package_multi"
    build_policy = 'missing'

    exports_sources = "CMakeLists.txt", "src/*"

    def build_requirements(self):
        self.build_requires("doxygen/[>=1.9.1]")

    def requirements(self):
        self.requires("eigen/[>=3.3.9]")
        # self.requires("libcurl/[>=7.80.0]")
        self.requires("abseil/20230125.2")  # Required by S2
        self.requires("h3/4.1.0")
        self.requires("proj/[>=8.1.1]")
        self.requires("geos/[>=3.10.2]")
        self.requires("spdlog/[>=1.10.0]")

    def imports(self):
        self.copy('*.dll', dst='bin', src='bin')  # Copies all dll files from packages bin folder to my "bin" folder
        self.copy('*.dylib*', dst='bin',
                  src='lib')  # Copies all dylib files from packages lib folder to my "bin" folder
        self.copy('*.so*', dst='bin', src='lib')  # Copies all so files from packages lib folder to my "bin" folder

    def layout(self):
        cmake_layout(self)

    def generate(self):
        self.deps_cpp_info["doxygen"].set_property("skip_deps_file", True)

        tc = CMakeToolchain(self)
        tc.variables["BUILD_DOC"] = "ON"
        tc.variables["CMAKE_FIND_PACKAGE_PREFER_CONFIG"] = "ON"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ['airspacebookingutils']
