if(ZLIB_FOUND)
    websocketpp_add_example(testee_server testee_server.cpp)
    target_link_libraries(testee_server PRIVATE ZLIB::ZLIB)
endif()
