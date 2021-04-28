#include "io.h"
#include "timer.h"
#include "scheduler.h"

void irq_parser(void *source_addr, long int cntpct_el0)
{
    printf("irq!");
    int source = *((int *)source_addr);
    if (source == 2) {
        core_timer_disable();
        printf("cntpct_el0: %ld\n", cntpct_el0/62500000);
        core_timer_enable();
    }
}


void exception_handler(int svc_num)
{
    switch ( svc_num ) {
        case 0:
            // schedule
            do_schedule();
            break;
        case 1:
            // uart read
            // do_uart_read();
            break;
        case 2:
            // uart write
            // do_uart_write();
            break;
        default:
            break;
    }
}