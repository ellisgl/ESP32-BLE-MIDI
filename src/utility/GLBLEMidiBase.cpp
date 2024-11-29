#include <Arduino.h>
#include "GLBLEMidiBase.h"

void GLBLEMidi::begin(const std::string deviceName)
{
    this->deviceName = deviceName;
    BLEDevice::init(deviceName);
}

void GLBLEMidi::end() {
    BLEDevice::deinit();
}

bool GLBLEMidi::isConnected()
{
    return connected;
}