ifndef __libscid_etc_make_cmake.mk
__libscid_etc_make_cmake.mk := 1

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
#   `libscid.capi.static`, which should generate its own targets but inherit source-directory,
#   build-targets, and test-labels from `libscid.capi`.
#
# Required project variables:
#   PROJECT.__cmake.build.targets (inherited from BASE_PROJECT if omitted)
#
# Optional project variables with defaults:
#   PROJECT.__component                      Derived from PROJECT (e.g. capi, internal)
#   PROJECT.__cmake.profile                  BASE_PROJECT.__cmake.profile or $(LIBSCID_PROFILE)
#   PROJECT.__cmake.linkage                  BASE_PROJECT.__cmake.linkage or $(LIBSCID_LINKAGE_TYPE)
#   PROJECT.__build.dir                      $(LIBSCID_STAGING_BUILD_DIR)$(PROJECT.__component)/$(PROJECT.__cmake.profile)/$(PROJECT.__cmake.linkage)/
#   PROJECT.__install.dir                    $(LIBSCID_STAGING_INSTALL_DIR)$(PROJECT.__component)/$(PROJECT.__cmake.profile)/
#   PROJECT.__cmake.source.root              $(ROOT)
#   PROJECT.__cmake.source.dir               BASE_PROJECT.__project.dir
#   PROJECT.__cmake.install                  ON
#   PROJECT.__qc.format.build.dir            $(LIBSCID_STAGING_BUILD_DIR)$(PROJECT.__component)/qc/format/
#   PROJECT.__qc.analysis.build.dir          $(LIBSCID_STAGING_BUILD_DIR)$(PROJECT.__component)/qc/analysis/
#   PROJECT.__qc.dynamic-analysis.build.dir  $(LIBSCID_STAGING_BUILD_DIR)$(PROJECT.__component)/qc/sanitisers/
#
# Optional project variables without defaults:
#   PROJECT.__cmake.test.labels              (inherited from BASE_PROJECT if omitted)
####################################################################################################

libscid.__asan.detect_leaks := $(if $(filter Darwin,$(libscid.__host.system)),0,1)

define libscid.cmake.__project.rules
####################################################################################################

$(1).__component ?= $(if $(2),$$($(2).__component),$(word 2,$(subst ., ,$(1))))
$(1).__cmake.profile ?= $(if $(2),$$($(2).__cmake.profile),$$(LIBSCID_PROFILE))
$(1).__cmake.build.targets ?= $$($(2).__cmake.build.targets)
$(1).__cmake.test.labels ?= $$($(2).__cmake.test.labels)
$(1).__cmake.linkage ?= $(if $(2),$$($(2).__cmake.linkage),$$(LIBSCID_LINKAGE_TYPE))
$(1).__cmake.install ?= $$($(2).__cmake.install)

$(1).__build.dir ?= $$(LIBSCID_STAGING_BUILD_DIR)$$($(1).__component)/$$($(1).__cmake.profile)/$$($(1).__cmake.linkage)/
$(1).__install.dir ?= $$(LIBSCID_STAGING_INSTALL_DIR)$$($(1).__component)/$$($(1).__cmake.profile)/
$(1).__qc.format.build.dir ?= $$(LIBSCID_STAGING_BUILD_DIR)$$($(1).__component)/qc/format/
$(1).__qc.analysis.build.dir ?= $$(LIBSCID_STAGING_BUILD_DIR)$$($(1).__component)/qc/analysis/
$(1).__qc.dynamic-analysis.build.dir ?= $$(LIBSCID_STAGING_BUILD_DIR)$$($(1).__component)/qc/sanitisers/

$(1).__cmake.contract : \
  bmakelib.default-if-blank( $(1).__component,$$(word 2,$$(subst ., ,$(1))) ) \
  bmakelib.default-if-blank( $(1).__cmake.profile,$$(LIBSCID_PROFILE) ) \
  bmakelib.enum.error-unless-member( LIBSCID_PROFILE,$(1).__cmake.profile ) \
  bmakelib.default-if-blank( $(1).__cmake.linkage,$$(LIBSCID_LINKAGE_TYPE) ) \
  bmakelib.enum.error-unless-member( LIBSCID_LINKAGE_TYPE,$(1).__cmake.linkage ) \
  bmakelib.default-if-blank( $(1).__build.dir,$$(call libscid.__make.word.escape,$$(LIBSCID_STAGING_BUILD_DIR)$$($(1).__component)/$$($(1).__cmake.profile)/$$($(1).__cmake.linkage)/) ) \
  bmakelib.default-if-blank( $(1).__install.dir,$$(call libscid.__make.word.escape,$$(LIBSCID_STAGING_INSTALL_DIR)$$($(1).__component)/$$($(1).__cmake.profile)/) ) \
  bmakelib.default-if-blank( $(1).__cmake.source.root,$$(call libscid.__make.word.escape,$$(ROOT)) ) \
  bmakelib.default-if-blank( $(1).__cmake.source.dir,$$(call libscid.__make.word.escape,$$($(or $(2),$(1)).__project.dir)) ) \
  bmakelib.default-if-blank( $(1).__cmake.install,ON ) \
  bmakelib.default-if-blank( $(1).__qc.format.build.dir,$$(call libscid.__make.word.escape,$$(LIBSCID_STAGING_BUILD_DIR)$$($(1).__component)/qc/format/) ) \
  bmakelib.default-if-blank( $(1).__qc.analysis.build.dir,$$(call libscid.__make.word.escape,$$(LIBSCID_STAGING_BUILD_DIR)$$($(1).__component)/qc/analysis/) ) \
  bmakelib.default-if-blank( $(1).__qc.dynamic-analysis.build.dir,$$(call libscid.__make.word.escape,$$(LIBSCID_STAGING_BUILD_DIR)$$($(1).__component)/qc/sanitisers/) ) \
  .WAIT \
  bmakelib.error-if-blank( \
    $(1).__component \
    $(1).__cmake.profile \
    $(1).__build.dir \
    $(1).__install.dir \
    $(1).__cmake.source.root \
    $(1).__cmake.source.dir \
    $(1).__cmake.build.targets \
    $(1).__cmake.linkage \
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
	    -DBUILD_SHARED_LIBS=$$(if $$(filter static,$$($(1).__cmake.linkage)),OFF,ON) \
	    -DLIBSCID_INSTALL=$$($(1).__cmake.install) \
	    "-DLIBSCID_SOURCE_ROOT=$$($(1).__cmake.source.root)" \
	    -DCMAKE_BUILD_TYPE=$$(LIBSCID_CMAKE_BUILD_TYPE) \
	    $$(libscid.cmake.__c.compiler.arg) \
	    $$(libscid.cmake.__cxx.compiler.arg) \
	    $$(libscid.cmake.__osx_sysroot.arg) \
	    $$(LIBSCID_CMAKE_CONFIGURE_ARGS)

.PHONY : $(1).configure

####################################################################################################

$(1).build : $(1).configure
	$$(LIBSCID_CMAKE) \
	    --build $$($(1).__build.dir) \
	    --config $$(LIBSCID_CMAKE_BUILD_TYPE) \
	    $$(foreach target,$$($(1).__cmake.build.targets),--target $$(target)) \
	    $$(LIBSCID_CMAKE_BUILD_ARGS)
	$$(if $$(filter ON,$$($(1).__cmake.install)),\
	    $$(LIBSCID_CMAKE) \
	        --install $$($(1).__build.dir) \
	        --config $$(LIBSCID_CMAKE_BUILD_TYPE) \
	        --prefix "$$($(1).__install.dir)")

.PHONY : $(1).build

####################################################################################################

$(1).test : $(1).build
	$$(LIBSCID_CTEST) \
	    --test-dir $$($(1).__build.dir) \
	    -C $$(LIBSCID_CMAKE_BUILD_TYPE) \
	    $$(if $$($(1).__cmake.test.labels),-L '$$($(1).__cmake.test.labels)') \
	    --timeout $$(LIBSCID_CTEST_TIMEOUT) \
	    --output-on-failure

.PHONY : $(1).test

####################################################################################################

$(1).install : $(1).build
	$$(if $$(filter ON,$$($(1).__cmake.install)),\
	    $$(if $$(PREFIX),,\
	        $$(error PREFIX must be set to install $(1))) \
	    mkdir -p "$$(DESTDIR)$$(PREFIX)" && \
	    cp -R "$$($(1).__install.dir)". "$$(DESTDIR)$$(PREFIX)")

.PHONY : $(1).install

####################################################################################################

$(1).clean : $(1).__cmake.contract
	-rm -rf $$($(1).__build.dir)
	-rm -rf $$($(1).__install.dir)
	-rm -rf $$($(1).__qc.format.build.dir)
	-rm -rf $$($(1).__qc.analysis.build.dir)
	-rm -rf $$($(1).__qc.dynamic-analysis.build.dir)

.PHONY : $(1).clean

####################################################################################################

$(1).qc-format : $(1).__cmake.contract
	$$(LIBSCID_CMAKE) \
	    -S $$($(1).__cmake.source.dir) \
	    -B $$($(1).__qc.format.build.dir) \
	    $$(libscid.cmake.__generator.arg) \
	    $$(libscid.cmake.__c.compiler.arg) \
	    $$(libscid.cmake.__cxx.compiler.arg) \
	    $$(libscid.cmake.__osx_sysroot.arg) \
	    -DCMAKE_BUILD_TYPE=Debug \
	    -DBUILD_TESTING=OFF \
	    -DLIBSCID_INSTALL=OFF \
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
	    $$(libscid.cmake.__osx_sysroot.arg) \
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
	    $$(libscid.cmake.__osx_sysroot.arg) \
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
	    $$(libscid.cmake.__osx_sysroot.arg) \
	    -DCMAKE_BUILD_TYPE=Debug \
	    -DBUILD_TESTING=ON \
	    -DLIBSCID_INSTALL=OFF \
	    -DLIBSCID_SANITISERS=address,undefined \
	    "-DLIBSCID_SOURCE_ROOT=$$($(1).__cmake.source.root)" \
	    $$(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$$(LIBSCID_CMAKE) \
	    --build $$($(1).__qc.dynamic-analysis.build.dir) \
	    $$(LIBSCID_CMAKE_BUILD_ARGS)
	ASAN_OPTIONS=detect_leaks=$(libscid.__asan.detect_leaks):halt_on_error=1 \
	UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
	    $$(LIBSCID_CTEST) \
	        --test-dir $$($(1).__qc.dynamic-analysis.build.dir) \
	        --timeout $$(LIBSCID_CTEST_TIMEOUT) \
	        --output-on-failure

.PHONY : $(1).qc-dynamic-analysis

####################################################################################################

$(1).qc-all : $(1).qc-format $(1).qc-static-analysis $(1).qc-dynamic-analysis

.PHONY : $(1).qc-all

####################################################################################################
endef

endif
