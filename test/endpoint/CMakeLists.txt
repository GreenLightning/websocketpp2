if(OPENSSL_FOUND)
    websocketpp_add_test(test_endpoint endpoint.cpp)
    target_link_libraries(test_endpoint PRIVATE OpenSSL::SSL)
endif()
