ifeq ($(.DEFAULT_GOAL),)
.DEFAULT_GOAL := libscid.capi.test
endif

ifndef __libscid_capi_Makefile
__libscid_capi_Makefile := 1

libscid.capi.__makefile := $(lastword $(MAKEFILE_LIST))
libscid.capi.__project.dir := $(dir $(abspath $(libscid.capi.__makefile)))
export ROOT ?= $(abspath $(libscid.capi.__project.dir)..)/

include $(ROOT)etc/make/common.mk

libscid.capi.__component := capi
libscid.capi.__cmake.build.targets := scid scid_tests_c scid_tests_cxx
libscid.capi.__cmake.test.labels := capi

libscid.capi.static.__cmake.linkage := static
libscid.capi.static.__cmake.install := OFF

libscid.capi.__docs.build.dir ?= $(LIBSCID_STAGING_BUILD_DIR)capi/docs/
libscid.capi.__release.build.dir ?= $(LIBSCID_STAGING_BUILD_DIR)capi/release/
libscid.capi.__release.package.build.dir ?= $(LIBSCID_STAGING_BUILD_DIR)capi/package/
libscid.capi.__release.smoke-test.dir ?= $(LIBSCID_STAGING_BUILD_DIR)capi/release-smoke-test/
libscid.capi.__release.source-smoke-test.dir ?= $(LIBSCID_STAGING_BUILD_DIR)capi/release-source-smoke-test/
libscid.capi.__release.build.docs ?= ON
libscid.capi.__release.artefact.dir ?= $(LIBSCID_STAGING_RELEASE_DIR)capi/
libscid.capi.__release.source.artefact.dir ?= $(LIBSCID_STAGING_RELEASE_DIR)source/
libscid.capi.artefact = $(libscid.capi.__build.dir)$(libscid.__library.name)
libscid.capi.release.artefact = $(libscid.capi.__release.build.dir)$(libscid.__library.name)

####################################################################################################

include $(ROOT)etc/make/cmake.mk

$(eval $(call libscid.cmake.__project.rules,libscid.capi))
$(eval $(call libscid.cmake.__project.rules,libscid.capi.static,libscid.capi))

libscid.capi.configure : ## Configure C API build with CMake
libscid.capi.build : ## Compile C API shared library and test binaries
libscid.capi.test : ## Run C API unit tests via CTest
libscid.capi.test.all : libscid.capi.test libscid.capi.static.test ## Run C API tests against shared and static builds
libscid.capi.static.test : ## Run unit tests against static C API library
libscid.capi.static.build : ## Compile C API static library and test binaries
libscid.capi.static.clean : ## Clean C API static build artefacts
libscid.capi.static.configure : ## Configure C API static build with CMake
libscid.capi.static.install : ## Install C API static library under PREFIX
libscid.capi.static.qc-all : ## Run all quality control checks for static C API build
libscid.capi.clean : ## Clean C API build and QC artefacts
libscid.capi.install : ## Install C API library, headers, and CMake config under PREFIX
libscid.capi.qc-format : ## Check C API formatting with clang-format
libscid.capi.qc-static-analysis : ## Run static analysis (cppcheck, clang-tidy) on C API
libscid.capi.qc-dynamic-analysis : ## Run C API tests under Address and Undefined Behaviour sanitisers
libscid.capi.qc-all : ## Run all quality control checks for C API

.PHONY : libscid.capi.test.all

####################################################################################################

$(call libscid.__make.word.escape,$(libscid.capi.artefact)) : libscid.capi.build

####################################################################################################

$(call libscid.__make.word.escape,$(libscid.capi.release.artefact)) : libscid.capi.release-library

####################################################################################################

libscid.capi.__release-% : export ROOT := $(ROOT)
libscid.capi.__release-% : export LIBSCID_CAPI_DIR := $(libscid.capi.__project.dir)
libscid.capi.__release-% : export LIBSCID_CAPI_RELEASE_BUILD_DIR := $(libscid.capi.__release.build.dir)
libscid.capi.__release-% : export LIBSCID_CAPI_RELEASE_PACKAGE_BUILD_DIR := $(libscid.capi.__release.package.build.dir)
libscid.capi.__release-% : export LIBSCID_CAPI_RELEASE_SMOKE_TEST_DIR := $(libscid.capi.__release.smoke-test.dir)
libscid.capi.__release-% : export LIBSCID_CAPI_RELEASE_SOURCE_SMOKE_TEST_DIR := $(libscid.capi.__release.source-smoke-test.dir)
libscid.capi.__release-% : export LIBSCID_CAPI_RELEASE_BUILD_DOCS := $(libscid.capi.__release.build.docs)
libscid.capi.__release-% : export LIBSCID_CAPI_RELEASE_ARTEFACT_DIR := $(libscid.capi.__release.artefact.dir)
libscid.capi.__release-% : export LIBSCID_RELEASE_SOURCE_ARTEFACT_DIR := $(libscid.capi.__release.source.artefact.dir)
libscid.capi.__release-% : export LIBSCID_HOST_SYSTEM := $(libscid.__host.system)
libscid.capi.__release-% : export LIBSCID_CMAKE := $(LIBSCID_CMAKE)
libscid.capi.__release-% : export LIBSCID_CPACK := $(LIBSCID_CPACK)
libscid.capi.__release-% : export LIBSCID_CTEST := $(LIBSCID_CTEST)
libscid.capi.__release-% : export LIBSCID_CTEST_TIMEOUT := $(LIBSCID_CTEST_TIMEOUT)
libscid.capi.__release-% : export LIBSCID_MAKE := $(MAKE)
libscid.capi.__release-% : export LIBSCID_CMAKE_BUILD_ARGS := $(LIBSCID_CMAKE_BUILD_ARGS)
libscid.capi.__release-% : export LIBSCID_CMAKE_CONFIGURE_ARGS := $(LIBSCID_CMAKE_CONFIGURE_ARGS)
libscid.capi.__release-% : export LIBSCID_CMAKE_GENERATOR := $(LIBSCID_CMAKE_GENERATOR)
libscid.capi.__release-% : export LIBSCID_CMAKE_C_COMPILER := $(LIBSCID_CMAKE_C_COMPILER)
libscid.capi.__release-% : export LIBSCID_CMAKE_CXX_COMPILER := $(LIBSCID_CMAKE_CXX_COMPILER)
libscid.capi.__release-% : export LIBSCID_PLANTUML_JAR_PATH := $(LIBSCID_PLANTUML_JAR_PATH)
libscid.capi.__release-% : export LIBSCID_RELEASE_PROJECT_VERSION := $(LIBSCID_RELEASE_PROJECT_VERSION)
libscid.capi.__release-% : export LIBSCID_RELEASE_VERSION := $(LIBSCID_RELEASE_VERSION)
libscid.capi.__release-% : export LIBSCID_RELEASE_PACKAGE_VERSION_LABEL := $(LIBSCID_RELEASE_PACKAGE_VERSION_LABEL)
libscid.capi.__release-% : export LIBSCID_RELEASE_PLATFORM := $(LIBSCID_RELEASE_PLATFORM)

####################################################################################################

libscid.capi.__release-library :
	$(libscid.capi.__project.dir)etc/release-library

.PHONY : libscid.capi.__release-library

####################################################################################################

libscid.capi.release-library : libscid.capi.__release-library ## Build release library package

.PHONY : libscid.capi.release-library

####################################################################################################

libscid.capi.__release-package : libscid.capi.__release-library
	$(libscid.capi.__project.dir)etc/release-package

.PHONY : libscid.capi.__release-package

####################################################################################################

libscid.capi.release-package : libscid.capi.__release-package ## Build release distributable package

.PHONY : libscid.capi.release-package

####################################################################################################

libscid.capi.__release-smoke-test : libscid.capi.release-package
	$(libscid.capi.__project.dir)etc/release-smoke-test

.PHONY : libscid.capi.__release-smoke-test

####################################################################################################

libscid.capi.__release-source-smoke-test : libscid.__release-source-package
	$(libscid.capi.__project.dir)etc/release-source-smoke-test

.PHONY : libscid.capi.__release-source-smoke-test

####################################################################################################

libscid.capi.__release.source.targets :=
ifeq ($(LIBSCID_BUILD_SOURCE_ARCHIVE),true)
libscid.capi.__release.source.targets := libscid.capi.__release-source-smoke-test
endif

libscid.capi.release : libscid.capi.release-package libscid.capi.__release-smoke-test $(libscid.capi.__release.source.targets) ## Build release packages and execute smoke tests

.PHONY : libscid.capi.release

####################################################################################################

libscid.capi.docs : libscid.capi.__cmake.contract ## Generate C API documentation with Doxygen
	$(LIBSCID_CMAKE) \
	    -S $(libscid.capi.__cmake.source.dir) \
	    -B $(libscid.capi.__docs.build.dir) \
	    $(libscid.cmake.__generator.arg) \
	    $(libscid.cmake.__c.compiler.arg) \
	    $(libscid.cmake.__cxx.compiler.arg) \
	    -DBUILD_TESTING=OFF \
	    -DLIBSCID_BUILD_DOCS=ON \
	    -DLIBSCID_INSTALL=OFF \
	    "-DLIBSCID_SOURCE_ROOT=$(libscid.capi.__cmake.source.root)" \
	    $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	    --build $(libscid.capi.__docs.build.dir) \
	    --target api-docs \
	    $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : libscid.capi.docs

####################################################################################################

endif


