#include "esphome.h"

static const char *const TAG = "vindriktning";
#define PACKET_SIZE 20

class VindriktningSensor : public Component, public Sensor, public UARTDevice {
    const char packetHeader[3] = {0x16, 0x11, 0x0b};
    u_char buffer[PACKET_SIZE];
    int bufferPos = 0;
    long lastTime = millis();

   public:
    VindriktningSensor(UARTComponent *parent)
        : Sensor("pm25sensor"), UARTDevice(parent) {}

    void setup() override {
        set_icon("mdi:air-filter");
        set_unit_of_measurement("µg/m³");
        set_accuracy_decimals(0);
    }

    void loop() override {
        while (available()) {
            char c = read();
            fillBuffer(&c);
            if (bufferReady()) parseBuffer();
        }
    }

   private:
    void fillBuffer(char *c) {
        long currentTime = millis();
        long delta = (currentTime - lastTime);
        // packets are at least 800ms apart
        if (delta > 800) {
            ESP_LOGV(TAG,
                     "flushing buffer, current bufferPos %u, time delta %d",
                     this->bufferPos, delta);
            bufferPos = 0;
        }
        if (bufferPos < PACKET_SIZE) buffer[bufferPos++] = *c;
        lastTime = currentTime;
    }

    bool bufferReady() { return bufferPos == PACKET_SIZE; }

    void parseBuffer() {
        for (int i = 0; i < sizeof(packetHeader) / sizeof(packetHeader[0]);
             i++) {
            if (buffer[i] != packetHeader[i]) {
                ESP_LOGW(TAG, "Invalid packet header, cannot parse");
                return;
            }
        }

        u_char checksum = 0;
        for (int i = 0; i < PACKET_SIZE; i++) {
            checksum = +buffer[i];
        }

        if (buffer[PACKET_SIZE - 1] != checksum) {
            ESP_LOGW(TAG,
                     "invalid checksum (computed %02X vs. received %02X), "
                     "ignoring reading",
                     checksum, buffer[PACKET_SIZE - 1]);
            return;
        }

        auto reading = ((((short)buffer[5]) << 8) | (0x00ff & buffer[6]));
        this->publish_state(reading);

        ESP_LOGD(TAG,
                 "time: %d, reading: %d,"
                 "buffer=[%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%"
                 "02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X]",
                 millis(), reading, buffer[0], buffer[1], buffer[2], buffer[3],
                 buffer[4], buffer[5], buffer[6], buffer[7], buffer[8],
                 buffer[9], buffer[10], buffer[11], buffer[12], buffer[13],
                 buffer[14], buffer[15], buffer[16], buffer[17], buffer[18],
                 buffer[19]);

        bufferPos = 0;
    }
};
