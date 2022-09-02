project(FiniteStateMachineCppApp)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)

add_executable(FiniteStateMachineCppApp FiniteStateMachineCppApp.cpp)

target_include_directories(FiniteStateMachineCppApp PRIVATE ../FiniteStateMachineCpp/)
