#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <vector>
#include <cstdint>
#include <iostream>

class Decoder {
public:
    Decoder();
    ~Decoder();

    bool Initialize();
    
    // Decodes an H264 NAL unit. Returns true if a frame is fully decoded.
    bool DecodeFrame(const uint8_t* data, size_t size);
    
    int GetWidth() const;
    int GetHeight() const;
    
    const uint8_t* GetYData() const;
    const uint8_t* GetUData() const;
    const uint8_t* GetVData() const;
    
    int GetYPitch() const;
    int GetUPitch() const;
    int GetVPitch() const;

private:
    const AVCodec* m_codec = nullptr;
    AVCodecContext* m_codecCtx = nullptr;
    AVFrame* m_frame = nullptr;
    AVPacket* m_packet = nullptr;
};
