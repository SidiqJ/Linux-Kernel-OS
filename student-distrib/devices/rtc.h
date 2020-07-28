#include "../types.h"
#include "../i8259.h"
#include "../lib.h"

int rtc_test_Flag;

#define IRQ_RTC     0x08
#define RTC_PORT    0x70
#define PORT_A      0x8A
#define PORT_B      0x8B
#define PORT_C      0x8C
#define CMOS_PORT   0x71
#define MASK_40     0x40
#define RATE_MASK   0xF0
#define DEFAULT_2HZ 2 
#define MAX_FREQ    1024

extern void initRTC();
extern void rtcHandler();
extern int rtcOpen(const uint8_t * filename);
extern int rtcRead(int32_t fd, void * buf, int32_t nbytes);
extern int rtcWrite(int32_t fd, const void * buf, int32_t nbytes);
extern int rtcClose(int32_t fd);
extern void changeRtcFreq(int newFreq);


