set( LIBSCID_SANITISERS "" CACHE STRING "Comma-separated sanitizer runtimes enabled for this build." )

if( NOT LIBSCID_SANITISERS )
    return()
endif()

if( NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" )
    message( FATAL_ERROR "LIBSCID_SANITISERS requires Clang or GCC." )
endif()

add_compile_options(
    "$<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=${LIBSCID_SANITISERS}>"
    "$<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer>" )
add_link_options( "-fsanitize=${LIBSCID_SANITISERS}" )
