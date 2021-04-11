# tzmalloc

## 介绍
在嵌入式领域并没有好用的内存管理框架，一般是直接使用malloc进行内存分配。当内存泄漏时没有好的办法定位到问题，容易造成稳定性问题。所以一般不建议在单片机中使用malloc，就是出于这方面考虑。

tzmalloc是适合于嵌入式领域的内存管理包，可以追踪到每个模块的内存使用，内存使用次数等等。如果有内存泄漏问题，可以很方便的定位到出问题的模块。有了tzmalloc，则可放心的在单片机等嵌入式系统中进行内存分配。


## 开源
- [github上的项目地址](https://github.com/jdhxyy/tzmalloc)
- [gitee上的项目地址](https://gitee.com/jdhxyy/tzmalloc)

## 特性
- 可以查看应用模块开辟空间，释放空间次数
- 可以检测出内存分配错误
- 支持多块独立的RAM

### 支持多块独立的RAM
有些单片机有多块RAM，比如单片机内部RAM，CCM以及外接RAM。tzmalloc支持多块独立的RAM，可以指定应用模块在某块RAM开辟空间。

默认最大支持3块RAM，bget.h中使用宏定义控制数量：
```c
// suport ram num
#define BGET_RAM_NUM 3
```

## bget
bget是一个有历史的内存分配包，创作于1972年，可替代标准库中的malloc。目前是一些RTOS内置的内存分配包。

tzmalloc使用bget作为内存分配的工具。

### 与malloc对比分配效率
实测bget比malloc分配效率高2倍左右：

测试1000万次内存分配和释放，每次分配1K字节。malloc测试需1006281us，bget需596616us：

![在这里插入图片描述](https://img-blog.csdnimg.cn/20200807174842551.png)

bget的详细资料可参考：[http://www.fourmilab.ch/bget/](http://www.fourmilab.ch/bget/)，可以从链接中下载bget源码。

## 单片机中使用assert
bget中大量使用了assert，如果芯片的开发环境使用标准库则已经支持。否则需要在MDK等环境中勾选使用MicroLib，并添加语句：
```c
// 为mirco lib适配assert
void __aeabi_assert(uint8_t* file, uint32_t line) {
    while (1);
}
```

## API
```c
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
```

## 使用方法
比如项目中有一个转发模块需要使用到内存分配，则需先定义模块名和最大分配的内存大小：
```c
// tzmalloc标签和字节数
#define FORWARD_MALLOC_TAG "forward"
#define FORWARD_MALLOC_TOTAL 4096
```

然后获得一个句柄：
```c
#define RAM_INTERNAL 0 
gMid = TZMallocRegister(RAM_INTERNAL, FORWARD_MALLOC_TAG, FORWARD_MALLOC_TOTAL);
```

后续在此模块内即可使用此句柄进行内存分配和释放：
```c
// 分配
TZBufferDynamic* buffer = (TZBufferDynamic*)TZMalloc(gMid, sizeof(TZBufferDynamic) + NLV1_HEAD_LEN + frame->Size);
if (buffer == NULL) {
    return;
}

// 释放
TZFree(buffer);
```

## 在仿真时实时查看内存使用情况
如下图所示，在仿真调试可以查看某个软件模块当前使用内存大小，分配次数，释放次数，以及是否有分配异常。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200807175958997.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2pkaDk5,size_16,color_FFFFFF,t_70)


## 支持多块RAM
比如某项目单片机有3块RAM：
- 内部RAM
- 内部CCM
- 外部RAM

```c
// 内存分为3片
// 单片机内部RAM
#define RAM_INTERNAL 0
// CCM RAM
#define RAM_CCM 1
// 外部RAM
#define RAM_EXTERNAL 2

// 给TZMALLOC使用的空间
// 使用的内部存储大小.单位:kbytes
#define RAM_INTERNAL_SIZE 70
// 使用的CCM大小
#define RAM_CCM_SIZE 64
// 使用的外部存储大小
#define RAM_EXTERNAL_SIZE 16384
```

则可使用以下代码载入：
```c
__attribute__((section (".CCM_RAM"))) static uint8_t gCCMRam[RAM_CCM_SIZE * 1024];
__attribute__((section (".SDRAM")))  static uint8_t gExternalRam[RAM_EXTERNAL_SIZE * 1024];

void init(void) {
    void* addr = malloc((size_t)RAM_INTERNAL_SIZE * 1024);
    if (addr != NULL) {
        TZMallocLoad(RAM_INTERNAL, TZMALLOC_USER_NUM_MAX, RAM_INTERNAL_SIZE * 1024, addr);
    }

    TZMallocLoad(RAM_CCM, TZMALLOC_USER_NUM_MAX, RAM_CCM_SIZE * 1024, (void*)gCCMRam);
    TZMallocLoad(RAM_EXTERNAL, TZMALLOC_USER_NUM_MAX, RAM_EXTERNAL_SIZE * 1024, (void*)gExternalRam);
}
```

应用模块注册用户时设置RAM序号即可使用指定的RAM。

## 测试
测试框架使用的是[适合于嵌入式系统的C语言单元测试框架:Scunit](https://blog.csdn.net/jdh99/article/details/100183066)

使用cip可以安装依赖的包:[cip：C/C++包管理工具](https://blog.csdn.net/jdh99/article/details/115590099)

```c
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
```

输出:
```text
print ram:0 user
mid:0 tag:RAM0-user0 total:20 used:0 mallocNum:0 freeNum:0
mid:1 tag:RAM0-user1 total:100 used:0 mallocNum:0 freeNum:0
mid:2 tag:RAM0-user2 total:256 used:0 mallocNum:0 freeNum:0
tzmalloc status:UsedSize=828 FreeSize=99164 MaxFreeSize=99164 MallocNum=1 FreeNum=0
print ram:1 user
mid:0 tag:RAM1-user0 total:256 used:0 mallocNum:0 freeNum:0
tzmalloc status:UsedSize=828 FreeSize=9164 MaxFreeSize=9164 MallocNum=1 FreeNum=0
print ram:2 user
mid:0 tag:RAM2-user0 total:256 used:0 mallocNum:0 freeNum:0
tzmalloc status:UsedSize=828 FreeSize=4164 MaxFreeSize=4164 MallocNum=1 FreeNum=0

Suite:test tzmalloc
-------------------->case:case0 begin
case0 tag:case0 SCUNIT_ASSERT pass
case0 tag:case0 SCUNIT_ASSERT pass
case0 tag:case0 SCUNIT_ASSERT pass
case0 tag:case0 SCUNIT_ASSERT pass
print ram:0 user
mid:0 tag:RAM0-user0 total:20 used:0 mallocNum:0 freeNum:0
mid:1 tag:RAM0-user1 total:100 used:0 mallocNum:1 freeNum:1
mid:2 tag:RAM0-user2 total:256 used:0 mallocNum:0 freeNum:0
tzmalloc status:UsedSize=828 FreeSize=99164 MaxFreeSize=99164 MallocNum=2 FreeNum=1
-------------------->case:case0 end

-------------------->case:case1 begin
print ram:0 user
mid:0 tag:RAM0-user0 total:20 used:0 mallocNum:0 freeNum:0
mid:1 tag:RAM0-user1 total:100 used:0 mallocNum:1 freeNum:1
mid:2 tag:RAM0-user2 total:256 used:238 mallocNum:7 freeNum:0
tzmalloc status:UsedSize=1216 FreeSize=98776 MaxFreeSize=98776 MallocNum=9 FreeNum=1
print ram:0 user
mid:0 tag:RAM0-user0 total:20 used:0 mallocNum:0 freeNum:0
mid:1 tag:RAM0-user1 total:100 used:0 mallocNum:1 freeNum:1
mid:2 tag:RAM0-user2 total:256 used:0 mallocNum:7 freeNum:7
tzmalloc status:UsedSize=828 FreeSize=99164 MaxFreeSize=99164 MallocNum=9 FreeNum=8
case1 tag:case1 SCUNIT_ASSERT pass
-------------------->case:case1 end

-------------------->case:case2 begin
print ram:0 user
mid:0 tag:RAM0-user0 total:20 used:0 mallocNum:0 freeNum:0
mid:1 tag:RAM0-user1 total:100 used:0 mallocNum:1 freeNum:1
mid:2 tag:RAM0-user2 total:256 used:0 mallocNum:10007 freeNum:10007
tzmalloc status:UsedSize=828 FreeSize=99164 MaxFreeSize=99164 MallocNum=10009 FreeNum=10008
case2 tag:case2 SCUNIT_ASSERT pass
-------------------->case:case2 end

-------------------->case:case3 begin
print ram:1 user
mid:0 tag:RAM1-user0 total:256 used:0 mallocNum:10000 freeNum:10000
tzmalloc status:UsedSize=828 FreeSize=9164 MaxFreeSize=9164 MallocNum=10001 FreeNum=10000
case3 tag:case3 SCUNIT_ASSERT pass
-------------------->case:case3 end

-------------------->case:case4 begin
print ram:2 user
mid:0 tag:RAM2-user0 total:256 used:0 mallocNum:10000 freeNum:10000
tzmalloc status:UsedSize=828 FreeSize=4164 MaxFreeSize=4164 MallocNum=10001 FreeNum=10000
case4 tag:case3 SCUNIT_ASSERT pass
-------------------->case:case4 end
```
