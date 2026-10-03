
############ Setup project and cmake
# Minimum cmake requirement. We should require a quite recent
# cmake for the dependency find macros etc. to be up to date.
cmake_minimum_required (VERSION 3.10)

############ Paths

set (WEBSOCKETPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set (WEBSOCKETPP_INCLUDE ${WEBSOCKETPP_ROOT}/websocketpp)
set (WEBSOCKETPP_BUILD_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set (WEBSOCKETPP_BIN ${WEBSOCKETPP_BUILD_ROOT}/bin)
set (WEBSOCKETPP_LIB ${WEBSOCKETPP_BUILD_ROOT}/lib)

# CMake install step prefix. I assume linux users want the prefix to
# be the default /usr or /usr/local so this is only adjusted on Windows.
# This must be set prior to any call to project or it will not be read correctly.
# - Windows: Build the INSTALL project in your solution file.
# - Linux/OSX: make install.
if (WIN32)
    set (CMAKE_INSTALL_PREFIX "${WEBSOCKETPP_ROOT}/install" CACHE PATH "")
endif ()

############ Project name and version
set (WEBSOCKETPP_MAJOR_VERSION 2)
set (WEBSOCKETPP_MINOR_VERSION 0)
set (WEBSOCKETPP_PATCH_VERSION 0)
set (WEBSOCKETPP_VERSION ${WEBSOCKETPP_MAJOR_VERSION}.${WEBSOCKETPP_MINOR_VERSION}.${WEBSOCKETPP_PATCH_VERSION})

project (websocketpp VERSION ${WEBSOCKETPP_VERSION})

# Require at least C++11, while respecting a newer standard selected by users.
if (NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 11 OR CMAKE_CXX_STANDARD EQUAL 98)
    set (CMAKE_CXX_STANDARD 11)
endif ()
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(GNUInstallDirs)

set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
if (WIN32 AND NOT CYGWIN)
  set (DEF_INSTALL_CMAKE_DIR cmake)
else ()
  set (DEF_INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/websocketpp)
endif ()
set (INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# Set CMake library search policy
if (COMMAND cmake_policy)
    cmake_policy (SET CMP0003 NEW)
    cmake_policy (SET CMP0005 NEW)
endif ()

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo;Debug" CACHE STRING "Configurations" FORCE)

# Include our cmake macros
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include (CMakeHelpers)


############  Build customization

# Override from command line "CMake -D<OPTION>=TRUE/FALSE/0/1/ON/OFF"
option (BUILD_EXAMPLES "Build websocketpp examples." FALSE)
option (BUILD_TESTS "Build websocketpp tests." FALSE)

if (BUILD_TESTS OR BUILD_EXAMPLES)

    enable_testing ()

    ############ Compiler specific setup

    set (WEBSOCKETPP_PLATFORM_LIBS "")
    set (WEBSOCKETPP_PLATFORM_TLS_LIBS "")
    set (WEBSOCKETPP_BOOST_LIBS "")

    # Detect clang. Not officially reported by cmake.
    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-v" ERROR_VARIABLE CXX_VER_STDERR)
    if ("${CXX_VER_STDERR}" MATCHES ".*clang.*")
        set (CMAKE_COMPILER_IS_CLANGXX 1)
    endif ()

    # Visual studio
    if (MSVC)
        set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL /Gy /GF /Ox /Ob2 /Ot /Oi /MP /arch:SSE2 /fp:fast")
        set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
        add_definitions (/W3 /wd4996 /wd4995 /wd4355)
        add_definitions (-DUNICODE -D_UNICODE)
        add_definitions (-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
        add_definitions (-DNOMINMAX)
    endif ()

    # g++
    if (CMAKE_COMPILER_IS_GNUCXX)
        if (NOT APPLE AND NOT HAIKU)
            set (WEBSOCKETPP_PLATFORM_LIBS pthread rt)
        else()
        if (HAIKU)
            set (WEBSOCKETPP_PLATFORM_LIBS pthread network)
        else()
            set (WEBSOCKETPP_PLATFORM_LIBS pthread)
        endif()
        endif()
        set (WEBSOCKETPP_PLATFORM_TLS_LIBS ssl crypto)
        if (NOT APPLE)
            add_definitions (-DNDEBUG -Wall -Wcast-align) # todo: should we use CMAKE_C_FLAGS for these?
        endif ()
    endif ()

    # clang
    if (CMAKE_COMPILER_IS_CLANGXX)
        if (APPLE)
            set (WEBSOCKETPP_PLATFORM_LIBS pthread)
        elseif (HAIKU)
            set (WEBSOCKETPP_PLATFORM_LIBS pthread network)
        else()
            set (WEBSOCKETPP_PLATFORM_LIBS pthread rt)
        endif()
        set (WEBSOCKETPP_PLATFORM_TLS_LIBS ssl crypto)
        if (NOT APPLE)
            add_definitions (-DNDEBUG -Wall -Wno-padded) # todo: should we use CMAKE_C_FLAGS for these?
        endif ()
    endif ()

    # OSX, can override above.
    if (APPLE)
        add_definitions (-DNDEBUG -Wall)
    endif ()

    if (BUILD_TESTS)
        list (APPEND WEBSOCKETPP_BOOST_LIBS unit_test_framework)
    endif()

    ############ Dependencies

    # Set BOOST_ROOT env variable or pass with cmake -DBOOST_ROOT=path.
    # BOOST_ROOT can also be defined by a previous run from cmake cache.
    # Preserve the legacy environment override for the C++11 Boost build.
    if (NOT "$ENV{BOOST_ROOT_CPP11}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{BOOST_ROOT_CPP11}" BOOST_ROOT)
        set (BOOST_ROOT ${BOOST_ROOT} CACHE PATH "BOOST_ROOT dependency path" FORCE)
    endif ()
    if ("${BOOST_ROOT}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{BOOST_ROOT}" BOOST_ROOT)
        # Cache BOOST_ROOT for runs that do not define $ENV{BOOST_ROOT}.
        set (BOOST_ROOT ${BOOST_ROOT} CACHE PATH "BOOST_ROOT dependency path" FORCE)
    endif ()

    message ("* Configuring Boost")
    message (STATUS "-- Using BOOST_ROOT")
    message (STATUS "       " ${BOOST_ROOT})

    if (MSVC)
        set (Boost_USE_MULTITHREADED TRUE)
        set (Boost_USE_STATIC_LIBS TRUE)
    else ()
        set (Boost_USE_MULTITHREADED FALSE)
        set (Boost_USE_STATIC_LIBS FALSE)
    endif ()

	if (BOOST_STATIC)
		set (Boost_USE_STATIC_LIBS TRUE)
	endif ()

    if (NOT Boost_USE_STATIC_LIBS)
        add_definitions (-DBOOST_TEST_DYN_LINK)
    endif ()

    set (Boost_FIND_REQUIRED TRUE)
    set (Boost_FIND_QUIETLY TRUE)
    set (Boost_DEBUG FALSE)
    set (Boost_USE_MULTITHREADED TRUE)
    find_package (Boost 1.49.0 COMPONENTS ${WEBSOCKETPP_BOOST_LIBS})
    # FindBoost before CMake 3.15 provides the version as separate fields.
    if (NOT DEFINED Boost_VERSION_STRING)
        set (Boost_VERSION_STRING "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
    endif ()
    # Boost.System became header-only in Boost 1.69.
    if (Boost_VERSION_STRING VERSION_LESS "1.69.0")
        list (APPEND WEBSOCKETPP_BOOST_LIBS system)
        find_package (Boost 1.49.0 COMPONENTS ${WEBSOCKETPP_BOOST_LIBS})
    endif ()

    if (Boost_FOUND)
        # Boost is a project wide global dependency.
        include_directories (${Boost_INCLUDE_DIRS})
        link_directories (${Boost_LIBRARY_DIRS})

        # Pretty print status
        message (STATUS "-- Include Directories")
        foreach (include_dir ${Boost_INCLUDE_DIRS})
            message (STATUS "       " ${include_dir})
        endforeach ()
        message (STATUS "-- Library Directories")
        foreach (library_dir ${Boost_LIBRARY_DIRS})
            message (STATUS "       " ${library_dir})
        endforeach ()
        message (STATUS "-- Libraries")
        foreach (boost_lib ${Boost_LIBRARIES})
            message (STATUS "       " ${boost_lib})
        endforeach ()
        message ("")
    else ()
        message (FATAL_ERROR "Failed to find required dependency: boost")
    endif ()

    find_package(OpenSSL)
    find_package(ZLIB)
endif()

############ Add projects

# Add main library
add_subdirectory (websocketpp)

# Add examples
if (BUILD_EXAMPLES)
    include_subdirs ("examples")
endif ()

# Add tests
if (BUILD_TESTS)
    include_subdirs ("test")
endif ()

print_used_build_config()

export (PACKAGE websocketpp)

include(CMakePackageConfigHelpers)
configure_package_config_file(websocketpp-config.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/websocketpp-config.cmake"
  PATH_VARS INSTALL_INCLUDE_DIR
  INSTALL_DESTINATION "${INSTALL_CMAKE_DIR}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
write_basic_package_version_file("${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/websocketpp-configVersion.cmake"
  VERSION ${WEBSOCKETPP_VERSION}
  COMPATIBILITY ExactVersion)

# Install the websocketpp-config.cmake and websocketpp-configVersion.cmake
install (FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/websocketpp-config.cmake"
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/websocketpp-configVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

