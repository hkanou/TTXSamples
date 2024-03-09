/*
 *  Copyright (c) 2024, Hirohisa Kanou
 *  (C) 2024 TeraTerm Project
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

// TTXWinSCP.c : Ver 0.4 2024/3/9

#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "tt_res.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winbase.h>

#define ORDER         2400 // TTXWinSCP(2400) -> TTXSSH(2500)
#define ID_MENU_BASE  55600
#define MAX_DIR_ITEMS 20

#define SECTION			    L"WinSCP"
#define DLL_NAME		    L"TTXWinSCP.dll"
#define DEFAULT_WinSCPMenuString    L"WinSCP(&J)"
#define DEFAULT_WinSCPPath	    L"C:\\Program Files (x86)\\WinSCP\\WinSCP.exe"
#define DEFAULT_WinSCPSchema	    L"scp"
#define DEFAULT_WinSCPPPort	    L"22"
#define DEFAULT_WinSCPUserEmbed	    L"off" // "off" or "AcceptTheRisk"
#define DEFAULT_WinSCPPasswordEmbed L"off" // "off" or "AcceptTheRisk"

static HANDLE hInst; /* Instance handle of TTX*.DLL */

typedef struct {
  PTTSet 	ts;
  PComVar 	cv;
  HMENU 	WinSCPMenu;
  wchar_t 	WinSCPMenuString[MAX_PATH];
  wchar_t 	DirList[MAX_DIR_ITEMS][MAX_PATH];
  int 		DirItems;
  PParseParam 	origParseParam;
  PReadIniFile 	origReadIniFile;
  wchar_t 	WinSCPPath[MAX_PATH];
  wchar_t 	WinSCPSchema[MAX_PATH];
  wchar_t 	WinSCPPort[MAX_PATH];
  wchar_t 	WinSCPUserEmbed[MAX_PATH];
  wchar_t 	WinSCPUser[MAX_PATH];
  wchar_t 	ssh2_username[MAX_PATH];
  wchar_t 	WinSCPPasswordEmbed[MAX_PATH];
  wchar_t 	ssh2_password[MAX_PATH];
} TInstVar;

static TInstVar *pvar;
static TInstVar InstVar;

static void PASCAL TTXInit(PTTSet ts, PComVar cv) {
  pvar->ts 			= ts;
  pvar->cv 			= cv;
  pvar->WinSCPMenu 		= NULL;
  pvar->WinSCPMenuString[0] 	= 0;
  pvar->DirItems 		= 0;
  pvar->origParseParam 		= NULL;
  pvar->origReadIniFile 	= NULL;
  pvar->WinSCPPath[0] 		= 0;
  pvar->WinSCPSchema[0] 	= 0;
  pvar->WinSCPPort[0] 		= 0;
  pvar->WinSCPUserEmbed[0] 	= 0;
  pvar->WinSCPUser[0] 		= 0;
  pvar->ssh2_username[0] 	= 0;
  pvar->WinSCPPasswordEmbed[0] 	= 0;
  pvar->ssh2_password[0] 	= 0;
}

static void PASCAL percent_encode(const wchar_t *src, wchar_t *dist){
  int i, len = wcslen(src);
  wchar_t c[2];

  c[1] = 0;
  dist[0] = 0;

  for (i = 0; i < len; i++) {
    c[0] = src[i];
    if      (wcscmp((const wchar_t *)c, L" ") == 0)  {wcscat_s(dist, MAX_PATH, L"%20");}
    else if (wcscmp((const wchar_t *)c, L"!") == 0)  {wcscat_s(dist, MAX_PATH, L"%21");}
    else if (wcscmp((const wchar_t *)c, L"\"") == 0) {wcscat_s(dist, MAX_PATH, L"%22");}
    else if (wcscmp((const wchar_t *)c, L"#") == 0)  {wcscat_s(dist, MAX_PATH, L"%23");}
    else if (wcscmp((const wchar_t *)c, L"$") == 0)  {wcscat_s(dist, MAX_PATH, L"%24");}
    else if (wcscmp((const wchar_t *)c, L"%") == 0)  {wcscat_s(dist, MAX_PATH, L"%25");}
    else if (wcscmp((const wchar_t *)c, L"&") == 0)  {wcscat_s(dist, MAX_PATH, L"%26");}
    else if (wcscmp((const wchar_t *)c, L"'") == 0)  {wcscat_s(dist, MAX_PATH, L"%27");}
    else if (wcscmp((const wchar_t *)c, L"(") == 0)  {wcscat_s(dist, MAX_PATH, L"%28");}
    else if (wcscmp((const wchar_t *)c, L")") == 0)  {wcscat_s(dist, MAX_PATH, L"%29");}
    else if (wcscmp((const wchar_t *)c, L"*") == 0)  {wcscat_s(dist, MAX_PATH, L"%2A");}
    else if (wcscmp((const wchar_t *)c, L"+") == 0)  {wcscat_s(dist, MAX_PATH, L"%2B");}
    else if (wcscmp((const wchar_t *)c, L",") == 0)  {wcscat_s(dist, MAX_PATH, L"%2C");}
    else if (wcscmp((const wchar_t *)c, L"/") == 0)  {wcscat_s(dist, MAX_PATH, L"%2F");}
    else if (wcscmp((const wchar_t *)c, L":") == 0)  {wcscat_s(dist, MAX_PATH, L"%3A");}
    else if (wcscmp((const wchar_t *)c, L";") == 0)  {wcscat_s(dist, MAX_PATH, L"%3B");}
    else if (wcscmp((const wchar_t *)c, L"<") == 0)  {wcscat_s(dist, MAX_PATH, L"%3C");}
    else if (wcscmp((const wchar_t *)c, L"=") == 0)  {wcscat_s(dist, MAX_PATH, L"%3D");}
    else if (wcscmp((const wchar_t *)c, L">") == 0)  {wcscat_s(dist, MAX_PATH, L"%3E");}
    else if (wcscmp((const wchar_t *)c, L"?") == 0)  {wcscat_s(dist, MAX_PATH, L"%3F");}
    else if (wcscmp((const wchar_t *)c, L"@") == 0)  {wcscat_s(dist, MAX_PATH, L"%40");}
    else if (wcscmp((const wchar_t *)c, L"[") == 0)  {wcscat_s(dist, MAX_PATH, L"%5B");}
    else if (wcscmp((const wchar_t *)c, L"]") == 0)  {wcscat_s(dist, MAX_PATH, L"%5D");}
    else if (wcscmp((const wchar_t *)c, L"^") == 0)  {wcscat_s(dist, MAX_PATH, L"%5E");}
    else if (wcscmp((const wchar_t *)c, L"`") == 0)  {wcscat_s(dist, MAX_PATH, L"%60");}
    else if (wcscmp((const wchar_t *)c, L"{") == 0)  {wcscat_s(dist, MAX_PATH, L"%7B");}
    else if (wcscmp((const wchar_t *)c, L"|") == 0)  {wcscat_s(dist, MAX_PATH, L"%7C");}
    else if (wcscmp((const wchar_t *)c, L"}") == 0)  {wcscat_s(dist, MAX_PATH, L"%7D");}
    else if (wcscmp((const wchar_t *)c, L"~") == 0)  {wcscat_s(dist, MAX_PATH, L"%7E");}
    else                                             {wcscat_s(dist, MAX_PATH, c);}
  }
}

static void PASCAL myTolower(const wchar_t *str, wchar_t stopchar) {
  wchar_t *p, *stop;

  stop = wcschr(str, stopchar);
  if (stop != NULL) {
    for (p = (wchar_t *)str; p != stop; p++) {
      p[0] = towlower(p[0]);
    }
  }
}

static void PASCAL TTXParseParam(wchar_t *Param, PTTSet ts, PCHAR DDETopic) {
  wchar_t buff[MAX_PATH];
  wchar_t tmpstr[MAX_PATH];
  wchar_t *start, *cur, *next;

  /* the first term shuld be executable filename of Tera Term */
  start = GetParam(buff, MAX_PATH, Param);
  cur = start;

  while (next = GetParam(buff, MAX_PATH, cur)) {
    DequoteParam(buff, MAX_PATH, buff);
    if (_wcsnicmp(buff, L"/user=", 6) == 0) {
      if (wcscmp(pvar->WinSCPUserEmbed, L"AcceptTheRisk") == 0) {
	myTolower(buff, '=');
	swscanf_s(buff, L"/user=%ls", tmpstr, MAX_PATH);
	percent_encode(tmpstr, pvar->ssh2_username);
      }
    } else if (_wcsnicmp(buff, L"/passwd=", 8) == 0) {
      if (wcscmp(pvar->WinSCPPasswordEmbed, L"AcceptTheRisk") == 0) {
	myTolower(buff, '=');
	swscanf_s(buff, L"/passwd=%ls", tmpstr, MAX_PATH);
	percent_encode(tmpstr, pvar->ssh2_password);
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
	swscanf_s(p, L"%[^@]", userinfo, MAX_PATH);
	if (wcschr(userinfo, ':') != NULL) {
	  swscanf_s(userinfo, L"%[^:]:%ls", tmp_username, MAX_PATH, tmp_password, MAX_PATH);
	  percent_encode(tmp_username, pvar->ssh2_username);
	  percent_encode(tmp_password, pvar->ssh2_password);
	} else {
	  swscanf_s(p, L"%[^@]", tmp_username, MAX_PATH);
	  percent_encode(tmp_username, pvar->ssh2_username);
	}
      }
    } else if(wcschr(buff, '@') != NULL) {
      wchar_t tmp_username[MAX_PATH];
      // <username>@<host>[:<#TCP port>]
      swscanf_s(buff, L"%[^@]",	tmp_username, MAX_PATH);
      percent_encode(tmp_username, pvar->ssh2_username);
    }
    cur = next;
  }
  pvar->origParseParam(Param, ts, DDETopic);
}

static void PASCAL WinSCPReadIniFile(const wchar_t *fn, PTTSet ts) {
  int i;
  wchar_t buff[MAX_PATH];

  pvar->origReadIniFile(fn, ts);

  GetPrivateProfileStringW(SECTION, L"WinSCPMenuString", DEFAULT_WinSCPMenuString, buff, MAX_PATH, fn);
  wcscpy_s(pvar->WinSCPMenuString, MAX_PATH, buff);

  GetPrivateProfileStringW(SECTION, L"WinSCPPath", DEFAULT_WinSCPPath, buff, MAX_PATH, fn);
  wcscpy_s(pvar->WinSCPPath, MAX_PATH, buff);

  GetPrivateProfileStringW(SECTION, L"WinSCPSchema", DEFAULT_WinSCPSchema, buff, MAX_PATH, fn);
  wcscpy_s(pvar->WinSCPSchema, MAX_PATH, buff);

  GetPrivateProfileStringW(SECTION, L"WinSCPPort", DEFAULT_WinSCPPPort, buff, MAX_PATH, fn);
  wcscpy_s(pvar->WinSCPPort, MAX_PATH, buff);

  GetPrivateProfileStringW(SECTION, L"WinSCPUserEmbed", DEFAULT_WinSCPUserEmbed, buff, MAX_PATH, fn);
  wcscpy_s(pvar->WinSCPUserEmbed, MAX_PATH, buff);
  if (wcscmp(pvar->WinSCPUserEmbed, L"AcceptTheRisk") == 0) {
    GetPrivateProfileStringW(SECTION, L"WinSCPUser", NULL, buff, MAX_PATH, fn);
    wcscpy_s(pvar->WinSCPUser, MAX_PATH, buff);
  }

  GetPrivateProfileStringW(SECTION, L"WinSCPPasswordEmbed", DEFAULT_WinSCPPasswordEmbed, buff, MAX_PATH, fn);
  wcscpy_s(pvar->WinSCPPasswordEmbed, MAX_PATH, buff);

  for (i = 0; i < MAX_DIR_ITEMS; i++) {
    wchar_t key[MAX_PATH];
    _snwprintf_s(key, MAX_PATH, _TRUNCATE, L"Dir%d", i + 1);
    GetPrivateProfileStringW(SECTION, key, NULL, buff, MAX_PATH, fn);
    if (buff[0] == 0) {
      break;
    } else {
      wcscpy_s(pvar->DirList[i], MAX_PATH, buff);
    }
  }
  pvar->DirItems = i;
}

static void PASCAL TTXGetSetupHooks(TTXSetupHooks *hooks) {
  pvar->origParseParam = *hooks->ParseParam;
  *hooks->ParseParam = TTXParseParam;

  pvar->origReadIniFile = *hooks->ReadIniFile;
  *hooks->ReadIniFile = WinSCPReadIniFile;
}

static void PASCAL TTXModifyMenu(HMENU menu) {
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
        _snwprintf_s(str, MAX_PATH, _TRUNCATE, L"%ws\tAlt+&%c", pvar->DirList[i], i + 0x31);
      } else {
        _snwprintf_s(str, MAX_PATH, _TRUNCATE, L"%ws\tAlt+&%c", pvar->DirList[i], i + 0x38);
      }
      AppendMenuW(pvar->WinSCPMenu, MF_BYCOMMAND | MF_STRING | MF_ENABLED, ID_MENU_BASE + i, str);
    }
  }
}

static int PASCAL myWinExec(wchar_t *exe, wchar_t *arg) {
  wchar_t cmd[MAX_PATH];
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  DWORD code;

  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  code = NO_ERROR;

  _snwprintf_s(cmd, MAX_PATH, _TRUNCATE, L"\"%ls\" %ls", exe, arg);
  if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) != TRUE) {
    wchar_t str[MAX_PATH], errmsg[MAX_PATH];
    code = GetLastError();
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, 0, errmsg, MAX_PATH, NULL);
    _snwprintf_s(str, MAX_PATH, _TRUNCATE,
		 L"Unable to execute file:\n%ws\n\nCreateProcess failed; code %d,\n%ws",
		 pvar->WinSCPPath, code, errmsg);
    MessageBoxExW(NULL, str, DLL_NAME, MB_OK | MB_ICONERROR, 0);
  }

  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  return code;
}

static int PASCAL TTXProcessCommand(HWND hWin, WORD cmd) {
  if (cmd >= ID_MENU_BASE && cmd < ID_MENU_BASE + pvar->DirItems) {
    wchar_t tmphn1[MAX_PATH], tmphn2[MAX_PATH], tmphn3[MAX_PATH], hostname[MAX_PATH];
    wchar_t username[MAX_PATH];
    wchar_t str[MAX_PATH];
    int num;

    num = cmd - ID_MENU_BASE;

    if (strchr(pvar->ts->HostName, (int)':')) {
      // IPv6
      MultiByteToWideChar(CP_ACP, 0, pvar->ts->HostName, -1, tmphn1, MAX_PATH);
      _snwprintf_s(tmphn2, MAX_PATH, _TRUNCATE, L"[%ws]", tmphn1);
      percent_encode(tmphn2, tmphn3);
      _snwprintf_s(hostname, MAX_PATH, _TRUNCATE, L"%ws", tmphn3);
    } else {
      _snwprintf_s(hostname, MAX_PATH, _TRUNCATE, L"%hs", pvar->ts->HostName);
    }

    if (pvar->ssh2_username[0] == 0) {
      _snwprintf_s(username, MAX_PATH, _TRUNCATE, L"%ws", pvar->WinSCPUser);
    } else {
      _snwprintf_s(username, MAX_PATH, _TRUNCATE, L"%ls", pvar->ssh2_username);
    }

    // winscp.exe [scp://[user[:passwd@]host:port[/path/]]
    _snwprintf_s(str, MAX_PATH, _TRUNCATE, L"%ws://", pvar->WinSCPSchema);
    if (username[0] != 0) {
      wcscat_s(str, MAX_PATH, username);
      if (pvar->ssh2_password[0] != 0) {
	_snwprintf_s(str, MAX_PATH, _TRUNCATE, L"%s:%ws", str, pvar->ssh2_password);
      }
      wcscat_s(str, MAX_PATH, L"@");
    }

    if (pvar->WinSCPPort[0] != 0) {
      _snwprintf_s(str, MAX_PATH, _TRUNCATE, L"%s%s:%ws", str, hostname, pvar->WinSCPPort);
    } else {
      _snwprintf_s(str, MAX_PATH, _TRUNCATE, L"%s%s:%d", str, hostname, pvar->ts->TCPPort);
    }

    if (pvar->DirList[num][0] != 0) {
      _snwprintf_s(str, MAX_PATH, _TRUNCATE, L"%s:%ws/", str, pvar->DirList[num]);
    }

    myWinExec(pvar->WinSCPPath, str);
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

BOOL __declspec(dllexport) PASCAL TTXBind(WORD Version, TTXExports *exports) {
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
      break;
  }
  return TRUE;
}
