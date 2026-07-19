add_executable(cpptypr_test_logger test_logger.cpp)
target_link_libraries(cpptypr_test_logger PRIVATE cpptypr)
add_test(NAME cpptypr_logger COMMAND cpptypr_test_logger)
set_tests_properties(cpptypr_logger PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cpptypr_test_engine test_engine.cpp)
target_link_libraries(cpptypr_test_engine PRIVATE cpptypr)
add_test(NAME cpptypr_engine COMMAND cpptypr_test_engine)
set_tests_properties(cpptypr_engine PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cpptypr_test_content test_content.cpp db_helpers.cpp)
target_link_libraries(cpptypr_test_content PRIVATE cpptypr sqlite3)
add_test(NAME cpptypr_content COMMAND cpptypr_test_content)
set_tests_properties(cpptypr_content PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cpptypr_test_repository test_repository.cpp)
target_link_libraries(cpptypr_test_repository PRIVATE cpptypr)
add_test(NAME cpptypr_repository COMMAND cpptypr_test_repository)
set_tests_properties(cpptypr_repository PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set_tests_properties(
    cpptypr_logger cpptypr_content cpptypr_repository
    PROPERTIES TIMEOUT 120
)
set_tests_properties(cpptypr_engine PROPERTIES TIMEOUT 300)
