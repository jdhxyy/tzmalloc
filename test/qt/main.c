#include <stdio.h>
#include <string.h>
#include "tzmalloc.h"
#include "scunit.h"

#define RAM0 0
#define RAM1 1
#define RAM2 2

static int gMidRam0[20] = {0};
static int gMidRam1[20] = {0};
static int gMidRam2[20] = {0};

static void printAllUser(int ramIndex);
static void case0(void);
static void case1(void);
static void case2(void);
static void case3(void);
static void case4(void);

int main() {
    TZMallocLoad(RAM0, 20, 100 * 1000, malloc(100 * 1000));
    TZMallocLoad(RAM1, 20, 10 * 1000, malloc(10 * 1000));

    static uint8_t arr[5 * 1000] = {0};
    TZMallocLoad(RAM2, 20, 5 * 1000, arr);

    gMidRam0[0] = TZMallocRegister(RAM0, "RAM0-user0", 20);
    gMidRam0[1] = TZMallocRegister(RAM0, "RAM0-user1", 100);
    gMidRam0[2] = TZMallocRegister(RAM0, "RAM0-user2", 256);
    gMidRam1[0] = TZMallocRegister(RAM1, "RAM1-user0", 256);
    gMidRam2[0] = TZMallocRegister(RAM2, "RAM2-user0", 256);

    printAllUser(RAM0);
    printAllUser(RAM1);
    printAllUser(RAM2);


    ScunitLoad((ScunitPrintFunc)printf);
    ScunitAddSuite("test tzmalloc");
    ScunitAddTest("case0", case0);
    ScunitAddTest("case1", case1);
    ScunitAddTest("case2", case2);
    ScunitAddTest("case3", case3);
    ScunitAddTest("case4", case4);
    return 0;
}

static void printAllUser(int ramIndex) {
    printf("print ram:%d user\n", ramIndex);
    TZMallocUser* user;
    int num = TZMallocGetUserNum(ramIndex);
    for (int i = 0; i < num; i++) {
        user = TZMallocGetUser(TZMALLOC_SUPPORT_USER_NUM_MAX * ramIndex + i);
        printf("mid:%d tag:%s total:%d used:%d mallocNum:%d freeNum:%d\n", i,
               user->Tag, user->Total, user->Used, user->MallocNum, user->FreeNum);
    }
    TZMallocStatus status = TZMallocGetStatus(ramIndex);
    printf("tzmalloc status:UsedSize=%ld FreeSize=%ld MaxFreeSize=%ld MallocNum=%ld FreeNum=%ld\n", status.UsedSize,
        status.FreeSize, status.MaxFreeSize, status.MallocNum, status.FreeNum);
}

static void case0(void) {
    uint8_t* data = TZMalloc(gMidRam0[0], 30);
    ScunitAssertMessage(data == NULL, "case0", "data is null");

    data = TZMalloc(gMidRam0[1], 30);
    ScunitAssertMessage(data != NULL, "case0", "data is not null");
    TZMallocUser* user;
    if (data != NULL) {
        user = TZMallocGetUser(gMidRam0[1]);
        ScunitAssert(user->Used == 30 && user->MallocNum == 1 && user->FreeNum == 0, "case0");

        TZFree(data);
        ScunitAssert(user->Used == 0 && user->MallocNum == 1 && user->FreeNum == 1, "case0");
    }
    printAllUser(RAM0);
}

static void case1(void) {
    uint8_t* data[100] = {0};
    for (int i = 0; i < 100; i++) {
        data[i] = TZMalloc(gMidRam0[2], i + 31);
    }
    printAllUser(RAM0);
    for (int i = 0; i < 100; i++) {
        TZFree(data[i]);
        data[i] = NULL;
    }
    printAllUser(RAM0);

    TZMallocUser* user = TZMallocGetUser(gMidRam0[2]);
    ScunitAssert(user->Used == 0, "case1");
}

static void case2(void) {
    uint8_t* data = NULL;
    for (int i = 0; i < 10000; i++) {
        data = TZMalloc(gMidRam0[2], 232);
        TZFree(data);
        data = NULL;
    }
    printAllUser(RAM0);

    TZMallocUser* user = TZMallocGetUser(gMidRam0[2]);
    ScunitAssert(user->Used == 0, "case2");
}

static void case3(void) {
    uint8_t* data = NULL;
    for (int i = 0; i < 10000; i++) {
        data = TZMalloc(gMidRam1[0], 232);
        TZFree(data);
        data = NULL;
    }
    printAllUser(RAM1);

    TZMallocUser* user = TZMallocGetUser(gMidRam1[0]);
    ScunitAssert(user->Used == 0, "case3");
}

static void case4(void) {
    uint8_t* data = NULL;
    for (int i = 0; i < 10000; i++) {
        data = TZMalloc(gMidRam2[0], 232);
        TZFree(data);
        data = NULL;
    }
    printAllUser(RAM2);

    TZMallocUser* user = TZMallocGetUser(gMidRam2[0]);
    ScunitAssert(user->Used == 0, "case3");
}
