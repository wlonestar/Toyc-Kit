add_definitions(-DBOOST_TEST_DYN_LINK) 

add_executable(TokenTest TokenTest.cpp)
target_link_libraries(TokenTest
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  fmt
)

enable_testing()
add_test(ToycTest ToycTest)
