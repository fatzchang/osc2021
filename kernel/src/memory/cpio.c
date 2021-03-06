#include "cpio.h"
#include "string.h"
#include "mini_uart.h"
#include "exception.h"
#include "io.h"
#include "def.h"
#include "process.h"
#include "thread.h"
#include "vfs.h"
#include "tmpfs.h"

void cpio_exec(char *path, int argc, char *argv[])
{
    CPIO_NEWC_HEADER *targetAddr = cpio_find_addr((CPIO_NEWC_HEADER *)RAMFS_ADDR ,path);
    if (targetAddr == 0) {
        printf("could not find the file. \r\n");
    } else {
        int filesize = cpio_attr_value(targetAddr, C_FILESIZE);
        create_process(cpio_content_addr(targetAddr), filesize, argc, argv);
        // printf("cpio executed. \r\n");
    }
}

void cpio_read(char *path)
{
    // start from root address
    CPIO_NEWC_HEADER *targetAddr = cpio_find_addr((CPIO_NEWC_HEADER *)RAMFS_ADDR ,path);
    if (targetAddr == 0) {
        printf("could not find the file. \r\n");
    } else {
        char *content_addr = (char *)cpio_content_addr(targetAddr);
        
        for (int i = 0; i < cpio_attr_value(targetAddr, C_FILESIZE); i++, content_addr++) {
            putchar(*content_addr);
        }

        printf("\r\n");
    }
}

CPIO_NEWC_HEADER * cpio_find_addr(CPIO_NEWC_HEADER *pCurrentFile, char *targetName)
{
    // c_namesize is 1 byte greater than real name size
    int namesize = cpio_attr_value(pCurrentFile, C_NAMESIZE) - 1;

    char curName[20];
    memset(curName, 0, sizeof(char) * 20);

    // get current filename
    char *pTemp = (char *)(pCurrentFile + 1);


    for (int i = 0; i < namesize; i++, pTemp++) {
        curName[i] = *pTemp;
    }

    if (strcmp(curName, "TRAILER!!!") == 0) {
        return 0;
    } else if (strcmp(curName, targetName) == 0) {
        // this file is the target
        return pCurrentFile;
    } else {
        char *pNextFile = (char *)(pCurrentFile + 1);
        pNextFile += namesize;
        
        // aligned multiple of four
        while (((pNextFile - (char *)pCurrentFile) % 4) != 0) {
            *pNextFile = '\0';
            pNextFile++;
        }

        pNextFile += cpio_attr_value(pCurrentFile, C_FILESIZE);
        
        while (((pNextFile - (char *)pCurrentFile) % 4) != 0) {
            *pNextFile = '\0';
            pNextFile++;
        }

        return cpio_find_addr((CPIO_NEWC_HEADER *)pNextFile, targetName);
        
    }

    return 0;
}

// convert cpio header attribute from hex string to int
int cpio_attr_value(CPIO_NEWC_HEADER *pRoot, CPIO_ATTR attr)
{
    char vStr[9];
    memset(vStr, 0, 9);

    char *pTemp = (char *)pRoot;

    if (attr == C_MAGIC) {
       for (int i = 0; i < 6; i ++) {
           vStr[i] = pTemp[i];
       } 
    } else {
        // first attr has only 6 byte
        pTemp += (8 * attr - 2);
        for (int i = 0; i < 8; i ++) {
           vStr[i] = pTemp[i];
       } 
    }

    return hextoi(vStr);
}

void * cpio_content_addr(CPIO_NEWC_HEADER *targetAddr)
{
    char *tmp = (char *)(targetAddr + 1);
    int namesize = cpio_attr_value(targetAddr, C_NAMESIZE) - 1;
    tmp += namesize;
    while (((tmp - (char *)targetAddr) % 4) != 0) {
        *tmp = '\0';
        tmp++;
    }

    return tmp;
}

void cpio_init_fs(struct vnode *root)
{
    CPIO_NEWC_HEADER *pCurrentFile = (CPIO_NEWC_HEADER *)RAMFS_ADDR;

    char curName[20] = { 0 };
    int namesize = 0;

    struct vnode *target = NULL;
    struct tmpfs_internal *target_internal  = NULL;

    for (;;) {
        // clear previous name
        memset(curName, 0, 20);

        // get current filename
        namesize = cpio_attr_value(pCurrentFile, C_NAMESIZE) - 1;
        char *pTemp = (char *)(pCurrentFile + 1);

        for (int i = 0; i < namesize; i++, pTemp++) {
            curName[i] = *pTemp;
        }

        // break if it's the end
        if (strcmp(curName, "TRAILER!!!") == 0) {
            break;
        }

        if (root->v_ops->create(root, &target, curName) != 0) {
            printf("vnode \"%s\" create failed!\n", curName);

            return;
        }

        target_internal = target->internal;
        target_internal->size = cpio_attr_value(pCurrentFile, C_FILESIZE);
        target_internal->start_addr = cpio_content_addr(pCurrentFile);

        // printf("vnode \"%s\"'s content address: %ld\n", curName, target_internal->start_addr);

        // proceed next file
        pCurrentFile = get_next_file(pCurrentFile);
        // printf("hi! name size: %d, name: %s\n", namesize, curName);
    }
}

CPIO_NEWC_HEADER *get_next_file(CPIO_NEWC_HEADER *pCurrentFile)
{
    int namesize = cpio_attr_value(pCurrentFile, C_NAMESIZE) - 1;
    char *pNextFile = (char *)(pCurrentFile + 1);

    pNextFile += namesize;
    
    // aligned multiple of four
    while (((pNextFile - (char *)pCurrentFile) % 4) != 0) {
        *pNextFile = '\0';
        pNextFile++;
    }

    pNextFile += cpio_attr_value(pCurrentFile, C_FILESIZE);
    
    while (((pNextFile - (char *)pCurrentFile) % 4) != 0) {
        *pNextFile = '\0';
        pNextFile++;
    }

    return (CPIO_NEWC_HEADER *)pNextFile;
}