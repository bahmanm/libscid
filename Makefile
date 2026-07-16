export LIBSCID_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include $(LIBSCID_ROOT)Makefile.conf

####################################################################################################

libscid.project.libscid := $(LIBSCID_ROOT)src/libscid/
libscid.project.internal := $(LIBSCID_ROOT)src/internal/
libscid.project.bindings.python := $(LIBSCID_ROOT)src/bindings/python/

libscid.build.dir ?= $(LIBSCID_BUILD_ROOT)libscid/
libscid.internal.build.dir ?= $(LIBSCID_BUILD_ROOT)internal/
libscid.library := $(libscid.build.dir)src/libscid/$(libscid.library.name)

####################################################################################################

configure :
	$(MAKE) -C $(libscid.project.internal) configure LIBSCID_BUILD_DIR=$(libscid.internal.build.dir)
	$(MAKE) -C $(libscid.project.libscid) configure LIBSCID_BUILD_DIR=$(libscid.build.dir)
	$(MAKE) -C $(libscid.project.bindings.python) configure

.PHONY : configure

####################################################################################################

build :
	$(MAKE) -C $(libscid.project.internal) build LIBSCID_BUILD_DIR=$(libscid.internal.build.dir)
	$(MAKE) -C $(libscid.project.libscid) build LIBSCID_BUILD_DIR=$(libscid.build.dir)
	$(MAKE) -C $(libscid.project.bindings.python) build

.PHONY : build

####################################################################################################

test :
	$(MAKE) -C $(libscid.project.internal) test LIBSCID_BUILD_DIR=$(libscid.internal.build.dir)
	$(MAKE) -C $(libscid.project.libscid) test LIBSCID_BUILD_DIR=$(libscid.build.dir)
	$(MAKE) -C $(libscid.project.bindings.python) test LIBSCID_LIBRARY=$(libscid.library)

.PHONY : test

####################################################################################################

clean :
	$(MAKE) -C $(libscid.project.internal) clean LIBSCID_BUILD_DIR=$(libscid.internal.build.dir)
	$(MAKE) -C $(libscid.project.libscid) clean LIBSCID_BUILD_DIR=$(libscid.build.dir)
	$(MAKE) -C $(libscid.project.bindings.python) clean

.PHONY : clean

####################################################################################################

ci : test

.PHONY : ci

####################################################################################################
