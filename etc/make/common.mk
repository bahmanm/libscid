SHELL := bash
.SHELLFLAGS := -euo pipefail -c

####################################################################################################

LIBSCID_BMAKELIB ?= $(firstword $(wildcard $(HOME)/.local/include/bmakelib/bmakelib.mk) bmakelib/bmakelib.mk)

include $(LIBSCID_BMAKELIB)

####################################################################################################

LIBSCID_BUILD_ROOT ?= $(ROOT)_build/
LIBSCID_RELEASE_ROOT ?= $(ROOT)_release/

LIBSCID_CMAKE ?= cmake
LIBSCID_CPACK ?= cpack
LIBSCID_CTEST ?= ctest
LIBSCID_UV ?= uv
LIBSCID_TOX ?= tox
LIBSCID_PYTHON ?= $(shell command -v python3 2>/dev/null || command -v python 2>/dev/null || echo python)
LIBSCID_PLANTUML_JAR_PATH ?=

LIBSCID_CMAKE_BUILD_TYPE ?= Release
LIBSCID_CMAKE_BUILD_ARGS ?=
LIBSCID_CMAKE_CONFIGURE_ARGS ?=
LIBSCID_CMAKE_GENERATOR ?=
LIBSCID_CMAKE_SHARED_LIBS ?= ON
LIBSCID_CMAKE_C_COMPILER ?= $(LIBSCID_C_COMPILER)
LIBSCID_CMAKE_CXX_COMPILER ?= $(LIBSCID_CXX_COMPILER)

LIBSCID_RELEASE_PROJECT_VERSION ?= 0.0.0
LIBSCID_RELEASE_VERSION ?= snapshot
LIBSCID_RELEASE_PACKAGE_VERSION_LABEL ?= $(LIBSCID_RELEASE_VERSION)
LIBSCID_RELEASE_PLATFORM ?= local

libscid.cmake.__generator.arg := $(if $(LIBSCID_CMAKE_GENERATOR),-G "$(LIBSCID_CMAKE_GENERATOR)")
libscid.cmake.__c.compiler.arg := $(if $(LIBSCID_CMAKE_C_COMPILER),"-DCMAKE_C_COMPILER=$(LIBSCID_CMAKE_C_COMPILER)")
libscid.cmake.__cxx.compiler.arg := $(if $(LIBSCID_CMAKE_CXX_COMPILER),"-DCMAKE_CXX_COMPILER=$(LIBSCID_CMAKE_CXX_COMPILER)")
libscid.cmake.__plantuml.jar.path.arg := $(if $(LIBSCID_PLANTUML_JAR_PATH),"-DLIBSCID_PLANTUML_JAR_PATH=$(LIBSCID_PLANTUML_JAR_PATH)")

####################################################################################################

libscid.__host.system := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(libscid.__host.system),Darwin)
libscid.__library.name := libscid.dylib
libscid.__venv.python := bin/python
else ifneq ($(filter MINGW% MSYS% CYGWIN%,$(libscid.__host.system)),)
libscid.__library.name := scid.dll
libscid.__venv.python := Scripts/python.exe
else
libscid.__library.name := libscid.so
libscid.__venv.python := bin/python
endif

####################################################################################################
