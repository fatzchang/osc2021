#include "peripheral.h"
#include "mini_uart.h"
#include "base_ops.h"

void init_uart()
{
    unsigned int selector;

    selector = get32(GPFSEL1);
    selector &= ~(7<<12);                   // clean gpio14
    selector |= 2<<12;                      // set alt5 for gpio14
    selector &= ~(7<<15);                   // clean gpio15
    selector |= 2<<15;                      // set alt5 for gpio 15
    put32(GPFSEL1,selector);

    put32(GPPUD,0);
    wait(150);
    put32(GPPUDCLK0,(1<<14)|(1<<15));
    wait(150);
    put32(GPPUDCLK0,0);

    put32(AUXENB, 1);
    put32(AUX_MU_CNTL_REG, 0);
    put32(AUX_MU_IER_REG, 0);
    put32(AUX_MU_LCR_REG, 3);
    put32(AUX_MU_MCR_REG, 0);
    put32(AUX_MU_BAUD_REG, 270); // set baud rate
    put32(AUX_MU_IIR_REG, 6);
    put32(AUX_MU_CNTL_REG, 3);
}

int putchar(char c)
{
    while(1) {
        if (get32(AUX_MU_LSR_REG) & 0x20) {
            break;
        }
    }

    put32(AUX_MU_IO_REG, (unsigned char)c);

    return (unsigned char)c;
}

int puts(const char *str)
{
    int i = 0;
    while(str[i] != '\0') {
        putchar(str[i]);
        i++;
    }

    return 0;
}

int getchar()
{
    while(1) {
        if (get32(AUX_MU_LSR_REG) & 0x01) {
            break;
        }
    }
    return (get32(AUX_MU_IO_REG) & 0xFF);
}