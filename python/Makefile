.DEFAULT_GOAL := libscid.python.test

libscid.python.__makefile := $(lastword $(MAKEFILE_LIST))
libscid.python.__project.dir := $(dir $(abspath $(libscid.python.__makefile)))
export ROOT ?= $(abspath $(libscid.python.__project.dir)..)/

include $(ROOT)etc/make/common.mk

libscid.python.__native.dir := $(libscid.python.__project.dir)src/libscid/_native/
libscid.python.__bundled.library := $(libscid.python.__native.dir)$(libscid.__library.name)
libscid.python.__release.library ?= $(if $(LIBSCID_LIBRARY),$(LIBSCID_LIBRARY),$(ROOT)capi/_build/release/$(libscid.__library.name))
libscid.python.__release.smoke-test.dir ?= $(libscid.python.__project.dir)_build/release-smoke-test/
libscid.python.__release.artifact.dir ?= $(LIBSCID_RELEASE_ROOT)
libscid.python.__release.version ?= $(LIBSCID_RELEASE_PROJECT_VERSION)
LIBSCID_UV_FLAGS ?= --system-certs
libscid.python.__qc.paths := src tests hatch_build.py

####################################################################################################

libscid.python.configure :
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) sync --group dev

.PHONY : libscid.python.configure

####################################################################################################

libscid.python.build : libscid.python.configure
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) build

.PHONY : libscid.python.build

####################################################################################################

libscid.python.test : libscid.python.configure
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run $(LIBSCID_TOX)

.PHONY : libscid.python.test

####################################################################################################

libscid.python.clean :
	-rm -rf $(libscid.python.__project.dir).tox $(libscid.python.__project.dir).venv
	-rm -rf $(libscid.python.__project.dir)build $(libscid.python.__project.dir)dist
	-rm -rf $(libscid.python.__project.dir)*.egg-info
	-rm -f $(libscid.python.__native.dir)*.dll $(libscid.python.__native.dir)*.dylib $(libscid.python.__native.dir)*.so

.PHONY : libscid.python.clean

####################################################################################################

libscid.python.__release-native-library : $(libscid.python.__release.library)
	mkdir -p $(libscid.python.__native.dir)
	cp $(libscid.python.__release.library) $(libscid.python.__bundled.library)

.PHONY : libscid.python.__release-native-library

####################################################################################################

libscid.python.__release-wheel : libscid.python.__release-native-library
	cd $(libscid.python.__project.dir) \
	&& rm -rf dist \
	&& LIBSCID_PYTHON_VERSION=$(libscid.python.__release.version) $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) build --wheel
	mkdir -p $(libscid.python.__release.artifact.dir)
	rm -f $(libscid.python.__release.artifact.dir)/libscid-*.whl
	cp $(libscid.python.__project.dir)dist/*.whl $(libscid.python.__release.artifact.dir)

.PHONY : libscid.python.__release-wheel

####################################################################################################

libscid.python.__release-smoke-test : libscid.python.__release-wheel
	shopt -s nullglob; \
	wheels=( "$(libscid.python.__project.dir)"dist/*.whl ); \
	[[ $${#wheels[@]} -gt 0 ]]; \
	wheel="$${wheels[$$(($${#wheels[@]} - 1))]}"; \
	rm -rf $(libscid.python.__release.smoke-test.dir); \
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) venv $(libscid.python.__release.smoke-test.dir); \
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) pip install --python $(libscid.python.__release.smoke-test.dir)$(libscid.__venv.python) "$$wheel"; \
	cd $(ROOT); \
	unset LIBSCID_LIBRARY; \
	$(libscid.python.__release.smoke-test.dir)$(libscid.__venv.python) -c \
	  "import libscid; game = libscid.Game.from_pgn('1. e4 e5 *'); assert game.mainline_move_count == 2"

.PHONY : libscid.python.__release-smoke-test

####################################################################################################

libscid.python.release : libscid.python.__release-wheel libscid.python.__release-smoke-test

.PHONY : libscid.python.release

####################################################################################################

libscid.python.qc-format :
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run ruff format --check $(libscid.python.__qc.paths) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run ruff check $(libscid.python.__qc.paths)

.PHONY : libscid.python.qc-format

####################################################################################################

libscid.python.qc-static-analysis :
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run pyright

.PHONY : libscid.python.qc-static-analysis

####################################################################################################

libscid.python.qc-dynamic-analysis :

.PHONY : libscid.python.qc-dynamic-analysis

####################################################################################################

libscid.python.qc-all : libscid.python.qc-format libscid.python.qc-static-analysis libscid.python.qc-dynamic-analysis

.PHONY : libscid.python.qc-all

####################################################################################################
