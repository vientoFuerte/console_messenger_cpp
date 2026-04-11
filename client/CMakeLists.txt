cmake_minimum_required(VERSION 3.12)

project(messenger_client)

option(WITH_BOOST_TEST "Whether to build Boost test" ON)
option(WITH_DOXYGEN "Build Doxygen documentation" OFF)

set(Boost_NO_BOOST_CMAKE ON)

# Ищем Boost для основной программы
find_package(Boost REQUIRED COMPONENTS filesystem)

add_executable(messenger_client main.cpp)

set_target_properties(messenger_client PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(messenger_client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(messenger_client PRIVATE 
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_REGEX_LIBRARY}
)

# Boost.Test
if(WITH_BOOST_TEST AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
    # Для тестов нужен unit_test_framework
    find_package(Boost REQUIRED COMPONENTS unit_test_framework filesystem)
    
    add_executable(messenger_client_tests test.cpp)
    
    set_target_properties(messenger_client_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    target_include_directories(messenger_client_tests PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(messenger_client_tests PRIVATE ${Boost_LIBRARIES})
    
    target_compile_definitions(messenger_client_tests PRIVATE BOOST_TEST_NO_LIB)
    
    enable_testing()
    add_test(NAME AllTests COMMAND messenger_client_tests)
    
    message(STATUS "Boost.Test enabled")
endif()

if (MSVC)
    target_compile_options(messenger_client PRIVATE /W4)
else ()
    target_compile_options(messenger_client PRIVATE
        -Wall -Wextra -pedantic
    )
endif()

install(TARGETS messenger_client RUNTIME DESTINATION bin)

include(CPack)
