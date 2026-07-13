#pragma once
#include <windows.h>
#include <ViGEm/Client.h>
#include <iostream>
#include <cstdint>

class InputInjector {
public:
    InputInjector();
    ~InputInjector();

    bool Initialize();
    
    // Injects an Xbox 360 controller state
    void SetGamepadState(uint16_t buttons, uint8_t leftTrigger, uint8_t rightTrigger, 
                         int16_t thumbLX, int16_t thumbLY, int16_t thumbRX, int16_t thumbRY);
                         
    void Cleanup();

private:
    PVIGEM_CLIENT m_client = nullptr;
    PVIGEM_TARGET m_pad = nullptr;
};
