.DEFAULT_GOAL := libscid.test

export ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

libscid.__components := internal capi python
libscid.__qc.stages := format static-analysis dynamic-analysis
libscid.__example.pgn.roundtrip := $(ROOT)examples/python/010-edit-pgn/main.py

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

libscid.__docs.public.dir := $(ROOT)_build/docs/public/
libscid.__docs.hub.dir := $(ROOT)docs/hub/
libscid.__docs.assets.dir := $(ROOT)docs/assets/

libscid.clean : $(libscid.__components:%=libscid.%.clean)
	-rm -rf $(LIBSCID_RELEASE_ROOT)
	-rm -rf $(libscid.__docs.public.dir)

.PHONY : libscid.clean

####################################################################################################

libscid.docs : $(libscid.__components:%=libscid.%.docs)
	mkdir -p $(libscid.__docs.public.dir)assets/img/
	mkdir -p $(libscid.__docs.public.dir)capi/
	mkdir -p $(libscid.__docs.public.dir)python/
	cp $(libscid.__docs.hub.dir)index.html $(libscid.__docs.public.dir)
	cp -r $(libscid.__docs.assets.dir)img/* $(libscid.__docs.public.dir)assets/img/
	cp -r $(ROOT)capi/_build/docs/doxygen/html/* $(libscid.__docs.public.dir)capi/
	cp -r $(ROOT)python/_build/docs/site/* $(libscid.__docs.public.dir)python/

.PHONY : libscid.docs

####################################################################################################

libscid.release : $(libscid.__components:%=libscid.%.release)

.PHONY : libscid.release

####################################################################################################

libscid.test-examples : libscid.test
libscid.test-examples : export LIBSCID_LIBRARY := $(libscid.capi.artefact)
libscid.test-examples : export PYTHONPATH := $(ROOT)python/src
libscid.test-examples :
	shopt -s nullglob; \
	for script in $(ROOT)examples/python/*/main.py; do \
	    $(LIBSCID_PYTHON) "$$script"; \
	done

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
