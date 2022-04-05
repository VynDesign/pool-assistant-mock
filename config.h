#define CONFIG_DEVICE_NAME "pool-assistant"
#define CONFIG_MQTT_BROKER IPAddress(192, 168, 1, 150)
#define CONFIG_MQTT_SENSOR_CHANNEL_WATER_TEMP "home-assistant/pool-assistant/sensors/pool-water-temp"
#define CONFIG_MQTT_SENSOR_CHANNEL_ORP "home-assistant/pool-assistant/sensors/pool-orp"
#define CONFIG_MQTT_SENSOR_CHANNEL_PH "home-assistant/pool-assistant/sensors/pool-ph"
#define CONFIG_SENSOR_READING_INTERVAL 300000
