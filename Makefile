.DEFAULT_GOAL := libscid.test

export ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

libscid.__components := internal capi python
libscid.__qc.stages := format static-analysis dynamic-analysis
libscid.__example.pgn.roundtrip := $(ROOT)examples/libscid/000-python-bindings/pgn_roundtrip.py

####################################################################################################

include $(ROOT)etc/make/common.mk
include $(libscid.__components:%=$(ROOT)%/Makefile)

####################################################################################################

libscid.python.test : libscid.capi.build
libscid.python.test : export LIBSCID_LIBRARY := $(libscid.capi.artefact)

libscid.python.release : libscid.capi.release-library
libscid.python.release : export LIBSCID_LIBRARY := $(libscid.capi.release.artefact)

####################################################################################################

libscid.configure : $(libscid.__components:%=libscid.%.configure)

.PHONY : libscid.configure

####################################################################################################

libscid.build : $(libscid.__components:%=libscid.%.build)

.PHONY : libscid.build

####################################################################################################

libscid.test : $(libscid.__components:%=libscid.%.test)

.PHONY : libscid.test

####################################################################################################

libscid.clean : $(libscid.__components:%=libscid.%.clean)
	-rm -rf $(LIBSCID_RELEASE_ROOT)

.PHONY : libscid.clean

####################################################################################################

libscid.release : $(libscid.__components:%=libscid.%.release)

.PHONY : libscid.release

####################################################################################################

libscid.test-examples : libscid.test
	$(LIBSCID_PYTHON) $(libscid.__example.pgn.roundtrip) --library $(libscid.capi.artefact)

.PHONY : libscid.test-examples

####################################################################################################

libscid.qc-format : $(libscid.__components:%=libscid.%.qc-format)

.PHONY : libscid.qc-format

####################################################################################################

libscid.qc-static-analysis : $(libscid.__components:%=libscid.%.qc-static-analysis)

.PHONY : libscid.qc-static-analysis

####################################################################################################

libscid.qc-dynamic-analysis : $(libscid.__components:%=libscid.%.qc-dynamic-analysis)

.PHONY : libscid.qc-dynamic-analysis

####################################################################################################

libscid.qc-all : $(libscid.__qc.stages:%=libscid.qc-%)

.PHONY : libscid.qc-all

####################################################################################################
