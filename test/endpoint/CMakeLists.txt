websocketpp_add_test(test_endpoint endpoint.cpp)

websocketpp_add_test(test_endpoint_tls tls.cpp)
target_link_libraries(test_endpoint_tls PRIVATE OpenSSL::SSL)
set_tests_properties(test_endpoint_tls PROPERTIES LABELS tls)
