#include <Pixy.h>
#include "XLCPixy.h"

XLCPixy<LinkSPI> pixy;

void setup()
{
    Serial.begin(9600);
    pixy.init();
}

void loop()
{
    /*
    while (!Serial.available());
    while (Serial.available()) Serial.read();
    */
    static uint32_t time = millis();
    Serial.println("new loop");
    time = millis();
    uint16_t count = pixy.getBlocks();
    Serial.println(millis() - time);
    for (uint8_t i = 0; i < count; i++) {
        pixy.blocks[i].print();
    }
    /*
    for (uint8_t i = 0; i < pixy.buffer_i; i++) {
        Serial.println(pixy.buffer[i]);
    }
    */
}
