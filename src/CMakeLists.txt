add_executable(toyc
  main.cpp
  Lexer.cpp
  Parser.cpp
  AST.cpp
  ASTPrint.cpp
  CodeGen.cpp
  Interpreter.cpp
)
target_compile_definitions(toyc PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(toyc
  fmt
  readline
  ${LLVM_LIBS}
)
