#include <windows.h>
//#include <iostream>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fstream>

#include "MinHook.h"
//#include "lua.hpp"

using namespace std;

static ofstream output_file;

typedef struct lua_State lua_State;

//typedef lua_State* (*LUAL_NEWSTATE)(void);
typedef int (*LUA_PCALL)(lua_State *L, int nargs, int nresults, int errfunc);
typedef int (*LUAL_LOADFILE)(lua_State*, const char*);
typedef int (*LUAL_LOADBUFFER)(lua_State *L, const char*, size_t, const char*);
//typedef void (*LUA_CLOSE)(lua_State*);

//LUAL_NEWSTATE fp_luaL_newstate = NULL;
LUA_PCALL fp_lua_pcall = NULL;
LUAL_LOADFILE fp_luaL_loadfile = NULL;
LUAL_LOADBUFFER fp_luaL_loadbuffer = NULL;
//LUA_CLOSE fp_lua_close = NULL;

/*
void makeDirectory(const string path) {
	size_t found;
	string t, d;
	struct stat info;
	
	//Init
	d = ".";
	t = path;
	found = t.find("/");
	
	while(found != std::string::npos) {
		d += "/" + t.substr(0, found);
		t = t.substr(found+1, t.size());
		
		cout << d << endl;
		
		if(stat(d.c_str(), &info) != 0 )
			CreateDirectory(d.c_str(), NULL);
		
		found = t.find("/");
	}
}

void Dump(string filename, const char *buff, size_t sz) {
	int i;
	string str_name;
	ofstream dump_file;
	
	//Init
	str_name = "dump/" + filename + "c";
	
	//Made directories
	makeDirectory(str_name);
	
	//Open file
	dump_file.open(str_name.c_str());
	
	//Save data
	for(i = 0; i < sz; i++)
		dump_file << buff[i];
	
	//Close file
	dump_file.close();
}


lua_State* Detour_luaL_newstate() {
	lua_State* L;
	
	L = fp_luaL_newstate();
	
	//Debug
	output_file << "luaL_newstate()" << endl;
	output_file << "	L adress:	" << &L << endl;
	output_file << "	L value:	" << L << endl;
	output_file << endl;
	
    return L;
}

int Detour_luaL_loadfile(lua_State *L, const char *filename) {
	//Debug
	output_file << "luaL_loadfile(" << endl;
	output_file << "	L adress:	" << &L << endl;
	output_file << "	L value:	" << L << endl;
	output_file << "	filename:	" << filename << endl;
	output_file << ")" << endl;
	
    return fp_luaL_loadfile(L, filename);
}
*/

int Detour_luaL_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name) {
	int r;
	/*
	string str_name(name);
	
	str_name.erase(0, 1); 
	
	//Debug
	output_file << "luaL_loadbuffer(" << endl;
	output_file << "	L adress:	" << &L << endl;
	output_file << "	L value:	" << L << endl;
	output_file << "	sz:	" << sz << endl;
	output_file << "	name:	" << str_name << endl;
	output_file << ")" << endl;
	*/
	r = fp_luaL_loadbuffer(L, buff, sz, name);
	
	if(strstr(name, "/dlc_01")) {
		//output_file << "execute: mod" << endl;
		
		fp_luaL_loadfile(L, "mod");
		fp_lua_pcall(L, 0, -1, 0);
	}
	
	//Dump(name, buff, sz);
	
    return r;
}
/*
void Detour_lua_close(lua_State* L) {
	//Debug
	output_file << "lua_close(" << endl;
	output_file << "	L adress:	" << &L << endl;
	output_file << "	L value:	" << L << endl;
	output_file << ")" << endl;
	
	return fp_lua_close(L);
}
*/

DWORD WINAPI MyThread(LPVOID);
DWORD g_threadID;
HMODULE g_hModule;

extern "C" BOOL WINAPI DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
    switch(Reason) {
		case DLL_PROCESS_ATTACH:
			g_hModule = hDLL;
			DisableThreadLibraryCalls(hDLL);
			CreateThread(NULL, NULL, &MyThread, NULL, NULL, &g_threadID);
		break;
		case DLL_THREAD_ATTACH:
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_DETACH:
			break;
    }
	
    return TRUE;
}

bool start() {
	HMODULE  hdll;
	
	//Init
	hdll = GetModuleHandle("lua51.dll");
	
	// Initialize MinHook.
	if(MH_Initialize() != MH_OK)
		return false;
	
	/*
	output_file << "lua5.1.dll address:	" << hdll << endl;
	output_file << "luaL_newstate address:	" << (HMODULE)((int)hdll + 0x474D0) << endl;
	output_file << "luaL_loadfile address:	" << (HMODULE)((int)hdll + 0x285B0) << endl;
	output_file << "luaL_loadbuffer address:	" << (HMODULE)((int)hdll + 0x28540) << endl;
	output_file << "lua_close address:	" << (HMODULE)((int)hdll + 0x3DFA0) << endl;
	*/
	
	/* --- Create hooks --- */
	//luaL_newstate
	//if(MH_CreateHook(
	//		reinterpret_cast<LPVOID*>((int)hdll + 0x474D0),
	//		reinterpret_cast<LPVOID*>(&Detour_luaL_newstate), 
	//		reinterpret_cast<LPVOID*>(&fp_luaL_newstate)) != MH_OK)
	//	return false;
	
	//luaL_loadfile
	//if(MH_CreateHook(
	//		reinterpret_cast<LPVOID*>((int)hdll + 0x285B0),
	//		reinterpret_cast<LPVOID*>(&Detour_luaL_loadfile), 
	//		reinterpret_cast<LPVOID*>(&fp_luaL_loadfile)) != MH_OK)
	//	return false;
	
	fp_lua_pcall = (LUA_PCALL)((int)hdll + 0x3B90);
	fp_luaL_loadfile = (LUAL_LOADFILE)((int)hdll + 0x285B0);
	
	//luaL_loadbuffer
	if(MH_CreateHook(
			reinterpret_cast<LPVOID*>((int)hdll + 0x28540),
			reinterpret_cast<LPVOID*>(&Detour_luaL_loadbuffer), 
			reinterpret_cast<LPVOID*>(&fp_luaL_loadbuffer)) != MH_OK)
		return false;
	
	//lua_close
	//if(MH_CreateHook(
	//		reinterpret_cast<LPVOID*>((int)hdll + 0x3DFA0),
	//		reinterpret_cast<LPVOID*>(&Detour_lua_close), 
	//		reinterpret_cast<LPVOID*>(&fp_lua_close)) != MH_OK)
	//	return false;
	
	/* --- Enable hooks --- */
	//luaL_newstate
	//if(MH_EnableHook(reinterpret_cast<LPVOID*>((int)hdll + 0x474D0)) != MH_OK)
	//	return false;
	
	//luaL_loadfile
	//if(MH_EnableHook(reinterpret_cast<LPVOID*>((int)hdll + 0x285B0)) != MH_OK)
	//	return false;
	
	//luaL_loadbuffer
	if(MH_EnableHook(reinterpret_cast<LPVOID*>((int)hdll + 0x28540)) != MH_OK)
		return false;
	
	//lua_close
	//if(MH_EnableHook(reinterpret_cast<LPVOID*>((int)hdll + 0x3DFA0)) != MH_OK)
	//	return false;

	return true;
}

bool stop() {
	if (MH_Uninitialize() != MH_OK)
		return false;
	
	return true;
}

DWORD WINAPI MyThread(LPVOID v) {
	//output_file.open("output.txt");
	
	//Start Hook
	start();
	
	//output_file << "Lua Functions Hooked" << endl;
	
    return 0;
}
