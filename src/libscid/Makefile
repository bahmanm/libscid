libscid.project.dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export LIBSCID_ROOT ?= $(abspath $(libscid.project.dir)../..)/

include $(LIBSCID_ROOT)mk/common.mk

LIBSCID_BUILD_DIR ?= $(LIBSCID_BUILD_ROOT)libscid/
LIBSCID_PROJECT_NAME := libscid
LIBSCID_CMAKE_BUILD_TARGETS := scid scid_tests_c
LIBSCID_CMAKE_TEST_LABELS := libscid

####################################################################################################

include $(LIBSCID_ROOT)mk/cmake.mk
