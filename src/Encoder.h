#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <iostream>

#include "core/Factory.h"
#include "components/VideoEncoderVCE.h"
#include "components/VideoEncoderHEVC.h"

using Microsoft::WRL::ComPtr;

class Encoder {
public:
    Encoder();
    ~Encoder();

    bool Initialize(ID3D11Device* device, int width, int height);
    bool EncodeFrame(ID3D11Texture2D* texture);
    void Cleanup();

private:
    HMODULE m_hAMFDLL = nullptr;
    amf::AMFFactory* m_pFactory = nullptr;
    amf::AMFContextPtr m_pContext = nullptr;
    amf::AMFComponentPtr m_pEncoder = nullptr;
};
