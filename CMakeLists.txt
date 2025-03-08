cmake_minimum_required(VERSION 3.10)

project(Toriel-bt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Toriel-bt 
    src/main.cpp
    src/process.cpp
    )

target_include_directories(Toriel-bt PRIVATE 
${CMAKE_SOURCE_DIR}/include
"C:/Libraries/json/include"
)