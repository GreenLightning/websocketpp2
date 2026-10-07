websocketpp_add_test(test_endpoint endpoint.cpp)
# Show which case was running if a platform-specific hang hits the CI timeout.
set_tests_properties(test_endpoint PROPERTIES
    ENVIRONMENT "BOOST_TEST_LOG_LEVEL=test_suite")

websocketpp_add_test(test_endpoint_tls tls.cpp)
target_link_libraries(test_endpoint_tls PRIVATE OpenSSL::SSL)
set_tests_properties(test_endpoint_tls PROPERTIES LABELS tls)
