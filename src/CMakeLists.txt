add_subdirectory(Preprocessor)
add_subdirectory(Lexer)
add_subdirectory(AST)
add_subdirectory(Sema)
add_subdirectory(Parser)
add_subdirectory(CodeGen)
add_subdirectory(Interpreter)

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
  Sema
  Parser
  CodeGen
  fmt
  readline
  ${LLVM_LIBS_C}
)

add_executable(toyci
  Interpreter/toyci.cpp
)
target_compile_definitions(toyci PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(toyci
  Prepprocessor
  Lexer
  AST
  Sema
  Parser
  InterpreterParser
  Interpreter
  CodeGen
  InterpreterCodeGen
  fmt
  ${LLVM_LIBS_I}
)

add_executable(toyc-repl
  Repl/ToycRepl.cpp
  Repl/LineEditor.cpp
)
target_compile_definitions(toyc-repl PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(toyc-repl
  Prepprocessor
  Lexer
  AST
  Sema
  Parser
  InterpreterParser
  CodeGen
  InterpreterCodeGen
  Interpreter
  fmt
  readline
  ${LLVM_LIBS_I}
)
