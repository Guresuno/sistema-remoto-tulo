#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <iostream>

#include "core/Factory.h"
#include "components/VideoEncoderVCE.h"
#include "components/VideoEncoderHEVC.h"

// Media Foundation
#include <mfapi.h>
#include <mftransform.h>
#include <mfidl.h>
#include <mferror.h>
#include <wmcodecdsp.h>
#include <codecapi.h>

using Microsoft::WRL::ComPtr;

enum EncoderType {
    ENCODER_NONE,
    ENCODER_AMF,
    ENCODER_MF
};

class Encoder {
public:
    Encoder();
    ~Encoder();

    bool Initialize(ID3D11Device* device, int width, int height);
    bool EncodeFrame(ID3D11Texture2D* texture);
    void Cleanup();

    EncoderType m_activeEncoder = ENCODER_NONE;

    // AMF Variables
    HMODULE m_hAMFDLL = nullptr;
    amf::AMFFactory* m_pFactory = nullptr;
    amf::AMFContextPtr m_pContext = nullptr;
    amf::AMFComponentPtr m_pEncoder = nullptr;

    // MF Variables
    bool InitializeMF(ID3D11Device* device, int width, int height);
    bool EncodeFrameMF(ID3D11Texture2D* texture);
    ComPtr<IMFTransform> m_pMFT = nullptr;
    ComPtr<IMFDXGIDeviceManager> m_pDeviceManager = nullptr;
    DWORD m_inputStreamID = 0;
    DWORD m_outputStreamID = 0;
    LONGLONG m_frameTime = 0;
};
