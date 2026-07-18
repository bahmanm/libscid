.DEFAULT_GOAL := libscid.library.test

libscid.library.__makefile := $(lastword $(MAKEFILE_LIST))
libscid.library.__project.dir := $(dir $(abspath $(libscid.library.__makefile)))
export ROOT ?= $(abspath $(libscid.library.__project.dir)../..)/

include $(ROOT)etc/make/common.mk

libscid.library.__build.root ?= $(libscid.library.__project.dir)_build/
libscid.library.__default.build.root ?= $(libscid.library.__project.dir)_build/default/

libscid.library.__build.dir ?= $(libscid.library.__build.root)
libscid.library.__cmake.source.root ?= $(ROOT)
libscid.library.__cmake.source.dir ?= $(libscid.library.__project.dir)
libscid.library.__cmake.build.targets := scid scid_tests_c
libscid.library.__cmake.test.labels := libscid
libscid.library.__cmake.shared.libs ?= $(LIBSCID_CMAKE_SHARED_LIBS)
libscid.library.__cmake.install := OFF
libscid.library.__qc.format.build.dir ?= $(libscid.library.__build.root)format/
libscid.library.__qc.analysis.build.dir ?= $(libscid.library.__build.root)analysis/
libscid.library.__qc.dynamic-analysis.build.dir ?= $(libscid.library.__build.root)sanitisers/

libscid.library.default.__build.dir ?= $(libscid.library.__default.build.root)
libscid.library.default.__cmake.source.root ?= $(ROOT)
libscid.library.default.__cmake.source.dir ?= $(libscid.library.__project.dir)
libscid.library.default.__cmake.build.targets := $(libscid.library.__cmake.build.targets)
libscid.library.default.__cmake.test.labels := $(libscid.library.__cmake.test.labels)
libscid.library.default.__cmake.shared.libs := OFF
libscid.library.default.__cmake.install := OFF
libscid.library.default.__qc.format.build.dir ?= $(libscid.library.__default.build.root)format/
libscid.library.default.__qc.analysis.build.dir ?= $(libscid.library.__default.build.root)analysis/
libscid.library.default.__qc.dynamic-analysis.build.dir ?= $(libscid.library.__default.build.root)sanitisers/

libscid.library.__release.build.dir ?= $(libscid.library.__project.dir)_build/release/
libscid.library.__release.package.build.dir ?= $(libscid.library.__project.dir)_build/package/
libscid.library.__release.smoke-test.dir ?= $(libscid.library.__project.dir)_build/release-smoke-test/
libscid.library.__release.build.docs ?= ON
libscid.library.__release.artifact.dir ?= $(LIBSCID_RELEASE_ROOT)
libscid.library.__release.artifact := $(libscid.library.__release.build.dir)$(libscid.__library.name)
libscid.library.artifact := $(libscid.library.__build.dir)$(libscid.__library.name)
libscid.library.release.artifact := $(libscid.library.__release.artifact)

####################################################################################################

include $(ROOT)etc/make/cmake.mk

$(eval $(call libscid.cmake.__project.rules,libscid.library))
$(eval $(call libscid.cmake.__project.rules,libscid.library.default))

####################################################################################################

$(libscid.library.artifact) : libscid.library.build

####################################################################################################

$(libscid.library.release.artifact) : libscid.library.release-library

####################################################################################################

libscid.library.__release-library :
	$(LIBSCID_CMAKE) \
	  -S $(libscid.library.__project.dir) \
	  -B $(libscid.library.__release.build.dir) \
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
	  --build $(libscid.library.__release.build.dir) \
	  --config Release \
	  --target scid \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : libscid.library.__release-library

####################################################################################################

libscid.library.release-library : libscid.library.__release-library

.PHONY : libscid.library.release-library

####################################################################################################

libscid.library.__release-package : libscid.library.__release-library
	$(LIBSCID_CMAKE) \
	  -S $(libscid.library.__project.dir) \
	  -B $(libscid.library.__release.package.build.dir) \
	  $(libscid.cmake.__generator.arg) \
	  -DBUILD_TESTING=OFF \
	  -DBUILD_SHARED_LIBS=ON \
	  -DLIBSCID_INSTALL=ON \
	  -DLIBSCID_BUILD_DOCS=$(libscid.library.__release.build.docs) \
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
	  --build $(libscid.library.__release.package.build.dir) \
	  --config Release \
	  $(LIBSCID_CMAKE_BUILD_ARGS)
	rm -f $(libscid.library.__release.package.build.dir)*.tar.gz
	$(LIBSCID_CPACK) \
	  --config $(libscid.library.__release.package.build.dir)CPackConfig.cmake \
	  -G TGZ
	mkdir -p $(libscid.library.__release.artifact.dir)
	rm -f $(libscid.library.__release.artifact.dir)/libscid__*.tar.gz
	cp $(libscid.library.__release.package.build.dir)*.tar.gz $(libscid.library.__release.artifact.dir)

.PHONY : libscid.library.__release-package

####################################################################################################

libscid.library.release-package : libscid.library.__release-package

.PHONY : libscid.library.release-package

####################################################################################################

libscid.library.__release-smoke-test : libscid.library.release-package
	shopt -s nullglob; \
	archives=( "$(libscid.library.__release.artifact.dir)"/libscid__*.tar.gz ); \
	[[ $${#archives[@]} -eq 1 ]]; \
	archive="$${archives[0]}"; \
	extract_root="$(libscid.library.__release.smoke-test.dir)package"; \
	install_root="$$extract_root/install"; \
	examples_build_dir="$(libscid.library.__release.smoke-test.dir)examples"; \
	rm -rf "$(libscid.library.__release.smoke-test.dir)"; \
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

.PHONY : libscid.library.__release-smoke-test

####################################################################################################

libscid.library.release : libscid.library.release-package libscid.library.__release-smoke-test

.PHONY : libscid.library.release

####################################################################################################
