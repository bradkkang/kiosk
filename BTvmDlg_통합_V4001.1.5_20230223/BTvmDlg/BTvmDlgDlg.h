
// BTvmDlgDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"

#define ID_MY_TIMER		1006

// CBTvmDlgDlg 대화 상자
class CBTvmDlgDlg : public CDialogEx
{
// 생성입니다.
public:
	CBTvmDlgDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_BTVMDLG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

	int ProcMain(void);

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()

public:
	CComboBox m_CmbCommand;
	CComboBox m_CmbTrState;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnClose();
};
