enable_testing()

file(GLOB test_sources "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

add_library(linctest STATIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/TestingException.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/TestingException.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Testing.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Testing.cpp")

target_include_directories(linctest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(linctest linccore)

target_compile_definitions(linctest PUBLIC
    LINC_TESTING_SNAPSHOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/snapshots"
)

foreach(file ${test_sources})
    message(file = ${file})
    get_filename_component(name ${file} NAME_WLE)

    add_executable(test__${name} ${file})
    target_link_libraries(test__${name} linctest linccore)
    add_test(NAME ${name} COMMAND "${CMAKE_BINARY_DIR}/test__${name}")
endforeach()
