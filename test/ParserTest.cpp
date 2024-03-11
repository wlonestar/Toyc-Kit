#include <Parser/Parser.h>

#include <gtest/gtest.h>

namespace toyc {

class ParserTest : public testing::Test {
protected:
  void SetUp() override {
    setenv("toycc", "/home/wjl/work/toyc/build/bin/toycc", 1);
    pathPrefix = "../test/Unit/Parser/";
  }

  Parser parser;
  std::string input;

  std::string pathPrefix;
};

TEST_F(ParserTest, PrimaryExprError) {
  std::string file = pathPrefix + "simple.toyc";
  ASSERT_TRUE(read_from(file, input));
  parser.addInput(input);

  std::string err;
  std::string errInfo =
      "\033[1;37mline:4:col:11:\033[0m \033[1;31merror:\033[0m \033[1;37mparse "
      "primary expression error\033[0m";
  try {
    parser.parse();
  } catch (ParserException e) {
    err = e.what();
  }
  EXPECT_EQ(err, errInfo);
}

TEST_F(ParserTest, AssignExpr) {
  std::string file = pathPrefix + "assign.toyc";
  std::string astFile = pathPrefix + "assign_ast.txt";
  std::string ast;
  ASSERT_TRUE(read_from(file, input) && read_from(astFile, ast));
  parser.addInput(input);

  auto translationUnit = parser.parse();
  std::stringstream ss;
  translationUnit->dump(ss);
  std::cout << ss.str();

  EXPECT_EQ(ss.str(), ast);
}

} // namespace toyc
