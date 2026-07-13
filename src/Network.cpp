#include "Network.h"
#include <iostream>
#include <algorithm>

UDPServer::UDPServer() : m_socket(INVALID_SOCKET), m_frameCounter(0) {}

UDPServer::~UDPServer() {
    Cleanup();
}

void UDPServer::Cleanup() {
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    WSACleanup();
}

bool UDPServer::Initialize() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return false;
    }

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return false;
    }

    // Set buffer sizes to be large enough for high bitrate video
    int rcvbuf = 1024 * 1024 * 8; // 8MB
    setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&rcvbuf, sizeof(int));
    setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&rcvbuf, sizeof(int));

    return true;
}

void UDPServer::SendFrame(const uint8_t* data, size_t size, const std::string& clientIP, uint16_t clientPort) {
    if (m_socket == INVALID_SOCKET) return;

    sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(clientPort);
    inet_pton(AF_INET, clientIP.c_str(), &destAddr.sin_addr);

    m_frameCounter++;
    
    uint16_t totalFragments = (uint16_t)((size + MAX_UDP_PAYLOAD - 1) / MAX_UDP_PAYLOAD);
    size_t offset = 0;

    for (uint16_t i = 0; i < totalFragments; ++i) {
        size_t currentPayloadSize = std::min(size - offset, (size_t)MAX_UDP_PAYLOAD);

        std::vector<uint8_t> packetData(sizeof(PacketHeader) + currentPayloadSize);
        PacketHeader* header = reinterpret_cast<PacketHeader*>(packetData.data());
        header->frameIndex = m_frameCounter;
        header->fragmentIndex = i;
        header->totalFragments = totalFragments;
        header->payloadSize = (uint32_t)currentPayloadSize;

        memcpy(packetData.data() + sizeof(PacketHeader), data + offset, currentPayloadSize);

        sendto(m_socket, (const char*)packetData.data(), (int)packetData.size(), 0, (sockaddr*)&destAddr, sizeof(destAddr));
        offset += currentPayloadSize;
    }
}


UDPClient::UDPClient() : m_socket(INVALID_SOCKET), m_currentFrameIndex(0), m_receivedFragments(0), m_expectedFragments(0) {}

UDPClient::~UDPClient() {
    Cleanup();
}

void UDPClient::Cleanup() {
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    WSACleanup();
}

bool UDPClient::Initialize(uint16_t port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return false;
    }

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return false;
    }

    // Set buffer sizes to be large enough for high bitrate video
    int rcvbuf = 1024 * 1024 * 8; // 8MB
    setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&rcvbuf, sizeof(int));

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port);
    localAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_socket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        return false;
    }

    return true;
}

bool UDPClient::ReceiveFrame(std::vector<uint8_t>& outData) {
    if (m_socket == INVALID_SOCKET) return false;

    char recvBuffer[65536];
    sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);

    int bytesReceived = recvfrom(m_socket, recvBuffer, sizeof(recvBuffer), 0, (sockaddr*)&senderAddr, &senderAddrSize);
    if (bytesReceived > 0 && bytesReceived >= sizeof(PacketHeader)) {
        PacketHeader* header = reinterpret_cast<PacketHeader*>(recvBuffer);
        
        // Very basic handling: drop old frames completely
        if (header->frameIndex > m_currentFrameIndex) {
            m_currentFrameIndex = header->frameIndex;
            m_receivedFragments = 0;
            m_expectedFragments = header->totalFragments;
            m_frameBuffer.resize(header->totalFragments * MAX_UDP_PAYLOAD); // Max possible size
        }

        if (header->frameIndex == m_currentFrameIndex) {
            size_t offset = header->fragmentIndex * MAX_UDP_PAYLOAD;
            if (offset + header->payloadSize <= m_frameBuffer.size()) {
                memcpy(m_frameBuffer.data() + offset, recvBuffer + sizeof(PacketHeader), header->payloadSize);
                m_receivedFragments++;

                if (m_receivedFragments == m_expectedFragments) {
                    // Frame complete! Calculate exact size
                    size_t exactSize = (m_expectedFragments - 1) * MAX_UDP_PAYLOAD + header->payloadSize;
                    outData.assign(m_frameBuffer.begin(), m_frameBuffer.begin() + exactSize);
                    return true;
                }
            }
        }
    }
    return false;
}
