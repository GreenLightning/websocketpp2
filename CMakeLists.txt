cmake_minimum_required(VERSION 3.18)
project(websocketpp VERSION 2.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BUILD_EXAMPLES "Build websocketpp examples." OFF)
option(BUILD_TESTS "Build websocketpp tests." OFF)

set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}" CACHE STRING
    "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
    set(INSTALL_CMAKE_DIR cmake CACHE STRING "Installation directory for CMake files")
else()
    set(INSTALL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/websocketpp" CACHE STRING
        "Installation directory for CMake files")
endif()

add_library(websocketpp INTERFACE)
add_library(websocketpp::websocketpp ALIAS websocketpp)
target_include_directories(websocketpp INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>")
target_compile_features(websocketpp INTERFACE cxx_std_11)

install(DIRECTORY websocketpp DESTINATION "${INSTALL_INCLUDE_DIR}"
    COMPONENT dev FILES_MATCHING PATTERN "*.hpp*")
install(TARGETS websocketpp EXPORT websocketpp-targets COMPONENT dev)
install(EXPORT websocketpp-targets NAMESPACE websocketpp::
    DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

configure_package_config_file(websocketpp-config.cmake.in
    "${PROJECT_BINARY_DIR}/CMakeFiles/websocketpp-config.cmake"
    PATH_VARS INSTALL_INCLUDE_DIR
    INSTALL_DESTINATION "${INSTALL_CMAKE_DIR}"
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/CMakeFiles/websocketpp-configVersion.cmake"
    VERSION "${PROJECT_VERSION}" COMPATIBILITY ExactVersion)
install(FILES
    "${PROJECT_BINARY_DIR}/CMakeFiles/websocketpp-config.cmake"
    "${PROJECT_BINARY_DIR}/CMakeFiles/websocketpp-configVersion.cmake"
    DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

if(BUILD_EXAMPLES OR BUILD_TESTS)
    if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    endif()

    option(BOOST_STATIC "Link Boost libraries statically." ${MSVC})
    if(BOOST_STATIC)
        set(Boost_USE_STATIC_LIBS ON)
    endif()
    # Preserve the legacy dependency override without changing the cache.
    if(NOT "$ENV{BOOST_ROOT_CPP11}" STREQUAL "")
        file(TO_CMAKE_PATH "$ENV{BOOST_ROOT_CPP11}" BOOST_ROOT)
    endif()
    set(Boost_USE_MULTITHREADED ON)
    set(boost_components)
    if(BUILD_TESTS)
        list(APPEND boost_components unit_test_framework)
    endif()
    find_package(Boost 1.66 REQUIRED COMPONENTS ${boost_components})
    # Boost.System became header-only in Boost 1.69.
    if(Boost_VERSION_STRING VERSION_LESS "1.69.0")
        list(APPEND boost_components system)
        find_package(Boost 1.66 REQUIRED COMPONENTS ${boost_components})
    endif()

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    if(BUILD_TESTS)
        # A test build always includes the TLS and compression suites.
        find_package(OpenSSL REQUIRED)
        find_package(ZLIB REQUIRED)
    else()
        find_package(OpenSSL QUIET)
        find_package(ZLIB QUIET)
    endif()

    # Settings for this project's executables, not for library consumers.
    add_library(websocketpp_build_options INTERFACE)
    target_link_libraries(websocketpp_build_options INTERFACE
        Boost::headers Threads::Threads)
    if(Boost_VERSION_STRING VERSION_LESS "1.69.0")
        target_link_libraries(websocketpp_build_options INTERFACE Boost::system)
    endif()
    if(CMAKE_SYSTEM_NAME STREQUAL "Haiku")
        target_link_libraries(websocketpp_build_options INTERFACE network)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(websocketpp_build_options INTERFACE rt)
    endif()

    if(MSVC)
        target_compile_options(websocketpp_build_options INTERFACE
            /W3 /wd4996 /wd4995 /wd4355
            "$<$<CONFIG:Release>:/GL;/Gy;/GF;/Ox;/Ob2;/Ot;/Oi;/MP;/arch:SSE2;/fp:fast>")
        target_link_options(websocketpp_build_options INTERFACE
            "$<$<CONFIG:Release>:/LTCG;/INCREMENTAL:NO;/OPT:REF;/OPT:ICF>")
        target_compile_definitions(websocketpp_build_options INTERFACE
            UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS NOMINMAX)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(websocketpp_build_options INTERFACE
            -Wall -Wcast-align -Wshadow -Wunused-parameter)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang)$")
        target_compile_options(websocketpp_build_options INTERFACE
            -Wall -Wno-padded -Wshadow -Wunused-parameter
            -Wsometimes-uninitialized -Wuninitialized)
    endif()

    function(websocketpp_add_executable name)
        add_executable(${name} ${ARGN})
        target_link_libraries(${name} PRIVATE
            websocketpp::websocketpp websocketpp_build_options)
        if(NOT CMAKE_CXX_STANDARD)
            set_property(TARGET ${name} PROPERTY CXX_STANDARD 11)
        endif()
        set_target_properties(${name} PROPERTIES
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
            DEBUG_POSTFIX d)
    endfunction()
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
