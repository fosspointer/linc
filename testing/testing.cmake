enable_testing()

file(GLOB test_sources "${CMAKE_CURRENT_SOURCE_DIR}/testing/tests/*.cpp")

add_library(linctest STATIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/testing/TestingException.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/testing/TestingException.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/testing/Testing.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/testing/Testing.cpp")

target_link_libraries(linctest linccore)

foreach(file ${test_sources})
    message(file = ${file})
    get_filename_component(name ${file} NAME_WLE)

    add_executable(test__${name} ${file})
    target_link_libraries(test__${name} linctest linccore)
    target_include_directories(test__${name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME ${name} COMMAND "${CMAKE_BINARY_DIR}/test__${name}")
endforeach()
