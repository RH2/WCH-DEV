#include "ch32v20x.h"
#include "debug.h"
#include <string.h>
#include <math.h>


#define NUM_LEDS 3
#define CYCLES_PER_NS (144000000 / 1000000000)

typedef struct {
    uint8_t green;
    uint8_t red;
    uint8_t blue;
} LEDColor;
LEDColor leds[NUM_LEDS];

void GPIO_Config(void);
static inline void delay_ns(uint32_t ns);
void sendByte(uint8_t byte);
void updateLEDs(void);
void setAllLEDs(uint8_t r, uint8_t g, uint8_t b);
void HSLtoRGB(float h, float s, float l, uint8_t *r, uint8_t *g, uint8_t *b);
void SystemInit2(void)
{
    RCC_DeInit();

    // Enable HSE (High-Speed External clock), or use HSI (internal)
    RCC_HSEConfig(RCC_HSE_OFF);     // Use internal HSI (8MHz RC oscillator)
    RCC_HSICmd(ENABLE);             // Make sure HSI is enabled
    while(!RCC_GetFlagStatus(RCC_FLAG_HSIRDY));

    // Configure Flash latency
//    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
//    FLASH_SetLatency(FLASH_Latency_2); // 2 wait states for 144MHz

    // PLL configuration: input = HSI (divided), output = 144MHz
    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_18); // 8MHz/2 * 36 = 144MHz
    RCC_PLLCmd(ENABLE);
    while(!RCC_GetFlagStatus(RCC_FLAG_PLLRDY));

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while(RCC_GetSYSCLKSource() != 0x08);

    // Update system core clock variable
    SystemCoreClockUpdate();
}



void GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}
static inline void delay_ns(uint32_t ns) {
    uint32_t cycles = ns * CYCLES_PER_NS;
    if (cycles < 10) cycles = 10;
    while (cycles--) {
        __asm__ volatile ("nop");
    }
}
void sendByte(uint8_t byte) {
    for (uint8_t i = 8; i > 0; i--) {
        if (byte & 0x80) {
            GPIOA->BSHR = GPIO_Pin_15;
            delay_ns(350);
            GPIOA->BCR = GPIO_Pin_15;
            delay_ns(800);
        } else {
            GPIOA->BSHR = GPIO_Pin_15;
            delay_ns(200);
            GPIOA->BCR = GPIO_Pin_15;
            delay_ns(1300);
        }
        byte <<= 1;
    }
}
void updateLEDs(void) {
    GPIO_ResetBits(GPIOA, GPIO_Pin_15);
    Delay_Us(50);

    for(int i = 0; i < NUM_LEDS; i++) {
        sendByte(leds[i].green);
        sendByte(leds[i].red);
        sendByte(leds[i].blue);
    }

    Delay_Us(50);
}
void setSingleLED(uint8_t i,uint8_t r, uint8_t g, uint8_t b){
    leds[i].red = r;
    leds[i].green = g;
    leds[i].blue = b;
}
void setAllLEDs(uint8_t r, uint8_t g, uint8_t b) {
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i].red = r;
        leds[i].green = g;
        leds[i].blue = b;
    }
}
float rfmod(float x, float y) {
    return x - (int)(x/y) * y;
}
float rfabs(float x) {
    return x < 0 ? -x : x;
}
void HSLtoRGB(float h, float s, float l, uint8_t *r, uint8_t *g, uint8_t *b) {
    h = rfmod(h, 360.0f);
    s = s > 1.0f ? 1.0f : s < 0.0f ? 0.0f : s;
    l = l > 1.0f ? 1.0f : l < 0.0f ? 0.0f : l;

    float c = (1.0f - rfabs(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - rfabs(rfmod(h / 60.0f, 2.0f) - 1.0f));
    float m = l - c / 2.0f;

    float rp, gp, bp;

    if(h < 60.0f) { rp = c; gp = x; bp = 0.0f; }
    else if(h < 120.0f) { rp = x; gp = c; bp = 0.0f; }
    else if(h < 180.0f) { rp = 0.0f; gp = c; bp = x; }
    else if(h < 240.0f) { rp = 0.0f; gp = x; bp = c; }
    else if(h < 300.0f) { rp = x; gp = 0.0f; bp = c; }
    else { rp = c; gp = 0.0f; bp = x; }

    *r = (uint8_t)((rp + m) * 255.0f);
    *g = (uint8_t)((gp + m) * 255.0f);
    *b = (uint8_t)((bp + m) * 255.0f);
}

int main(void) {
    SystemInit();
    Delay_Init();
    USART_Printf_Init(115200); // Needed for SDI print



    SystemInit2(); //handles clock
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Delay_Init();

    GPIO_Config();

    memset(leds, 0, sizeof(leds));
    updateLEDs();

    float hue = 0.0f;
    float hue2 = 0.3f;
    float hue3 = 0.6f;

    Delay_Ms(1000);

    while(1) {
        uint8_t r, g, b;
        uint8_t r2, g2, b2;
        uint8_t r3, g3, b3;
        HSLtoRGB(hue, 0.8f, 0.1f, &r, &g, &b);
        setSingleLED(0,r,g,b);
        HSLtoRGB(hue2, 0.8f, 0.1f, &r2, &g2, &b2);
        setSingleLED(1,r2,g2,b2);
        HSLtoRGB(hue3, 0.8f, 0.1f, &r3, &g3, &b3);
        setSingleLED(2,r3,g3,b3);

//        setAllLEDs(r, g, b);


        updateLEDs();

        hue += 1.0f;
        if(hue >= 360.0f) hue = 0.0f;

        Delay_Ms(20);
    }
}
