#include "Decoder.h"

Decoder::Decoder() {}

Decoder::~Decoder() {
    if (m_codecCtx) avcodec_free_context(&m_codecCtx);
    if (m_frame) av_frame_free(&m_frame);
    if (m_packet) av_packet_free(&m_packet);
}

bool Decoder::Initialize() {
    m_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!m_codec) {
        std::cerr << "Codec H264 not found in FFmpeg!" << std::endl;
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(m_codec);
    if (!m_codecCtx) {
        std::cerr << "Failed to allocate codec context" << std::endl;
        return false;
    }
    
    // Enable low latency optimizations
    m_codecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    m_codecCtx->thread_count = 1; // Para reducir latencia

    if (avcodec_open2(m_codecCtx, m_codec, nullptr) < 0) {
        std::cerr << "Failed to open codec" << std::endl;
        return false;
    }

    m_frame = av_frame_alloc();
    m_packet = av_packet_alloc();

    if (!m_frame || !m_packet) {
        std::cerr << "Failed to allocate AVFrame or AVPacket" << std::endl;
        return false;
    }

    std::cout << "FFmpeg H264 Decoder Initialized." << std::endl;
    return true;
}

bool Decoder::DecodeFrame(const uint8_t* data, size_t size) {
    if (!data || size == 0) return false;

    m_packet->data = (uint8_t*)data;
    m_packet->size = size;

    int ret = avcodec_send_packet(m_codecCtx, m_packet);
    if (ret < 0) {
        std::cerr << "Error sending packet to decoder." << std::endl;
        return false;
    }

    ret = avcodec_receive_frame(m_codecCtx, m_frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return false; // Frame not fully decoded yet
    } else if (ret < 0) {
        std::cerr << "Error decoding frame." << std::endl;
        return false;
    }

    return true; // Successfully decoded a frame
}

int Decoder::GetWidth() const { return m_frame->width; }
int Decoder::GetHeight() const { return m_frame->height; }
const uint8_t* Decoder::GetYData() const { return m_frame->data[0]; }
const uint8_t* Decoder::GetUData() const { return m_frame->data[1]; }
const uint8_t* Decoder::GetVData() const { return m_frame->data[2]; }
int Decoder::GetYPitch() const { return m_frame->linesize[0]; }
int Decoder::GetUPitch() const { return m_frame->linesize[1]; }
int Decoder::GetVPitch() const { return m_frame->linesize[2]; }
