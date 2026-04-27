
#define BOOST_TEST_MODULE ClientTests
#include <boost/test/included/unit_test.hpp>

#include "client.h"


// Тест распознавания команды выхода
BOOST_AUTO_TEST_CASE(test_is_quit) {
 
    //  Должны распознаваться как выход
    BOOST_CHECK(is_quit("quit"));
    BOOST_CHECK(is_quit("QUIT"));
    BOOST_CHECK(is_quit("Quit"));
    BOOST_CHECK(is_quit("  quit  "));
    BOOST_CHECK(is_quit("q"));
    BOOST_CHECK(is_quit("Q"));
    BOOST_CHECK(is_quit("  q  "));
    
    // Не должны распознаваться как выход
    BOOST_CHECK(!is_quit("hello"));
    BOOST_CHECK(!is_quit("quite"));
    BOOST_CHECK(!is_quit("quit123"));

    BOOST_CHECK(!is_quit("   "));
    BOOST_CHECK(!is_quit("@quit"));
    BOOST_CHECK(!is_quit("/quit"));
}
