SET(pwd ${CMAKE_CURRENT_LIST_DIR})

SET(src 
"${CMAKE_CURRENT_LIST_DIR}/camera.cc"
"${CMAKE_CURRENT_LIST_DIR}/main.cc"
"${CMAKE_CURRENT_LIST_DIR}/tictoc.c"
  )
add_executable(minecraft ${src})
message(STATUS "minecraft added")

target_link_libraries(minecraft ${stdgl_libraries})
