libscid.project.dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export LIBSCID_ROOT ?= $(abspath $(libscid.project.dir)../..)/
LIBSCID_BUILD_ROOT ?= $(libscid.project.dir)_build/

include $(LIBSCID_ROOT)mk/common.mk

LIBSCID_PROJECT_NAME := internal
LIBSCID_CMAKE_BUILD_TARGETS := scid_tests_core_cpp scid_tests_database_cpp scid_tests_eco_cpp scid_tests_spelling_cpp
LIBSCID_CMAKE_TEST_LABELS := core|database|eco|spelling

####################################################################################################

include $(LIBSCID_ROOT)mk/cmake.mk
