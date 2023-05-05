import os
import shutil
import stat
from skbuild import setup


# Remove previous build folder
def remove_readonly(func, path, excinfo):
    os.chmod(path, stat.S_IWRITE)
    os.remove(path)


if os.path.exists('_skbuild'):
    shutil.rmtree('_skbuild', onerror=remove_readonly)

install_requires = [
    'numpy',
    'pybind11',
    'scikit-build',
    'cmake',
    'ninja',
    'conan==1.59.0',
    'scikit-build-core>=0.3.3',
    'setuptools',
    'wheel'
]

cmake_args = [
    "-GNinja",
    '-DCMAKE_BUILD_TYPE=Release',
    # skip building tests & examples
    '-DBUILD_TESTING:BOOL=OFF',
    '-DBUILD_EXAMPLES:BOOL=OFF',
    # Unix: rpath to current dir when packaged
    '-DCMAKE_INSTALL_RPATH=$ORIGIN',
    '-DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON',
    '-DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=OFF',
    # Windows: will already have %PATH% in package dir
]

setup(
    name="pyairspacebooking",
    version="0.0.1",
    author="Aliaksei Pilko",
    author_email="dev@apilko.me",
    description="Strategic Deconfliction Utilities",
    install_requires=install_requires,

    # license="Proprietary",
    packages=['pyairspacebooking'],
    cmake_args=cmake_args,
    cmake_install_dir="python/pyairspacebooking",
    include_package_data=True,
    package_dir={"": "python"},
    python_requires=">=3.7",
)
