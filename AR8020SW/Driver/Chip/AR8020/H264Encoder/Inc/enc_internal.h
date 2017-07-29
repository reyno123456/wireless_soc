#ifndef ENC_INTERNAL_H
#define ENC_INTERNAL_H

#define abs(num) (num > 0 ? num : -num)

#define IN_CLK                  166000000
#define VSOC_GLOBAL2_BASE       0xA0030000
#define SDRAM_INIT_DONE         0x24    //sdram init done

#define REG_BASE_ADDR           0xA0010000

//Write or Read Byte_Word
#define REG8(add)  *((volatile unsigned char *)  (add))
#define REG16(add) *((volatile unsigned short *) (add))
#define REG32(add) *((volatile unsigned long *)  (add))

#define WRITE_WORD(addr, value) \
        do { \
              ((*(volatile unsigned int *) (addr)) = (value)); \
           } while(0)

#define READ_WORD(addr, value) \
        do { \
                ( value = (*(volatile unsigned int *) (addr)) ); \
           } while(0)


#define WRITE_BYTE(addr, value) \
        do { \
              ((*(volatile unsigned char *) (addr)) = (value)); \
           } while(0)

#define READ_BYTE(addr, value) \
        do { \
                (  value = (*(volatile unsigned char *) (addr)) ); \
           } while(0)

#define WRITE_WORD_ENC(addr, value) \
        do { \
              ((*(volatile unsigned int *) (REG_BASE_ADDR + addr)) = (value)); \
           } while(0)

#define READ_WORD_ENC(addr, value) \
        do { \
                ( value = (*(volatile unsigned int *) (REG_BASE_ADDR + addr)) ); \
           } while(0)

// Internal APIs
void encoder_delay(unsigned int delay);
void sdram_init_check(void);

typedef struct
{
    unsigned char running;
    unsigned int  resW;
    unsigned int  resH;
    unsigned char gop;
    unsigned char framerate;
    unsigned char bitrate;
    unsigned char brc_enable;
    unsigned char src;
    unsigned char over_flow;
    //unsigned int  bu_cnt;
} STRU_EncoderStatus;

#endif

