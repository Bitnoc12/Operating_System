#include <iostream>
#include <windows.h>
#include <string>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <TlHelp32.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <Psapi.h>

#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"kernel32.lib")

//用于取权限位
#define PAGE_NOACCESS           0x01    //0000 0001
#define PAGE_READONLY           0x02    //0000 0010
#define PAGE_READWRITE          0x04    //0000 0100 
#define PAGE_WRITECOPY          0x08    //0000 1000

using namespace std;

//全部函数声明
string FormatByteSize(DWORD64 bytes);
void printProtection(DWORD dwTarget);
void ShowHelp();
void displaySystemConfig();
void displayMemoryCondition();
void getAllProcessInformation();
void getProcessDetail(int pid);

int main(){

  //初始化一开始显示的输出
  cout<<endl<<"****************Welcome to MemoryManagement(1820211062)****************"<<endl<<"\n";
  cout<<"input 'help' for searching information"<<endl;
  
  //用于获取用户输入的字符串（命令）
  string cmd;
  //用于读取用户传入的输入
  char cmd_charstring[127];

  //循环访问
  while(true){
    //获取输入
    cout<<"Mm >";
    //读取用户输入
    cin.getline(cmd_charstring,127);
    cmd = cmd_charstring;
    //判断命令
    if (cmd == "help"){
      cout<<"\n";
      ShowHelp();
    }
    else if (cmd == "system"){
      cout<<"\n";
      displaySystemConfig();
    }
    else if (cmd == "memory"){
      cout<<"\n";
      displayMemoryCondition();
    }
    else if (cmd == "process"){
      cout<<"\n";
      getAllProcessInformation();
    }
    else if (cmd == "pid"){
      cout<<"PID> ";
      int pid = 0;
      cin>>pid;
      cin.getline(cmd_charstring,127);
      if(pid<=0)  continue;
      cout<<endl;
      getProcessDetail(pid);
    }
    else if (cmd == "quit"){
      cout<<"ByeBye,See you next time"<<endl;
      break;
    }
    else if (cmd == "clear" || cmd == "cls"){
      system("cls");
    }
    else{
      if (cmd!=""){
        cout<<"Illegal command, please use the 'help' command to view instructions."<<endl;
      }
      fflush(stdin);
      cin.clear();
      continue;
    }
    cin.clear();
  }

  return 0;

}

// 函数用于将字节数转换为格式化的大小字符串
string FormatByteSize(DWORD64 bytes) {
    static const char* suffixes[] = { "B", "KB", "MB", "GB", "TB", "PB" }; // 后缀单位数组
    int suffixIndex = 0; // 后缀索引
    double size = static_cast<double>(bytes); // 将字节数转换为双精度浮点数

    // 循环直到大小小于1024或者达到单位数组的最后一个单位
    while (size >= 1024 && suffixIndex < (sizeof(suffixes) / sizeof(suffixes[0])) - 1) {
        size /= 1024; // 将大小除以1024
        suffixIndex++; // 后缀索引加1
    }

    stringstream ss; // 创建字符串流对象
    ss << fixed << setprecision(2) << size << " " << suffixes[suffixIndex]; // 将格式化后的大小和后缀单位写入字符串流
    return ss.str(); // 返回字符串流中的字符串
}

//显示权限信息
void printProtection(DWORD dwTarget)
{
	char as[] = "----------";
	if (dwTarget & PAGE_NOACCESS) as[0] = 'N';
	if (dwTarget & PAGE_READONLY) as[1] = 'R';
	if (dwTarget & PAGE_READWRITE)as[2] = 'W';
	if (dwTarget & PAGE_WRITECOPY)as[3] = 'C';
	if (dwTarget & PAGE_EXECUTE) as[4] = 'X';
	if (dwTarget & PAGE_EXECUTE_READ) as[5] = 'r';
	if (dwTarget & PAGE_EXECUTE_READWRITE) as[6] = 'w';
	if (dwTarget & PAGE_EXECUTE_WRITECOPY) as[7] = 'c';
	if (dwTarget & PAGE_GUARD) as[8] = 'G';
	if (dwTarget & PAGE_NOCACHE) as[9] = 'D';
	if (dwTarget & PAGE_WRITECOMBINE) as[10] = 'B';
	printf("  %s  ", as);
}

//显示帮助
void ShowHelp(){
  cout<<"--------------------------------------------------------------"<<endl;
  cout<<"cmd type: "<<endl
    << "\"system\"  : Display overall computer information"<<endl
    << "\"memory\"  : Display active process information"<<endl
    << "\"process\"  : Display active process information"<<endl
    << "\"pid\"  : View detailed information for a specific process"<<endl
    << "\"help\"  : Display help"<<endl
    << "\"clear\" or \"cls\"  : clear such as use CTRL+l in cmd"<<endl
    << "\"quit\"  : Exit the program"<<endl;
  cout<<"--------------------------------------------------------------"<<endl;
  return;
}

//显示系统信息
void displaySystemConfig() {
  SYSTEM_INFO si;
  ZeroMemory(&si, sizeof(si));
  GetSystemInfo(&si);

  // 计算页面大小的不同单位
  DWORD pageSizeKB = si.dwPageSize / 1024;
  DWORD pageSizeMB = pageSizeKB / 1024;
  DWORD pageSizeGB = pageSizeMB / 1024;

  MEMORYSTATUSEX memStatus;
  memStatus.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memStatus);

  // 计算可用用户内存大小并将其格式化为字符串
  DWORDLONG availableUserMemory = memStatus.ullAvailPhys / (1024 * 1024);

  cout << "Computer Overall Information: " << endl;
  cout << "--------------------------------------------------------------" << endl;
  cout << "Processor Architecture    : " << (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ? 
                                          "x64" : "x86") << endl;
  cout << "Number of Cores           : " << si.dwNumberOfProcessors << endl;
  cout << "Memory Page Size          : " << pageSizeKB << " KB (" << pageSizeMB << " MB, " << pageSizeGB << " GB)" << endl;
  cout << "Minimum User Address      : 0x" << hex << setfill('0') << setw(8) << reinterpret_cast<DWORD_PTR>(si.lpMinimumApplicationAddress) << endl;
  cout << "Maximum User Address      : 0x" << hex << setw(8) << reinterpret_cast<DWORD_PTR>(si.lpMaximumApplicationAddress) << endl;
  cout << "Available User Memory     : " << FormatByteSize(availableUserMemory * (1024 * 1024)) << endl;
  cout << "--------------------------------------------------------------" << endl;
}

//显示内存信息
void displayMemoryCondition(){
  //定义大小
  long MB = 1024 * 1024;//1M
	long GB = MB * 1024;//1G
	MEMORYSTATUSEX stat;
	stat.dwLength = sizeof(stat);
  //获取内存信息
	GlobalMemoryStatusEx(&stat);	

	cout << "MemoryCondition: " << endl;
	cout << "--------------------------------------------------------------" << endl;
	cout<< "MemoryUsage                    : " << setbase(10) << stat.dwMemoryLoad << "%\n"
		<< "TotalPhysicalMemory            : " << setbase(10) << (float)stat.ullTotalPhys / GB << "GB\n"
		<< "AvailablePhysicalMemory        : " << setbase(10) << (float)stat.ullAvailPhys / GB << "GB\n"
		<< "TotalPageFileSize              : " << (float)stat.ullTotalPageFile / GB << "GB\n"
		<< "AvailablePageFileSize          : " << (float)stat.ullAvailPageFile / GB << "GB\n"
		<< "TotalVirtualMemory             : " << (float)stat.ullTotalVirtual / GB  << "GB\n"
		<< "AvailableVirtualMemory         : " << (float)stat.ullAvailVirtual / GB << "GB" << endl;
	 cout << "--------------------------------------------------------------" << endl;
}

//获取所有进程的信息
void getAllProcessInformation(){
  cout<<"AllProcessInformation: "<<endl;
  //创建快照
  HANDLE hProcessShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  //创建失败
	if (hProcessShot == INVALID_HANDLE_VALUE){
    cout<<"Error of create Snapshot!Try it again"<<endl;
    return;
  }
  //遍历快照
	cout << " |  num  |  pid  |  NameOfProcess" << endl;
	cout << "--------------------------------------------------------------" << endl;
  PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
  //获取第一个进程
	bool more= Process32First(hProcessShot, &pe32);
	int process_num = 1;
  //遍历获取到没有进程为止
  while(more){
    printf(" | %4d  | %5d  |  %s\n", process_num++,pe32.th32ProcessID, pe32.szExeFile);
    //获取洗一个进程
    more = Process32Next(hProcessShot, &pe32);
  }
	cout << "--------------------------------------------------------------" << endl;
  //关闭快照
  CloseHandle(hProcessShot);
}

//获取进程具体的信息
void getProcessDetail(int pid){
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	if (!hProcess) return;

  //建表
  cout<<" | "<<"   Memory Addr    | "<<"   Size    | "<<"PageStatus| "<<"    Protect    | "<<"  Type  | "<< " ModuleName"<< endl;

  //系统信息
  SYSTEM_INFO si;					
	ZeroMemory(&si, sizeof(si));
	GetSystemInfo(&si);

	MEMORY_BASIC_INFORMATION mbi;
	ZeroMemory(&mbi, sizeof(mbi));

  LPCVOID pBlock = (LPVOID)si.lpMinimumApplicationAddress;//从最低内存遍历进程所有内存
	while (pBlock < si.lpMaximumApplicationAddress) 
	{
		//给定进程句柄，从pBlock开始查询，将检查到的第一个内存区域信息存到mbi中
		VirtualQueryEx(hProcess, pBlock, &mbi, sizeof(mbi));
		LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;
		
		// 区域大小
    string sizeStr = FormatByteSize(mbi.RegionSize);

		// 地址区间与区域大小
		cout.fill('0');
		cout<<" | " << hex << setw(8) << reinterpret_cast<SIZE_T>(pBlock)
			<< "-"
			<< hex << setw(8) << reinterpret_cast<SIZE_T>(pEnd) - 1
			<< " | ";
		cout<<setw(11)<<sizeStr;

		// 输出块状态，提交，空闲，保留。
		switch (mbi.State)
		{

		case MEM_COMMIT:cout << " | " << setw(9) << "Committed" << " | "; break;
		case MEM_FREE:cout << " | " << setw(9) << "   Free  " << " | "; break;
		case MEM_RESERVE:cout << " | " << setw(9) << " Reserved" << " | "; break;
		default: cout << "   None   | "; break;
		}

		// 保护状态
		if (mbi.Protect == 0 && mbi.State != MEM_FREE)
		{

			mbi.Protect = PAGE_READONLY;

		}
		printProtection(mbi.Protect);

		//页面类型：可执行映像，私有内存区，内存映射文件
		switch (mbi.Type)
		{
		case MEM_IMAGE:cout << " |  Image  | "; break;
		case MEM_PRIVATE:cout << " | Private | "; break;
		case MEM_MAPPED:cout << " |  Mapped | "; break;
		default:cout << " |   None  | "; break;
		}

		// 模块名，如果有模块名，就输出
		TCHAR str_module_name[MAX_PATH];
		if (GetModuleFileName((HMODULE)pBlock, str_module_name, MAX_PATH) > 0) {
			// PathStripPath(str_module_name);
			printf("%s", str_module_name);
		}
		cout << endl;
		pBlock = pEnd;	// 切换基址
	}
}