/*
 *  Copyright (c) 2024-, Hirohisa Kanou
 *  (C) 2024- TeraTerm Project
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// TTXWinSCP.c : Ver 0.5 2026/6/17

#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "tt_res.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winbase.h>

#define ORDER			2400 // TTXWinSCP(2400) -> TTXSSH(2500)
#define ID_MENU_BASE	55600
#define MAX_DIR_ITEMS	20

#define SECTION						L"WinSCP"
#define DLL_NAME					L"TTXWinSCP.dll"
#define DEFAULT_WinSCPMenuString	L"WinSCP(&J)"
#define DEFAULT_WinSCPPath			L"C:\\Program Files (x86)\\WinSCP\\WinSCP.exe"
#define DEFAULT_WinSCPSchema		L"scp"
#define DEFAULT_WinSCPPPort			L"22"
#define DEFAULT_WinSCPUserEmbed		L"off" // "off" or "AcceptTheRisk"
#define DEFAULT_WinSCPPasswordEmbed	L"off" // "off" or "AcceptTheRisk"

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
	PTTSet			ts;
	PComVar			cv;
	HMENU			WinSCPMenu;
	wchar_t			WinSCPMenuString[64];
	wchar_t			DirList[MAX_DIR_ITEMS][MAX_PATH];
	int				DirItems;
	PParseParam		origParseParam;
	PReadIniFile	origReadIniFile;
	wchar_t			WinSCPPath[MAX_PATH];
	wchar_t			WinSCPSchema[16];
	wchar_t			WinSCPPort[8];
	wchar_t			WinSCPUserEmbed[16];
	wchar_t			WinSCPUser[MAX_PATH];
	wchar_t			ssh2_username[MAX_PATH];
	wchar_t			WinSCPPasswordEmbed[16];
	wchar_t			ssh2_password[256]; // CryptProtectMemoryを使用するため16バイト境界のサイズ
} TInstVar;

static TInstVar *pvar;
static TInstVar InstVar;

static void PASCAL TTXInit(PTTSet ts, PComVar cv)
{
	pvar->ts						= ts;
	pvar->cv						= cv;
	pvar->WinSCPMenu				= NULL;
	pvar->WinSCPMenuString[0]		= 0;
	pvar->DirItems					= 0;
	pvar->origParseParam			= NULL;
	pvar->origReadIniFile			= NULL;
	pvar->WinSCPPath[0]				= 0;
	pvar->WinSCPSchema[0]			= 0;
	pvar->WinSCPPort[0]				= 0;
	pvar->WinSCPUserEmbed[0]		= 0;
	pvar->WinSCPUser[0]				= 0;
	pvar->ssh2_username[0]			= 0;
	pvar->WinSCPPasswordEmbed[0]	= 0;
	pvar->ssh2_password[0]			= 0;
}

static void PASCAL percent_encode(const wchar_t *src, wchar_t *dist)
{
	if (src == NULL || dist == NULL) {
		return;
	}

	wchar_t escape[] = L" !\"#$%&'()*+,/:;<=>?@[]^`{|}~";
	wchar_t *encode[] = {
		L"%20",	L"%21",	L"%22",	L"%23",	L"%24",	L"%25",
		L"%26",	L"%27",	L"%28",	L"%29",	L"%2A",	L"%2B",
		L"%2C",	L"%2F",	L"%3A",	L"%3B",	L"%3C",	L"%3D",
		L"%3E",	L"%3F",	L"%40",	L"%5B",	L"%5D",	L"%5E",
		L"%60",	L"%7B",	L"%7C",	L"%7D",	L"%7E"
	};

	wchar_t *p = dist;
	int remain = MAX_PATH - 1;

	while(*src && remain > 0) {
		wchar_t c = *src++;

		int i;
		for (i = 0; escape[i] != L'\0'; i++) {
			if (c == escape[i]) {
				break;
			}
		}

		if (escape[i] != L'\0') {
			if (remain < 3) {
				break;
			}
			*p++ = encode[i][0];
			*p++ = encode[i][1];
			*p++ = encode[i][2];
			remain -= 3;
		} else {
			if (remain < 1) {
				break;
			}
			*p++ = c;
			remain -= 1;
		}
	}
	*p = L'\0';
}

static void PASCAL myTolower(wchar_t *str, wchar_t stopchar)
{
	wchar_t *p, *stop;

	stop = wcschr(str, stopchar);
	if (stop != NULL) {
		for (p = str; p != stop; p++) {
			*p = towlower(*p);
		}
	}
}

static void PASCAL TTXParseParam(wchar_t *Param, PTTSet ts, PCHAR DDETopic)
{
	wchar_t buff[MAX_PATH];
	wchar_t tmpstr[MAX_PATH];
	wchar_t *start, *cur, *next;

	/* the first term shuld be executable filename of Tera Term */
	start = GetParam(buff, _countof(buff), Param);
	cur = start;

	while (next = GetParam(buff, _countof(buff), cur)) {
		DequoteParam(buff, _countof(buff), buff);
		if (_wcsnicmp(buff, L"/user=", 6) == 0) {
			if (wcscmp(pvar->WinSCPUserEmbed, L"AcceptTheRisk") == 0) {
				myTolower(buff, '=');
				swscanf_s(buff, L"/user=%[^\t\n]", tmpstr, (unsigned)_countof(tmpstr));
				percent_encode(tmpstr, pvar->ssh2_username);
			}
		} else if (_wcsnicmp(buff, L"/passwd=", 8) == 0) {
			if (wcscmp(pvar->WinSCPPasswordEmbed, L"AcceptTheRisk") == 0) {
				myTolower(buff, '=');
				swscanf_s(buff, L"/passwd=%[^\t\n]", tmpstr, (unsigned)_countof(tmpstr));
				percent_encode(tmpstr, pvar->ssh2_password);
				CryptProtectMemory(pvar->ssh2_password, sizeof(pvar->ssh2_password), CRYPTPROTECTMEMORY_SAME_PROCESS);
			}
		} else if ((_wcsnicmp(buff, L"ssh://", 6) == 0) ||
				   (_wcsnicmp(buff, L"ssh1://", 7) == 0) ||
				   (_wcsnicmp(buff, L"ssh2://", 7) == 0) ||
				   (_wcsnicmp(buff, L"slogin://", 9) == 0) ||
				   (_wcsnicmp(buff, L"slogin1://", 10) == 0) ||
				   (_wcsnicmp(buff, L"slogin2://", 10) == 0)) {
			// <scheme>://[<username>[:<password>]@]<host>[:<TCP port>][/]
			wchar_t *p, userinfo[MAX_PATH], tmp_username[MAX_PATH], tmp_password[MAX_PATH];
			p = wcschr(buff, ':') + 3;
			if (wcschr(p, '@') != NULL) {
				swscanf_s(p, L"%[^@]", userinfo, (unsigned)_countof(userinfo));
				if (wcschr(userinfo, ':') != NULL) {
					swscanf_s(userinfo, L"%[^:]:%ls",
							  tmp_username, (unsigned)_countof(tmp_username),
							  tmp_password, (unsigned)_countof(tmp_password));
					percent_encode(tmp_username, pvar->ssh2_username);
					percent_encode(tmp_password, pvar->ssh2_password);
					CryptProtectMemory(pvar->ssh2_password, sizeof(pvar->ssh2_password), CRYPTPROTECTMEMORY_SAME_PROCESS);
				} else {
					swscanf_s(p, L"%[^@]", tmp_username, (unsigned)_countof(tmp_username));
					percent_encode(tmp_username, pvar->ssh2_username);
				}
			}
			SecureZeroMemory(userinfo, sizeof(userinfo));
			SecureZeroMemory(tmp_password, sizeof(tmp_password));
		} else if(wcschr(buff, '@') != NULL) {
			wchar_t tmp_username[MAX_PATH];
			// <username>@<host>[:<#TCP port>]
			swscanf_s(buff, L"%[^@]", tmp_username, (unsigned)_countof(tmp_username));
			percent_encode(tmp_username, pvar->ssh2_username);
		}
		cur = next;
	}

	SecureZeroMemory(buff, sizeof(buff));
	SecureZeroMemory(tmpstr, sizeof(tmpstr));

	pvar->origParseParam(Param, ts, DDETopic);
}

static void PASCAL WinSCPReadIniFile(const wchar_t *fn, PTTSet ts)
{
	pvar->origReadIniFile(fn, ts);

	GetPrivateProfileStringW(SECTION, L"WinSCPMenuString", DEFAULT_WinSCPMenuString,
							pvar->WinSCPMenuString, _countof(pvar->WinSCPMenuString), fn);

	GetPrivateProfileStringW(SECTION, L"WinSCPPath", DEFAULT_WinSCPPath,
							pvar->WinSCPPath, _countof(pvar->WinSCPPath), fn);

	GetPrivateProfileStringW(SECTION, L"WinSCPSchema", DEFAULT_WinSCPSchema,
							pvar->WinSCPSchema, _countof(pvar->WinSCPSchema), fn);

	GetPrivateProfileStringW(SECTION, L"WinSCPPort", DEFAULT_WinSCPPPort,
							pvar->WinSCPPort, _countof(pvar->WinSCPPort), fn);

	GetPrivateProfileStringW(SECTION, L"WinSCPUserEmbed", DEFAULT_WinSCPUserEmbed,
							pvar->WinSCPUserEmbed, _countof(pvar->WinSCPUserEmbed), fn);
	if (wcscmp(pvar->WinSCPUserEmbed, L"AcceptTheRisk") == 0) {
		GetPrivateProfileStringW(SECTION, L"WinSCPUser", L"",
							pvar->WinSCPUser, _countof(pvar->WinSCPUser), fn);
	}

	GetPrivateProfileStringW(SECTION, L"WinSCPPasswordEmbed", DEFAULT_WinSCPPasswordEmbed,
							pvar->WinSCPPasswordEmbed, _countof(pvar->WinSCPPasswordEmbed), fn);

	int i;
	for (i = 0; i < MAX_DIR_ITEMS; i++) {
		wchar_t key[32];
		_snwprintf_s(key, _countof(key), _TRUNCATE, L"Dir%d", i + 1);
		GetPrivateProfileStringW(SECTION, key, L"", pvar->DirList[i], _countof(pvar->DirList[i]), fn);
		if (pvar->DirList[i][0] == L'\0') {
			break;
		}
	}
	pvar->DirItems = i;
}

static void PASCAL TTXGetSetupHooks(TTXSetupHooks *hooks)
{
	pvar->origParseParam = *hooks->ParseParam;
	*hooks->ParseParam = TTXParseParam;

	pvar->origReadIniFile = *hooks->ReadIniFile;
	*hooks->ReadIniFile = WinSCPReadIniFile;
}

static void PASCAL TTXModifyMenu(HMENU menu)
{
	MENUITEMINFOW mi;
	int i;

	if (pvar->DirItems > 0) {
		pvar->WinSCPMenu = CreateMenu();
		memset(&mi, 0, sizeof(MENUITEMINFO) - sizeof(HBITMAP));
		mi.cbSize = sizeof(MENUITEMINFO) - sizeof(HBITMAP);
		mi.fMask  = MIIM_TYPE | MIIM_SUBMENU;
		mi.fType  = MFT_STRING;
		mi.hSubMenu = pvar->WinSCPMenu;
		mi.dwTypeData = pvar->WinSCPMenuString;
		InsertMenuItemW(menu, ID_HELPMENU, FALSE, &mi);

		wchar_t str[MAX_PATH];
		for (i = 0; i < pvar->DirItems; i++) {
			if (i < 9) {
				_snwprintf_s(str, _countof(str), _TRUNCATE, L"%ws\tAlt+&%c", pvar->DirList[i], i + 0x31);
			} else {
				_snwprintf_s(str, _countof(str), _TRUNCATE, L"%ws\tAlt+&%c", pvar->DirList[i], i + 0x38);
			}
			AppendMenuW(pvar->WinSCPMenu, MF_BYCOMMAND | MF_STRING | MF_ENABLED, ID_MENU_BASE + i, str);
		}
	}
}

static int PASCAL myWinExec(wchar_t *exe, wchar_t *arg)
{
	wchar_t cmd[MAX_PATH * 2 + 4];
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	DWORD code;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	code = NO_ERROR;

	_snwprintf_s(cmd, _countof(cmd), _TRUNCATE, L"\"%ls\" %ls", exe, arg);
	if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) != TRUE) {
		wchar_t str[MAX_PATH * 3], errmsg[MAX_PATH];
		code = GetLastError();
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, 0, errmsg, _countof(errmsg), NULL);
		_snwprintf_s(str, _countof(str), _TRUNCATE,
					 L"Unable to execute file:\n%ws\n\nCreateProcess failed; code %d,\n%ws",
					 pvar->WinSCPPath, code, errmsg);
		MessageBoxExW(NULL, str, DLL_NAME, MB_OK | MB_ICONERROR, 0);
	}

	SecureZeroMemory(cmd, sizeof(cmd));
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return code;
}

static int PASCAL TTXProcessCommand(HWND hWin, WORD cmd)
{
	if (cmd >= ID_MENU_BASE && cmd < ID_MENU_BASE + pvar->DirItems) {
		wchar_t hostname[MAX_PATH];
		wchar_t username[MAX_PATH];
		wchar_t str[MAX_PATH];
		wchar_t tmp[MAX_PATH], tmp1[MAX_PATH], tmp2[MAX_PATH];
		int num;

		num = cmd - ID_MENU_BASE;

		if (strchr(pvar->ts->HostName, ':')) {
			// IPv6
			MultiByteToWideChar(CP_UTF8, 0, pvar->ts->HostName, -1, tmp1, _countof(tmp1));
			_snwprintf_s(tmp2, _countof(tmp2), _TRUNCATE, L"[%ws]", tmp1);
			percent_encode(tmp2, hostname);
		} else {
			MultiByteToWideChar(CP_UTF8, 0, pvar->ts->HostName, -1, tmp2, _countof(tmp2));
			percent_encode(tmp2, hostname);
		}

		if (pvar->ssh2_username[0] == 0) {
			percent_encode(pvar->WinSCPUser, username);
		} else {
			_snwprintf_s(username, _countof(username), _TRUNCATE, L"%ls", pvar->ssh2_username);
		}

		// winscp.exe [scp://[user[:passwd@]host:port[/path/]]
		_snwprintf_s(str, _countof(str), _TRUNCATE, L"%ws://", pvar->WinSCPSchema);
		if (username[0] != 0) {
			wcscpy_s(tmp, _countof(tmp), str);
			if (pvar->ssh2_password[0] != 0) {
				CryptUnprotectMemory(pvar->ssh2_password, sizeof(pvar->ssh2_password), CRYPTPROTECTMEMORY_SAME_PROCESS);
				_snwprintf_s(str, _countof(str), _TRUNCATE, L"%ws%ws:%ws@", tmp, username, pvar->ssh2_password);
				CryptProtectMemory(pvar->ssh2_password, sizeof(pvar->ssh2_password), CRYPTPROTECTMEMORY_SAME_PROCESS);
			} else {
				_snwprintf_s(str, _countof(str), _TRUNCATE, L"%ws%ws@", tmp, username);
			}
		}

		wcscpy_s(tmp, _countof(tmp), str);
		if (pvar->WinSCPPort[0] != 0) {
			_snwprintf_s(str, _countof(str), _TRUNCATE, L"%ws%ws:%ws", tmp, hostname, pvar->WinSCPPort);
		} else {
			_snwprintf_s(str, _countof(str), _TRUNCATE, L"%ws%ws:%d", tmp, hostname, pvar->ts->TCPPort);
		}

		if (pvar->DirList[num][0] != 0) {
			wcscpy_s(tmp, _countof(tmp), str);
			percent_encode(pvar->DirList[num], tmp1);
			_snwprintf_s(str, _countof(str), _TRUNCATE, L"%ws/%ws/", tmp, &tmp1[3]);
		}

		myWinExec(pvar->WinSCPPath, str);
		SecureZeroMemory(str, sizeof(str));
		SecureZeroMemory(str, sizeof(tmp));
		return 1; // 1 = processed the message
	}
	return 0;
}

static TTXExports Exports = {
	sizeof(TTXExports),
	ORDER,

	TTXInit,
	NULL, // TTXGetUIHooks,
	TTXGetSetupHooks,
	NULL, // TTXOpenTCP,
	NULL, // TTXCloseTCP,
	NULL, // TTXSetWinSize,
	TTXModifyMenu,
	NULL, // TTXModifyPopupMenu,
	TTXProcessCommand,
	NULL, // TTXEnd,
	NULL, // TTXSetCommandLine,
	NULL, // TTXOpenFile,
	NULL  // TTXCloseFile
};

BOOL __declspec(dllexport) PASCAL TTXBind(WORD Version, TTXExports *exports)
{
	int size = sizeof(Exports) - sizeof(exports->size);
	if (size > exports->size) {
		size = exports->size;
	}
	memcpy((char *)exports + sizeof(exports->size),
		   (char *)&Exports + sizeof(exports->size),
		   size);
	return TRUE;
}

BOOL WINAPI DllMain(HANDLE hInstance,
					ULONG ul_reason_for_call,
					LPVOID lpReserved)
{
	switch( ul_reason_for_call ) {
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_ATTACH:
		hInst = hInstance;
		pvar = &InstVar;
		break;
    case DLL_PROCESS_DETACH:
		SecureZeroMemory(pvar->ssh2_password, sizeof(pvar->ssh2_password));
		break;
	}
	return TRUE;
}
