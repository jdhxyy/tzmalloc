// Copyright 2019-2021 The jdh99 Authors. All rights reserved.
// Secure memory allocation library for C
// Authors: jdh99 <jdh821@163.com>

#ifndef TZMALLOC_H
#define TZMALLOC_H

#include <stdint.h>
#include <stdbool.h>

// user's tag max len
#define TZMALLOC_TAG_LEN_MAX 16
// support ram num
#define TZMALLOC_RAM_NUM 3
// max user nums each ram
#define TZMALLOC_SUPPORT_USER_NUM_MAX 10000

#pragma pack(1)

// user info
typedef struct {
    int RamIndex;
    char Tag[TZMALLOC_TAG_LEN_MAX + 1];
    uint32_t Total;
    uint32_t Used;
    uint32_t MallocNum;
    uint32_t FreeNum;
    uint32_t ExceptionNum;
} TZMallocUser;

// memory usage status information
typedef struct {
    long UsedSize;
    long FreeSize;
    long MaxFreeSize;
    long MallocNum;
    long FreeNum;
} TZMallocStatus;

#pragma pack()

// TZMallocLoad module load
bool TZMallocLoad(int ramIndex, int maxUserNum, int totalMemorySize, void* addr);

// TZMallocRegister register user
// total is user's max allowable size
// register success return mid,else return -1
int TZMallocRegister(int ramIndex, const char* tag, int total);

// TZMalloc allocate space.return NULL if failed
// allocated space value is 0
void* TZMalloc(int mid, int size);

// TZFree free space
void TZFree(void* data);

// TZMallocGetUser get user info
// return NULL if get failed
TZMallocUser* TZMallocGetUser(int mid);

// TZMallocGetUserNum get user num
int TZMallocGetUserNum(int ramIndex);

// TZMallocGetStatus get memory status
TZMallocStatus TZMallocGetStatus(int ramIndex);

#endif
