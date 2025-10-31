#include <string>
#include <fstream>
#include <tchar.h>

void Replace(std::string &newstr, const std::string &repl, const std::string &with)
{
	int p;
	while((p = newstr.find(repl)) != std::string::npos)
	{
		const char *s = newstr.c_str();
		newstr = newstr.substr(0, p)+with+newstr.substr(p+repl.length(), newstr.length()-(p+repl.length()));
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::string arg;
	arg += argv[1][0];
	arg += argv[1][1];
	int maxCall = atoi(arg.c_str());

	std::string tab;
	tab += (char)13;
	tab += (char)10;
	tab += (char)9;
	tab += (char)9;
	tab += (char)9;

	std::string call = "Call";
	std::string typelist = "TYPELIST_";
	std::string getargs = "";
	std::string returnargs = "";
	std::string sendargs = "";
	std::string execute= "";
	std::string argsreflist = "";
	std::string arglist = "";
	std::string typenames = "";
	std::string dispatch = "";

	std::ifstream Header; // On the stack
	Header.open("Header.h", std::ifstream::binary);
	std::string header;
	while (!Header.eof())
	{
		char a;
		Header.get(a);
		header += a;
	}

	std::ifstream RFTemplate; // On the stack
	RFTemplate.open( "RFTemplate.h", std::ifstream::binary );
	std::string base;
	while(!RFTemplate.eof())
	{
		char a;
		RFTemplate.get(a);
		base += a;
	}

	for(int callNum = 1; callNum <= maxCall; callNum++)
	{
		std::string comma = "";
		if (callNum > 1)
			comma = ", ";
	
		char buffer[10];
		_itoa_s(callNum, buffer, 10);
		std::string callNumStr = buffer;
		call = "LuaCall" + callNumStr;
		typelist = "TYPELIST_" + callNumStr;
		getargs = getargs+"P"+callNumStr+" p"+callNumStr+ "; P" + callNumStr + "* s" + callNumStr + " = LuaParameters::Set(dispatchData->L, p" + callNumStr + ", " + callNumStr + ", myFirstReturnArg); "+tab;
		returnargs = returnargs + "LuaParameters::Return(s" + callNumStr + ", "+ callNumStr + ", myFirstReturnArg);" + tab;
		sendargs = sendargs + comma + "*s" + callNumStr;
		execute = "ExecutionState rv = myFunction(" + sendargs + ");";
		argsreflist += comma+"P"+callNumStr+"&";
		arglist += comma+"P"+callNumStr;
		typenames += comma+"typename P"+callNumStr;
		dispatch = "dispatchData->numReturnValues = " + callNumStr + " - myFirstReturnArg + 1;";

		std::string newstr = base;
		Replace(newstr, "__CALL", call);
		Replace(newstr, "__TYPELIST", typelist);
		Replace(newstr, "__GETARGS", getargs);
		Replace(newstr, "__EXECUTE", execute);
		Replace(newstr, "__RETURNARGS", returnargs);
		Replace(newstr, "__ARGSREFLIST", argsreflist);
		Replace(newstr, "__ARGLIST", arglist);
		Replace(newstr, "__TYPENAMES", typenames);
		Replace(newstr, "__DISPATCH", dispatch);
		header += newstr;


	}

	header += "} \n\n #endif";

	std::ofstream RFHeader; // On the stack
	RFHeader.open( "LuaRegister.h", std::ofstream::binary );
	int c = 0;
	while(c != header.size())
	{
		RFHeader.put(header[c]);
		c++;
	}
	return 0;
}



