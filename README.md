# Easynow - an ESPHome component to make sensor proxying via ESPNow easy in ESPHome.

### A modified version for a motion detector
# Individuality may be added later

## Example Config Files
If you have a pair of ESP32s and are running ESPHome you can try this right away thanks to ESPHome's ability to pull external components from a git repo.

Just add the following to your sender and receiver ESPHome YAML and fix up the MAC addresses

Also fix up the WiFi channel to be the same as the one you usually use.

You can get the mac address by flashing a default/empty ESPHome YAML and its the first thing the WiFi component outputs. It will be in the form XX:XX:XX:XX:XX:XX

Once these are running, you will see the sensors which are physically on the transmitter appear as entities in HomeAssistant on the receiver.

### Transmitter(s)

```
external_components:
  source:
    type: git
    url: https://github.com/MrFirewall/esphome.git
    ref: main
  refresh: 1min

sensor:
  - platform: esp32_hall
    id: "sensor_hall"
    update_interval: 1s
  - platform: adc
    pin: 34
    id: "sensor_adc"
    update_interval: 1s

proxy_transmitter:
  espnow_channel: 11
  receiver_mac_address: "34:86:5D:FD:84:58"
  deep_sleep_length: 60s  
  sensors:
    - sensor: "sensor_adc"
      proxy_id: "now_transmitter_adc"
    - sensor: "sensor_hall"
      proxy_id: "now_transmitter_hall"
```
### Receiver

```
external_components:
  source:
    type: git
    url: https://github.com/MrFirewall/esphome.git
    ref: main
  refresh: 1min

proxy_receiver:
  espnow_channel: 11
  transmitters:
    - mac_address: "34:86:5D:FC:5E:E0"
      name: now-transmitter
      proxied_sensors:
        - proxy_id: "now_transmitter_hall"
          name: "Now Transmitter Hall"
          unit_of_measurement: "µT"
        - proxy_id: "now_transmitter_adc"
          name: "Now Transmitter ADC"
          unit_of_measurement: "V"

    - mac_address: "C8:F0:9E:51:31:E4"
      name: another-transmitter
      proxied_sensors:
        - proxy_id: "t_c1_hall"
          name: "another-transmitter Hall"
          unit_of_measurement: "µT"
        - proxy_id: "t_c1_adc"
          name: "another-transmitter ADC"
          unit_of_measurement: "V"          
```
#### HomeAssistant proxied sensors:
![Remote Sensors](assets/images/remote-sensors.png)
#### HomeAssistant OTA switch
![Remote Sensors](assets/images/ota-mode-switch.png)
## Overview

Easynow is a pair of components which allow 1 or more battery powered ESP32 devices running [ESPHome](https://esphome.io/) to proxy sensor data over the [ESPNow](https://www.espressif.com/en/products/software/esp-now/overview ) protocol to a grid powered receiver ESP32 device also running ESPHome.

This means that the transmitter uses very little power - it wakes up, spends about 1s grabbing and sending sensor states over ESPNow (which is connectionless), then sleeps for a user defined period. My USB tester reads 0 when its sleeping (and 0.1 when awake on a cheap dev board).

Once configured in the ESPHome YAML (see above for examples) the receiver will have a regular sensor component created for each relayed sensor.

The sensor states will be updated via ESPNow from the remote device(s).

If the receiver is linked to [HomeAssistant](https://www.home-assistant.io/) the proxied sensors will appear automatically in the dashboard, along with a switch for each remote device to set it into OTA mode next time it wakes up.

## What works so far
- Sensor proxying from 1 or more transmitters to 1 receiver
- Configuration using YAML in the usual ESPHome fashion
- Sensors appear on HomeAssistant without any config outside of the 2 components
- A "ota mode" switch appears on HomeAssistant for each remote device.
- WiFi management. When its in safe mode (which can be selected from the HA UI) it has full OTA functionality. When its not, it doesnt turn on WiFi, except enough for ESPNow, which is super quick
- deep Sleep Wakeup with Pin GPIO27

## What doesnt work yet (in priority order)

- Ability to use a unconfigured instance of these components to determine mac address from log
- Support for esp8266 devices (low priority, but no reason to think its a lot of work, it might already be OK)
- Support for other entities like binary sensors

## Authors and acknowledgment
Main Version from [ripnetuk-root](https://gitlab.com/ripnetuk-root/ripnetuk-public/esphome/ripnetuk-esphome-easynow)
Written by George Styles (george@ripnet.co.uk)

Thanks to ssieb on the [ESPHome Discord](https://discord.gg/KhAMKrd) for tirelessly answering mine and others questions, and for suggesting I wrap this stuff up in a proper external component with Python config stuff, rather than continue battling with custom components :)

## License
This project is released under the MIT License.

## How it works
- The components contain a state machine, with states like READY, AWAIT_CHECKIN_ACK etc
- This is needed because ESPHome uses co-operative multi tasking, so we cannot spend more than 20-30ms in the loop
- So we must write it in a way that it does what it has to do, then yields to other components. We cannot wait in the usual maner by using sleep, or busy waiting while loops
- When a transmitter starts up, it sends a CHECKIN messsage to its receiver.
- The receiver responds with a acknowledgment containing a 'go into OTA mode' flag (which it gets from a switch component in the receiver, which in turn is set from the HomeAssistant UI if connected)
- Once the transmitter has got the acknowledgment, it will reboot into OTA mode if the flag is set
- Otherwise, it will set a 'not read' flag for each of its sensor.
- It hooks the updated state callback on each sensor, and as they are read (in whatever way the sensor usually reads states)
- When it receives a callback it marks that sensor as read and captures the state of the sensor
- Once all sensors are read, it moves onto sensing the states back to the receiver
- It sends each state as an ESPNow message along with a proxy_id which links the real sensor in the transmitter to the proxied sensor in the receiver
- Once its done this, it goes into deep sleep for the period defined in the YAML

- When it wakes up, it starts the cycle again
## Version History
### 1.0.0
- Added a counter for deep sleep to make sure that the GPIO27 pin is really LOW/0



