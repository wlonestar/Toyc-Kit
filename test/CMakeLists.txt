enable_testing()

add_executable(PreprocessorTest
  Unit/PreprocessorTest.cpp
  ../src/Preprocessor/Preprocessor.cpp
)
target_link_libraries(PreprocessorTest
  GTest::gtest_main
  fmt
)

include(GoogleTest)
gtest_discover_tests(PreprocessorTest)
