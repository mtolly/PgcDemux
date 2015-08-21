#if !defined(AFX_CUSTOMVOB_H__F6337452_1F96_46C6_99A0_C50F7899537E__INCLUDED_)
#define AFX_CUSTOMVOB_H__F6337452_1F96_46C6_99A0_C50F7899537E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomVob.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustomVob dialog

class CCustomVob : public CToolTipDialog
{
// Construction
public:
	CCustomVob(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCustomVob)
	enum { IDD = IDD_CUSTOMVOB };
	BOOL	m_bCheckIFrame;
	BOOL	m_bCheckVideoPack;
	BOOL	m_bCheckAudioPack;
	BOOL	m_bCheckNavPack;
	BOOL	m_bCheckSubPack;
	BOOL	m_bCheckLBA;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomVob)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();

	// Generated message map functions
	//{{AFX_MSG(CCustomVob)
	virtual void OnCancel();
	afx_msg void OnDefault();
	virtual void OnOK();
	afx_msg void OnCheckvideopack();
	//}}AFX_MSG

	virtual void CheckButtons();


	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMVOB_H__F6337452_1F96_46C6_99A0_C50F7899537E__INCLUDED_)
