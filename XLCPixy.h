#ifndef XLCPIXY_H
#define XLCPIXY_H

#include "Arduino.h"
#include "TPixy.h"

#define XLCPIXY_BLOCKS_COUNT 32
#define XLCPIXY_MAX_NO_DATA_TIME 20
#define XLCPIXY_DBG 0
#define XLCPIXY_DBG_BUFF_SIZE 100

template <class LinkType> class XLCPixy
{
public:
    XLCPixy(uint16_t arg=PIXY_DEFAULT_ARGVAL);

    uint16_t getBlocks(uint16_t maxBlocks=XLCPIXY_BLOCKS_COUNT,
                       uint32_t maxDeltaTime=XLCPIXY_MAX_NO_DATA_TIME);
    int8_t setServos(uint16_t s0, uint16_t s1);
    int8_t setBrightness(uint8_t brightness);
    int8_t setLED(uint8_t r, uint8_t g, uint8_t b);
    void init();

    Block blocks[XLCPIXY_BLOCKS_COUNT];
#if XLCPIXY_DBG == 1
    uint16_t buffer[XLCPIXY_DBG_BUFF_SIZE];
    uint16_t buffer_i;
#endif


private:
    boolean getStart(uint32_t maxDeltaTime);

    LinkType link;
    BlockType blockType;
    uint16_t blockCount;
};

template <class LinkType> XLCPixy<LinkType>::XLCPixy(uint16_t arg)
{
    blockCount = 0;
    link.setArg(arg);
}

template <class LinkType> void XLCPixy<LinkType>::init()
{
    link.init();
}

template <class LinkType> boolean
XLCPixy<LinkType>::getStart(uint32_t maxNoDataTime)
{
    uint16_t w, lastw;
    uint32_t noDataTime;

    lastw = 0xffff;

    while(true)
    {
        w = link.getWord();
#if XLCPIXY_DBG == 1
        buffer[buffer_i] = w;
        buffer_i = (buffer_i + 1) % XLCPIXY_DBG_BUFF_SIZE;
#endif
        if (w == 0)
        {
            if (lastw != 0) // start of no data
            {
                noDataTime = millis();
            }
            else if (millis() - noDataTime > maxNoDataTime) // still no data
            {
                delayMicroseconds(10);
                return false;
            }
        }
        else if (w==PIXY_START_WORD && lastw==PIXY_START_WORD)
        {
            blockType = NORMAL_BLOCK;
            return true;
        }
        else if (w==PIXY_START_WORD_CC && lastw==PIXY_START_WORD)
        {
            blockType = CC_BLOCK;
            return true;
        }
        else if (w==PIXY_START_WORDX)
        {
            Serial.println("reorder");
            link.getByte(); // resync
        }
        lastw = w; 
    }
}

template <class LinkType> uint16_t
XLCPixy<LinkType>::getBlocks(uint16_t maxBlocks, uint32_t maxNoDataTime)
{
    uint8_t i;
    uint16_t w, checksum, sum;
    Block *block;
#if XLCPIXY_DBG == 1
    buffer_i = 0;
#endif

    if (maxBlocks > XLCPIXY_BLOCKS_COUNT)
        maxBlocks = XLCPIXY_BLOCKS_COUNT;

    if (getStart(maxNoDataTime) == false)
        return 0;

    for(blockCount=0; blockCount<maxBlocks;)
    {
        checksum = link.getWord();
#if XPCPIXY_DBG == 1
        buffer[buffer_i] = checksum;
        buffer_i = (buffer_i + 1) % XLCPIXY_DBG_BUFF_SIZE;
#endif
        if (checksum==PIXY_START_WORD) // beginning of the next frame
        {
            blockCount = 0;
            blockType = NORMAL_BLOCK;
            checksum = link.getWord();
#if XLCPIXY_DBG == 1
            buffer[buffer_i] = checksum;
            buffer_i = (buffer_i + 1) % XLCPIXY_DBG_BUFF_SIZE;
#endif
        }
        else if (checksum==PIXY_START_WORD_CC)
        {
            blockCount = 0;
            blockType = CC_BLOCK;
            checksum = link.getWord();
#if XLCPIXY_DBG == 1
            buffer[buffer_i] = checksum;
            buffer_i = (buffer_i + 1) % XLCPIXY_DBG_BUFF_SIZE;
#endif
        }
        else if (checksum==0)
            return blockCount;

        block = blocks + blockCount;

        for (i=0, sum=0; i<sizeof(Block)/sizeof(uint16_t); i++)
        {
            if (blockType==NORMAL_BLOCK && i>=5) // skip 
            {
                block->angle = 0;
                break;
            }
            w = link.getWord();
#if XLCPIXY_DBG == 1
            buffer[buffer_i] = w;
            buffer_i = (buffer_i + 1) % XLCPIXY_DBG_BUFF_SIZE; 
#endif
            sum += w;
            *((uint16_t *)block + i) = w;
        }

        if (checksum==sum)
            blockCount++;
        else
            Serial.println("cs error");

        w = link.getWord();
#if XLCPIXY_DBG == 1
        buffer[buffer_i] = w;
        buffer_i = (buffer_i + 1) % XLCPIXY_DBG_BUFF_SIZE;
#endif
        if (w==PIXY_START_WORD)
            blockType = NORMAL_BLOCK;
        else if (w==PIXY_START_WORD_CC)
            blockType = CC_BLOCK;
        else
            return blockCount;
    }
}

template <class LinkType> int8_t
XLCPixy<LinkType>::setServos(uint16_t s0, uint16_t s1)
{
    uint8_t outBuf[6];

    outBuf[0] = 0x00;
    outBuf[1] = 0xff; 
    *(uint16_t *)(outBuf + 2) = s0;
    *(uint16_t *)(outBuf + 4) = s1;

    return link.send(outBuf, 6);
}

template <class LinkType> int8_t
XLCPixy<LinkType>::setBrightness(uint8_t brightness)
{
    uint8_t outBuf[3];

    outBuf[0] = 0x00;
    outBuf[1] = 0xfe; 
    outBuf[2] = brightness;

    return link.send(outBuf, 3);
}

template <class LinkType> int8_t
XLCPixy<LinkType>::setLED(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t outBuf[5];

    outBuf[0] = 0x00;
    outBuf[1] = 0xfd; 
    outBuf[2] = r;
    outBuf[3] = g;
    outBuf[4] = b;

    return link.send(outBuf, 5);
}

#endif /* XLCPIXY_H */
