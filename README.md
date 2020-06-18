# ESP8266 Flash Destroyer

Don't run this program.

It intentionally wears out a sector in the flash chip on the ESP8266.

The flash memory used on ESP8266 and ESP32 boards is cheap, NOR-based, SPI flash. It has a life cycle of between 10,000 to 100,000 writes.

Flash filesystems like SPIFFS and LittleFS do wear-leveling: software avoids rewriting the same flash page. This helps prolong the lifespan of the flash chip.

