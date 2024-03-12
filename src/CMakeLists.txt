add_subdirectory(Preprocessor)
add_subdirectory(Lexer)
add_subdirectory(AST)
add_subdirectory(Parser)
add_subdirectory(CodeGen)

add_executable(toycc
  Compiler/toycc.cpp
  Compiler/Compiler.cpp
)
target_compile_definitions(toycc PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(toycc PRIVATE
  Prepprocessor
  Lexer
  AST
  Parser
  CodeGen
  fmt
  readline
  ${LLVM_LIBS_C}
)

add_executable(toyci
  Interpreter/toyci.cpp
  Interpreter/Interpreter.cpp
)
target_compile_definitions(toyci PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(toyci
  Prepprocessor
  Lexer
  AST
  Parser
  InterpreterParser
  CodeGen
  InterpreterCodeGen
  fmt
  readline
  ${LLVM_LIBS_I}
)
