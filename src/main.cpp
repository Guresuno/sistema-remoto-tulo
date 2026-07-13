#include "Capture.h"
#include "Encoder.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "--- StreamEngine (Host) ---" << std::endl;
    std::cout << "Starting DXGI Capture..." << std::endl;

    Capture capture;
    if (!capture.Initialize(0)) { // Monitor 0
        std::cerr << "Failed to initialize capture module. Exiting." << std::endl;
        return -1;
    }

    Encoder encoder;
    bool encoderInitialized = false;

    std::cout << "Capturing and encoding 120 frames to test pipeline..." << std::endl;
    int framesCaptured = 0;
    
    while (framesCaptured < 120) {
        ComPtr<ID3D11Texture2D> frameTexture;
        if (capture.AcquireNextFrame(frameTexture)) {
            D3D11_TEXTURE2D_DESC desc;
            frameTexture->GetDesc(&desc);

            if (!encoderInitialized) {
                if (!encoder.Initialize(capture.GetDevice().Get(), desc.Width, desc.Height)) {
                    std::cerr << "Encoder Init failed!" << std::endl;
                    return -1;
                }
                encoderInitialized = true;
            }

            encoder.EncodeFrame(frameTexture.Get());
            framesCaptured++;
            
            if (framesCaptured % 10 == 0) {
                std::cout << "Frame " << framesCaptured << " captured and encoded." << std::endl;
            }
            
            capture.ReleaseFrame();
        } else {
            // No new frame or timeout. Sleep slightly to avoid spinning.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    std::cout << "Test completed successfully." << std::endl;
    return 0;
}
