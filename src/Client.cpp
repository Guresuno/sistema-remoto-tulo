#include "Network.h"
#include "Decoder.h"
#include <iostream>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    std::cout << "--- TULO Remote System (Cliente Gráfico) ---" << std::endl;
    
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "Failed to init SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("TULO Stream Client", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Texture* texture = nullptr;

    UDPClient udpClient;
    uint16_t port = 4000;
    if (!udpClient.Initialize(port)) {
        std::cerr << "Failed to init UDP Client on port " << port << std::endl;
        return -1;
    }
    
    Decoder decoder;
    if (!decoder.Initialize()) {
        std::cerr << "Failed to init Decoder." << std::endl;
        return -1;
    }

    std::cout << "Listening on port " << port << " for incoming stream..." << std::endl;
    
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            // TODO: Capture gamepad inputs for UDP transmission later
        }

        std::vector<uint8_t> frameData;
        if (udpClient.ReceiveFrame(frameData)) {
            // Decode H.264
            if (decoder.DecodeFrame(frameData.data(), frameData.size())) {
                int w = decoder.GetWidth();
                int h = decoder.GetHeight();
                
                if (!texture) {
                    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, w, h);
                }

                // Render YUV to screen
                SDL_UpdateYUVTexture(texture, nullptr,
                    decoder.GetYData(), decoder.GetYPitch(),
                    decoder.GetUData(), decoder.GetUPitch(),
                    decoder.GetVData(), decoder.GetVPitch()
                );

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, nullptr, nullptr);
                SDL_RenderPresent(renderer);
            }
        }
    }
    
    if (texture) SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
