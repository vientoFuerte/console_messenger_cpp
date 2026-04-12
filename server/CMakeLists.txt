cmake_minimum_required(VERSION 3.12)

project(messenger_server)

option(WITH_BOOST_TEST "Whether to build Boost test" ON)
option(WITH_DOXYGEN "Build Doxygen documentation" OFF)

set(Boost_NO_BOOST_CMAKE ON)

# Ищем Boost для основной программы
find_package(Boost REQUIRED COMPONENTS filesystem)

# Ищем SQLite3
find_package(SQLite3 REQUIRED)

add_executable(messenger_server main.cpp server.cpp database.cpp)

set_target_properties(messenger_server PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(messenger_server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(messenger_server PRIVATE 
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	SQLite::SQLite3
)

# Boost.Test
if(WITH_BOOST_TEST AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
    # Для тестов нужен unit_test_framework
    find_package(Boost REQUIRED COMPONENTS unit_test_framework filesystem)
    
    add_executable(messenger_server_tests test.cpp server.cpp database.cpp)
    
    set_target_properties(messenger_server_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    target_include_directories(messenger_server_tests PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(messenger_server_tests PRIVATE ${Boost_LIBRARIES})
    
    target_compile_definitions(messenger_server_tests PRIVATE BOOST_TEST_NO_LIB)
    
    enable_testing()
    add_test(NAME AllTests COMMAND messenger_server_tests)
    
    message(STATUS "Boost.Test enabled")
endif()

if (MSVC)
    target_compile_options(messenger_server PRIVATE /W4)
else ()
    target_compile_options(messenger_server PRIVATE
        -Wall -Wextra -pedantic
    )
endif()

install(TARGETS messenger_server RUNTIME DESTINATION bin)

include(CPack)
