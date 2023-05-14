#include "esphome.h"

class VindriktningSensor : public Component, public Sensor, public UARTDevice {
    static constexpr char *const TAG = "vindriktning";
    static constexpr char PM1006_RESPONSE_HEADER[3] = {0x16, 0x11, 0x0b};
    static constexpr int PM1006_RESPONSE_SIZE = 20;

    u_char uart_buffer[PM1006_RESPONSE_SIZE];
    int uart_buffer_position = 0;
    long time_last_uart_byte_received = millis();

   public:
    VindriktningSensor(UARTComponent *parent)
        : Sensor(), UARTDevice(parent) {}

    void setup() override {
        this->set_icon("mdi:air-filter");
        this->set_unit_of_measurement("µg/m³");
        this->set_accuracy_decimals(0);
    }

    void loop() override {
        while (this->available()) {
            char c = this->read();
            this->fill_uart_buffer_(&c);
            if (this->uart_buffer_ready_())
                this->parse_uart_buffer_and_publish_reading_();
        }
    }

   protected:
    void fill_uart_buffer_(char *c) {
        long current_time = millis();
        long time_delta_last_uart_byte_received =
            (current_time - time_last_uart_byte_received);

        // packets are at least 1 second apart, hold off time of 800ms is
        // on the safe side to reset the uart_buffer_position and assume
        // beginning of a new response
        if (time_delta_last_uart_byte_received > 800) uart_buffer_position = 0;

        if (uart_buffer_position < PM1006_RESPONSE_SIZE)
            uart_buffer[uart_buffer_position++] = *c;
        time_last_uart_byte_received = current_time;
    }

    bool uart_buffer_ready_() {
        return uart_buffer_position == PM1006_RESPONSE_SIZE;
    }

    void parse_uart_buffer_and_publish_reading_() {
        for (int i = 0; i < sizeof(PM1006_RESPONSE_HEADER) /
                                sizeof(PM1006_RESPONSE_HEADER[0]);
             i++) {
            if (uart_buffer[i] != PM1006_RESPONSE_HEADER[i]) {
                ESP_LOGW(TAG, "Invalid packet header, cannot parse");
                return;
            }
        }

        u_char checksum = 0;
        for (int i = 0; i < PM1006_RESPONSE_SIZE; i++) {
            checksum = +uart_buffer[i];
        }

        if (uart_buffer[PM1006_RESPONSE_SIZE - 1] != checksum) {
            ESP_LOGW(TAG,
                     "invalid checksum (computed %02X vs. received %02X), "
                     "ignoring reading",
                     checksum, uart_buffer[PM1006_RESPONSE_SIZE - 1]);
            return;
        }

        auto reading =
            ((((short)uart_buffer[5]) << 8) | (0x00ff & uart_buffer[6]));
        this->publish_state(reading);

        ESP_LOGV(
            TAG,
            "time: %d, reading: %d,"
            "uart_buffer=[%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%"
            "02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X]",
            millis(), reading, uart_buffer[0], uart_buffer[1], uart_buffer[2],
            uart_buffer[3], uart_buffer[4], uart_buffer[5], uart_buffer[6],
            uart_buffer[7], uart_buffer[8], uart_buffer[9], uart_buffer[10],
            uart_buffer[11], uart_buffer[12], uart_buffer[13], uart_buffer[14],
            uart_buffer[15], uart_buffer[16], uart_buffer[17], uart_buffer[18],
            uart_buffer[19]);

        uart_buffer_position = 0;
    }
};
