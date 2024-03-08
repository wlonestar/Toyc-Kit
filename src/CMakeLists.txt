llvm_map_components_to_libnames(LLVM_LIBS_C
  support
  core
)

add_executable(toycc
  Compiler/toycc.cpp
  Compiler/Compiler.cpp
  Preprocessor/Preprocessor.cpp
  Parser/Parser.cpp
  CodeGen/CodeGen.cpp
  Lexer/Lexer.cpp
  AST/AST.cpp
  AST/ASTPrint.cpp
)
target_compile_definitions(toycc PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(toycc PRIVATE
  fmt
  readline
  ${LLVM_LIBS_C}
)

# llvm_map_components_to_libnames(LLVM_LIBS_I
#   support
#   core
#   orcjit
#   native
#   # all
# )

# not implemented!
# add_executable(toyci
#   Interpreter/toyci.cpp
#   Interpreter/Interpreter.cpp
#   Parser/InterpreterParser.cpp
#   Parser/Parser.cpp
#   CodeGen/InterpreterCodeGen.cpp
#   CodeGen/CodeGen.cpp
#   Lexer/Lexer.cpp
#   AST/AST.cpp
#   AST/ASTPrint.cpp
# )
# target_compile_definitions(toyci PRIVATE
#   $<$<CONFIG:Debug>:DEBUG>
# )
# target_link_libraries(toyci
#   fmt
#   readline
#   ${LLVM_LIBS_I}
# )
