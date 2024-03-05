add_executable(toycc
  compiler/toycc.cpp
  compiler/Parser.cpp
  compiler/CodeGen.cpp
  compiler/Compiler.cpp
  Lexer.cpp
  AST.cpp
  ASTPrint.cpp
)
target_compile_definitions(toycc PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(toycc
  fmt
  readline
  ${LLVM_LIBS_C}
)

add_executable(toyci
  interpreter/toyci.cpp
  interpreter/Parser.cpp
  interpreter/CodeGen.cpp
  interpreter/Interpreter.cpp
  compiler/Parser.cpp
  compiler/CodeGen.cpp
  Lexer.cpp
  AST.cpp
  ASTPrint.cpp
)
target_compile_definitions(toyci PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(toyci
  fmt
  readline
  ${LLVM_LIBS_I}
)

