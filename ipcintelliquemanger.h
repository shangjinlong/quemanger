
#ifndef __IPC_INTELLI_QUE_MANGER_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#define __IPC_INTELLI_QUE_MANGER_H__

#include <pthread.h>
#include <stdio.h>

#define IPC_INTELLI_SOK                 (0)
#define IPC_INTELLI_SFAIL               (-1)
#define IPC_INTELLI_ASSERT(x)  \
            { \
                if( (x) == 0) { \
                    fprintf(stderr, " ASSERT (%s|%s|%d)\r\n", __FILE__, __func__, __LINE__); \
                    while (getchar()!='q'); \
                } \
            } 

typedef enum
{
    IPC_INTELLI_EMPTY_BUFFER_FLAG = 0,
    IPC_INTELLI_FULL_BUFFER_FLAG 
}EIntelliBufType;

typedef enum
{
    IPC_INTELLI_TIMEOUT_NONE = 0,
    IPC_INTELLI_TIMEOUT_2MSEC = 2,
}EIntelliBufTimeOutType;

typedef struct
{
    unsigned int curRd;
    unsigned int curWr;
    unsigned int len;
    unsigned int count;

    int *queue;

    pthread_mutex_t lock;
    pthread_cond_t  condRd;
    pthread_cond_t  condWr;
}TIpcIntelliQueHndl;

typedef struct
{
    TIpcIntelliQueHndl emptyQue;
    TIpcIntelliQueHndl fullQue;
}TIpcIntelliBufHndl;

typedef struct 
{
    pthread_mutex_t lock;
}TIpcIntelliMutexHndl;

int IpcIntelliMutexCreate(TIpcIntelliMutexHndl *hndl);
int IpcIntelliMutexLock(TIpcIntelliMutexHndl *hndl);
int IpcIntelliMutexUnlock(TIpcIntelliMutexHndl *hndl);
int IpcIntelliQueCreate(TIpcIntelliQueHndl *hndl, unsigned int maxLen);
int IpcIntelliQueDelete(TIpcIntelliQueHndl *hndl);
int IpcIntelliQuePut(TIpcIntelliQueHndl *hndl, int value, unsigned int timeout);
int IpcIntelliQueGet(TIpcIntelliQueHndl *hndl, int *value, unsigned int timeout);
int IpcIntelliQueGetQueuedCount(TIpcIntelliQueHndl *hndl);
int IpcIntelliQueIsEmpty(TIpcIntelliQueHndl *hndl);
int IpcIntelliBufPutEmpty(TIpcIntelliBufHndl *hndl, void *bufInfo);
int IpcIntelliBufGetEmpty(TIpcIntelliBufHndl *hndl, void **bufInfo);
int IpcIntelliBufPutFull(TIpcIntelliBufHndl *hndl, void *bufInfo);
int IpcIntelliGetFull(TIpcIntelliBufHndl *hndl, void **bufInfo);
unsigned int IpcIntelliBufGetEmptyCount(TIpcIntelliBufHndl *hndl);
unsigned int IpcIntelliBufGetFullCount(TIpcIntelliBufHndl *hndl);
int IpcIntelliBufCreate(TIpcIntelliBufHndl* hndl, unsigned int maxBufnum);
int IpcIntelliBufDelete(TIpcIntelliBufHndl *hndl);
int IpcIntelliPutBuffer(TIpcIntelliBufHndl *hndl, void *bufInfo, EIntelliBufType bufType);
void *IpcIntelliGetBuffer(TIpcIntelliBufHndl *hndl, EIntelliBufType bufType, EIntelliBufTimeOutType bufTimeOutType);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __INTELLI_QUE_MANGER_H__ */

