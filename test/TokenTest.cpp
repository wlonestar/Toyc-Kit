#include <boost/test/tools/old/interface.hpp>
#define BOOST_TEST_MODULE TokenTest
#include <boost/test/unit_test.hpp>

#include <Token.h>

using namespace toyc;

BOOST_AUTO_TEST_CASE(simple_test_1) {
  for (int i = AUTO; i != _EOF; i++) {
    std::cout << TokenTypeTable[(TokenType)i] << "\n";
  }
}

BOOST_AUTO_TEST_CASE(simple_test_2) {
  auto token = Token(IDENTIFIER, "adsda");
  std::cout << token.toString() << "\n";
}
