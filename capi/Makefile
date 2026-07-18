.DEFAULT_GOAL := libscid.capi.test

libscid.capi.__makefile := $(lastword $(MAKEFILE_LIST))
libscid.capi.__project.dir := $(dir $(abspath $(libscid.capi.__makefile)))
export ROOT ?= $(abspath $(libscid.capi.__project.dir)..)/

include $(ROOT)etc/make/common.mk

libscid.capi.__build.root ?= $(libscid.capi.__project.dir)_build/
libscid.capi.__default.build.root ?= $(libscid.capi.__project.dir)_build/default/

libscid.capi.__build.dir ?= $(libscid.capi.__build.root)
libscid.capi.__cmake.source.root ?= $(ROOT)
libscid.capi.__cmake.source.dir ?= $(libscid.capi.__project.dir)
libscid.capi.__cmake.build.targets := scid scid_tests_c
libscid.capi.__cmake.test.labels := libscid
libscid.capi.__cmake.shared.libs ?= $(LIBSCID_CMAKE_SHARED_LIBS)
libscid.capi.__cmake.install := OFF
libscid.capi.__qc.format.build.dir ?= $(libscid.capi.__build.root)format/
libscid.capi.__qc.analysis.build.dir ?= $(libscid.capi.__build.root)analysis/
libscid.capi.__qc.dynamic-analysis.build.dir ?= $(libscid.capi.__build.root)sanitisers/

libscid.capi.default.__build.dir ?= $(libscid.capi.__default.build.root)
libscid.capi.default.__cmake.source.root ?= $(ROOT)
libscid.capi.default.__cmake.source.dir ?= $(libscid.capi.__project.dir)
libscid.capi.default.__cmake.build.targets := $(libscid.capi.__cmake.build.targets)
libscid.capi.default.__cmake.test.labels := $(libscid.capi.__cmake.test.labels)
libscid.capi.default.__cmake.shared.libs := OFF
libscid.capi.default.__cmake.install := OFF
libscid.capi.default.__qc.format.build.dir ?= $(libscid.capi.__default.build.root)format/
libscid.capi.default.__qc.analysis.build.dir ?= $(libscid.capi.__default.build.root)analysis/
libscid.capi.default.__qc.dynamic-analysis.build.dir ?= $(libscid.capi.__default.build.root)sanitisers/

libscid.capi.__release.build.dir ?= $(libscid.capi.__project.dir)_build/release/
libscid.capi.__release.package.build.dir ?= $(libscid.capi.__project.dir)_build/package/
libscid.capi.__release.smoke-test.dir ?= $(libscid.capi.__project.dir)_build/release-smoke-test/
libscid.capi.__release.build.docs ?= ON
libscid.capi.__release.artifact.dir ?= $(LIBSCID_RELEASE_ROOT)
libscid.capi.__release.artifact := $(libscid.capi.__release.build.dir)$(libscid.__library.name)
libscid.capi.artifact := $(libscid.capi.__build.dir)$(libscid.__library.name)
libscid.capi.release.artifact := $(libscid.capi.__release.artifact)

####################################################################################################

include $(ROOT)etc/make/cmake.mk

$(eval $(call libscid.cmake.__project.rules,libscid.capi))
$(eval $(call libscid.cmake.__project.rules,libscid.capi.default))

####################################################################################################

$(libscid.capi.artifact) : libscid.capi.build

####################################################################################################

$(libscid.capi.release.artifact) : libscid.capi.release-library

####################################################################################################

libscid.capi.__release-library :
	$(LIBSCID_CMAKE) \
	  -S $(libscid.capi.__project.dir) \
	  -B $(libscid.capi.__release.build.dir) \
	  $(libscid.cmake.__generator.arg) \
	  -DBUILD_TESTING=OFF \
	  -DBUILD_SHARED_LIBS=ON \
	  -DLIBSCID_INSTALL=ON \
	  "-DLIBSCID_SOURCE_ROOT=$(ROOT)" \
	  -DCMAKE_BUILD_TYPE=Release \
	  "-DLIBSCID_PROJECT_VERSION=$(LIBSCID_RELEASE_PROJECT_VERSION)" \
	  "-DLIBSCID_VERSION=$(LIBSCID_RELEASE_VERSION)" \
	  $(libscid.cmake.__c.compiler.arg) \
	  $(libscid.cmake.__cxx.compiler.arg) \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build $(libscid.capi.__release.build.dir) \
	  --config Release \
	  --target scid \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : libscid.capi.__release-library

####################################################################################################

libscid.capi.release-library : libscid.capi.__release-library

.PHONY : libscid.capi.release-library

####################################################################################################

libscid.capi.__release-package : libscid.capi.__release-library
	$(LIBSCID_CMAKE) \
	  -S $(libscid.capi.__project.dir) \
	  -B $(libscid.capi.__release.package.build.dir) \
	  $(libscid.cmake.__generator.arg) \
	  -DBUILD_TESTING=OFF \
	  -DBUILD_SHARED_LIBS=ON \
	  -DLIBSCID_INSTALL=ON \
	  -DLIBSCID_BUILD_DOCS=$(libscid.capi.__release.build.docs) \
	  "-DLIBSCID_SOURCE_ROOT=$(ROOT)" \
	  -DCMAKE_BUILD_TYPE=Release \
	  "-DLIBSCID_PROJECT_VERSION=$(LIBSCID_RELEASE_PROJECT_VERSION)" \
	  "-DLIBSCID_VERSION=$(LIBSCID_RELEASE_VERSION)" \
	  "-DLIBSCID_PACKAGE_VERSION_LABEL=$(LIBSCID_RELEASE_PACKAGE_VERSION_LABEL)" \
	  "-DLIBSCID_PACKAGE_PLATFORM=$(LIBSCID_RELEASE_PLATFORM)" \
	  $(libscid.cmake.__plantuml.jar.path.arg) \
	  $(libscid.cmake.__c.compiler.arg) \
	  $(libscid.cmake.__cxx.compiler.arg) \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build $(libscid.capi.__release.package.build.dir) \
	  --config Release \
	  $(LIBSCID_CMAKE_BUILD_ARGS)
	rm -f $(libscid.capi.__release.package.build.dir)*.tar.gz
	$(LIBSCID_CPACK) \
	  --config $(libscid.capi.__release.package.build.dir)CPackConfig.cmake \
	  -G TGZ
	mkdir -p $(libscid.capi.__release.artifact.dir)
	rm -f $(libscid.capi.__release.artifact.dir)/libscid__*.tar.gz
	cp $(libscid.capi.__release.package.build.dir)*.tar.gz $(libscid.capi.__release.artifact.dir)

.PHONY : libscid.capi.__release-package

####################################################################################################

libscid.capi.release-package : libscid.capi.__release-package

.PHONY : libscid.capi.release-package

####################################################################################################

libscid.capi.__release-smoke-test : libscid.capi.release-package
	shopt -s nullglob; \
	archives=( "$(libscid.capi.__release.artifact.dir)"/libscid__*.tar.gz ); \
	[[ $${#archives[@]} -eq 1 ]]; \
	archive="$${archives[0]}"; \
	extract_root="$(libscid.capi.__release.smoke-test.dir)package"; \
	install_root="$$extract_root/install"; \
	examples_build_dir="$(libscid.capi.__release.smoke-test.dir)examples"; \
	rm -rf "$(libscid.capi.__release.smoke-test.dir)"; \
	mkdir -p "$$install_root"; \
	tar -xzf "$$archive" -C "$$install_root" --strip-components=1; \
	cmake_args=( \
	  -S "$$install_root/share/doc/libscid/examples" \
	  -B "$$examples_build_dir" \
	  "-DCMAKE_PREFIX_PATH=$$install_root" \
	); \
	[[ -z "$(LIBSCID_CMAKE_GENERATOR)" ]] || cmake_args+=( -G "$(LIBSCID_CMAKE_GENERATOR)" ); \
	[[ -z "$(LIBSCID_CMAKE_C_COMPILER)" ]] || cmake_args+=( "-DCMAKE_C_COMPILER=$(LIBSCID_CMAKE_C_COMPILER)" ); \
	[[ -z "$(LIBSCID_CMAKE_CXX_COMPILER)" ]] || cmake_args+=( "-DCMAKE_CXX_COMPILER=$(LIBSCID_CMAKE_CXX_COMPILER)" ); \
	cmake_args+=( $(LIBSCID_CMAKE_CONFIGURE_ARGS) ); \
	$(LIBSCID_CMAKE) "$${cmake_args[@]}"; \
	$(LIBSCID_CMAKE) --build "$$examples_build_dir" $(LIBSCID_CMAKE_BUILD_ARGS); \
	case "$(libscid.__host.system)" in \
	  Darwin) export DYLD_LIBRARY_PATH="$$install_root/lib:$${DYLD_LIBRARY_PATH:-}" ;; \
	  MINGW*|MSYS*|CYGWIN*) export PATH="$$install_root/bin:$$PATH" ;; \
	  *) export LD_LIBRARY_PATH="$$install_root/lib:$${LD_LIBRARY_PATH:-}" ;; \
	esac; \
	$(LIBSCID_CTEST) --test-dir "$$examples_build_dir" --output-on-failure

.PHONY : libscid.capi.__release-smoke-test

####################################################################################################

libscid.capi.release : libscid.capi.release-package libscid.capi.__release-smoke-test

.PHONY : libscid.capi.release

####################################################################################################
