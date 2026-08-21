.DEFAULT_GOAL := libscid.internal.test

libscid.internal.__makefile := $(lastword $(MAKEFILE_LIST))
libscid.internal.__project.dir := $(dir $(abspath $(libscid.internal.__makefile)))
export ROOT ?= $(abspath $(libscid.internal.__project.dir)..)/

include $(ROOT)etc/make/common.mk

libscid.internal.__cmake.build.targets := scid_tests_core_cpp scid_tests_database_cpp scid_tests_eco_cpp scid_tests_spelling_cpp
libscid.internal.__cmake.test.labels := core|database|eco|spelling

libscid.internal.default.__build.dir ?= $(libscid.internal.__project.dir)_build/default/
libscid.internal.default.__cmake.build.targets := $(libscid.internal.__cmake.build.targets)
libscid.internal.default.__cmake.test.labels := $(libscid.internal.__cmake.test.labels)
libscid.internal.default.__cmake.shared.libs := OFF
libscid.internal.default.__qc.format.build.dir ?= $(libscid.internal.default.__build.dir)format/
libscid.internal.default.__qc.analysis.build.dir ?= $(libscid.internal.default.__build.dir)analysis/
libscid.internal.default.__qc.dynamic-analysis.build.dir ?= $(libscid.internal.default.__build.dir)sanitisers/

####################################################################################################

include $(ROOT)etc/make/cmake.mk

$(eval $(call libscid.cmake.__project.rules,libscid.internal))
$(eval $(call libscid.cmake.__project.rules,libscid.internal.default,libscid.internal))

####################################################################################################

libscid.internal.release :

.PHONY : libscid.internal.release

####################################################################################################

libscid.internal.docs :

.PHONY : libscid.internal.docs

####################################################################################################

