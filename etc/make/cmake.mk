####################################################################################################
# $(call libscid.cmake.__project.rules,PROJECT[,BASE_PROJECT])
#
# Generates the standard CMake targets for `PROJECT`.
#
# PROJECT
#   Namespaces generated targets and variables.  For example, `libscid.capi` generates
#   `libscid.capi.configure`, `libscid.capi.build`, `libscid.capi.test`, and QC targets.
#
# BASE_PROJECT
#   Optional source project used for defaults.  This is useful for project variants such as
#   `libscid.capi.default`, which should generate its own targets but inherit source-directory and
#   default build-directory conventions from `libscid.capi`.
#
# Required project variables:
#   PROJECT.__cmake.build.targets
#
# Optional project variables with defaults:
#   PROJECT.__build.dir                      BASE_PROJECT.__project.dir + _build/
#   PROJECT.__cmake.source.root              $(ROOT)
#   PROJECT.__cmake.source.dir               BASE_PROJECT.__project.dir
#   PROJECT.__cmake.shared.libs              $(LIBSCID_CMAKE_SHARED_LIBS)
#   PROJECT.__cmake.install                  OFF
#   PROJECT.__qc.format.build.dir            BASE_PROJECT.__project.dir + _build/format/
#   PROJECT.__qc.analysis.build.dir          BASE_PROJECT.__project.dir + _build/analysis/
#   PROJECT.__qc.dynamic-analysis.build.dir  BASE_PROJECT.__project.dir + _build/sanitisers/
#
# Optional project variables without defaults:
#   PROJECT.__cmake.test.labels
####################################################################################################

define libscid.cmake.__project.rules
####################################################################################################

$(1).__cmake.contract : \
  bmakelib.default-if-blank( $(1).__build.dir,$$($(or $(2),$(1)).__project.dir)_build/ ) \
  bmakelib.default-if-blank( $(1).__cmake.source.root,$$(ROOT) ) \
  bmakelib.default-if-blank( $(1).__cmake.source.dir,$$($(or $(2),$(1)).__project.dir) ) \
  bmakelib.default-if-blank( $(1).__cmake.shared.libs,$$(LIBSCID_CMAKE_SHARED_LIBS) ) \
  bmakelib.default-if-blank( $(1).__cmake.install,OFF ) \
  bmakelib.default-if-blank( $(1).__qc.format.build.dir,$$($(or $(2),$(1)).__project.dir)_build/format/ ) \
  bmakelib.default-if-blank( $(1).__qc.analysis.build.dir,$$($(or $(2),$(1)).__project.dir)_build/analysis/ ) \
  bmakelib.default-if-blank( $(1).__qc.dynamic-analysis.build.dir,$$($(or $(2),$(1)).__project.dir)_build/sanitisers/ ) \
  .WAIT \
  bmakelib.error-if-blank( \
    $(1).__build.dir \
    $(1).__cmake.source.root \
    $(1).__cmake.source.dir \
    $(1).__cmake.build.targets \
    $(1).__cmake.shared.libs \
    $(1).__cmake.install \
    $(1).__qc.format.build.dir \
    $(1).__qc.analysis.build.dir \
    $(1).__qc.dynamic-analysis.build.dir \
  )

.PHONY : $(1).__cmake.contract

####################################################################################################

$(1).configure : $(1).__cmake.contract
	$$(LIBSCID_CMAKE) \
	  -S $$($(1).__cmake.source.dir) \
	  -B $$($(1).__build.dir) \
	  $$(libscid.cmake.__generator.arg) \
	  -DBUILD_TESTING=ON \
	  $$(if $$($(1).__cmake.shared.libs),-DBUILD_SHARED_LIBS=$$($(1).__cmake.shared.libs)) \
	  -DLIBSCID_INSTALL=$$($(1).__cmake.install) \
	  "-DLIBSCID_SOURCE_ROOT=$$($(1).__cmake.source.root)" \
	  -DCMAKE_BUILD_TYPE=$$(LIBSCID_CMAKE_BUILD_TYPE) \
	  $$(libscid.cmake.__c.compiler.arg) \
	  $$(libscid.cmake.__cxx.compiler.arg) \
	  $$(LIBSCID_CMAKE_CONFIGURE_ARGS)

.PHONY : $(1).configure

####################################################################################################

$(1).build : $(1).configure
	$$(LIBSCID_CMAKE) \
	  --build $$($(1).__build.dir) \
	  --config $$(LIBSCID_CMAKE_BUILD_TYPE) \
	  $$(foreach target,$$($(1).__cmake.build.targets),--target $$(target)) \
	  $$(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : $(1).build

####################################################################################################

$(1).test : $(1).build
	$$(LIBSCID_CTEST) \
	  --test-dir $$($(1).__build.dir) \
	  -C $$(LIBSCID_CMAKE_BUILD_TYPE) \
	  $$(if $$($(1).__cmake.test.labels),-L '$$($(1).__cmake.test.labels)') \
	  --output-on-failure

.PHONY : $(1).test

####################################################################################################

$(1).clean : $(1).__cmake.contract
	-rm -rf $$($(1).__build.dir)

.PHONY : $(1).clean

####################################################################################################

$(1).qc-format : $(1).__cmake.contract
	$$(LIBSCID_CMAKE) \
	  -S $$($(1).__cmake.source.dir) \
	  -B $$($(1).__qc.format.build.dir) \
	  $$(libscid.cmake.__generator.arg) \
	  $$(libscid.cmake.__c.compiler.arg) \
	  $$(libscid.cmake.__cxx.compiler.arg) \
	  "-DLIBSCID_SOURCE_ROOT=$$($(1).__cmake.source.root)" \
	  $$(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$$(LIBSCID_CMAKE) \
	  --build $$($(1).__qc.format.build.dir) \
	  --target format-check \
	  $$(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : $(1).qc-format

####################################################################################################

$(1).__qc-cppcheck : $(1).__cmake.contract
	$$(LIBSCID_CMAKE) \
	  -S $$($(1).__cmake.source.dir) \
	  -B $$($(1).__qc.analysis.build.dir) \
	  $$(libscid.cmake.__c.compiler.arg) \
	  $$(libscid.cmake.__cxx.compiler.arg) \
	  -DCMAKE_BUILD_TYPE=Debug \
	  -DBUILD_TESTING=OFF \
	  -DLIBSCID_INSTALL=OFF \
	  "-DLIBSCID_SOURCE_ROOT=$$($(1).__cmake.source.root)" \
	  $$(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$$(LIBSCID_CMAKE) \
	  --build $$($(1).__qc.analysis.build.dir) \
	  --target cppcheck \
	  $$(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : $(1).__qc-cppcheck

####################################################################################################

$(1).__qc-tidy : $(1).__cmake.contract
	$$(LIBSCID_CMAKE) \
	  -S $$($(1).__cmake.source.dir) \
	  -B $$($(1).__qc.analysis.build.dir) \
	  $$(libscid.cmake.__c.compiler.arg) \
	  $$(libscid.cmake.__cxx.compiler.arg) \
	  -DCMAKE_BUILD_TYPE=Debug \
	  -DBUILD_TESTING=OFF \
	  -DLIBSCID_INSTALL=OFF \
	  "-DLIBSCID_SOURCE_ROOT=$$($(1).__cmake.source.root)" \
	  $$(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$$(LIBSCID_CMAKE) \
	  --build $$($(1).__qc.analysis.build.dir) \
	  --target clang-tidy \
	  $$(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : $(1).__qc-tidy

####################################################################################################

$(1).qc-static-analysis : $(1).__qc-cppcheck $(1).__qc-tidy

.PHONY : $(1).qc-static-analysis

####################################################################################################

$(1).qc-dynamic-analysis : $(1).__cmake.contract
	$$(LIBSCID_CMAKE) \
	  -S $$($(1).__cmake.source.dir) \
	  -B $$($(1).__qc.dynamic-analysis.build.dir) \
	  $$(libscid.cmake.__c.compiler.arg) \
	  $$(libscid.cmake.__cxx.compiler.arg) \
	  -DCMAKE_BUILD_TYPE=Debug \
	  -DBUILD_TESTING=ON \
	  -DLIBSCID_INSTALL=OFF \
	  -DLIBSCID_SANITISERS=address,undefined \
	  "-DLIBSCID_SOURCE_ROOT=$$($(1).__cmake.source.root)" \
	  $$(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$$(LIBSCID_CMAKE) \
	  --build $$($(1).__qc.dynamic-analysis.build.dir) \
	  $$(LIBSCID_CMAKE_BUILD_ARGS)
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
	  $$(LIBSCID_CTEST) \
	    --test-dir $$($(1).__qc.dynamic-analysis.build.dir) \
	    --output-on-failure

.PHONY : $(1).qc-dynamic-analysis

####################################################################################################

$(1).qc-all : $(1).qc-format $(1).qc-static-analysis $(1).qc-dynamic-analysis

.PHONY : $(1).qc-all

####################################################################################################
endef
