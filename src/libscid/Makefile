libscid.project.dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export LIBSCID_ROOT ?= $(abspath $(libscid.project.dir)../..)/
LIBSCID_BUILD_ROOT ?= $(libscid.project.dir)_build/

include $(LIBSCID_ROOT)etc/make/common.mk

LIBSCID_PROJECT_NAME := libscid
LIBSCID_CMAKE_BUILD_TARGETS := scid scid_tests_c
LIBSCID_CMAKE_TEST_LABELS := libscid

####################################################################################################

include $(LIBSCID_ROOT)etc/make/cmake.mk
