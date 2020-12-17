#include <Windows.h>
#include <vector>
#include <string>

typedef std::vector<std::string>(*MYFUNC)();

TCHAR dllName[] = TEXT("PortScannerLib");

int main()
{
	HINSTANCE hMyDll;
	if ((hMyDll = LoadLibrary(dllName)) != NULL)
	{
		MYFUNC pfnMyFunction;

		pfnMyFunction = (MYFUNC)GetProcAddress(hMyDll, "GetPortsInfo");

		std::vector<std::string> data = pfnMyFunction();
	}
}