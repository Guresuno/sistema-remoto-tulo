#include "Encoder.h"
#include <windows.h>

Encoder::Encoder() {}

Encoder::~Encoder() {
    Cleanup();
}

void Encoder::Cleanup() {
    if (m_pEncoder) {
        m_pEncoder->Terminate();
        m_pEncoder = nullptr;
    }
    if (m_pContext) {
        m_pContext->Terminate();
        m_pContext = nullptr;
    }
    if (m_hAMFDLL) {
        FreeLibrary(m_hAMFDLL);
        m_hAMFDLL = nullptr;
    }
}

bool Encoder::Initialize(ID3D11Device* device, int width, int height) {
    // Load AMF Runtime DLL
    m_hAMFDLL = LoadLibraryW(L"amfrt64.dll");
    if (!m_hAMFDLL) {
        std::cerr << "Failed to load amfrt64.dll. Make sure AMD drivers are installed." << std::endl;
        return false;
    }

    AMFInit_Fn initFun = (AMFInit_Fn)GetProcAddress(m_hAMFDLL, AMF_INIT_FUNCTION_NAME);
    if (!initFun) {
        std::cerr << "Failed to find AMFInit in amfrt64.dll." << std::endl;
        return false;
    }

    AMFQueryVersion_Fn versionFun = (AMFQueryVersion_Fn)GetProcAddress(m_hAMFDLL, AMF_QUERY_VERSION_FUNCTION_NAME);
    amf_uint64 version = 0;
    versionFun(&version);

    AMF_RESULT res = initFun(AMF_FULL_VERSION, &m_pFactory);
    if (res != AMF_OK) {
        std::cerr << "AMFInit failed." << std::endl;
        return false;
    }

    // Create AMF Context
    res = m_pFactory->CreateContext(&m_pContext);
    if (res != AMF_OK) return false;

    // Initialize D3D11 with AMF
    res = m_pContext->InitDX11(device);
    if (res != AMF_OK) {
        std::cerr << "Failed to initialize AMF with D3D11 device." << std::endl;
        return false;
    }

    // Create H264 Encoder Component
    res = m_pFactory->CreateComponent(m_pContext, AMFVideoEncoderVCE_AVC, &m_pEncoder);
    if (res != AMF_OK) {
        std::cerr << "Failed to create H264 encoder component." << std::endl;
        return false;
    }

    // Configure for Ultra Low Latency Streaming
    m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
    m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0); // No B-Frames
    m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, 10000000); // 10 Mbps
    m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(width, height));
    m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(60, 1));
    m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_MAIN);

    res = m_pEncoder->Init(amf::AMF_SURFACE_NV12, width, height); // Usually AMF wants NV12, but we will pass BGRA later, AMF will convert or we might need to specify BGRA.
    // DXGI output is usually AMF_SURFACE_BGRA. Let's initialize with BGRA.
    m_pEncoder->Terminate();
    res = m_pEncoder->Init(amf::AMF_SURFACE_BGRA, width, height);
    if (res != AMF_OK) {
        std::cerr << "AMF Encoder Init failed." << std::endl;
        return false;
    }

    std::cout << "AMF Hardware Encoder Initialized!" << std::endl;
    return true;
}

bool Encoder::EncodeFrame(ID3D11Texture2D* texture) {
    if (!m_pEncoder || !m_pContext) return false;

    amf::AMFSurfacePtr surface;
    AMF_RESULT res = m_pContext->AllocSurface(amf::AMF_MEMORY_DX11, amf::AMF_SURFACE_BGRA, 0, 0, &surface);
    // Note: Wrapping existing texture instead of allocating:
    // To do true zero-copy, we should wrap the ID3D11Texture2D directly into AMFSurface without a copy, or copy it to an AMF surface if format conversion is needed.
    // For simplicity in this milestone, we wrap it.
    
    // Proper wrap:
    amf::AMFSurfacePtr wrappedSurface;
    res = m_pContext->CreateSurfaceFromDX11Native(texture, &wrappedSurface, nullptr);
    if (res != AMF_OK) {
        std::cerr << "Failed to wrap DX11 texture to AMF Surface." << std::endl;
        return false;
    }

    res = m_pEncoder->SubmitInput(wrappedSurface);
    if (res != AMF_OK) {
        std::cerr << "SubmitInput failed." << std::endl;
        return false;
    }

    amf::AMFDataPtr data;
    res = m_pEncoder->QueryOutput(&data);
    if (res == AMF_OK && data) {
        amf::AMFBufferPtr buffer(data);
        std::cout << "Encoded Packet Size: " << buffer->GetSize() << " bytes." << std::endl;
        // TODO: Send buffer->GetNative() over UDP!
    } else if (res != AMF_REPEAT) {
        // Repeat just means no output ready yet.
    }

    return true;
}
