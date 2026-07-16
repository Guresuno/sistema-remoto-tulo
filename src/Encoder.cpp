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

    // MF Cleanup
    if (m_activeEncoder == ENCODER_MF) {
        MFShutdown();
    }
    m_pMFT = nullptr;
    m_pDeviceManager = nullptr;
}

bool Encoder::Initialize(ID3D11Device* device, int width, int height) {
    // Attempt AMF First
    m_hAMFDLL = LoadLibraryW(L"amfrt64.dll");
    if (!m_hAMFDLL) {
        std::cerr << "amfrt64.dll not found. Falling back to Media Foundation (Intel/NVIDIA)..." << std::endl;
        return InitializeMF(device, width, height);
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
        std::cerr << "AMFInit failed. Falling back to Media Foundation..." << std::endl;
        return InitializeMF(device, width, height);
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
    m_activeEncoder = ENCODER_AMF;
    return true;
}

bool Encoder::InitializeMF(ID3D11Device* device, int width, int height) {
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) return false;

    // Set up D3D11 Device Manager
    UINT resetToken;
    hr = MFCreateDXGIDeviceManager(&resetToken, &m_pDeviceManager);
    if (FAILED(hr)) return false;
    hr = m_pDeviceManager->ResetDevice(device, resetToken);
    if (FAILED(hr)) return false;

    // Create H.264 Encoder MFT
    hr = CoCreateInstance(CLSID_CMSH264EncoderMFT, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pMFT));
    if (FAILED(hr)) {
        std::cerr << "Failed to create H264 Encoder MFT." << std::endl;
        return false;
    }

    // Unlock D3D11 Acceleration
    ComPtr<IMFAttributes> pAttributes;
    m_pMFT->GetAttributes(&pAttributes);
    pAttributes->SetUINT32(MF_SA_D3D11_AWARE, TRUE);

    ComPtr<IMFMediaEventGenerator> pEventGenerator;
    m_pMFT->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, reinterpret_cast<ULONG_PTR>(m_pDeviceManager.Get()));

    // Configure Input Type (NV12 or BGRA, usually MF prefers NV12 or ARGB)
    ComPtr<IMFMediaType> pInputType;
    MFCreateMediaType(&pInputType);
    pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    pInputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    MFSetAttributeSize(pInputType.Get(), MF_MT_FRAME_SIZE, width, height);
    MFSetAttributeRatio(pInputType.Get(), MF_MT_FRAME_RATE, 60, 1);
    MFSetAttributeRatio(pInputType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    pInputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    
    // Configure Output Type (H264)
    ComPtr<IMFMediaType> pOutputType;
    MFCreateMediaType(&pOutputType);
    pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    pOutputType->SetUINT32(MF_MT_AVG_BITRATE, 10000000);
    MFSetAttributeSize(pOutputType.Get(), MF_MT_FRAME_SIZE, width, height);
    MFSetAttributeRatio(pOutputType.Get(), MF_MT_FRAME_RATE, 60, 1);
    MFSetAttributeRatio(pOutputType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

    hr = m_pMFT->SetOutputType(0, pOutputType.Get(), 0);
    if (FAILED(hr)) return false;
    hr = m_pMFT->SetInputType(0, pInputType.Get(), 0);
    if (FAILED(hr)) return false;

    // Set Low Latency
    ComPtr<ICodecAPI> pCodecApi;
    const GUID IID_ICodecAPI_Local = { 0x901db4c7, 0x31ce, 0x41a2, { 0x85, 0xdc, 0x8f, 0xa0, 0xbf, 0x41, 0xb8, 0xda } };
    if (SUCCEEDED(m_pMFT->QueryInterface(IID_ICodecAPI_Local, (void**)&pCodecApi))) {
        VARIANT var;
        var.vt = VT_UI4;
        var.ulVal = 1;
        pCodecApi->SetValue(&CODECAPI_AVLowLatencyMode, &var);
    }

    hr = m_pMFT->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    hr = m_pMFT->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    hr = m_pMFT->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);

    m_activeEncoder = ENCODER_MF;
    std::cout << "Media Foundation Hardware Encoder Initialized (Intel/NVIDIA/AMD)!" << std::endl;
    return true;
}

bool Encoder::EncodeFrame(ID3D11Texture2D* texture) {
    if (m_activeEncoder == ENCODER_MF) return EncodeFrameMF(texture);
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

bool Encoder::EncodeFrameMF(ID3D11Texture2D* texture) {
    if (!m_pMFT) return false;

    // We need to wrap the ID3D11Texture2D into an IMFSample.
    // In a real scenario, format conversion to NV12 is needed if texture is BGRA.
    ComPtr<IMFMediaBuffer> pBuffer;
    HRESULT hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), texture, 0, FALSE, &pBuffer);
    if (FAILED(hr)) return false;

    ComPtr<IMFSample> pSample;
    hr = MFCreateSample(&pSample);
    if (FAILED(hr)) return false;

    pSample->AddBuffer(pBuffer.Get());
    pSample->SetSampleTime(m_frameTime);
    m_frameTime += 166667; // approx 60 fps (10,000,000 / 60)

    hr = m_pMFT->ProcessInput(0, pSample.Get(), 0);
    if (FAILED(hr)) return false;

    // Read outputs
    MFT_OUTPUT_DATA_BUFFER outputData = {0};
    DWORD status = 0;
    hr = m_pMFT->ProcessOutput(0, 1, &outputData, &status);
    
    if (SUCCEEDED(hr) && outputData.pSample) {
        ComPtr<IMFMediaBuffer> outBuffer;
        outputData.pSample->ConvertToContiguousBuffer(&outBuffer);
        
        DWORD currentLength = 0;
        outBuffer->GetCurrentLength(&currentLength);
        std::cout << "MF Encoded Packet Size: " << currentLength << " bytes." << std::endl;
        
        // TODO: Send over UDP
        
        outputData.pSample->Release();
    } else if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
        // Needs more input
    }
    
    if (outputData.pEvents) {
        outputData.pEvents->Release();
    }

    return true;
}
