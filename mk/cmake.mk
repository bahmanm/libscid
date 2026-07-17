LIBSCID_BUILD_DIR ?= $(LIBSCID_BUILD_ROOT)$(LIBSCID_PROJECT_NAME)/
LIBSCID_CMAKE_BUILD_TARGETS ?=
LIBSCID_CMAKE_TEST_LABELS ?=

LIBSCID_QC_FORMAT_BUILD_DIR ?= $(LIBSCID_BUILD_ROOT)format/
LIBSCID_QC_ANALYSIS_PRESET ?= analysis
LIBSCID_QC_SANITISERS_PRESET ?= sanitisers

####################################################################################################

configure :
	$(LIBSCID_CMAKE) \
	  -S $(LIBSCID_ROOT) \
	  -B $(LIBSCID_BUILD_DIR) \
	  $(libscid.cmake.generator.arg) \
	  -DBUILD_TESTING=ON \
	  $(libscid.cmake.shared.libs.arg) \
	  -DLIBSCID_INSTALL=OFF \
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
	  -S $(LIBSCID_ROOT) \
	  -B $(LIBSCID_QC_FORMAT_BUILD_DIR) \
	  $(libscid.cmake.generator.arg) \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build $(LIBSCID_QC_FORMAT_BUILD_DIR) \
	  --target format-check \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : qc-format

####################################################################################################

libscid.cmake.qc-cppcheck :
	$(LIBSCID_CMAKE) \
	  --preset $(LIBSCID_QC_ANALYSIS_PRESET) \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build \
	  --preset cppcheck \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : libscid.cmake.qc-cppcheck

####################################################################################################

libscid.cmake.qc-tidy :
	$(LIBSCID_CMAKE) \
	  --preset $(LIBSCID_QC_ANALYSIS_PRESET) \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build \
	  --preset clang-tidy \
	  $(LIBSCID_CMAKE_BUILD_ARGS)

.PHONY : libscid.cmake.qc-tidy

####################################################################################################

qc-static-analysis : libscid.cmake.qc-cppcheck libscid.cmake.qc-tidy

.PHONY : qc-static-analysis

####################################################################################################

qc-dynamic-analysis :
	$(LIBSCID_CMAKE) \
	  --preset $(LIBSCID_QC_SANITISERS_PRESET) \
	  $(libscid.cmake.c.compiler.arg) \
	  $(libscid.cmake.cxx.compiler.arg) \
	  $(LIBSCID_CMAKE_CONFIGURE_ARGS)
	$(LIBSCID_CMAKE) \
	  --build \
	  --preset $(LIBSCID_QC_SANITISERS_PRESET) \
	  $(LIBSCID_CMAKE_BUILD_ARGS)
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
	  $(LIBSCID_CTEST) \
	    --test-dir $(LIBSCID_BUILD_ROOT)$(LIBSCID_QC_SANITISERS_PRESET) \
	    --output-on-failure

.PHONY : qc-dynamic-analysis

####################################################################################################

qc-all : qc-format qc-static-analysis qc-dynamic-analysis

.PHONY : qc-all

####################################################################################################
