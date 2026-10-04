if(OPENSSL_FOUND)
    websocketpp_add_example(echo_server_both echo_server_both.cpp)
    target_link_libraries(echo_server_both PRIVATE OpenSSL::SSL)
endif()
