# Exercise both error backends without requiring standalone Asio headers.
foreach (backend boost standalone)
    init_target (test_common_${backend})
    build_test (${TARGET_NAME} compatibility.cpp)
    target_compile_definitions (${TARGET_NAME} PRIVATE
        _WEBSOCKETPP_NO_CPP11_MEMORY_
        _WEBSOCKETPP_NO_CPP11_FUNCTIONAL_
        _WEBSOCKETPP_NO_CPP11_THREAD_
        _WEBSOCKETPP_NO_CPP11_CHRONO_
        _WEBSOCKETPP_NO_CPP11_RANDOM_DEVICE_
        _WEBSOCKETPP_NO_CPP11_REGEX_
        _WEBSOCKETPP_NO_CPP11_TYPE_TRAITS_
        _WEBSOCKETPP_NO_CPP11_SYSTEM_ERROR_
        _WEBSOCKETPP_CPP11_SYSTEM_ERROR_)
    if (backend STREQUAL standalone)
        target_compile_definitions (${TARGET_NAME} PRIVATE ASIO_STANDALONE)
    endif ()
    if (backend STREQUAL boost AND Boost_VERSION_STRING VERSION_LESS "1.69.0")
        target_link_libraries (${TARGET_NAME} ${Boost_SYSTEM_LIBRARY})
    endif ()
    set_target_properties (${TARGET_NAME} PROPERTIES FOLDER "test")
endforeach ()
