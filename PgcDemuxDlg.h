// PgcDemuxDlg.h : header file
//

#if !defined(AFX_PGCDEMUXDLG_H__5231FBC1_E513_470B_9DA3_797DAA84B3FB__INCLUDED_)
#define AFX_PGCDEMUXDLG_H__5231FBC1_E513_470B_9DA3_797DAA84B3FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPgcDemuxDlg dialog

class CPgcDemuxDlg : public CToolTipDialog
{
// Construction
public:
	CPgcDemuxDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPgcDemuxDlg)
	enum { IDD = IDD_PGCDEMUX_DIALOG };
	CComboBox	m_ComboAng;
	CProgressCtrl	m_CProgress;
	CComboBox	m_ComboPgc;
	BOOL	m_bCheckAud;
	BOOL	m_bCheckSub;
	BOOL	m_bCheckVid;
	BOOL	m_bCheckVob;
	BOOL	m_bCheckLog;
	BOOL	m_bCheckCellt;
	int		m_iRadioT;
	CString	m_csEditInput;
	CString	m_csEditOutput;
	CString	m_csEditCells;
	int		m_iRadioMode;
	BOOL	m_bCheckVob2;
	BOOL	m_bCheckEndTime;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPgcDemuxDlg)
	protected:
	virtual	void OnDropFiles( HDROP hDropInfo);
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

public:
	virtual void UpdateProgress (int nPerc);

protected:
	virtual void CheckEnableButtons();
	virtual void ClearControls();
	virtual void ReadInputFile();



// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPgcDemuxDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtoninput();
	afx_msg void OnButtonoutput();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnAbort();
	afx_msg void OnRadiom();
	afx_msg void OnRadiot();
	afx_msg void OnSelchangeCombopgc();
	afx_msg void OnButtoncustom();
	afx_msg void OnCheckvob();
	afx_msg void OnRadiopgc();
	afx_msg void OnRadiovob();
	afx_msg void OnRadiocell();
	afx_msg void OnButtondelay();
	afx_msg void OnCheckcellt();
	afx_msg void OnCancel2();
	//}}AFX_MSG

protected:
	virtual void FillComboPgc();
	virtual void FillAnglePgc();
	virtual void ShowControls();
	virtual bool FilesAlreadyExist();
	virtual bool AnyFileExists(CString csFile);




	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PGCDEMUXDLG_H__5231FBC1_E513_470B_9DA3_797DAA84B3FB__INCLUDED_)
