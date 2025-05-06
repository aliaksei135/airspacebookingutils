from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout


class AbConan(ConanFile):
    name = "airspacebookingutils"
    version = "0.1"
    author = "Aliaksei Pilko <dev@apilko.me>"
    license = "Proprietary"
    settings = "os", "compiler", "arch", "build_type"
    # package_type = "library"
    exports_sources = "CMakeLists.txt", "src/*"
    requires = (
        "eigen/[>=3.3.9]",
        "h3/4.1.0",
        "proj/9.5.0",
        "geos/3.13.0",
        "spdlog/[>=1.10.0]",
        "s2geometry/0.11.1",
    )
    build_requires = ("doxygen/[>=1.9.1]",)
    build_policy = "missing"
    generators = "CMakeDeps", "CMakeToolchain"

    # def layout(self):
    #     cmake_layout(self)
