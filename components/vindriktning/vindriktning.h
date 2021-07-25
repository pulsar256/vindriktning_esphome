#include "esphome.h"

class VindriktningSensor : public Component, public Sensor, public UARTDevice {

    static constexpr char *const TAG = "vindriktning";
    static constexpr char PACKET_HEADER[3] = {0x16, 0x11, 0x0b};
    static constexpr int PM1006_RESPONSE_SIZE = 20;

    u_char buffer[PM1006_RESPONSE_SIZE];
    int buffer_position = 0;
    long last_time = millis();

   public:
    VindriktningSensor(UARTComponent *parent)
        : Sensor("pm25sensor"), UARTDevice(parent) {}

    void setup() override {
        this->set_icon("mdi:air-filter");
        this->set_unit_of_measurement("µg/m³");
        this->set_accuracy_decimals(0);
    }

    void loop() override {
        while (this->available()) {
            char c = this->read();
            this->fill_buffer_(&c);
            if (this->buffer_ready_()) this->parse_buffer_();
        }
    }

   protected:
    void fill_buffer_(char *c) {
        long current_time = millis();
        long time_delta = (current_time - last_time);
        // packets are at least 800ms apart
        if (time_delta > 800) {
            ESP_LOGV(TAG,
                     "flushing buffer, current bufferPos %u, time delta %d",
                     this->buffer_position, delta);
            buffer_position = 0;
        }
        if (buffer_position < PM1006_RESPONSE_SIZE) buffer[buffer_position++] = *c;
        last_time = current_time;
    }

    bool buffer_ready_() { return buffer_position == PM1006_RESPONSE_SIZE; }

    void parse_buffer_() {
        for (int i = 0; i < sizeof(PACKET_HEADER) / sizeof(PACKET_HEADER[0]);
             i++) {
            if (buffer[i] != PACKET_HEADER[i]) {
                ESP_LOGW(TAG, "Invalid packet header, cannot parse");
                return;
            }
        }

        u_char checksum = 0;
        for (int i = 0; i < PM1006_RESPONSE_SIZE; i++) {
            checksum = +buffer[i];
        }

        if (buffer[PM1006_RESPONSE_SIZE - 1] != checksum) {
            ESP_LOGW(TAG,
                     "invalid checksum (computed %02X vs. received %02X), "
                     "ignoring reading",
                     checksum, buffer[PM1006_RESPONSE_SIZE - 1]);
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

        buffer_position = 0;
    }
};
