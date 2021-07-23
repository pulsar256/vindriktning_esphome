# Vindriktning PM2.5 Sensor Custom Sensor

Based on findings of https://github.com/Hypfer/esp8266-vindriktning-particle-sensor - uses same hardware setup but with a different firmware approach. This one integrates into [esphome](https://github.com/esphome/esphome).

## Hardware TL;DR:

- Grab a d1 mini, preferably with an usb port (and thus 5V regulator). There seem to be a lot of versions available with slightly different features in that regards.
- Open the IKEA Vindriktning device, notice the testpoints on the edge of the board
- Connect: `v:+5V -> d1:5V`, `v:GND -> d1:G`, `c:REST -> d1:D2`

I still need to figure out how to make it a "proper" esphome compliant external component. For the time being, grab the [components/vindriktning](components/vindriktning) folder, drop it into a directory next to your gadget yaml configuration file so your directory structure looks like this:

```text
(...)
my-gadget.yaml
my_components/vindriktning/vindriktning.h
(...)
```

... and go about the configuration of your device like this:

my-gadget.yaml:

```yaml
esphome:
  name: vindriktning-pm25-01
  platform: ESP8266
  board: d1_mini
  includes:
    - my_components/vindriktning/vindriktning.h
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
logger:

# api, ota, wifi, etc config go here.
```
