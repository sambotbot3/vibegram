cmake_minimum_required(VERSION 3.20)
project(vibegram LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

# --- Dependencies via FetchContent ---
include(FetchContent)

function(vibegram_make_available dep_name dep_repo dep_tag)
    message(STATUS "[deps] Preparing ${dep_name} (${dep_tag})")
    message(STATUS "[deps] Fetching ${dep_name} from ${dep_repo}")
    FetchContent_MakeAvailable(${dep_name})
    message(STATUS "[deps] ${dep_name} is ready")
endfunction()

# efsw - cross-platform file watcher
FetchContent_Declare(
    efsw
    GIT_REPOSITORY https://github.com/SpartanJ/efsw.git
    GIT_TAG        1.4.1
    GIT_PROGRESS   TRUE
    GIT_SHALLOW    TRUE
)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_PROGRESS   TRUE
    GIT_SHALLOW    TRUE
)

# spdlog - logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.1
    GIT_PROGRESS   TRUE
    GIT_SHALLOW    TRUE
)

vibegram_make_available(efsw "https://github.com/SpartanJ/efsw.git" "1.4.1")
vibegram_make_available(json "https://github.com/nlohmann/json.git" "v3.11.3")
vibegram_make_available(spdlog "https://github.com/gabime/spdlog.git" "v1.15.1")

# libcurl - system package
find_package(CURL REQUIRED)

# --- Library target ---
add_library(vibegram_lib
    src/lib/watcher.cpp
    src/lib/llm_client.cpp
)

target_include_directories(vibegram_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(vibegram_lib PUBLIC
    efsw
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    CURL::libcurl
)

# --- Main executable ---
add_executable(vibegram
    app/vibegram/main.cpp
)

target_link_libraries(vibegram PRIVATE vibegram_lib)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

install(TARGETS vibegram RUNTIME DESTINATION bin)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/uninstall.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/uninstall.cmake
    @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake/uninstall.cmake
    COMMENT "Removing installed vibegram files listed in install_manifest.txt"
)

add_custom_target(remove DEPENDS uninstall)
