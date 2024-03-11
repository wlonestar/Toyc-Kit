#include <Preprocessor/Preprocessor.h>
#include <Util.h>

#include <gtest/gtest.h>

namespace toyc {

class PreprocessorTest : public testing::Test {
protected:
  void SetUp() override {
    /// for looking for standard libary filepath
    setenv("toycc", "/home/wjl/work/toyc/build/bin/toycc", 1);
  }

  Preprocessor processor;
  std::string expected;
  std::string actually;
  std::string input;
};

TEST_F(PreprocessorTest, Comment) {
  std::string file = "../../test/Unit/Preprocessor/comment.toyc";
  std::string file_expected = "../../test/Unit/Preprocessor/comment_expect.toyc";
  ASSERT_TRUE(read_from(file, input) && read_from(file_expected, expected));
  processor.setInput(input);
  actually = processor.process();
  EXPECT_EQ(expected, actually);
}

// Example test case
TEST_F(PreprocessorTest, Include) {
  std::string file = "../../test/Unit/Preprocessor/include.toyc";
  std::string file_expected = "../../test/Unit/Preprocessor/include_expect.toyc";
  ASSERT_TRUE(read_from(file, input) && read_from(file_expected, expected));
  processor.setInput(input);
  actually = processor.process();
  EXPECT_EQ(expected, actually);
}

} // namespace toyc
