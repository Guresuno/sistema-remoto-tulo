#include "Network.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "--- TULO Remote System (Cliente) ---" << std::endl;
    
    UDPClient udpClient;
    uint16_t port = 4000;
    if (!udpClient.Initialize(port)) {
        std::cerr << "Failed to init UDP Client on port " << port << std::endl;
        return -1;
    }
    
    std::cout << "Listening on port " << port << " for incoming stream..." << std::endl;
    std::cout << "Esperando modulo de captura de inputs (Teclado/Raton/Mando)..." << std::endl;
    
    while (true) {
        std::vector<uint8_t> frameData;
        if (udpClient.ReceiveFrame(frameData)) {
            std::cout << "Received Complete Frame! Size: " << frameData.size() << " bytes." << std::endl;
            // TODO: Pass frameData to H.264 Decoder (e.g. FFmpeg or AMF Decoder)
        }
    }
    
    return 0;
}
