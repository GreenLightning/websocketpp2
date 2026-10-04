if(ZLIB_FOUND)
    websocketpp_add_example(testee_client testee_client.cpp)
    target_link_libraries(testee_client PRIVATE ZLIB::ZLIB)
endif()
