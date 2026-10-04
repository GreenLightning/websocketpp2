# Exercise both error backends without requiring standalone Asio headers.
# An ASIO_STANDALONE build defines the macro globally, so only the standalone
# variant is meaningful there.
set(error_backends standalone)
if(NOT ASIO_STANDALONE)
    list(APPEND error_backends boost)
endif()
foreach(backend IN LISTS error_backends)
    websocketpp_add_test(test_common_${backend} NO_BOOST_TEST compatibility.cpp)
    target_compile_definitions(test_common_${backend} PRIVATE
        _WEBSOCKETPP_NO_CPP11_MEMORY_
        _WEBSOCKETPP_NO_CPP11_FUNCTIONAL_
        _WEBSOCKETPP_NO_CPP11_THREAD_
        _WEBSOCKETPP_NO_CPP11_CHRONO_
        _WEBSOCKETPP_NO_CPP11_RANDOM_DEVICE_
        _WEBSOCKETPP_NO_CPP11_REGEX_
        _WEBSOCKETPP_NO_CPP11_TYPE_TRAITS_
        _WEBSOCKETPP_NO_CPP11_SYSTEM_ERROR_
        _WEBSOCKETPP_CPP11_SYSTEM_ERROR_)
    if(backend STREQUAL standalone)
        target_compile_definitions(test_common_${backend} PRIVATE ASIO_STANDALONE)
    endif()
endforeach()

websocketpp_add_test(test_common_network network.cpp)
