export LIBSCID_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include $(LIBSCID_ROOT)Makefile.conf

####################################################################################################

libscid.project.libscid := $(LIBSCID_ROOT)src/libscid/
libscid.project.internal := $(LIBSCID_ROOT)src/internal/
libscid.project.bindings.python := $(LIBSCID_ROOT)src/bindings/python/

libscid.build.dir ?= $(LIBSCID_BUILD_ROOT)libscid/
libscid.internal.build.dir ?= $(LIBSCID_BUILD_ROOT)internal/
libscid.default.build.dir ?= $(LIBSCID_BUILD_ROOT)default/libscid/
libscid.default.internal.build.dir ?= $(LIBSCID_BUILD_ROOT)default/internal/
libscid.library := $(libscid.build.dir)src/libscid/$(libscid.library.name)
libscid.example.pgn.roundtrip := $(LIBSCID_ROOT)examples/libscid/000-python-bindings/pgn_roundtrip.py

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

ci : ci-shared

.PHONY : ci

####################################################################################################

test-default :
	$(MAKE) -C $(libscid.project.internal) test \
	  LIBSCID_BUILD_DIR=$(libscid.default.internal.build.dir) \
	  LIBSCID_CMAKE_SHARED_LIBS=OFF
	$(MAKE) -C $(libscid.project.libscid) test \
	  LIBSCID_BUILD_DIR=$(libscid.default.build.dir) \
	  LIBSCID_CMAKE_SHARED_LIBS=OFF

.PHONY : test-default

####################################################################################################

check-default-compilation-database : test-default
	test -s $(libscid.default.internal.build.dir)compile_commands.json
	test -s $(libscid.default.build.dir)compile_commands.json

.PHONY : check-default-compilation-database

####################################################################################################

check-shared-compilation-database : test
	test -s $(libscid.internal.build.dir)compile_commands.json
	test -s $(libscid.build.dir)compile_commands.json

.PHONY : check-shared-compilation-database

####################################################################################################

test-examples : test
	$(LIBSCID_PYTHON) $(libscid.example.pgn.roundtrip) --library $(libscid.library)

.PHONY : test-examples

####################################################################################################

ci-default : test-default check-default-compilation-database

.PHONY : ci-default

####################################################################################################

ci-shared : test check-shared-compilation-database test-examples

.PHONY : ci-shared

####################################################################################################
