#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

void sys_schedule();
void sys_uart_read();
void sys_uart_write();
void sys_exec(char *name, char **argv);

#endif