cmake_minimum_required(VERSION 3.16)
project(CBAKey VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(CBAKEY_BUILD_TESTS "Build unit tests" ON)
option(CBAKEY_BUILD_FCITX5_PLUGIN "Build real Fcitx5 plugin module" ON)
option(CBAKEY_BUILD_CORPUS_TOOLS "Build corpus BFS sequence finder (offline generator)" ON)

add_library(cbakey_core
    src/core/engine.cpp
    src/core/vi_syllable.cpp
    src/config/config.cpp
    src/common/logger.cpp
)

add_library(cbakey_fcitx5_adapter
    src/adapter/fcitx5/bridge.cpp
    src/adapter/fcitx5/preedit_strategy.cpp
)

target_include_directories(cbakey_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(cbakey_fcitx5_adapter
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(cbakey_core
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_compile_options(cbakey_fcitx5_adapter
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_link_libraries(cbakey_fcitx5_adapter
    PUBLIC
        cbakey_core
)

if(CBAKEY_BUILD_FCITX5_PLUGIN)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FCITX5 REQUIRED IMPORTED_TARGET
        Fcitx5Core
        Fcitx5Utils
        Fcitx5Config
        Fcitx5Module
    )

    add_library(cbakey_fcitx5plugin MODULE
        src/adapter/fcitx5/cbakey_fcitx5_engine.cpp
    )
    target_include_directories(cbakey_fcitx5plugin
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(cbakey_fcitx5plugin
        PRIVATE
            cbakey_fcitx5_adapter
            PkgConfig::FCITX5
    )
    target_compile_options(cbakey_fcitx5plugin
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
    set_target_properties(cbakey_fcitx5plugin PROPERTIES
        PREFIX ""
        OUTPUT_NAME "libcbakey"
    )
endif()

if(CBAKEY_BUILD_TESTS)
    include(FetchContent)
    set(FETCHCONTENT_QUIET OFF)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(googletest json)

    enable_testing()
    add_executable(cbakey_engine_smoke_test tests/engine_smoke_test.cpp)
    target_link_libraries(cbakey_engine_smoke_test PRIVATE cbakey_core)
    add_test(NAME cbakey_engine_smoke_test COMMAND cbakey_engine_smoke_test)

    add_executable(cbakey_config_smoke_test tests/config_smoke_test.cpp)
    target_link_libraries(cbakey_config_smoke_test PRIVATE cbakey_core)
    add_test(NAME cbakey_config_smoke_test COMMAND cbakey_config_smoke_test)

    add_executable(cbakey_vi_syllable_smoke_test tests/vi_syllable_smoke_test.cpp)
    target_link_libraries(cbakey_vi_syllable_smoke_test PRIVATE cbakey_core)
    add_test(NAME cbakey_vi_syllable_smoke_test COMMAND cbakey_vi_syllable_smoke_test)

    add_executable(cbakey_keypress_benchmark tests/keypress_benchmark.cpp)
    target_link_libraries(cbakey_keypress_benchmark PRIVATE cbakey_core)

    add_executable(cbakey_fcitx5_bridge_smoke_test tests/fcitx5_bridge_smoke_test.cpp)
    target_link_libraries(cbakey_fcitx5_bridge_smoke_test PRIVATE cbakey_fcitx5_adapter)
    add_test(NAME cbakey_fcitx5_bridge_smoke_test COMMAND cbakey_fcitx5_bridge_smoke_test)

    add_executable(cbakey_fcitx5_preedit_strategy_smoke_test tests/fcitx5_preedit_strategy_smoke_test.cpp)
    target_link_libraries(cbakey_fcitx5_preedit_strategy_smoke_test PRIVATE cbakey_fcitx5_adapter)
    add_test(NAME cbakey_fcitx5_preedit_strategy_smoke_test COMMAND cbakey_fcitx5_preedit_strategy_smoke_test)

    add_executable(cbakey_corpus_test tests/corpus_gtest.cpp tests/corpus_driver.cpp)
    target_link_libraries(cbakey_corpus_test PRIVATE cbakey_core GTest::gtest_main nlohmann_json::nlohmann_json)
    target_compile_options(cbakey_corpus_test PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME cbakey_corpus_test COMMAND cbakey_corpus_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

if(CBAKEY_BUILD_CORPUS_TOOLS)
    add_executable(cbakey_corpus_bfs tools/corpus_bfs_main.cpp)
    target_link_libraries(cbakey_corpus_bfs PRIVATE cbakey_core)
    target_include_directories(cbakey_corpus_bfs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_options(cbakey_corpus_bfs PRIVATE -Wall -Wextra -Wpedantic)

    if(CBAKEY_BUILD_TESTS)
        add_executable(cbakey_m1_fallback_audit tools/m1_fallback_audit.cpp)
        target_link_libraries(cbakey_m1_fallback_audit PRIVATE cbakey_core nlohmann_json::nlohmann_json)
        target_include_directories(cbakey_m1_fallback_audit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_compile_options(cbakey_m1_fallback_audit PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
