#include <Arduino.h>
#include <EEPROM.h>

/*
 * demonstration - destroy flash memory
 *
 * write to flash
 * sleep for one second
 * do it again
 * wheel turnin' round and round
 */

#include "ESP8266TrueRandom.h"

// https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/RTCUserMemory/RTCUserMemory.ino

struct {
  uint32_t crc32;
  uint32_t counter;
  uint32_t data;
} rtcData, eepromData;

uint32_t calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

void readEEPROM(), writeEEPROM();

void setup() {
  EEPROM.begin(256);
  Serial.begin(74880);

  Serial.println("hello world");
  delay(500);

#if 0
  Serial.println(EEPROM.read(0));
  EEPROM.write(0, EEPROM.read(0) + 1);
  EEPROM.commit();

  delay(1000);
  ESP.restart();
#endif

  rtcData.crc32 = 0;
  rtcData.counter = 0;
  rtcData.data = 0;

  eepromData.crc32 = 0;
  eepromData.counter = 0;
  eepromData.data = 0;

  if(!ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    Serial.println("RTC read failed");
    while(1)
      delay(1);
  }

  // if the RTC data isn't right, this is our first run
  if(calculateCRC32((const uint8_t *)&rtcData.counter, sizeof(rtcData.counter)*2) != rtcData.crc32) {
    Serial.println("RTC mismatch, assuming first run");

    Serial.print("Flash chip ID ");
    Serial.println(ESP.getFlashChipId());

    Serial.print("Flash chip vendor ID ");
    Serial.println(ESP.getFlashChipVendorId());

    Serial.print("Flash chip size ");
    Serial.println(ESP.getFlashChipSize());

    Serial.print("Flash chip speed ");
    Serial.println(ESP.getFlashChipSpeed());

    Serial.print("Flash chip mode ");
    Serial.println(ESP.getFlashChipMode());

    rtcData.counter = 0;
  } else {
    readEEPROM();

    if((calculateCRC32((const uint8_t *)&eepromData.counter, sizeof(eepromData.counter)*2) != eepromData.crc32) ||
       (eepromData.counter != rtcData.counter)) {
      Serial.println("EEPROM fail");
      Serial.printf("EEPROM counter %u, data %8x, crc %08x\n", eepromData.counter, eepromData.data, eepromData.crc32);
      Serial.printf("RTC counter %u, data %8x, crc %08x\n", rtcData.counter, rtcData.data, rtcData.crc32);
    } else {
      Serial.printf("Iteration %u okay\n", rtcData.counter);
    }
  }

  rtcData.counter++;
  rtcData.data = (uint32_t)ESP8266TrueRandom.random();
  rtcData.crc32 = calculateCRC32((const uint8_t *)&rtcData.counter, sizeof(rtcData.counter)*2);

  if(calculateCRC32((const uint8_t *)&rtcData.counter, sizeof(rtcData.counter)*2) != rtcData.crc32) {
    Serial.println("generated CRC fail");
    while(1)
      delay(1);
  }  

  if(!ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    Serial.println("RTC write failed");
    while(1)
      delay(1);
  }

  eepromData.counter = rtcData.counter;
  eepromData.data = rtcData.data;
  eepromData.crc32 = rtcData.crc32;

  writeEEPROM();

  delay(200);
  ESP.reset();
}

void loop() {
}

void readEEPROM() {
  byte *ptr = (byte *)&eepromData;

  for(unsigned i = 0; i < sizeof(eepromData); i++) {
    ptr[i] = EEPROM.read(i);
  }
}

void writeEEPROM() {
  byte *ptr = (byte *)&eepromData;

  for(unsigned i = 0; i < sizeof(eepromData); i++) {
    EEPROM.write(i, ptr[i]);
  }

  EEPROM.commit();
}
