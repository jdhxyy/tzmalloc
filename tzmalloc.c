// Copyright 2019-2021 The jdh99 Authors. All rights reserved.
// Secure memory allocation library for C
// Authors: jdh99 <jdh821@163.com>

#include "tzmalloc.h"
#include "bget.h"
#include <string.h>

#define MAGIC_NUMBER 0x2020

#pragma pack(1)
// tUnit malloc unit header
typedef struct {
    uint16_t magicNumber;
    uint8_t ramIndex;
    uint8_t mid;
    uint32_t size;
    uint8_t reserved[3];
    // check must the last field
    uint8_t check;
} tUnit;

#pragma pack()

static int gMaxUserNum[TZMALLOC_RAM_NUM] = {0};
static TZMallocUser* gUsers[TZMALLOC_RAM_NUM] = {NULL};
static int gUsersNum[TZMALLOC_RAM_NUM] = {0};

static uint8_t calcCheck(tUnit* unit);

// TZMallocLoad module load
bool TZMallocLoad(int ramIndex, int maxUserNum, int totalMemorySize, void* addr) {
    if (gMaxUserNum[ramIndex] == maxUserNum) {
        return true;
    }
    if (gUsersNum[ramIndex] != 0) {
        return false;
    }

    bpool(ramIndex, addr, totalMemorySize);
    gUsers[ramIndex] = bget(ramIndex, (size_t)maxUserNum * sizeof(TZMallocUser));
    if (gUsers[ramIndex] == NULL) {
        return false;
    }
    gMaxUserNum[ramIndex] = maxUserNum;
    return true;
}

// TZMallocRegister register user
// total is user's max allowable size
// register success return mid,else return -1
int TZMallocRegister(int ramIndex, const char* tag, int total) {
    if (gUsersNum[ramIndex] >= gMaxUserNum[ramIndex]) {
        return -1;
    }

    int tagLen = (int)strlen(tag);
    if (tagLen == 0) {
        return -1;
    }
    if (tagLen > TZMALLOC_TAG_LEN_MAX) {
        tagLen = TZMALLOC_TAG_LEN_MAX;
    }

    memset(&gUsers[ramIndex][gUsersNum[ramIndex]], 0, sizeof(TZMallocUser));
    gUsers[ramIndex][gUsersNum[ramIndex]].RamIndex = ramIndex;
    memcpy(gUsers[ramIndex][gUsersNum[ramIndex]].Tag, tag, (size_t)tagLen);
    gUsers[ramIndex][gUsersNum[ramIndex]].Total = (uint32_t)total;
    gUsersNum[ramIndex]++;

    return (TZMALLOC_SUPPORT_USER_NUM_MAX * ramIndex + gUsersNum[ramIndex] - 1);
}

// TZMalloc allocate space.return NULL if failed
// allocated space value is 0
void* TZMalloc(int mid, int size) {
    if (mid < 0 || size <= 0) {
        return NULL;
    }

    int ramIndex = mid / TZMALLOC_SUPPORT_USER_NUM_MAX;
    mid %= TZMALLOC_SUPPORT_USER_NUM_MAX;

    if (mid >= gUsersNum[ramIndex]) {
        return NULL;
    }
    if (gUsers[ramIndex][mid].Total <= gUsers[ramIndex][mid].Used) {
        if (gUsers[ramIndex][mid].Total < gUsers[ramIndex][mid].Used) {
            gUsers[ramIndex][mid].ExceptionNum++;
        }
        return NULL;
    }
    if (size > (int)(gUsers[ramIndex][mid].Total - gUsers[ramIndex][mid].Used)) {
        return NULL;
    }

    uint8_t* data = bget(ramIndex, sizeof(tUnit) + (size_t)size);
    if (data == NULL) {
        return NULL;
    }
    memset(data, 0, sizeof(tUnit) + (size_t)size);

    tUnit* unit = (tUnit*)data;
    unit->magicNumber = MAGIC_NUMBER;
    unit->ramIndex = (uint8_t)ramIndex;
    unit->mid = (uint8_t)mid;
    unit->size = (uint32_t)size;
    unit->check = calcCheck(unit);

    gUsers[ramIndex][mid].Used += (uint32_t)size;
    gUsers[ramIndex][mid].MallocNum++;

    return data + sizeof(tUnit);
}

static uint8_t calcCheck(tUnit* unit) {
    uint8_t check = 0;
    uint8_t* data = (uint8_t*)unit;
    int num = sizeof(tUnit) - 1;
    for (int i = 0; i < num; i++) {
        check ^= data[i];
    }
    return check;
}

// TZFree free space
void TZFree(void* data) {
    if (data == NULL) {
        return;
    }
    tUnit* unit = (tUnit*)((intptr_t)data - (intptr_t)sizeof(tUnit));
    if (unit->mid > gUsersNum[unit->ramIndex]) {
        gUsers[unit->ramIndex][unit->mid].ExceptionNum++;
        return;
    }
    if (unit->magicNumber != MAGIC_NUMBER || unit->check != calcCheck(unit)) {
        gUsers[unit->ramIndex][unit->mid].ExceptionNum++;
        return;
    }

    if (gUsers[unit->ramIndex][unit->mid].Used >= unit->size) {
        gUsers[unit->ramIndex][unit->mid].Used -= unit->size;
    } else {
        gUsers[unit->ramIndex][unit->mid].ExceptionNum++;
        return;
    }
    gUsers[unit->ramIndex][unit->mid].FreeNum++;
    brel(unit->ramIndex, unit);
}

// TZMallocGetUser get user info
// return NULL if get failed
TZMallocUser* TZMallocGetUser(int mid) {
    if (mid < 0) {
        return NULL;
    }

    int ramIndex = mid / TZMALLOC_SUPPORT_USER_NUM_MAX;
    mid %= TZMALLOC_SUPPORT_USER_NUM_MAX;

    if (mid >= gUsersNum[ramIndex]) {
        return NULL;
    }
    return &gUsers[ramIndex][mid];
}

// TZMallocGetUser get user info
// return NULL if get failed
int TZMallocGetUserNum(int ramIndex) {
    return gUsersNum[ramIndex];
}

// TZMallocGetStatus get memory status
TZMallocStatus TZMallocGetStatus(int ramIndex) {
    TZMallocStatus status;
    bstats(ramIndex, &status.UsedSize, &status.FreeSize, &status.MaxFreeSize, &status.MallocNum, &status.FreeNum);
    return status;
}

// TZMallocGetUserMid get user mid
// return -1 if get failed
int TZMallocGetUserMid(int ramIndex, int index) {
    if (index < 0 || index >= gUsersNum[ramIndex]) {
        return -1;
    }
    return (TZMALLOC_SUPPORT_USER_NUM_MAX * ramIndex + index);
}
