ifndef __libscid_etc_make_common.mk
__libscid_etc_make_common.mk := 1

SHELL := bash
.SHELLFLAGS := -euo pipefail -c

####################################################################################################

LIBSCID_BMAKELIB_VERSION ?= v0.9.1
LIBSCID_BMAKELIB_DIR ?= $(ROOT).bmakelib
LIBSCID_BMAKELIB ?= $(LIBSCID_BMAKELIB_DIR)/bmakelib.mk

bmakelib.conf.help.variables ?= no
bmakelib.conf.help.scope ?= local

-include $(LIBSCID_BMAKELIB)

$(LIBSCID_BMAKELIB) :
	@mkdir -p $(@D)
	@curl -fsSL "https://github.com/bahmanm/bmakelib/releases/download/$(LIBSCID_BMAKELIB_VERSION)/bmakelib-portable.tar.gz" \
		| tar -xz -C $(@D) --strip-components=3

export LIBSCID_BMAKELIB

####################################################################################################

LIBSCID_STAGING_ROOT ?= $(ROOT)_staging/
LIBSCID_STAGING_BUILD_DIR ?= $(LIBSCID_STAGING_ROOT)build/
LIBSCID_STAGING_INSTALL_DIR ?= $(LIBSCID_STAGING_ROOT)install/
LIBSCID_STAGING_RELEASE_DIR ?= $(LIBSCID_STAGING_ROOT)release/

LIBSCID_BUILD_ROOT ?= $(LIBSCID_STAGING_BUILD_DIR)
LIBSCID_RELEASE_ROOT ?= $(LIBSCID_STAGING_RELEASE_DIR)

$(call bmakelib.enum.define,LIBSCID_PROFILE/debug,release)
ifneq ($(filter install libscid.install libscid.capi.install libscid.python.install,$(MAKECMDGOALS)),)
LIBSCID_PROFILE ?= $(or $(PROFILE),release)
else
LIBSCID_PROFILE ?= $(or $(PROFILE),debug)
endif
PROFILE := $(LIBSCID_PROFILE) ## Build profile (debug|release)
$(call bmakelib.enum.error-unless-member,LIBSCID_PROFILE,LIBSCID_PROFILE)

PREFIX ?= $(HOME)/.local/opt/libscid ## Base installation directory prefix

LIBSCID_CMAKE ?= cmake
LIBSCID_CPACK ?= cpack
LIBSCID_CTEST ?= ctest
LIBSCID_CTEST_TIMEOUT ?= 60
LIBSCID_UV ?= uv
LIBSCID_TOX ?= tox
LIBSCID_PYTHON ?= $(shell command -v python3 2>/dev/null || command -v python 2>/dev/null || echo python)
LIBSCID_PLANTUML_JAR_PATH ?=

ifeq ($(LIBSCID_PROFILE),release)
LIBSCID_CMAKE_BUILD_TYPE ?= Release
else
LIBSCID_CMAKE_BUILD_TYPE ?= Debug
endif

LIBSCID_CMAKE_BUILD_ARGS ?=
LIBSCID_CMAKE_CONFIGURE_ARGS ?=
LIBSCID_CMAKE_GENERATOR ?=
$(call bmakelib.enum.define,LIBSCID_LINKAGE_TYPE/shared,static)
LIBSCID_LINKAGE_TYPE ?= shared ## Library linkage type (shared|static)
libscid.__host.system := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(libscid.__host.system),Darwin)
LIBSCID_CMAKE_C_COMPILER ?= clang
LIBSCID_CMAKE_CXX_COMPILER ?= clang++
libscid.__library.name := libscid.dylib
libscid.__venv.python := bin/python
libscid.__osx.sysroot := $(shell xcrun --show-sdk-path 2>/dev/null)
LIBSCID_CMAKE_OSX_SYSROOT ?= $(libscid.__osx.sysroot)
else ifneq ($(filter MINGW% MSYS% CYGWIN%,$(libscid.__host.system)),)
libscid.__library.name := scid.dll
libscid.__venv.python := Scripts/python.exe
else
libscid.__library.name := libscid.so
libscid.__venv.python := bin/python
endif

LIBSCID_CMAKE_C_COMPILER ?= $(LIBSCID_C_COMPILER)
LIBSCID_CMAKE_CXX_COMPILER ?= $(LIBSCID_CXX_COMPILER)

libscid.__release.versions := $(call bmakelib.shell.error-if-nonzero,$(LIBSCID_CMAKE) -P "$(ROOT)etc/cmake/version.cmake")
LIBSCID_RELEASE_PROJECT_VERSION ?= $(word 1,$(libscid.__release.versions))
LIBSCID_RELEASE_VERSION ?= $(word 2,$(libscid.__release.versions))
LIBSCID_RELEASE_PACKAGE_VERSION_LABEL ?= $(LIBSCID_RELEASE_VERSION)
LIBSCID_RELEASE_PLATFORM ?= local

libscid.cmake.__generator.arg = $(if $(LIBSCID_CMAKE_GENERATOR),-G "$(LIBSCID_CMAKE_GENERATOR)")
libscid.cmake.__c.compiler.arg = $(if $(LIBSCID_CMAKE_C_COMPILER),"-DCMAKE_C_COMPILER=$(LIBSCID_CMAKE_C_COMPILER)")
libscid.cmake.__cxx.compiler.arg = $(if $(LIBSCID_CMAKE_CXX_COMPILER),"-DCMAKE_CXX_COMPILER=$(LIBSCID_CMAKE_CXX_COMPILER)")
libscid.cmake.__osx_sysroot.arg = $(if $(LIBSCID_CMAKE_OSX_SYSROOT),"-DCMAKE_OSX_SYSROOT=$(LIBSCID_CMAKE_OSX_SYSROOT)")
libscid.cmake.__plantuml.jar.path.arg = $(if $(LIBSCID_PLANTUML_JAR_PATH),"-DLIBSCID_PLANTUML_JAR_PATH=$(LIBSCID_PLANTUML_JAR_PATH)")
libscid.__make.word.escape = $(subst :,\:,$(1))

####################################################################################################

endif
