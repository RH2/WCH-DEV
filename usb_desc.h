#include "ch32v20x.h"
#include "ch32v20x_rcc.h"
#include "usb_lib.h"
#include "debug.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include <stdio.h>


// Global variables
volatile uint32_t ms_count = 0;
char usb_buffer[64];

//// Add to usb_conf.h or your main header file
#define CDC_IN_EP  0x81  // EP1 for TX
//#define CDC_OUT_EP 0x01  // EP1 for RX
//#define CDC_CMD_EP 0x82  // EP2 for commands
//
//// USB device state
//__IO uint32_t bDeviceState = UNCONNECTED;

// System clock initialization
void SystemClock_Config(void) {
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
    RCC_PLLCmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while(RCC_GetSYSCLKSource() != 0x08);
    
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

// SysTick handler for millisecond counting
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void) {
    ms_count++;
    SysTick->SR = 0;
}

// Initialize SysTick for 1ms interrupts
void SysTick_Init(void) {
    SysTick->CTLR = 0;
    SysTick->SR = 0;
    SysTick->CNT = 0;
    SysTick->CMP = SystemCoreClock / 1000 - 1;
    SysTick->CTLR = 0xF;
    NVIC_EnableIRQ(SysTick_IRQn);
}

// USB CDC related functions
void USB_SendData(uint8_t* data, uint16_t length) {
    uint16_t ptr;
    uint16_t CDC_DATA_MAX_PACKET_SIZE = 256;
    if (length > CDC_DATA_MAX_PACKET_SIZE) {
        length = CDC_DATA_MAX_PACKET_SIZE;
    }

    ptr = USB_Tx_Write_Pointer(CDC_IN_EP);
    UserToPMABufferCopy(data, ptr, length);
    USB_Tx_Write_Pointer(CDC_IN_EP, ptr + length);

    SetEPTxCount(CDC_IN_EP, length);
    SetEPTxValid(CDC_IN_EP);
}

// Main application
int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemClock_Config();
    Delay_Init();
    SysTick_Init();
    USB_Init();

    while(1) {
        if (bDeviceState == CONFIGURED) {
            static uint32_t last_send = 0;

            if ((ms_count - last_send) >= 500) {
                last_send = ms_count;

                int len = snprintf(usb_buffer, sizeof(usb_buffer),
                                  "%lu ms: hello wec dev\r\n", ms_count);

                USB_SendData((uint8_t*)usb_buffer, len);
            }
        }
        Delay_Ms(1);
    }
}
