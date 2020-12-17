#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <map>
#include <string>
#include <vector>
#include <tlhelp32.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

typedef struct
{
    DWORD Port;
    DWORD PID;
    std::wstring Name;
} PORT_INFO;

typedef struct
{
    std::vector<PORT_INFO> PortInfo;
} PORT_INFO_VECTOR;

PORT_INFO_VECTOR pi;

void GetProccesesNamesAndPIDs(std::map<DWORD, std::wstring>& mNamesAndPIDs)
{
    mNamesAndPIDs.clear();

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            mNamesAndPIDs[pe32.th32ProcessID] = pe32.szExeFile;
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
}

bool GetPortInfo(PORT_INFO_VECTOR* info)
{
    PMIB_TCPTABLE_OWNER_MODULE pTcpTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    dwSize = sizeof(MIB_TCPTABLE_OWNER_MODULE);
    pTcpTable = (MIB_TCPTABLE_OWNER_MODULE*)MALLOC(dwSize);

    if (pTcpTable == NULL)
    {
        return false;
    }

    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_MODULE_LISTENER, 0)) == ERROR_INSUFFICIENT_BUFFER)
    {
        FREE(pTcpTable);
        pTcpTable = (MIB_TCPTABLE_OWNER_MODULE*)MALLOC(dwSize);
        if (pTcpTable == NULL)
        {
            return false;
        }
    }

    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_MODULE_LISTENER, 0)) == NO_ERROR)
    {
        std::map<DWORD, std::wstring> mNamesAndPIDs;
        GetProccesesNamesAndPIDs(mNamesAndPIDs);

        for (UINT i = 0; i < pTcpTable->dwNumEntries; i++)
        {
            USHORT localport = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
            DWORD pid = pTcpTable->table[i].dwOwningPid;

            PORT_INFO pi;
            pi.Port = localport;
            pi.PID = pid;
            pi.Name = mNamesAndPIDs[pid];

            info->PortInfo.push_back(pi);
        }
    }
    else
    {        
        return false;
    }

    if (pTcpTable != NULL)
    {
        FREE(pTcpTable);
    }
    
    return true;
}

std::string TextAlign(std::string text)
{
    for (int i = text.length(); i < 20; i++)
    {
        text += " ";
    }
    return text;
}

std::string TextAlign(int text)
{
    std::string temp = std::to_string(text);

    while (temp.length() < 20)
    {
        temp += " ";
    }
    return temp;
}

std::string Transform(int i)
{
    std::string temp = "";
    std::wstring t = pi.PortInfo[i].Name;
    std::string name(t.begin(), t.end());

    temp += TextAlign(name);
    temp += " | ";
    temp += TextAlign(pi.PortInfo[i].PID);
    temp += " | ";
    temp += TextAlign(pi.PortInfo[i].Port);

    return temp;
}

std::vector<std::string> TransformInfosInString()
{
    std::vector<std::string> result;

    for (int i = 0; i < pi.PortInfo.size(); i++)
    {
        result.push_back(Transform(i));
    }
    return result;
}

extern "C" _declspec(dllexport) std::vector<std::string> GetPortsInfo()
{
    if (pi.PortInfo.empty())
    {
        GetPortInfo(&pi);
    }

    std::vector<std::string> result = TransformInfosInString();
    return result;
}

extern "C" _declspec(dllexport) std::vector<std::string> CheckSpecificPort(int port)
{
    if (pi.PortInfo.empty())
    {
        GetPortInfo(&pi);
    }
    std::vector<std::string> result;

    if (port >= 0 && port <= 65535)
    {
        bool flag = true; // open = true close = flase
        for (int i = 0; i < pi.PortInfo.size(); i++)
        {
            if (port == pi.PortInfo[i].Port)
            {
                result.push_back(Transform(i));
                flag = false;
            }
        }
        if (flag)
        {
            result.push_back("Port is opened.");
        }
    }
    else
    { 
        result.push_back("Port does not exist.");
    }
    return result;
}