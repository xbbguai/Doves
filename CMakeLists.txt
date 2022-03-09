cmake_minimum_required(VERSION 3.0.0)
project(Doves VERSION 0.0.1)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
add_executable(doves 
    TableTypes.h 
    HashTable.h 
    HashTable.cpp
    WordReader.h 
    WordReader.cpp 
    var.h 
    var.cpp 
    Expression.h 
    Expression.cpp 
    BuiltInFuncs.h 
    BuiltInFuncs.cpp 
    BuiltInStatements.h 
    BuiltInStatements.cpp 
    BuiltInStructs.cpp 
    BuiltInStructs_Thread.cpp
    parser.h 
    parser.cpp 
    textencoding.h
    textencoding.cpp
    main.cpp
)

if (UNIX)
    target_link_libraries(doves pthread)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
