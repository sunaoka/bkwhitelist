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

#pragma warning (disable: 4786) // ﾃﾞﾊﾞｯｸﾞ情報で識別子が 255 文字に切り捨てられました。
#pragma warning (disable: 4503) // 装飾された名前の長さが限界を越えました。名前は切り捨てられます。

#include <windows.h>
#include "BeckyAPI.h"
#include "BkCommon.h"

#include "bkwhitelist.h"
#include "resource.h"
#include "PropertyDlg.h"

extern char szIni[_MAX_PATH+2];
extern CBeckyAPI bka;
extern HINSTANCE g_hInstance;
extern TMailBox g_temporary_mailbox;
extern std::string g_current_mailbox;

BOOL InitPropertyDialog(HWND hWnd)
{
	// メールアドレスを取得し，テキストボックスに入れる
	pocari_org::array array;
	std::string eMails = array.join(g_temporary_mailbox[g_current_mailbox], "\r\n");
	if (!eMails.empty()) {
		eMails.append("\r\n");
	}
	SetDlgItemText(hWnd, IDC_EDIT_EMAIL, eMails.c_str());

	return true;
}

BOOL OnOK(HWND hWnd)
{
	std::vector<char> buffer(32768);

	GetDlgItemText(hWnd, IDC_EDIT_EMAIL, &buffer[0], buffer.size());

	// メールアドレスを改行で分解
	pocari_org::tokenizer token;
	TSet mailAddresses;
	token.parse(mailAddresses, &buffer[0], "\r\n");

	// メールアドレスをセット
	g_temporary_mailbox[g_current_mailbox] = mailAddresses;

	return true;
}

BOOL CALLBACK PropertyDlgFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		return InitPropertyDialog(hWnd);

	case WM_COMMAND:
		switch (LOWORD(wParam))	{
		case IDOK:
			OnOK(hWnd);
			EndDialog(hWnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;

		case IDC_EDIT_EMAIL:
			switch(HIWORD(wParam)) {
			case EN_SETFOCUS:
				// 選択解除
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_EMAIL), EM_SETSEL, -1, -1);
				break ;
			}
		}
	}
	return false;
}