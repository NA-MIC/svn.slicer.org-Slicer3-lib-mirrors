#ifndef BLT_POOL_H
#define BLT_POOL_H

#define BLT_STRING_ITEMS		0
#define BLT_FIXED_SIZE_ITEMS		1
#define BLT_VARIABLE_SIZE_ITEMS		2

typedef struct Blt_PoolStruct *Blt_Pool;
typedef void *(Blt_PoolAllocProc) _ANSI_ARGS_((Blt_Pool pool, size_t size));
typedef void (Blt_PoolFreeProc) _ANSI_ARGS_((Blt_Pool pool, void *item));

struct Blt_PoolStruct {
    Blt_PoolAllocProc *allocProc;
    Blt_PoolFreeProc *freeProc;
};

BLT_EXTERN Blt_Pool Blt_PoolCreate _ANSI_ARGS_((int type));
BLT_EXTERN void Blt_PoolDestroy _ANSI_ARGS_((Blt_Pool pool));

#define Blt_PoolAllocItem(pool, n) (*((pool)->allocProc))(pool, n)
#define Blt_PoolFreeItem(pool, item) (*((pool)->freeProc))(pool, item)

#endif /* BLT_POOL_H */
