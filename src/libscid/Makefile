libscid.project.dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export LIBSCID_ROOT ?= $(abspath $(libscid.project.dir)../..)/
LIBSCID_BUILD_ROOT ?= $(libscid.project.dir)_build/

include $(LIBSCID_ROOT)etc/make/common.mk

LIBSCID_PROJECT_NAME := libscid
LIBSCID_CMAKE_BUILD_TARGETS := scid scid_tests_c
LIBSCID_CMAKE_TEST_LABELS := libscid
LIBSCID_RELEASE_BUILD_DIR ?= $(libscid.project.dir)_build/release/
LIBSCID_RELEASE_PACKAGE_BUILD_DIR ?= $(libscid.project.dir)_build/package/
LIBSCID_RELEASE_BUILD_DOCS ?= ON
LIBSCID_RELEASE_ARTIFACT_DIR ?= $(LIBSCID_RELEASE_ROOT)

####################################################################################################

include $(LIBSCID_ROOT)etc/make/cmake.mk

####################################################################################################

release-library :
	$(LIBSCID_CMAKE) \
	  -S $(libscid.project.dir) \
	  -B $(LIBSCID_RELEASE_BUILD_DIR) \
	  $(libscid.cmake.generator.arg) \
	  -DBUILD_TESTING=OFF \
	  -DBUILD_SHARED_LIBS=ON \
	  -DLIBSCID_INSTALL=ON \
	  "-DLIBSCID_SOURCE_ROOT=$(LIBSCID_ROOT)" \
	  -DCMAKE_BUILD_TYPE=Release \
	  "-DLIBSCID_PROJECT_VERSION=$(LIBSCID_RELEASE_PROJECT_VERSION)" \
	  "-DLIBSCID_VERSION=$(LIBSCID_RELEASE_VERSION)" \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build $(LIBSCID_RELEASE_BUILD_DIR) \
	  --config Release \
	  --target scid \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : release-library

####################################################################################################

release-package :
	$(LIBSCID_CMAKE) \
	  -S $(libscid.project.dir) \
	  -B $(LIBSCID_RELEASE_PACKAGE_BUILD_DIR) \
	  $(libscid.cmake.generator.arg) \
	  -DBUILD_TESTING=OFF \
	  -DBUILD_SHARED_LIBS=ON \
	  -DLIBSCID_INSTALL=ON \
	  -DLIBSCID_BUILD_DOCS=$(LIBSCID_RELEASE_BUILD_DOCS) \
	  "-DLIBSCID_SOURCE_ROOT=$(LIBSCID_ROOT)" \
	  -DCMAKE_BUILD_TYPE=Release \
	  "-DLIBSCID_PROJECT_VERSION=$(LIBSCID_RELEASE_PROJECT_VERSION)" \
	  "-DLIBSCID_VERSION=$(LIBSCID_RELEASE_VERSION)" \
	  "-DLIBSCID_PACKAGE_VERSION_LABEL=$(LIBSCID_RELEASE_PACKAGE_VERSION_LABEL)" \
	  "-DLIBSCID_PACKAGE_PLATFORM=$(LIBSCID_RELEASE_PLATFORM)" \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build $(LIBSCID_RELEASE_PACKAGE_BUILD_DIR) \
	  --config Release \
	  $(LIBSCID_CMAKE_BUILD_ARGS)
	rm -f $(LIBSCID_RELEASE_PACKAGE_BUILD_DIR)*.tar.gz
	$(LIBSCID_CPACK) \
	  --config $(LIBSCID_RELEASE_PACKAGE_BUILD_DIR)CPackConfig.cmake \
	  -G TGZ
	mkdir -p $(LIBSCID_RELEASE_ARTIFACT_DIR)
	rm -f $(LIBSCID_RELEASE_ARTIFACT_DIR)/libscid__*.tar.gz
	cp $(LIBSCID_RELEASE_PACKAGE_BUILD_DIR)*.tar.gz $(LIBSCID_RELEASE_ARTIFACT_DIR)

.PHONY : release-package

####################################################################################################

release : release-library release-package

.PHONY : release

####################################################################################################
