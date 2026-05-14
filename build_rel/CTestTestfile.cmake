# CMake generated Testfile for 
# Source directory: /home/derrick/Project/CBAKey
# Build directory: /home/derrick/Project/CBAKey/build_rel
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(cbakey_engine_smoke_test "/home/derrick/Project/CBAKey/build_rel/cbakey_engine_smoke_test")
set_tests_properties(cbakey_engine_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;110;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_config_smoke_test "/home/derrick/Project/CBAKey/build_rel/cbakey_config_smoke_test")
set_tests_properties(cbakey_config_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;114;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_vi_syllable_smoke_test "/home/derrick/Project/CBAKey/build_rel/cbakey_vi_syllable_smoke_test")
set_tests_properties(cbakey_vi_syllable_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;118;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_fcitx5_bridge_smoke_test "/home/derrick/Project/CBAKey/build_rel/cbakey_fcitx5_bridge_smoke_test")
set_tests_properties(cbakey_fcitx5_bridge_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;125;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_fcitx5_preedit_strategy_smoke_test "/home/derrick/Project/CBAKey/build_rel/cbakey_fcitx5_preedit_strategy_smoke_test")
set_tests_properties(cbakey_fcitx5_preedit_strategy_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;129;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_compose_anchor_smoke_test "/home/derrick/Project/CBAKey/build_rel/cbakey_compose_anchor_smoke_test")
set_tests_properties(cbakey_compose_anchor_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;133;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_m6_3a_rewrite_smoke_test "/home/derrick/Project/CBAKey/build_rel/cbakey_m6_3a_rewrite_smoke_test")
set_tests_properties(cbakey_m6_3a_rewrite_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;137;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_m8_user_dict_smoke_test "/home/derrick/Project/CBAKey/build_rel/cbakey_m8_user_dict_smoke_test")
set_tests_properties(cbakey_m8_user_dict_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;141;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_surrounding_cursor_normalize_smoke_test "/home/derrick/Project/CBAKey/build_rel/cbakey_surrounding_cursor_normalize_smoke_test")
set_tests_properties(cbakey_surrounding_cursor_normalize_smoke_test PROPERTIES  _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;146;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
add_test(cbakey_corpus_test "/home/derrick/Project/CBAKey/build_rel/cbakey_corpus_test")
set_tests_properties(cbakey_corpus_test PROPERTIES  WORKING_DIRECTORY "/home/derrick/Project/CBAKey" _BACKTRACE_TRIPLES "/home/derrick/Project/CBAKey/CMakeLists.txt;152;add_test;/home/derrick/Project/CBAKey/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
subdirs("_deps/json-build")
