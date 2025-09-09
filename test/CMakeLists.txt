file(GLOB PINE_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*.cpp")
######################################################################################################################
# DEPENDENCIES
######################################################################################################################

######################################################################################################################
# MAKE TARGETS
######################################################################################################################

##########################################
# "make check-tests"
##########################################
add_custom_target(list-tests COMMAND ${CMAKE_CTEST_COMMAND} --show-only)
add_custom_target(run-tests COMMAND ${CMAKE_CTEST_COMMAND} --verbose)

##########################################
# "make server client ..."
##########################################

foreach(pine_test_source ${PINE_TEST_SOURCES})
    # 获取文件的名字
    get_filename_component(pine_test_filename ${pine_test_source} NAME)
    string(REPLACE ".cpp" "" pine_test_name ${pine_test_filename})

    # 创建可执行文件
    add_executable(${pine_test_name} EXCLUDE_FROM_ALL ${pine_test_filename})    # EXCLUDE_FROM_ALL表示不把这个文件添加到all中
    # 添加依赖
    add_dependencies(list-tests ${pine_test_name})
    add_dependencies(run-tests ${pine_test_name})
    # 连接库
    target_link_libraries(${pine_test_name} pine_shared)
    # 设置目标属性
    set_target_properties(${pine_test_name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            COMMAND ${pine_test_name}
    )
endforeach(pine_test_source ${PINE_TEST_SOURCES})
