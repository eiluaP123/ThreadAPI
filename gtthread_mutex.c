#include "gtthread.h"

int  gtthread_mutex_init(gtthread_mutex_t *mutex){
        mutex->lock=0;
        mutex->oid=-1;
        return 0;
}

int  gtthread_mutex_lock(gtthread_mutex_t *mutex){
	while(mutex->lock);
        mutex->lock=1;
        mutex->oid=curr->tid;
        return 0;
}

int  gtthread_mutex_unlock(gtthread_mutex_t *mutex){
        if (mutex->lock==1 && mutex->oid==curr->tid){
                mutex->lock=0;
                mutex->oid=0;
                return 0;
        }
        return -1;
}
