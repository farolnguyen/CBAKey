# CMake generated Testfile for 
# Source directory: /home/derrick/Project/CBAKey
# Build directory: /home/derrick/Project/CBAKey/build_release
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(cbakey_engine_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_engine_smoke_test")
set_tests_properties(cbakey_engine_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;134;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_config_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_config_smoke_test")
set_tests_properties(cbakey_config_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;138;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_vi_syllable_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_vi_syllable_smoke_test")
set_tests_properties(cbakey_vi_syllable_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;142;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_fcitx5_bridge_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_fcitx5_bridge_smoke_test")
set_tests_properties(cbakey_fcitx5_bridge_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;149;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_fcitx5_preedit_strategy_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_fcitx5_preedit_strategy_smoke_test")
set_tests_properties(cbakey_fcitx5_preedit_strategy_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;153;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_compose_anchor_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_compose_anchor_smoke_test")
set_tests_properties(cbakey_compose_anchor_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;157;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_m6_3a_rewrite_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_m6_3a_rewrite_smoke_test")
set_tests_properties(cbakey_m6_3a_rewrite_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;161;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_m8_user_dict_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_m8_user_dict_smoke_test")
set_tests_properties(cbakey_m8_user_dict_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;165;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_m13_abbrev_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_m13_abbrev_smoke_test")
set_tests_properties(cbakey_m13_abbrev_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;169;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_surrounding_cursor_normalize_smoke_test "/home/derrick/Project/CBAKey/build_release/cbakey_surrounding_cursor_normalize_smoke_test")
set_tests_properties(cbakey_surrounding_cursor_normalize_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;174;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_corpus_test "/home/derrick/Project/CBAKey/build_release/cbakey_corpus_test")
set_tests_properties(cbakey_corpus_test PROPERTIES  WORKING_DIRECTORY "/home/derrick/Project/CBAKey" _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;180;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
subdirs("_deps/json-build")
