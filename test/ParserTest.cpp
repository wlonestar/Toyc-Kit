#include <Parser/Parser.h>

#include <gtest/gtest.h>

namespace toyc {

class ParserTest : public testing::Test {
protected:
  void SetUp() override {
    setenv("toycc", "/home/wjl/work/Toyc-Kit/build/bin/toycc", 1);
    path_prefix_ = "../test/Unit/Parser/";
  }

  Parser parser_;
  std::string input_;

  std::string path_prefix_;
};

TEST_F(ParserTest, PrimaryExprError) {
  std::string file = path_prefix_ + "simple.toyc";
  ASSERT_TRUE(ReadFrom(file, input_));
  parser_.AddInput(input_);

  std::string err;
  std::string err_info =
      "\033[1;37mline:4:col:11:\033[0m \033[1;31merror:\033[0m \033[1;37mparse "
      "primary expression error\033[0m";
  try {
    parser_.Parse();
  } catch (ParserException e) {
    err = e.what();
  }
  EXPECT_EQ(err, err_info);
}

TEST_F(ParserTest, AssignExpr) {
  std::string file = path_prefix_ + "assign.toyc";
  std::string ast_file = path_prefix_ + "assign_ast.txt";
  std::string ast;
  ASSERT_TRUE(ReadFrom(file, input_) && ReadFrom(ast_file, ast));
  parser_.AddInput(input_);

  auto translation_unit = parser_.Parse();
  std::stringstream ss;
  translation_unit->Dump(ss);
  std::cout << ss.str();

  EXPECT_EQ(ss.str(), ast);
}

} // namespace toyc
