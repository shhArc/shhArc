/////////////////////////////////////////////////////////////////////////////////
// Copright 2024 David K Bhowmik
// This file is part of shhArc.
//
// shhArc is free software: you can redistribute it and/or modify
// it under the terms of the Lesser GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// shhArc is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the Lesser GNU General Public License
// along with shhArc.  If not, see <http://www.gnu.org/licenses/>.
//
// You may alternatively use this source under the terms of a specific 
// version of the shhArc Unrestricted License provided you have obtained 
// such a license from the copyright holder.
/////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503 )
#endif

#define NOMINMAX
#include <windows.h>
#undef GetObject
#include "../Common/Dictionary.h"
#include "../Common/PreciseTime.h"
#include "../Config/GCPtr.h"
#include "../Arc/Registry.h"
#include "../VM/Scheduler.h"
#include "../VM/Process.h"
#include "../VM/ClassManager.h"
#include "../Arc/Api.h"
#include "../Schema/Node.h"
#include "../Schema/Agent.h"
#include "../LuaProcess/LuaProcess.h"
#include "../Arc/God.h"
#include "VectorModule.h"
#include "Vector.h"
#include <windows.h>

using namespace shh;



class DemoHardProcess : public Process
{
public:

	DemoHardProcess(Privileges privileges, const GCPtr<Process> spawnFrom) : Process(privileges)
	{
	}

	virtual GCPtr<Process> Clone()
	{
		GCPtr<DemoHardProcess> newProcess(new DemoHardProcess(myPrivileges, GCPtr<DemoHardProcess>(this)));
		return newProcess;
	}

	virtual bool Busy() const { return false; }
};


int TestAgent(const std::string & bootFilename, const std::string& updateFilename);


REGISTER_MODULE(VectorModule)



using namespace shh;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	CoInitialize(NULL);
	SetPriorityClass(GetCurrentProcess(), 0);	// thread priority (0 normal, 1 default, 2 high, 15 realtime)
	SetThreadPriority(GetCurrentThread(), 0);	// thread priority (0 normal, 1 default, 2 high, 15 realtime)


	char buffer[500];
	wcstombs(buffer, lpCmdLine, 500);
	std::string ourCommandLineArgs(buffer);

	std::string bootFilename;
	int switchPos = (int)ourCommandLineArgs.find("-boot");
	if (switchPos < ourCommandLineArgs.size())
	{
		int startPos = (int)ourCommandLineArgs.find_first_of(" ", switchPos) + 1;
		int endPos = (int)ourCommandLineArgs.find_first_of(" ", startPos);
		bootFilename = ourCommandLineArgs.substr(startPos, endPos - startPos);
	}
	else
	{
		bootFilename = "default.lua";
	}

	std::string updateFilename;
	switchPos = (int)ourCommandLineArgs.find("-update");
	if (switchPos < ourCommandLineArgs.size())
	{
		int startPos = (int)ourCommandLineArgs.find_first_of(" ", switchPos) + 1;
		int endPos = (int)ourCommandLineArgs.find_first_of(" ", startPos);
		updateFilename = ourCommandLineArgs.substr(startPos, endPos - startPos);
	}
	else
	{
		updateFilename = "update.lua";
	}


	TestAgent(bootFilename, updateFilename);

	CoUninitialize();
	return 0;
}

int TestAgent(const std::string& bootFilename, const std::string& updateFilename)
{

	try
	{
		GCPtr<DemoHardProcess> demoProcess(new DemoHardProcess(BasicPrivilege, GCPtr<DemoHardProcess>()));
		GCPtr<Class> cls = Api::CreateClass("DemoHardNode", "Node", demoProcess, GENERICPROCESSCREATE, NODECREATE);
		Registry::GetRegistry().RegisterHardClass(cls);

		Api::CreateGod("test", "god", bootFilename, updateFilename);
		GCPtr<God> god = Api::GetGod();

		for (double time = 0; time < 100000; time += 10000)
			Api::UpdateGod(time);

		Api::CloseDown();
		
	}
	catch (shh::Exception& e)
	{
		shh::Exception& x = dynamic_cast<shh::Exception&>(e);
		std::string et("Error");
		std::wstring title(et.begin(), et.end());
		MessageBoxW(0, (LPCWSTR)x.What(), (LPCWSTR)title.c_str(), (MB_OK | MB_ICONERROR));
		return FALSE;
	}
	return 0;
}







