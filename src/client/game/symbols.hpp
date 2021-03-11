#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	/***************************************************************
	 * Functions
	 **************************************************************/

	WEAK symbol<void(void*, void*)> AimAssist_AddToTargetList{0, 0x140001730};

	WEAK symbol<void(errorParm code, const char* message, ...)> Com_Error{0x1402F7570, 0x1403CE480};
	WEAK symbol<void()> Com_Frame_Try_Block_Function{0x1402F7E10, 0x1403CEF30};
	WEAK symbol<CodPlayMode()> Com_GetCurrentCoDPlayMode{0, 0x1404C9690};
	WEAK symbol<void()> Com_Quit_f{0x1402F9390, 0x1403D08C0};

	WEAK symbol<void(const char* cmdName, void (), cmd_function_s* allocedCmd)> Cmd_AddCommandInternal{
		0x1402EDDB0, 0x1403AF2C0
	};
	WEAK symbol<void(int localClientNum, int controllerIndex, const char* text)> Cmd_ExecuteSingleCommand{
		0x1402EE350, 0x1403AF900
	};
	WEAK symbol<void(const char*)> Cmd_RemoveCommand{0x1402EE910, 0x1403AFEF0};
	WEAK symbol<void(const char* text_in)> Cmd_TokenizeString{0x1402EEA30, 0x1403B0020};
	WEAK symbol<void()> Cmd_EndTokenizeString{0x1402EE000, 0x1403AF5B0};

	WEAK symbol<void(const char* message)> Conbuf_AppendText{0x14038F220, 0x1404D9040};

	WEAK symbol<void(int localClientNum, void (*)(int localClientNum))> Cbuf_AddCall{0x1402ED820, 0x1403AECF0};
	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{0x1402ED890, 0x1403AED70};
	WEAK symbol<void(int localClientNum, int controllerIndex, const char* buffer,
	                 void (int, int, const char*))> Cbuf_ExecuteBufferInternal{0x1402ED9A0, 0x1403AEE80};

	WEAK symbol<bool()> CL_IsCgameInitialized{0x140136560, 0x1401FD510};

	WEAK symbol<void(int localClientNum, const char* message)> CG_GameMessage{0x1400EE500, 0x1401A3050};
	WEAK symbol<void(int localClientNum, /*mp::cg_s**/void* cg, const char* dvar, const char* value)>
	CG_SetClientDvarFromServer
		{0, 0x1401BF0A0};

	WEAK symbol<void(XAssetType type, void (__cdecl* func)(XAssetHeader, void*), void* inData, bool includeOverride)>
	DB_EnumXAssets_FastFile{0x14017D7C0, 0x14026EC10};
	WEAK symbol<int(XAssetType type)> DB_GetXAssetTypeSize{0x140151C20, 0x140240DF0};
	WEAK symbol<void(XZoneInfo* zoneInfo, unsigned int zoneCount, DBSyncMode syncMode)> DB_LoadXAssets{
		0x1402F8B50, 0x140270F30
	};

	WEAK symbol<dvar_t*(const char* name)> Dvar_FindVar{0x140370860, 0x1404BF8B0};
	WEAK symbol<void(const dvar_t* dvar)> Dvar_ClearModified{0x140370700, 0x1404BF690};
	WEAK symbol<void(char* buffer, int index)> Dvar_GetCombinedString{0x1402FB590, 0x1403D3290};
	WEAK symbol<bool(const char* name)> Dvar_IsValidName{0x140370CB0, 0x1404BFF70};
	WEAK symbol<void(dvar_t* dvar, DvarSetSource source)> Dvar_Reset{0x140372950, 0x1404C1DB0};
	WEAK symbol<void(const char* dvar, const char* buffer)> Dvar_SetCommand{0x1403730D0, 0x1404C2520};
	WEAK symbol<void(dvar_t* dvar, const char* string)> Dvar_SetString{0x140373DE0, 0x1404C3610};
	WEAK symbol<void(const char*, const char*, DvarSetSource)> Dvar_SetFromStringByNameFromSource{
		0x1403737D0, 0x1404C2E40
	};
	WEAK symbol<const char*(dvar_t* dvar, dvar_value value)> Dvar_ValueToString{0x140374E10, 0x1404C47B0};

	WEAK symbol<dvar_t*(const char* dvarName, bool value, unsigned int flags, const char* description)>
	Dvar_RegisterBool{0x140371850, 0x1404C0BE0};
	WEAK symbol<dvar_t*(const char* dvarName, const char** valueList, int defaultIndex, unsigned int flags,
	                    const char* description)> Dvar_RegisterEnum{0x140371B30, 0x1404C0EC0};
	WEAK symbol<dvar_t*(const char* dvarName, float value, float min, float max, unsigned int flags,
	                    const char* description)> Dvar_RegisterFloat{0x140371C20, 0x1404C0FB0};
	WEAK symbol<dvar_t*(const char* dvarName, int value, int min, int max, unsigned int flags, const char* desc)>
	Dvar_RegisterInt{0x140371CF0, 0x1404C1080};
	WEAK symbol<dvar_t*(const char* dvarName, const char* value, unsigned int flags, const char* description)>
	Dvar_RegisterString{0x140372050, 0x1404C1450};
	WEAK symbol<dvar_t*(const char* dvarName, float x, float y, float z, float w, float min, float max,
	                    unsigned int flags, const char* description)> Dvar_RegisterVec4{0x140372430, 0x1404C1800};

	WEAK symbol<DWOnlineStatus()> dwGetLogOnStatus{0, 0x14053CCB0};

	WEAK symbol<long long(const char* qpath, char** buffer)> FS_ReadFile{0x140362390, 0x1404AF380};
	WEAK symbol<void(void* buffer)> FS_FreeFile{0x140362380, 0x1404AF370};

	WEAK symbol<void()> G_Glass_Update{0x14021D540, 0x1402EDEE0};

	WEAK symbol<unsigned int(const char* name)> G_GetWeaponForName{0x140274590, 0x14033FF60};
	WEAK symbol<int(playerState_s* ps, unsigned int weapon, int dualWield, int startInAltMode, int, int, int, char,
	                ...)>
	G_GivePlayerWeapon{0x1402749B0, 0x140340470};
	WEAK symbol<void(playerState_s* ps, unsigned int weapon, int hadWeapon)> G_InitializeAmmo{0x1402217F0, 0x1402F22B0};
	WEAK symbol<void(int clientNum, unsigned int weapon)> G_SelectWeapon{0x140275380, 0x140340D50};
	WEAK symbol<int(playerState_s* ps, unsigned int weapon)> G_TakePlayerWeapon{0x1402754E0, 0x1403411D0};

	WEAK symbol<char*(char* string)> I_CleanStr{0x140379010, 0x1404C99A0};

	WEAK symbol<const char*(int, int, int)> Key_KeynumToString{0x14013F380, 0x140207C50};

	WEAK symbol<unsigned int(int)> Live_SyncOnlineDataFlags{0x1404459A0, 0x140562830};

	WEAK symbol<void(int clientNum, const char* menu, int a3, int a4, unsigned int a5)> LUI_OpenMenu{0, 0x14048E450};

	WEAK symbol<bool(int clientNum, const char* menu)> Menu_IsMenuOpenAndVisible{0, 0x140488570};

	WEAK symbol<Material*(const char* material)> Material_RegisterHandle{0x1404919D0, 0x1405AFBE0};

	WEAK symbol<void(netadr_s*, sockaddr*)> NetadrToSockadr{0, 0x1404B6F10};

	WEAK symbol<void(netsrc_t, netadr_s*, const char*)> NET_OutOfBandPrint{0, 0x1403DADC0};
	WEAK symbol<void(netsrc_t sock, int length, const void* data, const netadr_s* to)> NET_SendLoopPacket{
		0, 0x1403DAF80
	};
	WEAK symbol<bool(const char* s, netadr_s* a)> NET_StringToAdr{0, 0x1403DB070};

	WEAK symbol<void(float x, float y, float width, float height, float s0, float t0, float s1, float t1,
	                 float* color, Material* material)> R_AddCmdDrawStretchPic{0x1404A2580, 0x1405C0CB0};
	WEAK symbol<void(const char*, int, Font_s*, float, float, float, float, float, float*, int)> R_AddCmdDrawText{
		0x1404A2BF0, 0x1405C1320
	};

	WEAK symbol<void(const char*, int, Font_s*, float, float, float, float, float, const float*, int, int, char)>
	R_AddCmdDrawTextWithCursor{0x1404A35E0, 0x1405C1D10};
	WEAK symbol<Font_s*(const char* font)> R_RegisterFont{0x140481F90, 0x14059F3C0};
	WEAK symbol<void()> R_SyncRenderThread{0x1404A4D60, 0x1405C34F0};
	WEAK symbol<int(const char* text, int maxChars, Font_s* font)> R_TextWidth{0x140482270, 0x14059F6B0};

	WEAK symbol<ScreenPlacement*()> ScrPlace_GetViewPlacement{0x14014FA70, 0x14023CB50};

	WEAK symbol<unsigned int()> Scr_AllocArray{0x140317C50, 0x1403F4280};
	WEAK symbol<const float*(const float* v)> Scr_AllocVector{0x140317D10, 0x1403F4370};
	WEAK symbol<const char*(int index)> Scr_GetString{0x14031C570, 0x1403F8C50};
	WEAK symbol<unsigned int()> Scr_GetInt{0x14031C1F0, 0x1403F88D0};
	WEAK symbol<float(int index)> Scr_GetFloat{0x14031C090, 0x1403F8820};
	WEAK symbol<int()> Scr_GetNumParam{0x14031C2A0, 0x1403F8980};
	WEAK symbol<void()> Scr_ClearOutParams{0x14031B7C0, 0x1403F8040};

	WEAK symbol<scr_string_t(const char* str)> SL_FindString{0x140314AF0, 0x1403F11C0};

	WEAK symbol<void(const char* text_in)> SV_Cmd_TokenizeString{0, 0x1403B0640};
	WEAK symbol<void()> SV_Cmd_EndTokenizedString{0, 0x1403B0600};

	WEAK symbol<mp::gentity_s*(const char* name)> SV_AddBot{0, 0x140438EC0};
	WEAK symbol<bool(int clientNum)> SV_BotIsBot{0, 0x140427300};
	WEAK symbol<mp::gentity_s*(int)> SV_AddTestClient{0, 0x140439190};
	WEAK symbol<bool(mp::gentity_s*)> SV_CanSpawnTestClient{0, 0x140439460};
	WEAK symbol<int(mp::gentity_s* ent)> SV_SpawnTestClient{0, 0x14043C750};

	WEAK symbol<void(mp::gentity_s*)> SV_AddEntity{0, 0x1403388B0};

	WEAK symbol<void(netadr_s* from)> SV_DirectConnect{0, 0x1404397A0};
	WEAK symbol<void(mp::client_t* client)> SV_DropClient{0, 0x140438A30};
	WEAK symbol<void(mp::client_t*, const char*, int)> SV_ExecuteClientCommand{0, 0x15121D8E6};
	WEAK symbol<void(int localClientNum)> SV_FastRestart{0, 0x1404374E0};
	WEAK symbol<void(int clientNum, svscmd_type type, const char* text)> SV_GameSendServerCommand{
		0x1403F3A70, 0x14043E120
	};
	WEAK symbol<const char*(int clientNum)> SV_GetGuid{0, 0x14043E1E0};
	WEAK symbol<playerState_s*(int num)> SV_GetPlayerstateForClientNum{0x1403F3AB0, 0x14043E260};
	WEAK symbol<void(int clientNum, const char* reason)> SV_KickClientNum{0, 0x1404377A0};
	WEAK symbol<bool()> SV_Loaded{0x1403F42C0, 0x14043FA50};
	WEAK symbol<bool(const char* map)> SV_MapExists{0, 0x140437800};
	WEAK symbol<void(int localClientNum, const char* map, bool mapIsPreloaded)> SV_StartMap{0, 0x140438320};
	WEAK symbol<void(int localClientNum, const char* map, bool mapIsPreloaded, bool migrate)> SV_StartMapForParty{
		0, 0x140438490
	};

	WEAK symbol<void(char* path, int pathSize, Sys_Folder folder, const char* filename, const char* ext)>
	Sys_BuildAbsPath{0x14037BBE0, 0x1404CC7E0};
	WEAK symbol<HANDLE(int folder, const char* baseFileName)> Sys_CreateFile{0x14037BCA0, 0x1404CC8A0};
	WEAK symbol<void(const char* error, ...)> Sys_Error{0x14038C770, 0x1404D6260};
	WEAK symbol<bool(const char* path)> Sys_FileExists{0x14038C810, 0x1404D6310};
	WEAK symbol<bool()> Sys_IsDatabaseReady2{0x1402FF980, 0x1403E1840};
	WEAK symbol<int()> Sys_Milliseconds{0x14038E9F0, 0x1404D8730};
	WEAK symbol<bool(int, void const*, const netadr_s*)> Sys_SendPacket{0x14038E720, 0x1404D8460};
	WEAK symbol<void(Sys_Folder, const char* path)> Sys_SetFolder{0x14037BDD0, 0x1404CCA10};
	WEAK symbol<void()> Sys_ShowConsole{0x14038FA90, 0x1404D98B0};

	WEAK symbol<const char*(const char*)> UI_GetMapDisplayName{0, 0x1403B1CD0};
	WEAK symbol<const char*(const char*)> UI_GetGameTypeDisplayName{0, 0x1403B1670};
	WEAK symbol<void(unsigned int localClientNum, const char** args)> UI_RunMenuScript{0, 0x140490060};
	WEAK symbol<int(const char* text, int maxChars, Font_s* font, float scale)> UI_TextWidth{0, 0x140492380};

	/***************************************************************
	 * Variables
	 **************************************************************/

	WEAK symbol<int> keyCatchers{0x1413D5B00, 0x1417E168C};
	WEAK symbol<PlayerKeyState> playerKeys{0x1413BC5DC, 0x1417DA46C};

	WEAK symbol<CmdArgs> cmd_args{0x1492EC6F0, 0x1479ECB00};
	WEAK symbol<CmdArgs> sv_cmd_args{0x1492EC7A0, 0x1479ECBB0};
	WEAK symbol<cmd_function_s*> cmd_functions{0x1492EC848, 0x1479ECC58};

	WEAK symbol<int> dvarCount{0x14A7BFF34, 0x14B32AA30};
	WEAK symbol<dvar_t*> sortedDvars{0x14A7BFF50, 0x14B32AA50};

	WEAK symbol<const char*> command_whitelist{0x140808EF0, 0x1409B8DC0};

	WEAK symbol<SOCKET> query_socket{0, 0x14B5B9180};

	WEAK symbol<void*> DB_XAssetPool{0x140804690, 0x1409B40D0};
	WEAK symbol<int> g_poolSize{0x140804140, 0x1409B4B90};
	WEAK symbol<const char*> g_assetNames{0x140803C90, 0x1409B3180};

	WEAK symbol<DWORD> threadIds{0x149632EC0, 0x147DCEA30};

	WEAK symbol<GfxDrawMethod_s> gfxDrawMethod{0x14CDFAFE8, 0x14D80FD98};

	namespace mp
	{
		WEAK symbol<gentity_s> g_entities{0, 0x144758C70};
		WEAK symbol<client_t> svs_clients{0, 0x1496C4B10};
		WEAK symbol<int> svs_numclients{0, 0x1496C4B0C};

		WEAK symbol<int> sv_serverId_value{0, 0x1488A9A60};

		WEAK symbol<char> virtualLobby_loaded{0, 0x1417E161D};
	}

	namespace sp
	{
		WEAK symbol<gentity_s> g_entities{0x143C26DC0, 0};
	}
}
