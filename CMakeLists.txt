cmake_minimum_required(VERSION 3.20)
project(StreamEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Librerias de DirectX y Windows
link_libraries(d3d11 dxgi user32 ws2_32 setupapi)

# Include AMF Headers
include_directories(AMF/amf/public/include)

# ViGEmClient
add_subdirectory(ViGEmClient)
include_directories(ViGEmClient/include)

add_executable(TuloServer 
    src/main.cpp
    src/Capture.cpp
    src/Capture.h
    src/Encoder.cpp
    src/Encoder.h
    src/Network.cpp
    src/Network.h
    src/InputInjector.cpp
    src/InputInjector.h
)

target_link_libraries(TuloServer ViGEmClient)

add_executable(TuloClient
    src/Client.cpp
    src/Network.cpp
    src/Network.h
    src/Decoder.cpp
    src/Decoder.h
)

# Enlazar FFmpeg y SDL2 al Cliente
target_link_libraries(TuloClient ws2_32 avcodec avutil swscale SDL2)
