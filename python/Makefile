ifeq ($(.DEFAULT_GOAL),)
.DEFAULT_GOAL := libscid.python.test
endif

ifndef __libscid_python_Makefile
__libscid_python_Makefile := 1

libscid.python.__makefile := $(lastword $(MAKEFILE_LIST))
libscid.python.__project.dir := $(dir $(abspath $(libscid.python.__makefile)))
export ROOT ?= $(abspath $(libscid.python.__project.dir)..)/

include $(ROOT)etc/make/common.mk

libscid.python.__build.dir ?= $(LIBSCID_STAGING_BUILD_DIR)python/
libscid.python.__dist.dir ?= $(libscid.python.__build.dir)dist/
libscid.python.__install.dir ?= $(LIBSCID_STAGING_INSTALL_DIR)python/$(LIBSCID_PROFILE)/
libscid.python.__native.dir := $(libscid.python.__project.dir)src/libscid/_native/
libscid.python.__release.library ?= $(if $(LIBSCID_LIBRARY_PATH),$(LIBSCID_LIBRARY_PATH),$(LIBSCID_STAGING_INSTALL_DIR)capi/release/lib/$(libscid.__library.name))
libscid.python.__release.smoke-test.dir ?= $(libscid.python.__build.dir)release-smoke-test/
libscid.python.__release.source-smoke-test.dir ?= $(libscid.python.__build.dir)release-source-smoke-test/
libscid.python.__release.source-smoke-test.src.dir ?= $(libscid.capi.__release.source-smoke-test.dir)src/
libscid.python.__release.artefact.dir ?= $(LIBSCID_STAGING_RELEASE_DIR)python/
libscid.python.__release.version ?= $(if $(LIBSCID_RELEASE_PYTHON_VERSION),$(LIBSCID_RELEASE_PYTHON_VERSION),$(LIBSCID_RELEASE_VERSION))
libscid.python.__bin.dir := $(libscid.python.__project.dir).venv/$(dir $(libscid.__venv.python))
LIBSCID_UV_FLAGS ?=
LIBSCID_PYTHON_BUILD_SDIST ?= true
LIBSCID_PYTHON_INSTALL_BREAK_SYSTEM_PACKAGES ?= true
LIBSCID_PYTHON_INSTALL_ARGS ?= $(if $(filter true 1,$(LIBSCID_PYTHON_INSTALL_BREAK_SYSTEM_PACKAGES)),--break-system-packages)
libscid.python.__qc.paths := src tests hatch_build.py ../examples/python


####################################################################################################

libscid.python.configure : ## Synchronise Python virtual environment and dependencies using uv
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) sync $(LIBSCID_UV_FLAGS)

.PHONY : libscid.python.configure

####################################################################################################

libscid.python.build : libscid.python.configure ## Build Python wheel package
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) build --out-dir $(libscid.python.__dist.dir)

.PHONY : libscid.python.build

####################################################################################################

libscid.python.test : libscid.python.configure ## Run Python test matrix via tox
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run $(LIBSCID_TOX)

.PHONY : libscid.python.test

####################################################################################################

libscid.python.clean : ## Remove Python virtual environments, build trees, and caches
	-rm -rf $(libscid.python.__project.dir).tox $(libscid.python.__project.dir).venv
	-rm -rf $(libscid.python.__project.dir)build $(libscid.python.__project.dir)dist
	-rm -rf $(libscid.python.__project.dir)*.egg-info
	-rm -rf $(libscid.python.__project.dir)_build/docs
	-rm -f $(libscid.python.__native.dir)*.dll $(libscid.python.__native.dir)*.dylib $(libscid.python.__native.dir)*.so
	-rm -rf $(libscid.python.__build.dir)
	-rm -rf $(libscid.python.__release.artefact.dir)
	-rm -rf $(libscid.python.__install.dir)

.PHONY : libscid.python.clean

####################################################################################################

libscid.python.install : libscid.python.__release-wheel ## Install libscid Python package into active environment or prefix
	shopt -s nullglob; \
	wheels=( "$(libscid.python.__release.artefact.dir)"libscid-*.whl ); \
	[[ $${#wheels[@]} -gt 0 ]] || { echo "No wheel found to install" >&2; exit 1; }; \
	wheel="$${wheels[$$(($${#wheels[@]} - 1))]}"; \
	if [ -n "$${DESTDIR:-}" ]; then \
		$(LIBSCID_PYTHON) -m pip install \
			--no-deps \
			$$(if [ -n "$${PREFIX:-}" ]; then echo "--prefix $${PREFIX}"; fi) \
			--root "$${DESTDIR}" \
			$(LIBSCID_PYTHON_INSTALL_ARGS) \
			"$$wheel"; \
	elif [ -n "$${PREFIX:-}" ]; then \
		$(LIBSCID_UV) pip install \
			--python "$(LIBSCID_PYTHON)" \
			--no-deps \
			--prefix "$${PREFIX}" \
			$(LIBSCID_PYTHON_INSTALL_ARGS) \
			"$$wheel"; \
	else \
		$(LIBSCID_UV) pip install \
			--python "$(LIBSCID_PYTHON)" \
			--no-deps \
			--prefix "$(libscid.python.__install.dir)" \
			$(LIBSCID_PYTHON_INSTALL_ARGS) \
			"$$wheel"; \
	fi

.PHONY : libscid.python.install

####################################################################################################

libscid.python.__release-wheel :
	cd $(libscid.python.__project.dir) \
	&& mkdir -p $(libscid.python.__dist.dir) \
	&& rm -f $(libscid.python.__dist.dir)*.whl \
	&& LIBSCID_PYTHON_VERSION=$(libscid.python.__release.version) \
	   LIBSCID_LIBRARY_PATH=$(libscid.python.__release.library) \
	   $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) build --wheel --out-dir $(libscid.python.__dist.dir)
	mkdir -p $(libscid.python.__release.artefact.dir)
	rm -f $(libscid.python.__release.artefact.dir)libscid-*.whl
	cp $(libscid.python.__dist.dir)*.whl $(libscid.python.__release.artefact.dir)

.PHONY : libscid.python.__release-wheel

####################################################################################################

libscid.python.__release-sdist :
	cd $(libscid.python.__project.dir) \
	&& mkdir -p $(libscid.python.__dist.dir) \
	&& rm -f $(libscid.python.__dist.dir)*.tar.gz \
	&& LIBSCID_PYTHON_VERSION=$(libscid.python.__release.version) \
	   $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) build --sdist --out-dir $(libscid.python.__dist.dir)
	mkdir -p $(libscid.python.__release.artefact.dir)
	rm -f $(libscid.python.__release.artefact.dir)libscid-*.tar.gz
	cp $(libscid.python.__dist.dir)*.tar.gz $(libscid.python.__release.artefact.dir)

.PHONY : libscid.python.__release-sdist

####################################################################################################

libscid.python.__release-smoke-test : libscid.python.__release-wheel
	shopt -s nullglob; \
	wheels=( "$(libscid.python.__release.artefact.dir)"libscid-*.whl ); \
	[[ $${#wheels[@]} -gt 0 ]] || { echo "No wheel found to test in $(libscid.python.__release.artefact.dir)" >&2; exit 1; }; \
	wheel="$${wheels[$$(($${#wheels[@]} - 1))]}"; \
	rm -rf $(libscid.python.__release.smoke-test.dir); \
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) venv $(libscid.python.__release.smoke-test.dir); \
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) pip install --python $(libscid.python.__release.smoke-test.dir)$(libscid.__venv.python) "$$wheel"; \
	cd $(ROOT); \
	unset LIBSCID_LIBRARY_PATH; \
	unset LIBSCID_LIBRARY_PREFIX; \
	$(libscid.python.__release.smoke-test.dir)$(libscid.__venv.python) -c \
	    "import libscid; game = libscid.Game.from_pgn('1. e4 e5 *'); assert game.mainline_move_count == 2"

.PHONY : libscid.python.__release-smoke-test

####################################################################################################

libscid.python.__release-sdist-smoke-test : libscid.python.__release-sdist
	shopt -s nullglob; \
	sdists=( "$(libscid.python.__release.artefact.dir)"libscid-*.tar.gz ); \
	[[ $${#sdists[@]} -gt 0 ]] || { echo "No sdist found to test in $(libscid.python.__release.artefact.dir)" >&2; exit 1; }; \
	sdist="$${sdists[$$(($${#sdists[@]} - 1))]}"; \
	rm -rf $(libscid.python.__release.smoke-test.dir); \
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) venv $(libscid.python.__release.smoke-test.dir); \
	unset LIBSCID_LIBRARY_PATH; \
	unset LIBSCID_LIBRARY_PREFIX; \
	output=$$($(LIBSCID_UV) $(LIBSCID_UV_FLAGS) pip install --no-cache --python $(libscid.python.__release.smoke-test.dir)$(libscid.__venv.python) "$$sdist" 2>&1 || true); \
	echo "$$output"; \
	echo "$$output" | grep -q "There are no prebuilt libscid packages for your platform"

.PHONY : libscid.python.__release-sdist-smoke-test

####################################################################################################

####################################################################################################

libscid.python.__release-source-smoke-test : libscid.capi.__release-source-smoke-test
	$(MAKE) -C $(libscid.python.__release.source-smoke-test.src.dir) libscid.python.__release-wheel
	shopt -s nullglob; \
	wheels=( "$(libscid.python.__release.source-smoke-test.src.dir)_staging/release/python/"libscid-*.whl ); \
	[[ $${#wheels[@]} -gt 0 ]] || { echo "No wheel found to test in source smoke test" >&2; exit 1; }; \
	wheel="$${wheels[$$(($${#wheels[@]} - 1))]}"; \
	rm -rf $(libscid.python.__release.source-smoke-test.dir); \
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) venv $(libscid.python.__release.source-smoke-test.dir); \
	$(LIBSCID_UV) $(LIBSCID_UV_FLAGS) pip install --python $(libscid.python.__release.source-smoke-test.dir)$(libscid.__venv.python) "$$wheel"; \
	cd $(ROOT); \
	unset LIBSCID_LIBRARY_PATH; \
	unset LIBSCID_LIBRARY_PREFIX; \
	$(libscid.python.__release.source-smoke-test.dir)$(libscid.__venv.python) -c \
	    "import libscid; game = libscid.Game.from_pgn('1. e4 e5 *'); assert game.mainline_move_count == 2"

.PHONY : libscid.python.__release-source-smoke-test

####################################################################################################

libscid.python.__release.source.targets :=
ifeq ($(LIBSCID_BUILD_SOURCE_ARCHIVE),true)
libscid.python.__release.source.targets := libscid.python.__release-source-smoke-test
endif

libscid.python.__release.sdist.targets :=
ifeq ($(LIBSCID_PYTHON_BUILD_SDIST),true)
libscid.python.__release.sdist.targets := libscid.python.__release-sdist libscid.python.__release-sdist-smoke-test
endif

libscid.python.release : libscid.python.__release-wheel libscid.python.__release-smoke-test $(libscid.python.__release.source.targets) $(libscid.python.__release.sdist.targets) ## Build Python wheel, sdist, and run smoke tests

.PHONY : libscid.python.release


####################################################################################################

libscid.python.qc-format : libscid.python.configure ## Check and verify Python formatting with ruff
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run --no-sync ruff format --check $(libscid.python.__qc.paths) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run --no-sync ruff check $(libscid.python.__qc.paths)

.PHONY : libscid.python.qc-format

####################################################################################################

libscid.python.qc-static-analysis : libscid.python.configure ## Run static type checking with pyright
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run --no-sync pyright

.PHONY : libscid.python.qc-static-analysis

####################################################################################################

libscid.python.qc-dynamic-analysis : ## Run dynamic analysis for Python package (noop)

.PHONY : libscid.python.qc-dynamic-analysis

####################################################################################################

libscid.python.qc-all : libscid.python.qc-format libscid.python.qc-static-analysis libscid.python.qc-dynamic-analysis ## Run all quality control checks for Python package

.PHONY : libscid.python.qc-all

####################################################################################################

libscid.python.docs-configure : libscid.python.configure ## Install documentation dependencies for Python package
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) sync $(LIBSCID_UV_FLAGS) --group docs

.PHONY : libscid.python.docs-configure

####################################################################################################

libscid.python.docs : libscid.python.docs-configure ## Build Python documentation site via MkDocs
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run --group docs --no-sync mkdocs build -f mkdocs.yml -d $(libscid.python.__build.dir)docs/site

.PHONY : libscid.python.docs

####################################################################################################

libscid.python.docs-serve : libscid.python.docs-configure ## Launch local MkDocs live documentation server
	cd $(libscid.python.__project.dir) \
	&& $(LIBSCID_UV) $(LIBSCID_UV_FLAGS) run --group docs --no-sync mkdocs serve -f mkdocs.yml -d $(libscid.python.__build.dir)docs/site

.PHONY : libscid.python.docs-serve

####################################################################################################

endif


