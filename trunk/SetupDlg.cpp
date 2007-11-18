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

#pragma warning (disable: 4786) // ���ޯ�ޏ��Ŏ��ʎq�� 255 �����ɐ؂�̂Ă��܂����B
#pragma warning (disable: 4503) // �������ꂽ���O�̒��������E���z���܂����B���O�͐؂�̂Ă��܂��B

#ifndef ListView_SetCheckState
#define ListView_SetCheckState(hWndLV, i, fCheck) \
		ListView_SetItemState(hWndLV, i, INDEXTOSTATEIMAGEMASK((fCheck) + 1), LVIS_STATEIMAGEMASK)
#endif

#include <windows.h>
#include <commctrl.h>
#include "BeckyAPI.h"
#include "BkCommon.h"

#include "bkwhitelist.h"
#include "resource.h"
#include "SetupDlg.h"
#include "PropertyDlg.h"

extern char szIni[_MAX_PATH+2];
extern CBeckyAPI bka;
extern HINSTANCE g_hInstance;
extern TMailBox g_mailbox;
TMailBox g_temporary_mailbox;

std::string g_current_mailbox;

BOOL EnumMailBox(THash& MailBoxes)
{
	std::string path = bka.GetDataFolder();
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(path.append("*.mb").c_str(), &fd);
	
	if (hFind == INVALID_HANDLE_VALUE) {
		return false;
	}

	do {
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			continue;
		}
		std::string mailbox_dir(fd.cFileName);
		std::string mailbox_name = bka.GetFolderDisplayName(mailbox_dir.append("\\").c_str());
		mailbox_name = mailbox_name.substr(1, mailbox_name.size() - 3);
		MailBoxes.insert(std::pair<std::string, std::string>(mailbox_dir, mailbox_name));
	} while (FindNextFile(hFind, &fd));

	FindClose(hFind);

	return true;
}

void InitTabContorol(HWND hWnd)
{
	HWND hTabCtrl = NULL;
	TC_ITEM tcItem;

	hTabCtrl = GetDlgItem(hWnd, IDC_TAB);

	// 1 �ԑO�̍��ڂɕ�������`����
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = "�S��";
	SendMessage(hTabCtrl, TCM_INSERTITEM, (WPARAM)0, (LPARAM)&tcItem);
}

void InitListViewContorol(HWND hWnd)
{
	HWND hWndLV = GetDlgItem(hWnd, IDC_LIST_MAILBOX);
	LV_COLUMN	lc;
	LV_ITEM		li;
	HIMAGELIST	hImgList;
	RECT		rect;

	GetClientRect(hWndLV, &rect);
	
	// �C���[�W���X�g�̍쐬
	hImgList = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 1 , 1);
	ImageList_AddIcon(hImgList, LoadIcon(g_hInstance, "IDI_MAILBOX"));

	ListView_SetImageList(hWndLV, hImgList, LVSIL_SMALL);
	ListView_SetExtendedListViewStyle(hWndLV, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	// �w�b�_��ǉ� (�\�����Ȃ�)
	lc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lc.fmt = LVCFMT_LEFT;

	lc.pszText = "���[���{�b�N�X";
	lc.cx = rect.right;
	lc.iSubItem = 0;
	lc.cchTextMax = strlen(lc.pszText);
	ListView_InsertColumn(hWndLV, 0, &lc);

	lc.pszText = "�t�H���_��";
	lc.cx = 0;
	lc.iSubItem = 1;
	lc.cchTextMax = strlen(lc.pszText);
	ListView_InsertColumn(hWndLV, 1, &lc);
	
	// ���[���{�b�N�X���
	THash MailBoxes;
	EnumMailBox(MailBoxes);

	// ���[���{�b�N�X��ǉ�
	int i = 0;
	for (THash::iterator itr = MailBoxes.begin(); itr != MailBoxes.end(); ++itr) {
		li.mask = LVIF_TEXT | LVIF_IMAGE;
		li.iItem = i;
		li.iImage = 0;
		// �\����
		li.pszText = (char *)((*itr).second).c_str();
		li.iSubItem = 0;
		ListView_InsertItem(hWndLV, &li);
		// ���[���{�b�N�X�̃f�B���N�g����
		li.pszText = (char *)((*itr).first).c_str();
		li.iSubItem = 1;
		ListView_SetItem(hWndLV, &li);
		
		// �ݒ肳��Ă��郁�[���{�b�N�X�Ƀ`�F�b�N������
		if (g_temporary_mailbox.count((*itr).first) == 1) {
			ListView_SetCheckState(hWndLV, i, 1);
		}

		i++;
	}
}

BOOL InitDialog(HWND hWnd)
{
	// ���[���{�b�N�X���R�s�[
	g_temporary_mailbox = g_mailbox;

	// �R�����R���g���[���̏�����
	InitCommonControls();

	// �^�u�̏�����
	InitTabContorol(hWnd);
	// ���X�g�r���[�̏�����
	InitListViewContorol(hWnd);
	
	// �v���p�e�B�{�^���𖳌��ɂ���
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY), false);

	return true;
}

BOOL OnNotifyListView(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND hWndLV = GetDlgItem(hWnd, IDC_LIST_MAILBOX);
	LV_DISPINFO *lvinfo = (LV_DISPINFO *)lParam;
	LPNMLISTVIEW pnmlist = (LPNMLISTVIEW)lParam;

	// �I������Ă���C���f�b�N�X���擾
	int iItem = ListView_GetNextItem(hWndLV, -1, LVNI_ALL | LVNI_SELECTED);
	// �`�F�b�N�{�b�N�X�̏�Ԃ��擾
	int lvChecked = ListView_GetCheckState(hWndLV, iItem);
	// �A�C�e���̏�Ԃ��擾
	int state = pnmlist->uNewState;

	switch (lvinfo->hdr.code) {
	case LVN_ITEMCHANGED:	
		// �v���p�e�B�{�^���̗L��/������؂�ւ���
		EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY), lvChecked);
		if (pnmlist->iItem == iItem) {
			// �I������Ă���A�C�e���̃`�F�b�N�{�b�N�X
			if (state != (pnmlist->uOldState & LVIS_STATEIMAGEMASK)) {
				if (state == INDEXTOSTATEIMAGEMASK(2)) {
					// �`�F�b�N���ꂽ
					EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY), true);
				} else if (state == INDEXTOSTATEIMAGEMASK(1)) {
					// �`�F�b�N���O���ꂽ
					EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY), false);
				}
			}
		}
		break;

	case NM_DBLCLK:
		if (lvChecked) {
			// �`�F�b�N����Ă�����v���p�e�B��ʂ̌Ăяo��
			OnProperty(hWnd);
		}
		break;
	}

	return true;
}

BOOL OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case IDC_LIST_MAILBOX:
		return OnNotifyListView(hWnd, wParam, lParam);
		break;
	}
	return true;
}

BOOL OnProperty(HWND hWnd)
{
	HWND hWndLV = GetDlgItem(hWnd, IDC_LIST_MAILBOX);

	// �I������Ă���C���f�b�N�X���擾
	int iItem = ListView_GetNextItem(hWndLV, -1, LVNI_ALL | LVNI_SELECTED);
	if (iItem == -1) {
		return true;
	}

	// �I������Ă��郁�[���{�b�N�X�����擾
	char szName[13];
	ListView_GetItemText(hWndLV, iItem, 1, szName, sizeof(szName));
	g_current_mailbox = szName;

	DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_PROPERTY), hWnd, (DLGPROC)PropertyDlgFunc);
	return true;
}

BOOL OnSetupOK(HWND hWnd)
{
	// ���X�g�r���[�𑖍�
	HWND hWndLV = GetDlgItem(hWnd, IDC_LIST_MAILBOX);
	int iCount = ListView_GetItemCount(hWndLV);
	char szName[13];

	// �`�F�b�N�{�b�N�X�Ƀ`�F�b�N���������̂́C���[���{�b�N�X����폜
	for (int i = 0; i < iCount; ++i) {
		if (!ListView_GetCheckState(hWndLV, i)) {
			ListView_GetItemText(hWndLV, i, 1, szName, sizeof(szName));
			g_temporary_mailbox.erase(szName);
		}
	}

	// ini �t�@�C���� MailBoxes �Z�N�V�������N���A
	WritePrivateProfileString("MailBoxes", NULL, NULL, szIni);

	common::array array;
	for (TMailBox::iterator mailbox = g_temporary_mailbox.begin(); mailbox != g_temporary_mailbox.end(); ++mailbox) {
		// ini �t�@�C���ɏ�������
		WritePrivateProfileString("MailBoxes", ((*mailbox).first).c_str(), (array.join((*mailbox).second, ",")).c_str(), szIni);
	}

	// ���[���{�b�N�X���R�s�[
	g_mailbox = g_temporary_mailbox;

	return true;
}

BOOL CALLBACK SetupDlgFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		return InitDialog(hWnd);

	case WM_NOTIFY:
		OnNotify(hWnd, wParam, lParam);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))	{
		case IDOK:
			OnSetupOK(hWnd);
			EndDialog(hWnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;

		case IDC_PROPERTY: // �v���p�e�B
			OnProperty(hWnd);
			break;
		}
	}
	return false;
}
