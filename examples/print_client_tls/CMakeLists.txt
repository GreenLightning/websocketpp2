if(OPENSSL_FOUND)
    websocketpp_add_example(print_client_tls print_client_tls.cpp)
    target_link_libraries(print_client_tls PRIVATE OpenSSL::SSL)
endif()
