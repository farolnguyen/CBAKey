cmake_minimum_required(VERSION 3.16)
project(FarolKey VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(FAROLKEY_BUILD_TESTS "Build unit tests" ON)
option(FAROLKEY_BUILD_FCITX5_PLUGIN "Build real Fcitx5 plugin module" ON)
option(FAROLKEY_BUILD_CORPUS_TOOLS "Build corpus BFS sequence finder (offline generator)" ON)

add_library(farolkey_core
    src/core/engine.cpp
    src/core/vi_syllable.cpp
    src/core/user_dict.cpp
    src/config/config.cpp
    src/common/logger.cpp
)

add_library(farolkey_fcitx5_adapter
    src/adapter/fcitx5/bridge.cpp
    src/adapter/fcitx5/preedit_strategy.cpp
    src/adapter/fcitx5/surrounding_cursor_normalize.cpp
)

target_include_directories(farolkey_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(farolkey_fcitx5_adapter
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(farolkey_core
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_compile_options(farolkey_fcitx5_adapter
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_link_libraries(farolkey_fcitx5_adapter
    PUBLIC
        farolkey_core
)

if(FAROLKEY_BUILD_FCITX5_PLUGIN)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FCITX5 REQUIRED IMPORTED_TARGET
        Fcitx5Core
        Fcitx5Utils
        Fcitx5Config
        Fcitx5Module
    )

    # Optional X11/XCB support for click-away pre-interception (X11 sessions only).
    # Requires: libxcb1-dev (headers) + fcitx5-modules-dev (Fcitx5ModuleXCB).
    set(FAROLKEY_X11_SOURCES src/adapter/fcitx5/x11_click_interceptor.cpp)
    set(FAROLKEY_X11_DEFINITIONS "")
    set(FAROLKEY_X11_LIBRARIES "")
    set(FAROLKEY_X11_INCLUDE_DIRS "")

    find_package(Fcitx5ModuleXCB QUIET)
    find_path(XCB_INCLUDE_DIR xcb/xcb.h)
    find_library(XCB_LIBRARY xcb)

    if(Fcitx5ModuleXCB_FOUND AND XCB_INCLUDE_DIR AND XCB_LIBRARY)
        message(STATUS "FarolKey: X11 click-away interception ENABLED")
        set(FAROLKEY_X11_DEFINITIONS FAROLKEY_ENABLE_X11)
        set(FAROLKEY_X11_LIBRARIES Fcitx5::Module::XCB ${XCB_LIBRARY})
        set(FAROLKEY_X11_INCLUDE_DIRS ${XCB_INCLUDE_DIR})
    else()
        message(STATUS "FarolKey: X11 click-away interception DISABLED (run: sudo apt install libxcb1-dev)")
    endif()

    add_library(farolkey_fcitx5plugin MODULE
        src/adapter/fcitx5/farolkey_fcitx5_engine.cpp
        src/adapter/fcitx5/compose_anchor_fcitx5.cpp
        src/adapter/fcitx5/committed_rewrite_fcitx5.cpp
        ${FAROLKEY_X11_SOURCES}
    )
    target_include_directories(farolkey_fcitx5plugin
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${FAROLKEY_X11_INCLUDE_DIRS}
    )
    target_compile_definitions(farolkey_fcitx5plugin PRIVATE
        ${FAROLKEY_X11_DEFINITIONS}
        FAROLKEY_VERSION="${PROJECT_VERSION}"
    )
    target_link_libraries(farolkey_fcitx5plugin
        PRIVATE
            farolkey_fcitx5_adapter
            PkgConfig::FCITX5
            ${FAROLKEY_X11_LIBRARIES}
    )
    target_compile_options(farolkey_fcitx5plugin
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
    set_target_properties(farolkey_fcitx5plugin PROPERTIES
        PREFIX ""
        OUTPUT_NAME "libfarolkey"
    )
endif()

if(FAROLKEY_BUILD_TESTS)
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
    add_executable(farolkey_engine_smoke_test tests/engine_smoke_test.cpp)
    target_link_libraries(farolkey_engine_smoke_test PRIVATE farolkey_core)
    add_test(NAME farolkey_engine_smoke_test COMMAND farolkey_engine_smoke_test)

    add_executable(farolkey_config_smoke_test tests/config_smoke_test.cpp)
    target_link_libraries(farolkey_config_smoke_test PRIVATE farolkey_core)
    add_test(NAME farolkey_config_smoke_test COMMAND farolkey_config_smoke_test)

    add_executable(farolkey_vi_syllable_smoke_test tests/vi_syllable_smoke_test.cpp)
    target_link_libraries(farolkey_vi_syllable_smoke_test PRIVATE farolkey_core)
    add_test(NAME farolkey_vi_syllable_smoke_test COMMAND farolkey_vi_syllable_smoke_test)

    add_executable(farolkey_keypress_benchmark tests/keypress_benchmark.cpp)
    target_link_libraries(farolkey_keypress_benchmark PRIVATE farolkey_core)

    add_executable(farolkey_fcitx5_bridge_smoke_test tests/fcitx5_bridge_smoke_test.cpp)
    target_link_libraries(farolkey_fcitx5_bridge_smoke_test PRIVATE farolkey_fcitx5_adapter)
    add_test(NAME farolkey_fcitx5_bridge_smoke_test COMMAND farolkey_fcitx5_bridge_smoke_test)

    add_executable(farolkey_fcitx5_preedit_strategy_smoke_test tests/fcitx5_preedit_strategy_smoke_test.cpp)
    target_link_libraries(farolkey_fcitx5_preedit_strategy_smoke_test PRIVATE farolkey_fcitx5_adapter)
    add_test(NAME farolkey_fcitx5_preedit_strategy_smoke_test COMMAND farolkey_fcitx5_preedit_strategy_smoke_test)

    add_executable(farolkey_compose_anchor_smoke_test tests/compose_anchor_smoke_test.cpp)
    target_include_directories(farolkey_compose_anchor_smoke_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME farolkey_compose_anchor_smoke_test COMMAND farolkey_compose_anchor_smoke_test)

    add_executable(farolkey_m6_3a_rewrite_smoke_test tests/m6_3a_rewrite_smoke_test.cpp)
    target_link_libraries(farolkey_m6_3a_rewrite_smoke_test PRIVATE farolkey_core)
    add_test(NAME farolkey_m6_3a_rewrite_smoke_test COMMAND farolkey_m6_3a_rewrite_smoke_test)

    add_executable(farolkey_m8_user_dict_smoke_test tests/m8_user_dict_smoke_test.cpp)
    target_link_libraries(farolkey_m8_user_dict_smoke_test PRIVATE farolkey_core)
    add_test(NAME farolkey_m8_user_dict_smoke_test COMMAND farolkey_m8_user_dict_smoke_test)

    add_executable(farolkey_m13_abbrev_smoke_test tests/m13_abbrev_smoke_test.cpp)
    target_link_libraries(farolkey_m13_abbrev_smoke_test PRIVATE farolkey_core GTest::gtest_main)
    add_test(NAME farolkey_m13_abbrev_smoke_test COMMAND farolkey_m13_abbrev_smoke_test)

    add_executable(farolkey_surrounding_cursor_normalize_smoke_test
                   tests/surrounding_cursor_normalize_smoke_test.cpp)
    target_link_libraries(farolkey_surrounding_cursor_normalize_smoke_test PRIVATE farolkey_fcitx5_adapter)
    add_test(NAME farolkey_surrounding_cursor_normalize_smoke_test
             COMMAND farolkey_surrounding_cursor_normalize_smoke_test)

    add_executable(farolkey_corpus_test tests/corpus_gtest.cpp tests/corpus_driver.cpp)
    target_link_libraries(farolkey_corpus_test PRIVATE farolkey_core GTest::gtest_main nlohmann_json::nlohmann_json)
    target_compile_options(farolkey_corpus_test PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME farolkey_corpus_test COMMAND farolkey_corpus_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

if(FAROLKEY_BUILD_CORPUS_TOOLS)
    add_executable(farolkey_corpus_bfs tools/corpus_bfs_main.cpp)
    target_link_libraries(farolkey_corpus_bfs PRIVATE farolkey_core)
    target_include_directories(farolkey_corpus_bfs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_options(farolkey_corpus_bfs PRIVATE -Wall -Wextra -Wpedantic)

    # M13.4: farolkey-abbrev CLI management tool
    add_executable(farolkey_abbrev tools/farolkey_abbrev_main.cpp)
    target_link_libraries(farolkey_abbrev PRIVATE farolkey_core)
    target_include_directories(farolkey_abbrev PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_options(farolkey_abbrev PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(farolkey_abbrev PROPERTIES OUTPUT_NAME "farolkey-abbrev")

    if(FAROLKEY_BUILD_TESTS)
        add_executable(farolkey_m1_fallback_audit tools/m1_fallback_audit.cpp)
        target_link_libraries(farolkey_m1_fallback_audit PRIVATE farolkey_core nlohmann_json::nlohmann_json)
        target_include_directories(farolkey_m1_fallback_audit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_compile_options(farolkey_m1_fallback_audit PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# ── M10: Install targets ──────────────────────────────────────────────────────

if(FAROLKEY_BUILD_FCITX5_PLUGIN)
    # Determine Fcitx5 addon library install path (multiarch-aware).
    # On Debian/Ubuntu: /usr/lib/x86_64-linux-gnu/fcitx5
    # Fallback:         /usr/lib/fcitx5
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FAROLKEY_MULTIARCH_TUPLE "x86_64-linux-gnu" CACHE STRING "Multiarch tuple")
    else()
        set(FAROLKEY_MULTIARCH_TUPLE "i386-linux-gnu" CACHE STRING "Multiarch tuple")
    endif()

    install(TARGETS farolkey_fcitx5plugin
        LIBRARY DESTINATION "lib/${FAROLKEY_MULTIARCH_TUPLE}/fcitx5"
        COMPONENT runtime
    )
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/deploy/fcitx5/addon/farolkey.conf"
        DESTINATION "share/fcitx5/addon"
        COMPONENT runtime
    )
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/deploy/fcitx5/inputmethod/farolkey.conf"
        DESTINATION "share/fcitx5/inputmethod"
        COMPONENT runtime
    )
    # Mode indicator icons — installed to /usr/share/farolkey/icons/
    # The engine looks here first before falling back to ~/.cache/farolkey/.
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/deploy/icons/mode_vi.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/deploy/icons/mode_en.svg"
        DESTINATION "share/farolkey/icons"
        COMPONENT runtime
    )

    # Dictionary Manager GUI (Python + GTK3)
    install(PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/farolkey-dict-gui"
        DESTINATION "bin"
        COMPONENT runtime
    )

    # Shared Python logging module + export-log tool
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/farolkey_log.py"
        DESTINATION "bin"
        COMPONENT runtime
    )
    install(PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/farolkey-export-log"
        DESTINATION "bin"
        COMPONENT runtime
    )

    # Clipboard History daemon + UI (Python + GTK3)
    install(PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clipboard/farolkey-clipboard"
        DESTINATION "bin"
        COMPONENT runtime
    )

    # Smart Templates CLI (Python + Jinja2)
    install(PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/template/farolkey-template"
        DESTINATION "bin"
        COMPONENT runtime
    )

    # Screenshot tool, daemon, and settings (Python + GTK3)
    install(PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/screenshot/farolkey-screenshot"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/screenshot/farolkey-screenshot-daemon"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/screenshot/farolkey-screenshot-settings"
        DESTINATION "bin"
        COMPONENT runtime
    )

    # Autostart desktop entries (clipboard + screenshot daemons)
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/deploy/autostart/farolkey-clipboard.desktop"
        "${CMAKE_CURRENT_SOURCE_DIR}/deploy/autostart/farolkey-screenshot.desktop"
        DESTINATION "/etc/xdg/autostart"
        COMPONENT runtime
    )
endif()

# ── M10: CPack DEB ───────────────────────────────────────────────────────────

set(CPACK_PACKAGE_NAME "farolkey")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "FarolKey Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FarolKey Vietnamese Input Method Engine for Fcitx5")
set(CPACK_PACKAGE_DESCRIPTION
    "FarolKey is a production-grade Vietnamese Telex/VNI input method engine\n"
    "for Fcitx5 on Linux. Supports full tone placement, syllable model,\n"
    "user dictionary expansion, and C1 committed-syllable rewrite.")
set(CPACK_PACKAGE_CONTACT "farolkey@example.com")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
# SONAME varies by distro/Ubuntu release: .so.7 (noble), .so.8–.so.10 on others.
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libfcitx5core10 | libfcitx5core9 | libfcitx5core8 | libfcitx5core7 | libfcitx5core6, libstdc++6 (>= 11)")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "fcitx5")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "FarolKey Maintainers <farolkey@example.com>")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

# Post-install: restart fcitx5 if running
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_SOURCE_DIR}/deploy/deb/postinst"
    "${CMAKE_CURRENT_SOURCE_DIR}/deploy/deb/prerm"
)

set(CPACK_COMPONENTS_ALL runtime)
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "FarolKey runtime plugin")

include(CPack)
