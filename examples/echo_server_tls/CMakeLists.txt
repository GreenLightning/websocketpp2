if(OPENSSL_FOUND)
    websocketpp_add_example(echo_server_tls echo_server_tls.cpp)
    target_link_libraries(echo_server_tls PRIVATE OpenSSL::SSL)
endif()
