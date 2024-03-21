enable_testing()

add_executable(ExpectedTest ExpectedTest.cpp)
target_link_libraries(ExpectedTest
  GTest::gtest_main
  fmt
)

# copy test files into build dir
file(COPY ${CMAKE_SOURCE_DIR}/test/Unit/Preprocessor DESTINATION ${CMAKE_BINARY_DIR}/test/Unit)
file(COPY ${CMAKE_SOURCE_DIR}/test/Unit/Parser DESTINATION ${CMAKE_BINARY_DIR}/test/Unit)

add_executable(PreprocessorTest
  PreprocessorTest.cpp
  ../src/Preprocessor/Preprocessor.cpp
)
target_link_libraries(PreprocessorTest
  GTest::gtest_main
  fmt
)

add_executable(ParserTest
  ParserTest.cpp
  ../src/Parser/Parser.cpp
  ../src/Lexer/Lexer.cpp
  ../src/AST/AST.cpp
  ../src/AST/ASTPrint.cpp  
  ../src/Sema/Sema.cpp
)
target_link_libraries(ParserTest
  ${LLVM_LIBS_C}
  GTest::gtest_main
  fmt
)

include(GoogleTest)
gtest_discover_tests(ExpectedTest)
gtest_discover_tests(PreprocessorTest)
gtest_discover_tests(ParserTest)
