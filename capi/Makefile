ifeq ($(.DEFAULT_GOAL),)
.DEFAULT_GOAL := libscid.capi.test
endif

ifndef __libscid_capi_makefile
__libscid_capi_makefile := 1

libscid.capi.__makefile := $(lastword $(MAKEFILE_LIST))
libscid.capi.__project.dir := $(dir $(abspath $(libscid.capi.__makefile)))
export ROOT ?= $(abspath $(libscid.capi.__project.dir)..)/

include $(ROOT)etc/make/common.mk

libscid.capi.__build.dir ?= $(libscid.capi.__project.dir)_build/
libscid.capi.__cmake.build.targets := scid scid_tests_c
libscid.capi.__cmake.test.labels := capi

libscid.capi.default.__build.dir ?= $(libscid.capi.__project.dir)_build/default/
libscid.capi.default.__cmake.build.targets := $(libscid.capi.__cmake.build.targets)
libscid.capi.default.__cmake.test.labels := $(libscid.capi.__cmake.test.labels)
libscid.capi.default.__cmake.shared.libs := OFF
libscid.capi.default.__qc.format.build.dir ?= $(libscid.capi.default.__build.dir)format/
libscid.capi.default.__qc.analysis.build.dir ?= $(libscid.capi.default.__build.dir)analysis/
libscid.capi.default.__qc.dynamic-analysis.build.dir ?= $(libscid.capi.default.__build.dir)sanitisers/

libscid.capi.__docs.build.dir ?= $(libscid.capi.__project.dir)_build/docs/
libscid.capi.__release.build.dir ?= $(libscid.capi.__project.dir)_build/release/
libscid.capi.__release.package.build.dir ?= $(libscid.capi.__project.dir)_build/package/
libscid.capi.__release.smoke-test.dir ?= $(libscid.capi.__project.dir)_build/release-smoke-test/
libscid.capi.__release.build.docs ?= ON
libscid.capi.__release.artefact.dir ?= $(LIBSCID_RELEASE_ROOT)
libscid.capi.artefact := $(libscid.capi.__build.dir)$(libscid.__library.name)
libscid.capi.release.artefact := $(libscid.capi.__release.build.dir)$(libscid.__library.name)

####################################################################################################

include $(ROOT)etc/make/cmake.mk

$(eval $(call libscid.cmake.__project.rules,libscid.capi))
$(eval $(call libscid.cmake.__project.rules,libscid.capi.default,libscid.capi))

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
libscid.capi.__release-% : export LIBSCID_CAPI_RELEASE_BUILD_DOCS := $(libscid.capi.__release.build.docs)
libscid.capi.__release-% : export LIBSCID_CAPI_RELEASE_ARTEFACT_DIR := $(libscid.capi.__release.artefact.dir)
libscid.capi.__release-% : export LIBSCID_HOST_SYSTEM := $(libscid.__host.system)
libscid.capi.__release-% : export LIBSCID_CMAKE := $(LIBSCID_CMAKE)
libscid.capi.__release-% : export LIBSCID_CPACK := $(LIBSCID_CPACK)
libscid.capi.__release-% : export LIBSCID_CTEST := $(LIBSCID_CTEST)
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

libscid.capi.release-library : libscid.capi.__release-library

.PHONY : libscid.capi.release-library

####################################################################################################

libscid.capi.__release-package : libscid.capi.__release-library
	$(libscid.capi.__project.dir)etc/release-package

.PHONY : libscid.capi.__release-package

####################################################################################################

libscid.capi.release-package : libscid.capi.__release-package

.PHONY : libscid.capi.release-package

####################################################################################################

libscid.capi.__release-smoke-test : libscid.capi.release-package
	$(libscid.capi.__project.dir)etc/release-smoke-test

.PHONY : libscid.capi.__release-smoke-test

####################################################################################################

libscid.capi.release : libscid.capi.release-package libscid.capi.__release-smoke-test

.PHONY : libscid.capi.release

####################################################################################################

libscid.capi.docs : libscid.capi.__cmake.contract
	$(LIBSCID_CMAKE) \
	    -S $(libscid.capi.__cmake.source.dir) \
	    -B $(libscid.capi.__docs.build.dir) \
	    $(libscid.cmake.__generator.arg) \
	    $(libscid.cmake.__c.compiler.arg) \
	    $(libscid.cmake.__cxx.compiler.arg) \
	    -DBUILD_TESTING=OFF \
	    -DLIBSCID_BUILD_DOCS=ON \
	    "-DLIBSCID_SOURCE_ROOT=$(libscid.capi.__cmake.source.root)" \
	    $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	    --build $(libscid.capi.__docs.build.dir) \
	    --target api-docs \
	    $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : libscid.capi.docs

####################################################################################################

endif

