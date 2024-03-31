#include <Preprocessor/Preprocessor.h>

#include <gtest/gtest.h>

namespace toyc {

class PreprocessorTest : public testing::Test {
protected:
  void SetUp() override {
    /// for looking for standard libary filepath
    setenv("toycc", "/home/wjl/work/Toyc-Kit/build/bin/toycc", 1);
    path_prefix_ = "../test/Unit/Preprocessor/";
  }

  Preprocessor processor_;
  std::string expected_;
  std::string actually_;
  std::string input_;

  std::string path_prefix_;
};

TEST_F(PreprocessorTest, Comment) {
  std::string file = path_prefix_ + "comment.toyc";
  std::string file_expected = path_prefix_ + "comment_expect.toyc";
  ASSERT_TRUE(ReadFrom(file, input_) && ReadFrom(file_expected, expected_));
  processor_.SetInput(input_);
  actually_ = processor_.Process();
  EXPECT_EQ(expected_, actually_);
}

// Example test case
TEST_F(PreprocessorTest, Include) {
  std::string file = path_prefix_ + "include.toyc";
  std::string file_expected = path_prefix_ + "include_expect.toyc";
  ASSERT_TRUE(ReadFrom(file, input_) && ReadFrom(file_expected, expected_));
  processor_.SetInput(input_);
  actually_ = processor_.Process();
  EXPECT_EQ(expected_, actually_);
}

} // namespace toyc
