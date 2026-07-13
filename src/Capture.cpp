#include "Capture.h"

Capture::Capture() {}
Capture::~Capture() {}

bool Capture::Initialize(int outputIndex) {
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel;
    
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        0, featureLevels, 1, D3D11_SDK_VERSION,
        &m_Device, &featureLevel, &m_Context
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device." << std::endl;
        return false;
    }

    ComPtr<IDXGIDevice> dxgiDevice;
    hr = m_Device.As(&dxgiDevice);
    if (FAILED(hr)) return false;

    ComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), &dxgiAdapter);
    if (FAILED(hr)) return false;

    ComPtr<IDXGIOutput> dxgiOutput;
    hr = dxgiAdapter->EnumOutputs(outputIndex, &dxgiOutput);
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI Output (Monitor " << outputIndex << ")." << std::endl;
        return false;
    }

    ComPtr<IDXGIOutput1> dxgiOutput1;
    hr = dxgiOutput.As(&dxgiOutput1);
    if (FAILED(hr)) return false;

    hr = dxgiOutput1->DuplicateOutput(m_Device.Get(), &m_Duplication);
    if (FAILED(hr)) {
        std::cerr << "Failed to duplicate output. Are you running in a headless session?" << std::endl;
        return false;
    }

    std::cout << "DXGI Desktop Duplication Initialized successfully." << std::endl;
    return true;
}

bool Capture::AcquireNextFrame(ComPtr<ID3D11Texture2D>& outTexture) {
    if (!m_Duplication) return false;

    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    ComPtr<IDXGIResource> desktopResource;
    
    // Timeout of 10ms
    HRESULT hr = m_Duplication->AcquireNextFrame(10, &frameInfo, &desktopResource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return false; // No new frame yet
    }
    if (FAILED(hr)) {
        std::cerr << "Failed to acquire next frame." << std::endl;
        return false;
    }

    hr = desktopResource.As(&outTexture);
    if (FAILED(hr)) {
        ReleaseFrame();
        return false;
    }

    return true;
}

void Capture::ReleaseFrame() {
    if (m_Duplication) {
        m_Duplication->ReleaseFrame();
    }
}
