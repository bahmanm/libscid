ifeq ($(.DEFAULT_GOAL),)
.DEFAULT_GOAL := libscid.internal.test
endif

ifndef __libscid_internal_Makefile
__libscid_internal_Makefile := 1

libscid.internal.__makefile := $(lastword $(MAKEFILE_LIST))
libscid.internal.__project.dir := $(dir $(abspath $(libscid.internal.__makefile)))
export ROOT ?= $(abspath $(libscid.internal.__project.dir)..)/

include $(ROOT)etc/make/common.mk

libscid.internal.__cmake.build.targets := scid_tests_core_cpp scid_tests_database_cpp scid_tests_eco_cpp scid_tests_spelling_cpp
libscid.internal.__cmake.test.labels := core|database|eco|spelling
libscid.internal.__cmake.install := OFF

libscid.internal.static.__cmake.linkage := static
libscid.internal.static.__cmake.install := OFF

####################################################################################################

include $(ROOT)etc/make/cmake.mk

$(eval $(call libscid.cmake.__project.rules,libscid.internal))
$(eval $(call libscid.cmake.__project.rules,libscid.internal.static,libscid.internal))

libscid.internal.configure : ## Configure internal C++ build with CMake
libscid.internal.build : ## Compile internal C++ library and unit tests
libscid.internal.test : ## Run internal C++ unit tests via CTest
libscid.internal.test.all : libscid.internal.test libscid.internal.static.test ## Run internal tests against shared and static builds
libscid.internal.static.test : ## Run unit tests against static internal C++ library
libscid.internal.static.build : ## Compile internal C++ static library and unit tests
libscid.internal.static.clean : ## Clean internal static build artefacts
libscid.internal.static.configure : ## Configure internal static build with CMake
libscid.internal.static.install : ## Install internal static artefacts (noop)
libscid.internal.static.qc-all : ## Run all quality control checks for static internal C++ build
libscid.internal.clean : ## Clean internal build artefacts
libscid.internal.install : ## Install internal C++ artefacts (noop)
libscid.internal.qc-format : ## Check internal C++ formatting with clang-format
libscid.internal.qc-static-analysis : ## Run static analysis (cppcheck, clang-tidy) on internal C++ code
libscid.internal.qc-dynamic-analysis : ## Run internal C++ tests under Address and Undefined Behaviour sanitisers
libscid.internal.qc-all : ## Run all quality control checks for internal C++ code

.PHONY : libscid.internal.test.all

####################################################################################################

libscid.internal.release : ## Package internal release artefacts (noop)

.PHONY : libscid.internal.release

####################################################################################################

libscid.internal.docs : ## Generate internal C++ documentation (noop)

.PHONY : libscid.internal.docs

####################################################################################################

endif


