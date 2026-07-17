libscid.project.dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export LIBSCID_ROOT ?= $(abspath $(libscid.project.dir)../../..)/

include $(LIBSCID_ROOT)etc/make/common.mk

libscid.python.native.dir := $(libscid.project.dir)src/libscid/_native/
libscid.python.bundled.library := $(libscid.python.native.dir)$(libscid.library.name)
LIBSCID_PYTHON_RELEASE_LIBRARY ?= $(if $(LIBSCID_LIBRARY),$(LIBSCID_LIBRARY),$(LIBSCID_ROOT)src/libscid/_build/release/$(libscid.library.name))
LIBSCID_PYTHON_RELEASE_VERIFY_DIR ?= $(libscid.project.dir)_build/release-verify/
LIBSCID_PYTHON_RELEASE_ARTIFACT_DIR ?= $(LIBSCID_RELEASE_ROOT)
LIBSCID_PYTHON_RELEASE_VERSION ?= $(LIBSCID_RELEASE_PROJECT_VERSION)
LIBSCID_UV_FLAGS ?= --system-certs
libscid.python.venv.bin := $(libscid.project.dir).venv/bin
libscid.python.venv.python := $(libscid.python.venv.bin)/python
LIBSCID_RUFF ?= $(if $(wildcard $(libscid.python.venv.bin)/ruff),$(libscid.python.venv.bin)/ruff,ruff)
LIBSCID_PYRIGHT ?= $(if $(wildcard $(libscid.python.venv.bin)/pyright),$(libscid.python.venv.bin)/pyright,pyright)
LIBSCID_PYRIGHT_FLAGS ?= $(if $(wildcard $(libscid.python.venv.python)),--pythonpath $(libscid.python.venv.python))
libscid.python.qc.paths := src ../../../tests/bindings/python hatch_build.py

####################################################################################################

configure :
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) sync --group dev

.PHONY : configure

####################################################################################################

build : configure
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) build

.PHONY : build

####################################################################################################

test : configure
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run $(LIBSCID_TOX)

.PHONY : test

####################################################################################################

clean :
	-rm -rf .tox .venv build dist *.egg-info
	-rm -f src/libscid/_native/*.dll src/libscid/_native/*.dylib src/libscid/_native/*.so

.PHONY : clean

####################################################################################################

release-native-library :
	$(if $(LIBSCID_LIBRARY),,$(MAKE) -C $(LIBSCID_ROOT)src/libscid release-library)
	test -f $(LIBSCID_PYTHON_RELEASE_LIBRARY)
	mkdir -p $(libscid.python.native.dir)
	cp $(LIBSCID_PYTHON_RELEASE_LIBRARY) $(libscid.python.bundled.library)

.PHONY : release-native-library

####################################################################################################

release : release-native-library
	rm -rf dist
	LIBSCID_PYTHON_VERSION=$(LIBSCID_PYTHON_RELEASE_VERSION) $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) build --wheel
	mkdir -p $(LIBSCID_PYTHON_RELEASE_ARTIFACT_DIR)
	rm -f $(LIBSCID_PYTHON_RELEASE_ARTIFACT_DIR)/libscid-*.whl
	cp dist/*.whl $(LIBSCID_PYTHON_RELEASE_ARTIFACT_DIR)

.PHONY : release

####################################################################################################

release-verify : release
	wheel=$$(ls -1 dist/*.whl | tail -n 1); \
	rm -rf $(LIBSCID_PYTHON_RELEASE_VERIFY_DIR); \
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) venv $(LIBSCID_PYTHON_RELEASE_VERIFY_DIR); \
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) pip install --python $(LIBSCID_PYTHON_RELEASE_VERIFY_DIR)$(libscid.venv.python) "$$wheel"; \
	cd $(LIBSCID_ROOT); \
	env -u LIBSCID_LIBRARY $(LIBSCID_PYTHON_RELEASE_VERIFY_DIR)$(libscid.venv.python) -c \
	  "import libscid; game = libscid.Game.from_pgn('1. e4 e5 *'); assert game.mainline_move_count == 2"

.PHONY : release-verify

####################################################################################################

qc-format :
	$(LIBSCID_RUFF) format --check $(libscid.python.qc.paths)
	$(LIBSCID_RUFF) check $(libscid.python.qc.paths)

.PHONY : qc-format

####################################################################################################

qc-static-analysis :
	$(LIBSCID_PYRIGHT) $(LIBSCID_PYRIGHT_FLAGS)

.PHONY : qc-static-analysis

####################################################################################################

qc-dynamic-analysis :

.PHONY : qc-dynamic-analysis

####################################################################################################

qc-all : qc-format qc-static-analysis qc-dynamic-analysis

.PHONY : qc-all

####################################################################################################
