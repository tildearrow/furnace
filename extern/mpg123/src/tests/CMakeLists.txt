add_executable(seek_whence "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/tests/seek_whence.c")
target_link_libraries(seek_whence PRIVATE lib${PROJECT_NAME})

add_executable(noise "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/tests/noise.c")
target_link_libraries(noise PRIVATE lib${PROJECT_NAME})

add_executable(text "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/tests/text.c")
target_link_libraries(text PRIVATE lib${PROJECT_NAME})
add_test(NAME text COMMAND text)

add_executable(plain_id3 "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/tests/plain_id3.c")
target_link_libraries(plain_id3 PRIVATE lib${PROJECT_NAME})
add_test(NAME plain_id3 COMMAND plain_id3)
