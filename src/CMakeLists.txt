add_executable(toyc
  main.cpp
  Lexer.cpp
)
target_compile_definitions(toyc PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(toyc
  fmt
  readline
)
