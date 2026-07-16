libscid.project.dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export LIBSCID_ROOT ?= $(abspath $(libscid.project.dir)../..)/

include $(LIBSCID_ROOT)Makefile.conf

LIBSCID_BUILD_DIR ?= $(LIBSCID_BUILD_ROOT)internal/

####################################################################################################

configure :
	$(LIBSCID_CMAKE) \
	  -S $(LIBSCID_ROOT) \
	  -B $(LIBSCID_BUILD_DIR) \
	  $(libscid.cmake.generator.arg) \
	  -DBUILD_TESTING=ON \
	  -DBUILD_SHARED_LIBS=ON \
	  -DLIBSCID_INSTALL=OFF \
	  -DCMAKE_BUILD_TYPE=$(LIBSCID_CMAKE_BUILD_TYPE) \
	  $(if $(LIBSCID_CMAKE_C_COMPILER),"-DCMAKE_C_COMPILER=$(LIBSCID_CMAKE_C_COMPILER)") \
	  $(if $(LIBSCID_CMAKE_CXX_COMPILER),"-DCMAKE_CXX_COMPILER=$(LIBSCID_CMAKE_CXX_COMPILER)") \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)

.PHONY : configure

####################################################################################################

build : configure
	$(LIBSCID_CMAKE) \
	  --build $(LIBSCID_BUILD_DIR) \
	  --config $(LIBSCID_CMAKE_BUILD_TYPE) \
	  --target scid_tests_core_cpp scid_tests_database_cpp scid_tests_eco_cpp scid_tests_spelling_cpp \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : build

####################################################################################################

test : build
	$(LIBSCID_CTEST) \
	  --test-dir $(LIBSCID_BUILD_DIR) \
	  -C $(LIBSCID_CMAKE_BUILD_TYPE) \
	  -L 'core|database|eco|spelling' \
	  --output-on-failure

.PHONY : test

####################################################################################################

clean :
	-rm -rf $(LIBSCID_BUILD_DIR)

.PHONY : clean

####################################################################################################
