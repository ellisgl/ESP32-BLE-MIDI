#include "GLBLEMidiServer.h"

CharacteristicCallback::CharacteristicCallback(std::function<void(uint8_t*, uint8_t)> onWriteCallback) : onWriteCallback(onWriteCallback) {}

void CharacteristicCallback::onWrite(BLECharacteristic *pCharacteristic)
{
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0 && onWriteCallback != nullptr)
        onWriteCallback((uint8_t*)rxValue.c_str(), rxValue.length());

    vTaskDelay(0);      // We leave some time for the IDLE task call esp_task_wdt_reset_watchdog
                        // See comment from atanisoft here : https://github.com/espressif/arduino-esp32/issues/2493

}

void GLBLEMidiServerClass::begin(const std::string deviceName)
{
    GLBLEMidi::begin(deviceName);
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);
    BLEService *pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));
    pCharacteristic = pService->createCharacteristic(
        BLEUUID(MIDI_CHARACTERISTIC_UUID),
        NIMBLE_PROPERTY::READ   |
        NIMBLE_PROPERTY::WRITE  |
        NIMBLE_PROPERTY::NOTIFY |
        NIMBLE_PROPERTY::WRITE_NR
    );
    pCharacteristic->setCallbacks(
        new CharacteristicCallback(
            [this](uint8_t *data, uint8_t size) {
                this->receivePacket(data, size);
             }
        )
    );

    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();
}

void GLBLEMidiServerClass::begin(const std::string deviceName, CharacteristicCallback *pCharacteristicCallback)
{
    GLBLEMidi::begin(deviceName);
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);
    BLEService *pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));
    pCharacteristic = pService->createCharacteristic(
        BLEUUID(MIDI_CHARACTERISTIC_UUID),
        NIMBLE_PROPERTY::READ   |
        NIMBLE_PROPERTY::WRITE  |
        NIMBLE_PROPERTY::NOTIFY |
        NIMBLE_PROPERTY::WRITE_NR
    );
    pCharacteristic->setCallbacks(pCharacteristicCallback);
    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();
}

void GLBLEMidiServerClass::setOnConnectCallback(void (*const onConnectCallback)())
{
    this->onConnectCallback = onConnectCallback;
}

void GLBLEMidiServerClass::setOnDisconnectCallback(void (*const onDisconnectCallback)())
{
    this->onDisconnectCallback = onDisconnectCallback;
}

void GLBLEMidiServerClass::sendPacket(uint8_t *packet, uint8_t packetSize)
{
    if(!connected)
        return;
    pCharacteristic->setValue(packet, packetSize);
    pCharacteristic->notify();
}

void GLBLEMidiServerClass::onConnect(BLEServer* pServer)
{
    connected = true;
    if(onConnectCallback != nullptr)
        onConnectCallback();
}

void GLBLEMidiServerClass::onDisconnect(BLEServer* pServer)
{
    connected = false;
    if(onDisconnectCallback != nullptr)
        onDisconnectCallback();
    pServer->startAdvertising();
}

GLBLEMidiServerClass GLBLEMidiServer;