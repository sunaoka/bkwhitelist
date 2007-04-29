/***********************************************************************
 * Copyright (C) 2007 SUNAOKA Norifumi All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************************/

////////////////////////////////////////////////////////////////////////////////////
// Template file for plugin.
//
// You can modify and redistribute this file without any permission.
//
// Note:
// Create a sub folder under "PlugInSDK" folder. e.g. "PlugInSDK\MyProject\" and
// place your project files there.

#pragma warning (disable: 4786) // ﾃﾞﾊﾞｯｸﾞ情報で識別子が 255 文字に切り捨てられました。
#pragma warning (disable: 4503) // 装飾された名前の長さが限界を越えました。名前は切り捨てられます。

#include <windows.h>
#include <stdio.h>
#include "BeckyAPI.h"
#include "BkCommon.h"

#include "bkwhitelist.h"
#include "SetupDlg.h"
#include "resource.h"

#define PLUGIN_NAME "Becky! White list plug-in"

CBeckyAPI bka; // You can have only one instance in a project.

HINSTANCE g_hInstance = NULL;

char szIni[_MAX_PATH+2]; // Ini file to save your plugin settings.

// メールボックス-メールアドレスの配列
TMailBox g_mailbox;

/////////////////////////////////////////////////////////////////////////////
// MessageBox をフックする
HHOOK hook;

LRESULT CALLBACK MsgBoxHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0) {
		char title[64];
		GetWindowText((HWND)wParam, title, sizeof(title));
		if (strcmp(title, PLUGIN_NAME) == 0) {
			// TODO: Esc のキャンセル
			SetDlgItemText((HWND)wParam, IDCANCEL, "ホワイトリスト");
			RemoveMenu(GetSystemMenu((HWND)wParam, 0), SC_CLOSE, MF_BYCOMMAND);
		}
	}

	return CallNextHookEx(hook, nCode, wParam, lParam);
} 

/////////////////////////////////////////////////////////////////////////////
// ini ファイルから設定を読み出す
void GetSettings()
{
	char enumKeyBuffer[32767];
	char mailAddress[32767];

	// キー (メールボックス) の列挙
	GetPrivateProfileString("MailBoxes", NULL, "", enumKeyBuffer, 32767, szIni);

	for (int i = 0; i < 32768; ++i) {
		std::string mailBox = &enumKeyBuffer[i];
		if (mailBox == "") {
			break;
		}
		i += mailBox.size();
		// メールアドレスを取得
		GetPrivateProfileString("MailBoxes", mailBox.c_str(), "", mailAddress, 32767, szIni);
		// メールアドレスを分解
		pocari_org::tokenizer token;
		// カンマで分割
		TSet mailAddresses;
		token.parse(mailAddresses, mailAddress, ",");
		g_mailbox.insert(std::pair<std::string, TSet>(mailBox, mailAddresses));
	}
}

/////////////////////////////////////////////////////////////////////////////
// To/Cc/Bcc のいずれかに指定されたメールアドレスを取得する
void GetReptEMail(HWND hWnd, TSet& EMails, LPCSTR lpSpecifiedHeader)
{
	char szHeader[32768];
	char szName[256], szEMail[256];

	bka.CompGetSpecifiedHeader(hWnd, lpSpecifiedHeader, szHeader, 32768);
	
	LPSTR lpHeader = bka.SerializeRcpts(szHeader);
	LPSTR lpTok = TokenAddr(lpHeader);
	while (lpTok) {
		GetNameAndAddr(szName, 256, szEMail, 256, lpTok);
		EMails.insert(szEMail);
		lpTok = TokenAddr(NULL);
	}
	bka.Free(lpHeader);	
}
/////////////////////////////////////////////////////////////////////////////
// To/Cc/Bcc のいずれかに指定されたメールアドレスを取得する
TSet EnumReptEMail(HWND hWnd)
{
	TSet EMails;
	GetReptEMail(hWnd, EMails, "To");
	GetReptEMail(hWnd, EMails, "Cc");
	GetReptEMail(hWnd, EMails, "Bcc");

	return EMails;
}

/////////////////////////////////////////////////////////////////////////////
// DLL entry point
BOOL APIENTRY DllMain( HANDLE hModule, 
					   DWORD  ul_reason_for_call, 
					   LPVOID lpReserved
					 )
{
	g_hInstance = (HINSTANCE)hModule;
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				if (!bka.InitAPI()) {
					return FALSE;
				}
				GetModuleFileName((HINSTANCE)hModule, szIni, _MAX_PATH);
				LPSTR lpExt = strrchr(szIni, '.');
				if (lpExt) {
					strcpy(lpExt, ".ini");
				} else {
					// just in case
					strcat(szIni, ".ini");
				}
				GetSettings();
			}
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks from Becky!

#ifdef __cplusplus
extern "C"{
#endif

////////////////////////////////////////////////////////////////////////
// Called when the program is started and the main window is created.
int WINAPI BKC_OnStart()
{
	/*
	Since BKC_OnStart is called after Becky!'s main window is
	created, at least BKC_OnMenuInit with BKC_MENU_MAIN is called
	before BKC_OnStart. So, do not assume BKC_OnStart is called
	prior to any other callback.
	*/
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when the main window is closing.
int WINAPI BKC_OnExit()
{
	// Return -1 if you don't want to quit.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when menu is intialized.
int WINAPI BKC_OnMenuInit(HWND hWnd, HMENU hMenu, int nType)
{
	switch (nType) {
	case BKC_MENU_MAIN:
		{
			/* Sample of adding menu items
			HMENU hSubMenu = GetSubMenu(hMenu, 4);
			// Define CmdProc as "void WINAPI CmdProc(HWND, LPARAM)"
			UINT nID = bka.RegisterCommand("Information about this Command", nType,CmdProc);
			AppendMenu(hSubMenu, MF_STRING, nID, "&Menu item");
			*/
			/* If needed, you can register the command UI update callback.
			// Define CmdUIProc as "UINT WINAPI CmdUIProc(HWND, LPARAM)"
			bka.RegisterUICallback(nID, CmdUIProc);
			*/
		}
		break;
	case BKC_MENU_LISTVIEW:
		break;
	case BKC_MENU_TREEVIEW:
		break;
	case BKC_MENU_MSGVIEW:
		break;
	case BKC_MENU_MSGEDIT:
		break;
	case BKC_MENU_TASKTRAY:
		break;
	case BKC_MENU_COMPOSE:
		break;
	case BKC_MENU_COMPEDIT:
		break;
	case BKC_MENU_COMPREF:
		break;
	default:
		break;
	}
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a folder is opened.
int WINAPI BKC_OnOpenFolder(LPCTSTR lpFolderID)
{
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a mail is selected.
int WINAPI BKC_OnOpenMail(LPCTSTR lpMailID)
{
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called every minute.
int WINAPI BKC_OnEveryMinute()
{
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a compose windows is opened.
int WINAPI BKC_OnOpenCompose(HWND hWnd, int nMode/* See COMPOSE_MODE_* in BeckyApi.h */)
{
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when the composing message is saved.
int WINAPI BKC_OnOutgoing(HWND hWnd, int nMode/* 0:SaveToOutbox, 1:SaveToDraft, 2:SaveToReminder*/) 
{
	pocari_org::array array;

	// 直ちに送信と送信箱に保存以外は終了
	if (nMode != 0) {
		return 0;
	}
	
	// 指定されたメールボックスではない場合は終了
	std::string CurrentMailBox(bka.GetCurrentMailBox());
	if (g_mailbox.count(CurrentMailBox) == 0) {
		return 0;
	}

	// 送信先のメールアドレスを列挙
	TSet EMails = EnumReptEMail(hWnd);

	// メールアドレスのチェック
	TSet blachlistEMails = EMails;
	for (TSet::iterator whiteAddr = g_mailbox[CurrentMailBox].begin(); whiteAddr != g_mailbox[CurrentMailBox].end(); ++whiteAddr) {
		for (TSet::iterator rcptAddr = EMails.begin(); rcptAddr != EMails.end(); ++rcptAddr) {
			if (*whiteAddr == *rcptAddr) {
				blachlistEMails.erase(*rcptAddr);
			}
		}
	}	
	
	// ホワイトリストにあれば終了
	if (blachlistEMails.empty()) {
		return 0;
	}
	
	// 警告のメッセージを表示
	std::string warnningMessage = "以下のメールアドレスはホワイトリストに設定されていません。\n\n";
	warnningMessage += array.join(blachlistEMails, "\n");
	warnningMessage += "\n\n本当にメッセージを送信しますか？";
	
	hook = SetWindowsHookEx(WH_CBT, MsgBoxHookProc, NULL, GetCurrentThreadId());
	int result = MessageBox(hWnd, warnningMessage.c_str(), PLUGIN_NAME, MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2);
	UnhookWindowsHookEx(hook);
	
	int retval;
	switch (result) {
	case IDCANCEL:
		{
		std::string message("ホワイトリストに設定されているメールアドレスの一覧\n\n");
		message += array.join(g_mailbox[CurrentMailBox], "\n");
		MessageBox(hWnd, message.c_str(), PLUGIN_NAME, MB_ICONINFORMATION);
		retval = -1;
		break;
		}
	case IDYES:
		retval = 0;
		break;
	case IDNO:
	default:
		retval = -1;
		break;
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////
// Called when a key is pressed.
int WINAPI BKC_OnKeyDispatch(HWND hWnd, int nKey/* virtual key code */, int nShift/* Shift state. 0x40=Shift, 0x20=Ctrl, 0x60=Shift+Ctrl, 0xfe=Alt*/)
{
	// Return TRUE if you want to suppress subsequent command associated to this key.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a message is retrieved and saved to a folder
int WINAPI BKC_OnRetrieve(LPCTSTR lpMessage/* Message source*/, LPCTSTR lpMailID/* Mail ID*/)
{
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a message is spooled
int WINAPI BKC_OnSend(LPCTSTR lpMessage/* Message source */)
{
	// Return BKC_ONSEND_PROCESSED, if you have processed this message
	// and don't need Becky! to send it.
	// Becky! will move this message to Sent box when the sending
	// operation is done.
	// CAUTION: You are responsible for the destination of this
	// message if you return BKC_ONSEND_PROCESSED.

	// Return BKC_ONSEND_ERROR, if you want to cancel the sending operation.
	// You are responsible for displaying an error message.

	// Return 0 to proceed the sending operation.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when all messages are retrieved
int WINAPI BKC_OnFinishRetrieve(int nNumber/* Number of messages*/)
{
	// Always return 0.
	return 0;
}


////////////////////////////////////////////////////////////////////////
// Called when plug-in setup is needed.
int WINAPI BKC_OnPlugInSetup(HWND hWnd)
{
	// Return nonzero if you have processed.
	// return 1;
	DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_SETUP), hWnd, (DLGPROC)SetupDlgFunc);
	return 1;
}


////////////////////////////////////////////////////////////////////////
// Called when plug-in information is being retrieved.
typedef struct tagBKPLUGININFO
{
	char szPlugInName[80]; // Name of the plug-in
	char szVendor[80]; // Name of the vendor
	char szVersion[80]; // Version string
	char szDescription[256]; // Short description about this plugin
} BKPLUGININFO, *LPBKPLUGININFO;

int WINAPI BKC_OnPlugInInfo(LPBKPLUGININFO lpPlugInInfo)
{
	/* You MUST specify at least szPlugInName and szVendor.
	   otherwise Becky! will silently ignore your plug-in. */
	strcpy(lpPlugInInfo->szPlugInName, PLUGIN_NAME);
	strcpy(lpPlugInInfo->szVendor, "SUNAOKA, Norifumi");
	strcpy(lpPlugInInfo->szVersion, "1.0.0");
	strcpy(lpPlugInInfo->szDescription, "指定したメールアドレス以外に送信する際に確認を促すプラグイン");

	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when drag and drop operation occurs.
int WINAPI BKC_OnDragDrop(LPCSTR lpTgt, LPCSTR lpSrc, int nCount, int dropEffect)
{
	/*
	lpTgt:	A folder ID of the target folder.
			You can assume it is a root mailbox, if the string
			contains only one '\' character.
	lpSrc:	Either a folder ID or mail IDs. Multiple mail IDs are
			separated by '\n' (0x0a).
			You can assume it is a folder ID, if the string
			doesn't contain '?' character.
	nCount:	Number of items to be dropped.
			It can be more than one, if you drop mail items.
	dropEffect: Type of drag and drop operation
			1: Copy
			2: Move
			4: Link (Used for filtering setup in Becky!)
	*/
	// If you want to cancel the default drag and drop action,
	// return -1;
	// Do not assume the default action (copy, move, etc.) is always
	// processed, because other plug-ins might cancel the operation.
	return 0;
}


////////////////////////////////////////////////////////////////////////
// Called when a message was retrieved and about to be filtered.
int WINAPI BKC_OnBeforeFilter2(LPCSTR lpMessage, LPCSTR lpMailBox, int* lpnAction, char** lppParam)
{
	/*
    lpMessage: A complete message source, which ends with
    "<CRLF>.<CRLF>".
    lpnAction:	[out] Returns the filtering action to be applied.
    	#define ACTION_NOTHING		-1	// Do nothing
		#define ACTION_MOVEFOLDER	0	// Move to a folder
		#define ACTION_COLORLABEL	1	// Set the color label
		#define ACTION_SETFLAG		2	// Set the flag
		#define ACTION_SOUND		3	// Make a sound
		#define ACTION_RUNEXE		4	// Run executable file
		#define ACTION_REPLY		5	// Reply to the message
		#define ACTION_FORWARD		6	// Forward the message
		#define ACTION_LEAVESERVER	7	// Leave/delete on the server.
		#define ACTION_ADDHEADER	8	// Add "X-" header to the top of the message. (Plug-in only feature)
	lpMailBox: ID of the mailbox that is retrieving the message. (XXXXXXXX.mb\)
	lppParam:	[out] Returns the pointer to the filtering parameter string.
		ACTION_MOVEFOLDER:	Folder name.
							e.g. XXXXXXXX.mb\FolderName\
							or \FolderName\ (begin with '\') to use
							the mailbox the message belongs.
		ACTION_COLORLABEL:	Color code(BGR) in hexadecimal. e.g. 0088FF
		ACTION_SETFLAG:		"F" to set flag. "R" to set read.
		ACTION_SOUND:		Name of the sound file.
		ACTION_RUNEXE:		Command line to execute. %1 will be replaced with the path to the file that contains the entire message.
		ACTION_REPLY:		Path to the template file without extension.
								e.g. #Reply\MyReply
		ACTION_FORWARD:		Path to the template file without extension. + "*" + Address to forward.
								e.g. #Forward\MyForward*mail@address
									 *mail@address (no template)
		ACTION_LEAVESERVER:	The string consists of one or two decimals. The second decimal is optional.
							Two decimals must be separated with a space.
							First decimal	1: Delete the message from the server.
											0: Leave the message on the server.
							Second decimal	1: Do not store the message to the folder.
											0: Store the message to the folder. (default action)
							e.g. 0 (Leave the message on the server.)
								 1 1 (Delete the message on the server and do not save. (Means KILL))
		ACTION_ADDHEADER	"X-Header:data" that will be added at the top of the incoming message.
							You can specify multiple headers by separating CRLF, but each header must
							begin with "X-". e.g. "X-Plugindata1: test\r\nX-Plugindata2: test2";
	*/
	
	/* Return values
	BKC_FILTER_DEFAULT	Do nothing and apply default filtering rules.
	BKC_FILTER_PASS		Apply default filtering rules after applying the rule it returns.
	BKC_FILTER_DONE		Do not apply default rules.
	BKC_FILTER_NEXT		Request Becky! to call this callback again so that another rules can be added.
	*/
    return BKC_FILTER_DEFAULT;
}

#ifdef __cplusplus
}
#endif
