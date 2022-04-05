# Pool Assistant (mock)
A sketch for Arduino Uno Wifi Rev. 2 emulating pH, ORP, and temperature sensors that sends values to an MQTT broker, such as Mosquitto for Home Assistant

## Purpose
Test connectivity between the Arduino board and an instance of Home Assistant running Mosquitto Broker, ensuring that the MQTT messages are recieved and can be handled by automations, scripts, or Lovelace dashboard elements.

Ultimately, this Arduino will be outfitted with a Whitebox T2 and connected to actual pH, ORP, and Temperature probes and Atlas Scientific EZO circuits, sending actual data from a swimming pool circulation system in order to remotely keep tabs on chemical levels.

## Hardware
[Arduino Uno Wifi Rev. 2](http://store-usa.arduino.cc/products/arduino-uno-wifi-rev2)

## Dependencies
The following libraries may need to be downloaded:
* WiFiNINA.h
* ArduinoMqttClient.h

That's it!

## Secrets
The file included in this repo called "secrets_example.h" provides the properties that should be present in the expected "secrets.h" file that is *includ*ed in the main sketch. These properties are:
* SECRET_WIFI_SSID
* SECRET_WIFI_PASS
* SECRET_MQTT_USER
* SECRET_MQTT_PASS

These values should be edited to match your specific setup. They should be relatively self-explanatory.

## Configuration
The file "config.h" provides relatively non-sensitive configuration options. Some may suit your needs as-is, or (like the IP Address of your MQTT broker) may need to be edited to your specific setup. The MQTT channels should match those that are configured in your MQTT subscriber, if they are modified from the examples provided further below. These properties include:

#### CONFIG_DEVICE_NAME
- a unique name used to identify this device when connecting to the MQTT broker
#### CONFIG_MQTT_BROKER
- the IP Address of the MQTT broker. If using a FQDN, the type should be changed from IPAddress to char[]
#### CONFIG_MQTT_SENSOR_CHANNEL_WATER_TEMP
- the topic name/path for the water temperature sensor readings
#### CONFIG_MQTT_SENSOR_CHANNEL_ORP
- the topic name/path for the ORP sensor readings
#### CONFIG_MQTT_SENSOR_CHANNEL_PH
- the topic name/path for the pH sensor readings
#### CONFIG_SENSOR_READING_INTERVAL
- the time in milliseconds between readings
<br />
<br />
<br />

# Configuration - Home Assistant
First, install and configure the Mosquitto Broker add-on for HomeAssistant if you've not already done so. See https://www.home-assistant.io/integrations/mqtt/

Next, we will configure three sensors using the "mqtt" platform that will be the receivers of the data being published by the Arduino. This will be done via configuration yaml file edits. I have a 'sensors.yaml' file being included in the main 'configuration.yaml' file, but there are other ways of doing this, so your configuration may need some tweaking. These three sensors are defined as:

```
- platform: mqtt
  name: "Pool Assistant - pH"
  state_topic: "home-assistant/pool-assistant/sensors/pool-ph"
  unit_of_measurement: "No."
- platform: mqtt
  name: "Pool Assistant - ORP"
  state_topic: "home-assistant/pool-assistant/sensors/pool-orp"
  device_class: "voltage"
  unit_of_measurement: "mV"
- platform: mqtt
  name: "Pool Assistant - Temperature"
  state_topic: "home-assistant/pool-assistant/sensors/pool-water-temp"
  device_class: "temperature"
  unit_of_measurement: "°F"
```

For good measure, I've also added these sensors to a group in groups.yaml:

```
Pool Assistant:
  entities:
    - sensor.pool_assistant_ph
    - sensor.pool_assistant_orp
    - sensor.pool_assistant_temp
```

Once that is in place, you should be able to run the sketch locally and see the sensors get updated based on the interval set in the configurations. I attempted at this time to add these sensors to a Lovelace dashhboard using the Gauge card, only to find that they are not recognized as numeric. I then created (via the UI) three "input_number" helpers to act as the numeric placeholder for these values and a script to transpose the value from the sensor to the input_number entity, but the current UI implementation of input_number only allows for a "step" value in whole numbers. Since I wanted at least one decimal point of precision, I added an "input_number.yaml" configuration file and defined the following:

```
pool_orp:
  name: Pool ORP
  min: 0.0
  max: 1500.0
  step: 0.1
  icon: mdi:sine-wave
  unit_of_measurement: mV

pool_ph:
  name: Pool pH
  min: 0.0
  max: 14.0
  step: 0.1
  icon: mdi:flask

pool_temp:
  name: Pool Temperature
  min: 0.0
  max: 200.0
  step: 0.1
  icon: mdi:coolant-temperature
  unit_of_measurement: °F
  ```

Then I wrote the automation to update the values of each input_number based on the state change trigger associated to each MQTT sensor:

```
alias: Pool Sensor Change
description: ''
trigger:
  - platform: state
    id: pool_orp_change
    entity_id: sensor.pool_assistant_orp
  - platform: state
    entity_id: sensor.pool_assistant_ph
    id: pool_ph_change
  - platform: state
    entity_id: sensor.pool_assistant_temperature
    id: pool_water_temp
condition: []
action:
  - choose:
      - conditions:
          - condition: trigger
            id: pool_orp_change
        sequence:
          - service: input_number.set_value
            data:
              value: '{{ states(''sensor.pool_assistant_orp'') | float }}'
            target:
              entity_id: input_number.pool_orp
      - conditions:
          - condition: trigger
            id: pool_ph_change
        sequence:
          - service: input_number.set_value
            data:
              value: '{{ states(''sensor.pool_assistant_ph'') | float }}'
            target:
              entity_id: input_number.pool_ph
      - conditions:
          - condition: trigger
            id: pool_water_temp
        sequence:
          - service: input_number.set_value
            data:
              value: '{{ states(''sensor.pool_assistant_temperature'') | float }}'
            target:
              entity_id: input_number.pool_temp
    default: []
mode: single
```

Finally, pulling it all together, I created the Lovelace dashboard view to display a gauge card, sensor timeline card, and entity card stacked inside inside a grid card for each of the three sensors. This ends up looking like:

![Screen Shot 2022-04-05 at 2 06 28 PM](https://user-images.githubusercontent.com/882135/161823170-a8b069d3-a2a8-4271-8da0-2a64832ba27b.png)

Here is the backing YAML for that specific view:

```
  - theme: Backend-selected
    title: Pool
    path: pool
    icon: mdi:waves
    badges: []
    cards:
      - square: false
        columns: 1
        type: grid
        cards:
          - type: gauge
            entity: input_number.pool_orp
            max: 1500
            min: 0
            needle: true
            name: ORP
          - hours_to_show: 24
            graph: line
            type: sensor
            entity: input_number.pool_orp
            detail: 2
            unit: mV
            name: Pool ORP Over Time
          - type: entities
            entities:
              - entity: sensor.pool_assistant_orp
      - square: false
        columns: 1
        type: grid
        cards:
          - type: gauge
            entity: input_number.pool_ph
            max: 14
            min: 0
            name: pH
            unit: ' '
            needle: true
          - hours_to_show: 24
            graph: line
            type: sensor
            entity: input_number.pool_ph
            detail: 2
            icon: mdi:flask
            name: Pool pH Over Time
          - type: entities
            entities:
              - entity: sensor.pool_assistant_ph
                icon: mdi:flask
      - square: false
        columns: 1
        type: grid
        cards:
          - type: gauge
            entity: input_number.pool_temp
            name: Water Temperature
            unit: °F
            min: 0
            max: 200
            needle: true
            severity:
              green: 90
              yellow: 90
              red: 140
          - hours_to_show: 24
            graph: line
            type: sensor
            entity: input_number.pool_temp
            detail: 2
            name: Pool Water Temp Over Time
          - type: entities
            entities:
              - entity: sensor.pool_assistant_temperature
```
