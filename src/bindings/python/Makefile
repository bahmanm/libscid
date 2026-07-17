libscid.project.dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export LIBSCID_ROOT ?= $(abspath $(libscid.project.dir)../../..)/

include $(LIBSCID_ROOT)mk/common.mk

####################################################################################################

configure :
	$(LIBSCID_UV) sync --group dev

.PHONY : configure

####################################################################################################

build : configure
	$(LIBSCID_UV) build

.PHONY : build

####################################################################################################

test : configure
	$(LIBSCID_UV) run $(LIBSCID_TOX)

.PHONY : test

####################################################################################################

clean :
	-rm -rf .tox .venv build dist *.egg-info

.PHONY : clean

####################################################################################################

qc-format :

.PHONY : qc-format

####################################################################################################

qc-static-analysis :

.PHONY : qc-static-analysis

####################################################################################################

qc-dynamic-analysis :

.PHONY : qc-dynamic-analysis

####################################################################################################

qc-all : qc-format qc-static-analysis qc-dynamic-analysis

.PHONY : qc-all

####################################################################################################
