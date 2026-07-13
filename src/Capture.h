#pragma once
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <iostream>

using Microsoft::WRL::ComPtr;

class Capture {
public:
    Capture();
    ~Capture();

    bool Initialize(int outputIndex = 0);
    bool AcquireNextFrame(ComPtr<ID3D11Texture2D>& outTexture);
    void ReleaseFrame();

    ComPtr<ID3D11Device> GetDevice() const { return m_Device; }

private:
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    ComPtr<IDXGIOutputDuplication> m_Duplication;
};
