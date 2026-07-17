LIBSCID_BUILD_DIR ?= $(LIBSCID_BUILD_ROOT)
LIBSCID_CMAKE_SOURCE_ROOT ?= $(LIBSCID_ROOT)
LIBSCID_CMAKE_SOURCE_DIR ?= $(libscid.project.dir)
LIBSCID_CMAKE_BUILD_TARGETS ?=
LIBSCID_CMAKE_TEST_LABELS ?=

LIBSCID_QC_FORMAT_BUILD_DIR ?= $(LIBSCID_BUILD_ROOT)format/
LIBSCID_QC_ANALYSIS_BUILD_DIR ?= $(LIBSCID_BUILD_ROOT)analysis/
LIBSCID_QC_DYNAMIC_ANALYSIS_BUILD_DIR ?= $(LIBSCID_BUILD_ROOT)sanitisers/

####################################################################################################

configure :
	$(LIBSCID_CMAKE) \
	  -S $(LIBSCID_CMAKE_SOURCE_DIR) \
	  -B $(LIBSCID_BUILD_DIR) \
	  $(libscid.cmake.generator.arg) \
	  -DBUILD_TESTING=ON \
	  $(libscid.cmake.shared.libs.arg) \
	  -DLIBSCID_INSTALL=OFF \
	  "-DLIBSCID_SOURCE_ROOT=$(LIBSCID_CMAKE_SOURCE_ROOT)" \
	  -DCMAKE_BUILD_TYPE=$(LIBSCID_CMAKE_BUILD_TYPE) \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)

.PHONY : configure

####################################################################################################

build : configure
	$(LIBSCID_CMAKE) \
	  --build $(LIBSCID_BUILD_DIR) \
	  --config $(LIBSCID_CMAKE_BUILD_TYPE) \
	  $(foreach target,$(LIBSCID_CMAKE_BUILD_TARGETS),--target $(target)) \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : build

####################################################################################################

test : build
	$(LIBSCID_CTEST) \
	  --test-dir $(LIBSCID_BUILD_DIR) \
	  -C $(LIBSCID_CMAKE_BUILD_TYPE) \
	  $(if $(LIBSCID_CMAKE_TEST_LABELS),-L '$(LIBSCID_CMAKE_TEST_LABELS)') \
	  --output-on-failure

.PHONY : test

####################################################################################################

clean :
	-rm -rf $(LIBSCID_BUILD_DIR)

.PHONY : clean

####################################################################################################

qc-format :
	$(LIBSCID_CMAKE) \
	  -S $(LIBSCID_CMAKE_SOURCE_DIR) \
	  -B $(LIBSCID_QC_FORMAT_BUILD_DIR) \
	  $(libscid.cmake.generator.arg) \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  "-DLIBSCID_SOURCE_ROOT=$(LIBSCID_CMAKE_SOURCE_ROOT)" \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build $(LIBSCID_QC_FORMAT_BUILD_DIR) \
	  --target format-check \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : qc-format

####################################################################################################

libscid.cmake.qc-cppcheck :
	$(LIBSCID_CMAKE) \
	  -S $(LIBSCID_CMAKE_SOURCE_DIR) \
	  -B $(LIBSCID_QC_ANALYSIS_BUILD_DIR) \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  -DCMAKE_BUILD_TYPE=Debug \
	  -DBUILD_TESTING=OFF \
	  -DLIBSCID_INSTALL=OFF \
	  "-DLIBSCID_SOURCE_ROOT=$(LIBSCID_CMAKE_SOURCE_ROOT)" \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build $(LIBSCID_QC_ANALYSIS_BUILD_DIR) \
	  --target cppcheck \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : libscid.cmake.qc-cppcheck

####################################################################################################

libscid.cmake.qc-tidy :
	$(LIBSCID_CMAKE) \
	  -S $(LIBSCID_CMAKE_SOURCE_DIR) \
	  -B $(LIBSCID_QC_ANALYSIS_BUILD_DIR) \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  -DCMAKE_BUILD_TYPE=Debug \
	  -DBUILD_TESTING=OFF \
	  -DLIBSCID_INSTALL=OFF \
	  "-DLIBSCID_SOURCE_ROOT=$(LIBSCID_CMAKE_SOURCE_ROOT)" \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build $(LIBSCID_QC_ANALYSIS_BUILD_DIR) \
	  --target clang-tidy \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : libscid.cmake.qc-tidy

####################################################################################################

qc-static-analysis : libscid.cmake.qc-cppcheck libscid.cmake.qc-tidy

.PHONY : qc-static-analysis

####################################################################################################

qc-dynamic-analysis :
	$(LIBSCID_CMAKE) \
	  -S $(LIBSCID_CMAKE_SOURCE_DIR) \
	  -B $(LIBSCID_QC_DYNAMIC_ANALYSIS_BUILD_DIR) \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  -DCMAKE_BUILD_TYPE=Debug \
	  -DBUILD_TESTING=ON \
	  -DLIBSCID_INSTALL=OFF \
	  -DLIBSCID_SANITISERS=address,undefined \
	  "-DLIBSCID_SOURCE_ROOT=$(LIBSCID_CMAKE_SOURCE_ROOT)" \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build $(LIBSCID_QC_DYNAMIC_ANALYSIS_BUILD_DIR) \
	  $(LIBSCID_CMAKE_BUILD_ARGS)
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
	  $(LIBSCID_CTEST) \
	    --test-dir $(LIBSCID_QC_DYNAMIC_ANALYSIS_BUILD_DIR) \
	    --output-on-failure

.PHONY : qc-dynamic-analysis

####################################################################################################

qc-all : qc-format qc-static-analysis qc-dynamic-analysis

.PHONY : qc-all

####################################################################################################
