define libscid.cmake.__project.rules
####################################################################################################

$(1).configure :
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

$(1).clean :
	-rm -rf $$($(1).__build.dir)

.PHONY : $(1).clean

####################################################################################################

$(1).qc-format :
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

$(1).__qc-cppcheck :
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

$(1).__qc-tidy :
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

$(1).qc-dynamic-analysis :
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
