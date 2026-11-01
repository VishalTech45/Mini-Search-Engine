
macro(add_search_engine_test test_name test_source)

    add_executable(${test_name} ${test_source})

    target_link_libraries(${test_name}
        PRIVATE search_engine_core
    )

    target_include_directories(${test_name}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_test(
        NAME ${test_name}
        COMMAND ${test_name}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

endmacro()


add_search_engine_test(test_text_processor test_text_processor.cpp)
add_search_engine_test(test_inverted_index test_inverted_index.cpp)
add_search_engine_test(test_ranking test_ranking.cpp)
add_search_engine_test(test_popularity test_popularity.cpp)
add_search_engine_test(test_search_engine test_search_engine.cpp)
