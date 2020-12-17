#include <Windows.h>
#include <vector>
#include <string>

typedef std::vector<std::string>(*ALLPORTS)();
typedef std::string(*CERTAINPORT)(int);

TCHAR dllName[] = TEXT("PortScannerLib");

int main()
{
	HINSTANCE hMyDll;
	if ((hMyDll = LoadLibrary(dllName)) != NULL)
	{
		ALLPORTS allPorts;
		CERTAINPORT certainPort;

		allPorts = (ALLPORTS)GetProcAddress(hMyDll, "GetPortsInfo");
		certainPort = (CERTAINPORT)GetProcAddress(hMyDll, "CheckSpecificPort");

		std::vector<std::string> data = allPorts();
		std::string a = certainPort(4);
		std::string b = certainPort(90909090);
	}
}