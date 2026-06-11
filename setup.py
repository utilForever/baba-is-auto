
# Original code from https://github.com/pybind/cmake_example/blob/master/setup.py

import os
import re
import sys
import sysconfig
import platform
import subprocess
import multiprocessing

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(
                re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.8.2':
                raise RuntimeError("CMake >= 3.8.2 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(
            self.get_ext_fullpath(ext.name)))
        python_include_dir = sysconfig.get_path('include')
        python_library_name = sysconfig.get_config_var('LIBRARY') or \
            sysconfig.get_config_var('LDLIBRARY')
        if not python_library_name:
            if platform.system() == "Windows":
                python_library_name = 'python{}{}.lib'.format(
                    sys.version_info.major, sys.version_info.minor)
            elif platform.system() == "Darwin":
                python_library_name = 'libpython{}.{}.dylib'.format(
                    sys.version_info.major, sys.version_info.minor)
            else:
                python_library_name = 'libpython{}.{}.so'.format(
                    sys.version_info.major, sys.version_info.minor)
        python_library_dir = sysconfig.get_config_var('LIBDIR') or \
            os.path.join(sys.base_prefix, 'libs')
        python_extension_suffix = sysconfig.get_config_var('EXT_SUFFIX')
        if not python_extension_suffix:
            raise RuntimeError("Python EXT_SUFFIX is required to build pyBaba")
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DBABA_PYTHON_EXECUTABLE=' + sys.executable,
                      '-DBABA_PYTHON_INCLUDE_DIR=' + python_include_dir,
                      '-DBABA_PYTHON_EXTENSION_SUFFIX=' + python_extension_suffix,
                      '-DBABA_PYTHON_OUTPUT_DIRECTORY=' + extdir,
                      '-DBUILD_FROM_PIP=ON']
        python_library = os.path.join(python_library_dir, python_library_name)
        if os.path.exists(python_library):
            cmake_args += ['-DBABA_PYTHON_LIBRARY=' + python_library]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        env = os.environ.copy()

        cmake_toolchain_file = env.get('CMAKE_TOOLCHAIN_FILE')
        if not cmake_toolchain_file and env.get('VCPKG_ROOT'):
            cmake_toolchain_file = os.path.join(
                env['VCPKG_ROOT'], 'scripts', 'buildsystems', 'vcpkg.cmake')
        if cmake_toolchain_file:
            cmake_args += ['-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_file]

        if platform.system() == "Windows":
            cmake_args += [
                '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            num_jobs = env.get('NUM_JOBS', multiprocessing.cpu_count())
            build_args += ['--', '-j%s' % str(num_jobs), 'pyBaba']

        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] +
                              cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] +
                              build_args, cwd=self.build_temp)


setup(
    name='pyBaba',
    version='0.1',
    author='Chris Ohk',
    author_email='utilforever@gmail.com',
    description='Baba Is You simulator with some reinforcement learning',
    long_description='',
    ext_modules=[CMakeExtension('pyBaba')],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
)
