
#include "ipcintelliquemanger.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int IpcIntelliMutexCreate(TIpcIntelliMutexHndl *hndl)
{
    pthread_mutexattr_t mutex_attr;
    int status = IPC_INTELLI_SOK;
 
    status |= pthread_mutexattr_init(&mutex_attr);
    status |= pthread_mutex_init(&hndl->lock, &mutex_attr);
    IPC_INTELLI_ASSERT(status == IPC_INTELLI_SOK);

    pthread_mutexattr_destroy(&mutex_attr);
    
    return status;
}

int IpcIntelliMutexLock(TIpcIntelliMutexHndl *hndl)
{
    return pthread_mutex_lock(&hndl->lock);
}

int IpcIntelliMutexUnlock(TIpcIntelliMutexHndl *hndl)
{
    return pthread_mutex_unlock(&hndl->lock);
}

int IpcIntelliQueCreate(TIpcIntelliQueHndl *hndl, unsigned int maxLen)
{
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;
    int status = IPC_INTELLI_SOK;

    hndl->curRd = hndl->curWr = 0;
    hndl->count = 0;
    hndl->len   = maxLen;
    hndl->queue = malloc(sizeof(int)*hndl->len);
    
    status |= pthread_mutexattr_init(&mutex_attr);
    status |= pthread_condattr_init(&cond_attr);  

    status |= pthread_mutex_init(&hndl->lock, &mutex_attr);
    status |= pthread_cond_init(&hndl->condRd, &cond_attr);    
    status |= pthread_cond_init(&hndl->condWr, &cond_attr);  
    
    pthread_condattr_destroy(&cond_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    return status;
}

int IpcIntelliQueDelete(TIpcIntelliQueHndl *hndl)
{
    if(hndl->queue != NULL)
        free(hndl->queue);

    pthread_cond_destroy(&hndl->condRd);
    pthread_cond_destroy(&hndl->condWr);
    pthread_mutex_destroy(&hndl->lock);  
  
    return 0;
}

int IpcIntelliQuePut(TIpcIntelliQueHndl *hndl, int value, unsigned int timeout)
{
    int status = -1;

    pthread_mutex_lock(&hndl->lock);

    while(1) {
        if( hndl->count < hndl->len ) {
            hndl->queue[hndl->curWr] = value;
            hndl->curWr = (hndl->curWr+1)%hndl->len;
            hndl->count++;
            status = IPC_INTELLI_SOK;
            pthread_cond_signal(&hndl->condRd);
            break;
        } else {
            if(timeout == IPC_INTELLI_TIMEOUT_NONE)
                break;
            status = pthread_cond_wait(&hndl->condWr, &hndl->lock);
        }
    }

    pthread_mutex_unlock(&hndl->lock);

    return status;
}

int IpcIntelliQueGet(TIpcIntelliQueHndl *hndl, int *value, unsigned int timeout)
{
    int status = IPC_INTELLI_SFAIL;

    pthread_mutex_lock(&hndl->lock);

    while(1) {
        if(hndl->count > 0 ) {
            if(value != NULL) {
                *value = hndl->queue[hndl->curRd];
            }

            hndl->curRd = (hndl->curRd+1)%hndl->len;
            hndl->count--;
            status = IPC_INTELLI_SOK;
            pthread_cond_signal(&hndl->condWr);
            break;
        } else {
            if(timeout == IPC_INTELLI_TIMEOUT_NONE)
            break;
            status = pthread_cond_wait(&hndl->condRd, &hndl->lock);
        }
    }

    pthread_mutex_unlock(&hndl->lock);

    return status;
}

int IpcIntelliQueGetQueuedCount(TIpcIntelliQueHndl *hndl)
{
      unsigned int queuedCount = 0;

      pthread_mutex_lock(&hndl->lock);
      queuedCount = hndl->count;
      pthread_mutex_unlock(&hndl->lock);
      return queuedCount;
}

int IpcIntelliQueIsEmpty(TIpcIntelliQueHndl *hndl)
{
    int isEmpty;

    pthread_mutex_lock(&hndl->lock);
    if (hndl->count == 0)
    {
        isEmpty = 1;
    }
    else
    {
        isEmpty = 0;
    }
    pthread_mutex_unlock(&hndl->lock);

    return isEmpty;
}

int IpcIntelliBufGetEmpty(TIpcIntelliBufHndl *hndl, void **bufInfo)
{
    int status;
    int value;
    
    if(hndl == NULL || bufInfo == NULL)
        return IPC_INTELLI_SFAIL;

    status = IpcIntelliQueGet(&hndl->emptyQue, &value, IPC_INTELLI_TIMEOUT_NONE);

    if(status != IPC_INTELLI_SOK)
    {
        *bufInfo = NULL;
        return status;
    }
    
    *bufInfo = (void *)value;
    
    return status;
}

int IpcIntelliBufPutFull(TIpcIntelliBufHndl *hndl, void *bufInfo)
{
    int status;

    if(hndl == NULL)
        return IPC_INTELLI_SFAIL;

    status = IpcIntelliQuePut(&hndl->fullQue, (int)bufInfo, IPC_INTELLI_TIMEOUT_NONE);

    return status;
}

int IpcIntelliGetFull(TIpcIntelliBufHndl *hndl, void **bufInfo)
{
    int status;
    int value;
    
    if(hndl == NULL || bufInfo == NULL)
        return IPC_INTELLI_SFAIL;

    status = IpcIntelliQueGet(&hndl->fullQue, &value, IPC_INTELLI_TIMEOUT_NONE);

    if(status != IPC_INTELLI_SOK)
    {
        *bufInfo = NULL;
        return status;
    }

    *bufInfo = (void *)value;

    return status;
}

int IpcIntelliBufPutEmpty(TIpcIntelliBufHndl *hndl, void *bufInfo)
{
    int status;

    if(hndl == NULL)
        return IPC_INTELLI_SFAIL;

    status = IpcIntelliQuePut(&hndl->emptyQue, (int )bufInfo, IPC_INTELLI_TIMEOUT_NONE);

    return status;
}

unsigned int IpcIntelliBufGetEmptyCount(TIpcIntelliBufHndl *hndl)
{
    return IpcIntelliQueGetQueuedCount(&hndl->emptyQue);
}

unsigned int IpcIntelliBufGetFullCount(TIpcIntelliBufHndl *hndl)
{
    return IpcIntelliQueGetQueuedCount(&hndl->fullQue);
}

int IpcIntelliBufCreate(TIpcIntelliBufHndl* hndl, unsigned int maxBufnum)
{
    int status;
    
    status = IpcIntelliQueCreate(&hndl->emptyQue, maxBufnum);

    if(status != IPC_INTELLI_SOK)
    {
        return IPC_INTELLI_SFAIL;
    }

    status = IpcIntelliQueCreate(&hndl->fullQue, maxBufnum);
    if(status != IPC_INTELLI_SOK)
    {
        IpcIntelliQueDelete(&hndl->emptyQue);
        return IPC_INTELLI_SFAIL;
    }

    return status;

}

int IpcIntelliBufDelete(TIpcIntelliBufHndl *hndl)
{
    int status = IPC_INTELLI_SOK;

    if(hndl == NULL)
        return IPC_INTELLI_SFAIL;

    status = IpcIntelliQueDelete(&hndl->emptyQue);
    status |= IpcIntelliQueDelete(&hndl->fullQue);

    return status;
}

int IpcIntelliPutBuffer(TIpcIntelliBufHndl *hndl, void *bufInfo, EIntelliBufType bufType)
{
    if(hndl == NULL)
    {
        return IPC_INTELLI_SFAIL;
    }

    if(IPC_INTELLI_EMPTY_BUFFER_FLAG == bufType)
    {
        IpcIntelliBufPutEmpty(hndl, bufInfo);
    }
    else if(IPC_INTELLI_FULL_BUFFER_FLAG == bufType)
    {
        IpcIntelliBufPutFull(hndl, bufInfo);
    }

    return IPC_INTELLI_SOK;
}

void *IpcIntelliGetBuffer(TIpcIntelliBufHndl *hndl,
                          EIntelliBufType bufType,
                          EIntelliBufTimeOutType bufTimeOutType)
{
    int nRet;
    void *pBuffer = NULL;

    while(1)
    {
        if(IPC_INTELLI_EMPTY_BUFFER_FLAG == bufType)
        {
            nRet = IpcIntelliBufGetEmpty(hndl, (void *)&pBuffer);
        }
        else
        {
            nRet = IpcIntelliGetFull(hndl, (void *)&pBuffer);
        }

        if(nRet != IPC_INTELLI_SOK && bufTimeOutType != IPC_INTELLI_TIMEOUT_NONE)
        {
            usleep(bufTimeOutType*1000);
        }
        else
        {
            break;
        }
    }

    return pBuffer;
}

