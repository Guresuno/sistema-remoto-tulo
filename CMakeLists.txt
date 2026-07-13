cmake_minimum_required(VERSION 3.20)
project(StreamEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Librerias de DirectX y Windows
link_libraries(d3d11 dxgi user32)

# Include AMF Headers
include_directories(AMF/amf/public/include)

add_executable(TuloServer 
    src/main.cpp
    src/Capture.cpp
    src/Capture.h
    src/Encoder.cpp
    src/Encoder.h
)

add_executable(TuloClient
    src/Client.cpp
)
