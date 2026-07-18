.DEFAULT_GOAL := libscid.test

libscid.__component.names := internal library python
libscid.__component.srcdir := \
  $(ROOT)src/internal \
  $(ROOT)src/libscid \
  $(ROOT)src/bindings/python
libscid.__qc.stages := format static-analysis dynamic-analysis
libscid.__example.pgn.roundtrip := $(ROOT)examples/libscid/000-python-bindings/pgn_roundtrip.py

####################################################################################################

include $(ROOT)etc/make/common.mk
include $(libscid.__component.srcdir:%=%/Makefile)

####################################################################################################

libscid.python.test : libscid.library.build
libscid.python.test : export LIBSCID_LIBRARY := $(libscid.library.artifact)

libscid.python.release : libscid.library.release-library
libscid.python.release : export LIBSCID_LIBRARY := $(libscid.library.release.artifact)

####################################################################################################

libscid.configure : $(libscid.__component.names:%=libscid.%.configure)

.PHONY : libscid.configure

####################################################################################################

libscid.build : $(libscid.__component.names:%=libscid.%.build)

.PHONY : libscid.build

####################################################################################################

libscid.test : $(libscid.__component.names:%=libscid.%.test)

.PHONY : libscid.test

####################################################################################################

libscid.clean : $(libscid.__component.names:%=libscid.%.clean)
	-rm -rf $(LIBSCID_RELEASE_ROOT)

.PHONY : libscid.clean

####################################################################################################

libscid.release-library : libscid.library.release-library

.PHONY : libscid.release-library

####################################################################################################

libscid.release-package : libscid.library.release-package

.PHONY : libscid.release-package

####################################################################################################

libscid.release-libscid : libscid.library.release

.PHONY : libscid.release-libscid

####################################################################################################

libscid.release-python : libscid.python.release

.PHONY : libscid.release-python

####################################################################################################

libscid.release : $(libscid.__component.names:%=libscid.%.release)

.PHONY : libscid.release

####################################################################################################

libscid.test-examples : libscid.test
	$(LIBSCID_PYTHON) $(libscid.__example.pgn.roundtrip) --library $(libscid.library.artifact)

.PHONY : libscid.test-examples

####################################################################################################

libscid.qc-format : $(libscid.__component.names:%=libscid.%.qc-format)

.PHONY : libscid.qc-format

####################################################################################################

libscid.qc-static-analysis : $(libscid.__component.names:%=libscid.%.qc-static-analysis)

.PHONY : libscid.qc-static-analysis

####################################################################################################

libscid.qc-dynamic-analysis : $(libscid.__component.names:%=libscid.%.qc-dynamic-analysis)

.PHONY : libscid.qc-dynamic-analysis

####################################################################################################

libscid.qc-all : $(libscid.__qc.stages:%=libscid.qc-%)

.PHONY : libscid.qc-all

####################################################################################################
