#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")

#define MAX_UDP_PAYLOAD 1400

struct PacketHeader {
    uint32_t frameIndex;
    uint16_t fragmentIndex;
    uint16_t totalFragments;
    uint32_t payloadSize;
};

class UDPServer {
public:
    UDPServer();
    ~UDPServer();
    bool Initialize();
    void SendFrame(const uint8_t* data, size_t size, const std::string& clientIP, uint16_t clientPort);
    void Cleanup();

private:
    SOCKET m_socket;
    uint32_t m_frameCounter;
};

class UDPClient {
public:
    UDPClient();
    ~UDPClient();
    bool Initialize(uint16_t port);
    
    // Returns a complete frame when all fragments are assembled.
    // If it returns true, outData contains the assembled frame.
    bool ReceiveFrame(std::vector<uint8_t>& outData);
    void Cleanup();

private:
    SOCKET m_socket;
    
    // Simple reassembly buffer for the current frame
    uint32_t m_currentFrameIndex;
    std::vector<uint8_t> m_frameBuffer;
    uint16_t m_receivedFragments;
    uint16_t m_expectedFragments;
};
