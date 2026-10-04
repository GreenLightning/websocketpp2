if(OPENSSL_FOUND)
    websocketpp_add_example(debug_client debug_client.cpp)
    target_link_libraries(debug_client PRIVATE OpenSSL::SSL)
endif()
