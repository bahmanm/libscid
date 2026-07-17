SHELL := bash
.SHELLFLAGS := -euo pipefail -c
.DEFAULT_GOAL := test

####################################################################################################

LIBSCID_BUILD_ROOT ?= $(LIBSCID_ROOT)_build/
LIBSCID_RELEASE_ROOT ?= $(LIBSCID_ROOT)_release/

LIBSCID_CMAKE ?= cmake
LIBSCID_CPACK ?= cpack
LIBSCID_CTEST ?= ctest
LIBSCID_UV ?= uv
LIBSCID_TOX ?= tox
LIBSCID_PYTHON ?= $(shell command -v python3 2>/dev/null || command -v python 2>/dev/null || echo python)

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

libscid.cmake.generator.arg := $(if $(LIBSCID_CMAKE_GENERATOR),-G "$(LIBSCID_CMAKE_GENERATOR)")
libscid.cmake.shared.libs.arg := $(if $(LIBSCID_CMAKE_SHARED_LIBS),-DBUILD_SHARED_LIBS=$(LIBSCID_CMAKE_SHARED_LIBS))
libscid.cmake.c.compiler.arg := $(if $(LIBSCID_CMAKE_C_COMPILER),"-DCMAKE_C_COMPILER=$(LIBSCID_CMAKE_C_COMPILER)")
libscid.cmake.cxx.compiler.arg := $(if $(LIBSCID_CMAKE_CXX_COMPILER),"-DCMAKE_CXX_COMPILER=$(LIBSCID_CMAKE_CXX_COMPILER)")

####################################################################################################

libscid.host.system := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(libscid.host.system),Darwin)
libscid.library.name := libscid.dylib
libscid.venv.python := bin/python
else ifneq ($(filter MINGW% MSYS% CYGWIN%,$(libscid.host.system)),)
libscid.library.name := scid.dll
libscid.venv.python := Scripts/python.exe
else
libscid.library.name := libscid.so
libscid.venv.python := bin/python
endif

####################################################################################################
