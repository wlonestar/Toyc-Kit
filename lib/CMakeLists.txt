# copy standard library
file(COPY ${CMAKE_SOURCE_DIR}/lib/include DESTINATION ${CMAKE_BINARY_DIR})

# add the library target
# there is something wrong that the compiled can not work
# add_library(libtoyc SHARED libtoyc.cpp)
# target_link_libraries(libtoyc PRIVATE m)
# set_target_properties(libtoyc PROPERTIES POSITION_INDEPENDENT_CODE ON)
# set_target_properties(libtoyc PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set_target_properties(libtoyc PROPERTIES LIBRARY_OUTPUT_NAME toyc)
# install(TARGETS libtoyc DESTINATION lib)
