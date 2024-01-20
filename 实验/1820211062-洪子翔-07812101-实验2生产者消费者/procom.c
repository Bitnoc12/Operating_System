#include<windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<time.h>

#define numberProducer 2 //生产者的数量
#define numberConsumer 3 //消费者的数量
#define numberProcess 5
#define repeatProduce 12 //每个生产者重复放入的次数
#define repeatConsume 8; //每个消费者重复消费的次数
#define lenBuffer 6 //缓冲区大小

// int producerWaitTime = 1000; // 设置生产者等待时间为1秒
// int consumerWaitTime = 2000; // 设置消费者等待时间为2秒

int producerWaitTime = 2000; // 设置生产者等待时间为1秒
int consumerWaitTime = 1000; // 设置消费者等待时间为2秒


int countProducer = 0;
int countConsumer = 0;

//定义缓冲区
struct buffer{
  int s[lenBuffer];
  int head;
  int tail;
};

//显示缓冲区的内容
void ShowBuffer(struct buffer *p){
  printf("Current Data In Buffer: ");
  for(int i = 0; i<lenBuffer; i++)
  {
    printf("%d ",p->s[i]);
  }
  printf("\n");
}

//创建共享文件映射
HANDLE MakeSharedFile(){
  //创建共享内存，由filemapping实现
  //创建一个临时文件映射内核对象
  HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,
                                      0,sizeof(struct buffer),"BUFFER");
  if (hMapping == NULL)
    {//映射对象无退出程序
        printf("CreateFileMapping error\n");
        exit(0);
    }
  //在文件映射上创建视图，返回起始虚地址
  LPVOID pData = MapViewOfFile(hMapping,FILE_MAP_ALL_ACCESS,0,0,0);
  if (pData == NULL)
    {
        printf("MapViewOfFile error\n");
        exit(0);
    }
  if (pData != NULL)
    {
        ZeroMemory(pData, sizeof(struct buffer));
    }
  //解除当前地址空间映射
  UnmapViewOfFile(pData);
  return (hMapping);
}

//信号量句柄
HANDLE empty,full,mutex;
//进程句柄数组
HANDLE Handle_process[numberProducer + 5];

//创建信号量
void CreateSemap(){
  //表示空缓冲区的个数，初值为k
  empty = CreateSemaphore(NULL,lenBuffer,lenBuffer,"EMPTY");
  //有数据的缓冲区个数，初值为0
  full = CreateSemaphore(NULL,0,lenBuffer,"FULL");
  //互斥访问临界区的信号量，初值为1
  mutex = CreateSemaphore(NULL,1,1,"MUTEX");
}

//打开信号量
void OpenSemap(){
  empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS,FALSE,"EMPTY");
  full = OpenSemaphore(SEMAPHORE_ALL_ACCESS,FALSE,"FULL");
  mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS,FALSE,"MUTEX");
}

//关闭信号量
void CloseSemap(){
  CloseHandle(empty);
  CloseHandle(full);
  CloseHandle(mutex);
}

//创建新的子进程
void CreateNewSubProcess(int ID){
  STARTUPINFO si;
  //初始化为0
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  //用于接收新进程的信息（进程句柄等）
  PROCESS_INFORMATION pi;

  //存储命令行字符
  char Cmdstr[105];
  //存储程序文件名
  char CurFile[105];

  //获取程序文件名
  GetModuleFileName(NULL, CurFile, sizeof(CurFile));
  //表示将要运行的新进程
  sprintf(Cmdstr, "%s %d", CurFile, ID);

  //创建新进程
  CreateProcess(NULL, Cmdstr, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
  //将新进程存储在进程数组中
  Handle_process[ID] = pi.hProcess;
  return;
}

//生产者进程
void Producer(int ID){
  HANDLE hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "BUFFER");
  if (hMapping == NULL)
    {
        printf("OpenFileMapping error!\n");
        exit(0);
    }
  LPVOID pFile = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (pFile == NULL)
    {
        printf("MapViewOfFile error\n");
        exit(0);
    }
  //创建共享内存
  struct buffer *p = (struct buffer *)(pFile);
  //打开信号量
  OpenSemap();
  //PV操作
  for (int i=0 ; i<repeatProduce; i++){
    srand((unsigned int)time(NULL) + ID);
    int temp = (rand() % 5 + 1) * 500;
    //生产者休眠一段时间，模拟生产过程中的耗时
    Sleep(producerWaitTime);
    //申请空缓冲
    WaitForSingleObject(empty,INFINITE);
    //申请互斥访问缓冲区
    WaitForSingleObject(mutex,INFINITE);
    //展示时间
    SYSTEMTIME time;
    GetLocalTime(&time);
    printf("\nTime: %02d:%02d:%02d:%d\n", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
    printf("Producer %d putting %d at %d\n", ID, temp,p->tail);
    //向缓冲区中添加数据
    p->s[p->tail] = temp;
    p->tail = (p->tail + 1) % lenBuffer;
    countProducer++;
    printf("CountProducer: %d\n",countProducer);
    ShowBuffer(p);
    //释放同步信号
    ReleaseSemaphore(full,1,NULL);
    //释放互斥信号
    ReleaseSemaphore(mutex,1,NULL);
  }
  //关闭信号量
  CloseSemap();
  UnmapViewOfFile(pFile);
  CloseHandle(hMapping);
}

//消费者进程
void Consumer(int ID){
  HANDLE hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "BUFFER");
  if (hMapping == NULL)
    {
        printf("OpenFileMapping error!\n");
        exit(0);
    }
  LPVOID pFile = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (pFile == NULL)
    {
        printf("MapViewOfFile error\n");
        exit(0);
    }
  //创建共享内存
  struct buffer *p = (struct buffer *)(pFile);
  //打开信号量
  OpenSemap();

  //PV操作
  for (int i=0 ; i<repeatProduce; i++){
    srand((unsigned int)time(NULL) + ID);
    int temp = (rand() % 5 + 1) * 1000;
    //生产者休眠一段时间，模拟生产过程中的耗时
    Sleep(consumerWaitTime);
    //申请满缓冲
    WaitForSingleObject(full,INFINITE);
    //申请互斥访问缓冲区
    WaitForSingleObject(mutex,INFINITE);

    //向缓冲区中取走数据
    if (p->s[p->head] != 0) {
      //展示时间
      SYSTEMTIME time;
      GetLocalTime(&time);
      printf("\nTime: %02d:%02d:%02d:%d\n", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
      printf("Consumer %d removing %d from %d\n", ID, p->s[p->head],p->head);
      p->s[p->head] = 0;
      p->head = (p->head + 1) % lenBuffer;
    
      countConsumer++;
      printf("CountConsumer: %d\n",countConsumer);
      ShowBuffer(p);
    }
    //释放同步信号
    ReleaseSemaphore(empty,1,NULL);
    //释放互斥信号
    ReleaseSemaphore(mutex,1,NULL);
  }

  //关闭信号量
  CloseSemap();
  UnmapViewOfFile(pFile);
  CloseHandle(hMapping);

}

int main(int argc, char *argv[]){
  if (argc == 1)
    {
        HANDLE hMapping = MakeSharedFile(); // 创建共享文件映射
        CreateSemap(); // 创建信号量
        printf("0:createSubProcess,1~2:ProducerProcess,3~5:ConsumerProcess\n");
        for (int i = 1; i <= numberProcess; i++)
        {
            CreateNewSubProcess(i); // 创建子进程
        }
        WaitForMultipleObjects(numberProcess, Handle_process + 1, TRUE, INFINITE);
        for (int i = 1; i <= numberProcess; i++)
        {
            CloseHandle(Handle_process[i]); // 关闭子进程句柄
        }
        CloseSemap(); // 关闭信号量
        printf("0:end\n");
        CloseHandle(hMapping); // 关闭共享文件映射
    }
    else
    {
        int ID = atoi(argv[1]); // 获取传入的参数
        if (ID <= numberProducer)
            Producer(ID); // 如果参数小于等于生产者数量，执行生产者函数
        else
            Consumer(ID); // 否则执行消费者函数     
    }

    
}
