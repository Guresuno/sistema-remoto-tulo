#include "InputInjector.h"

InputInjector::InputInjector() {}

InputInjector::~InputInjector() {
    Cleanup();
}

void InputInjector::Cleanup() {
    if (m_pad) {
        vigem_target_remove(m_client, m_pad);
        vigem_target_free(m_pad);
        m_pad = nullptr;
    }
    if (m_client) {
        vigem_disconnect(m_client);
        vigem_free(m_client);
        m_client = nullptr;
    }
}

bool InputInjector::Initialize() {
    m_client = vigem_alloc();
    if (m_client == nullptr) {
        std::cerr << "Not enough memory to allocate ViGEm client!" << std::endl;
        return false;
    }

    VIGEM_ERROR err = vigem_connect(m_client);
    if (err != VIGEM_ERROR_NONE) {
        std::cerr << "ViGEm Bus connection failed! Is the driver installed? Error code: " << err << std::endl;
        vigem_free(m_client);
        m_client = nullptr;
        return false;
    }

    m_pad = vigem_target_x360_alloc();
    err = vigem_target_add(m_client, m_pad);
    if (err != VIGEM_ERROR_NONE) {
        std::cerr << "Failed to add Xbox 360 virtual gamepad! Error code: " << err << std::endl;
        vigem_free(m_client);
        m_client = nullptr;
        return false;
    }

    std::cout << "Virtual Xbox 360 controller successfully connected via ViGEmBus!" << std::endl;
    return true;
}

void InputInjector::SetGamepadState(uint16_t buttons, uint8_t leftTrigger, uint8_t rightTrigger, 
                                    int16_t thumbLX, int16_t thumbLY, int16_t thumbRX, int16_t thumbRY) {
    if (!m_client || !m_pad) return;

    XUSB_REPORT report = {};
    report.wButtons = buttons;
    report.bLeftTrigger = leftTrigger;
    report.bRightTrigger = rightTrigger;
    report.sThumbLX = thumbLX;
    report.sThumbLY = thumbLY;
    report.sThumbRX = thumbRX;
    report.sThumbRY = thumbRY;

    vigem_target_x360_update(m_client, m_pad, report);
}
