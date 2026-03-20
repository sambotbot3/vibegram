add_executable(llm_client_test
    llm_client_test.cpp
)

target_link_libraries(llm_client_test PRIVATE vibegram_lib)

add_test(NAME llm_client_test COMMAND llm_client_test)

add_executable(watcher_test
    watcher_test.cpp
)

target_link_libraries(watcher_test PRIVATE vibegram_lib)

add_test(NAME watcher_test COMMAND watcher_test)
