# Vindriktning PM2.5 Sensor Custom Sensor

Based on findings of https://github.com/Hypfer/esp8266-vindriktning-particle-sensor - uses same hardware setup but with a different firmware approach. This one integrates into [esphome](https://github.com/esphome/esphome).

Detailed documentation will follow.

```yaml
Configuration Example:

uart:
  id: uart_bus
  tx_pin: D0
  rx_pin: D2
  baud_rate: 9600

sensor:
- platform: custom
  lambda: |-
    auto my_sensor = new VindriktningSensor(id(uart_bus));
    App.register_component(my_sensor);
    return {my_sensor};

  sensors:
    name: "PM2.5 Sensor"
    unit_of_measurement: µg/m³
    accuracy_decimals: 0
```
