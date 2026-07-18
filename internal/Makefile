.DEFAULT_GOAL := libscid.internal.test

libscid.internal.__makefile := $(lastword $(MAKEFILE_LIST))
libscid.internal.__project.dir := $(dir $(abspath $(libscid.internal.__makefile)))
export ROOT ?= $(abspath $(libscid.internal.__project.dir)..)/

include $(ROOT)etc/make/common.mk

libscid.internal.__build.root ?= $(libscid.internal.__project.dir)_build/
libscid.internal.__default.build.root ?= $(libscid.internal.__project.dir)_build/default/

libscid.internal.__build.dir ?= $(libscid.internal.__build.root)
libscid.internal.__cmake.source.root ?= $(ROOT)
libscid.internal.__cmake.source.dir ?= $(libscid.internal.__project.dir)
libscid.internal.__cmake.build.targets := scid_tests_core_cpp scid_tests_database_cpp scid_tests_eco_cpp scid_tests_spelling_cpp
libscid.internal.__cmake.test.labels := core|database|eco|spelling
libscid.internal.__cmake.shared.libs ?= $(LIBSCID_CMAKE_SHARED_LIBS)
libscid.internal.__cmake.install := OFF
libscid.internal.__qc.format.build.dir ?= $(libscid.internal.__build.root)format/
libscid.internal.__qc.analysis.build.dir ?= $(libscid.internal.__build.root)analysis/
libscid.internal.__qc.dynamic-analysis.build.dir ?= $(libscid.internal.__build.root)sanitisers/

libscid.internal.default.__build.dir ?= $(libscid.internal.__default.build.root)
libscid.internal.default.__cmake.source.root ?= $(ROOT)
libscid.internal.default.__cmake.source.dir ?= $(libscid.internal.__project.dir)
libscid.internal.default.__cmake.build.targets := $(libscid.internal.__cmake.build.targets)
libscid.internal.default.__cmake.test.labels := $(libscid.internal.__cmake.test.labels)
libscid.internal.default.__cmake.shared.libs := OFF
libscid.internal.default.__cmake.install := OFF
libscid.internal.default.__qc.format.build.dir ?= $(libscid.internal.__default.build.root)format/
libscid.internal.default.__qc.analysis.build.dir ?= $(libscid.internal.__default.build.root)analysis/
libscid.internal.default.__qc.dynamic-analysis.build.dir ?= $(libscid.internal.__default.build.root)sanitisers/

####################################################################################################

include $(ROOT)etc/make/cmake.mk

$(eval $(call libscid.cmake.__project.rules,libscid.internal))
$(eval $(call libscid.cmake.__project.rules,libscid.internal.default))

####################################################################################################

libscid.internal.release :

.PHONY : libscid.internal.release

####################################################################################################
