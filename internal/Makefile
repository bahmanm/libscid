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

libscid.internal.static.__build.dir ?= $(libscid.internal.__project.dir)_build/static/
libscid.internal.static.__cmake.linkage := static

####################################################################################################

include $(ROOT)etc/make/cmake.mk

$(eval $(call libscid.cmake.__project.rules,libscid.internal))
$(eval $(call libscid.cmake.__project.rules,libscid.internal.static,libscid.internal))

libscid.internal.test.all : libscid.internal.test libscid.internal.static.test

.PHONY : libscid.internal.test.all

####################################################################################################

libscid.internal.release :

.PHONY : libscid.internal.release

####################################################################################################

libscid.internal.docs :

.PHONY : libscid.internal.docs

####################################################################################################

endif


