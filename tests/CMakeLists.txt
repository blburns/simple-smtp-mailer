# Tests CMakeLists.txt for simple-smtp-mailer
# Using Google Test framework for comprehensive testing

# Find Google Test
# First try to find GTest using the standard method
find_package(GTest QUIET)

# If not found, try to find it manually
if(NOT GTest_FOUND)
    # Look for GTest libraries in common locations
    find_library(GTEST_LIBRARY
        NAMES gtest
        PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
        NO_DEFAULT_PATH
    )
    
    find_library(GTEST_MAIN_LIBRARY
        NAMES gtest_main
        PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
        NO_DEFAULT_PATH
    )
    
    # Look for GTest headers
    find_path(GTEST_INCLUDE_DIR
        NAMES gtest/gtest.h
        PATHS /usr/include /usr/local/include
        NO_DEFAULT_PATH
    )
    
    # Check if we found all required components
    if(GTEST_LIBRARY AND GTEST_MAIN_LIBRARY AND GTEST_INCLUDE_DIR)
        set(GTest_FOUND TRUE)
        set(GTEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})
        set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})
        
        # Create imported targets
        add_library(GTest::GTest UNKNOWN IMPORTED)
        set_target_properties(GTest::GTest PROPERTIES
            IMPORTED_LOCATION ${GTEST_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR}
        )
        
        add_library(GTest::Main UNKNOWN IMPORTED)
        set_target_properties(GTest::Main PROPERTIES
            IMPORTED_LOCATION ${GTEST_MAIN_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR}
        )
        
        message(STATUS "Found GTest: ${GTEST_LIBRARIES}")
        message(STATUS "GTest include dir: ${GTEST_INCLUDE_DIRS}")
    else()
        message(FATAL_ERROR "Could not find GTest libraries. Please install libgtest-dev package.")
    endif()
endif()

find_package(PkgConfig REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Test source files
set(TEST_SOURCES
    test_main.cpp
    test_basic.cpp
    test_api_client.cpp
    test_all_providers.cpp
    test_http_client.cpp
    test_mailgun_integration.cpp
    test_amazon_ses_integration.cpp
    test_sendgrid_integration.cpp
    test_postmark_integration.cpp
    test_sparkpost_integration.cpp
    test_mailjet_integration.cpp
    test_webhook_processing.cpp
    test_json_logging.cpp
    test_token_manager.cpp
)

# Create test executable
add_executable(simple-smtp-mailer-tests ${TEST_SOURCES})

# Link with main library and Google Test
target_link_libraries(simple-smtp-mailer-tests
    simple-smtp-mailer-lib
    GTest::GTest
    GTest::Main
    Threads::Threads
)

# Link with optional dependencies if enabled
if(ENABLE_SSL)
    target_link_libraries(simple-smtp-mailer-tests OpenSSL::SSL OpenSSL::Crypto)
endif()

if(ENABLE_JSON)
    if(ENABLE_STATIC_LINKING)
        find_library(JSONCPP_STATIC_LIB jsoncpp PATHS /opt/local/lib /usr/local/lib /usr/lib)
        if(JSONCPP_STATIC_LIB)
            target_link_libraries(simple-smtp-mailer-tests ${JSONCPP_STATIC_LIB})
        else()
            target_link_libraries(simple-smtp-mailer-tests ${JSONCPP_LIBRARIES})
        endif()
    else()
        target_link_libraries(simple-smtp-mailer-tests ${JSONCPP_LIBRARIES})
    endif()
    target_include_directories(simple-smtp-mailer-tests PRIVATE ${JSONCPP_INCLUDE_DIRS})
    target_compile_options(simple-smtp-mailer-tests PRIVATE ${JSONCPP_CFLAGS_OTHER})
    target_link_directories(simple-smtp-mailer-tests PRIVATE ${JSONCPP_LIBRARY_DIRS})
endif()

if(ENABLE_CURL)
    target_link_libraries(simple-smtp-mailer-tests ${CURL_LIBRARIES})
    target_include_directories(simple-smtp-mailer-tests PRIVATE ${CURL_INCLUDE_DIRS})
endif()

# Add tests to CTest
add_test(NAME simple-smtp-mailer-tests COMMAND simple-smtp-mailer-tests)

# Set test properties
set_tests_properties(simple-smtp-mailer-tests PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Enable test discovery
include(GoogleTest)
gtest_discover_tests(simple-smtp-mailer-tests)
