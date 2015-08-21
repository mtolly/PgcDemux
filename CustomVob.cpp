// CustomVob.cpp : implementation file
//

#include "stdafx.h"
#include "ToolTipDialog.h"
#include "PgcDemux.h"
#include "CustomVob.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CPgcDemuxApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCustomVob dialog


CCustomVob::CCustomVob(CWnd* pParent /*=NULL*/)
	: CToolTipDialog(CCustomVob::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCustomVob)
	m_bCheckIFrame = FALSE;
	m_bCheckVideoPack = FALSE;
	m_bCheckAudioPack = FALSE;
	m_bCheckNavPack = FALSE;
	m_bCheckSubPack = FALSE;
	m_bCheckLBA = FALSE;
	//}}AFX_DATA_INIT
}


void CCustomVob::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomVob)
	DDX_Check(pDX, IDC_CHECKIFRAME, m_bCheckIFrame);
	DDX_Check(pDX, IDC_CHECKVIDEOPACK, m_bCheckVideoPack);
	DDX_Check(pDX, IDC_CHECKAUDIOPACK, m_bCheckAudioPack);
	DDX_Check(pDX, IDC_CHECKNAVPACK, m_bCheckNavPack);
	DDX_Check(pDX, IDC_CHECKSUBPACK, m_bCheckSubPack);
	DDX_Check(pDX, IDC_CHECKLBA, m_bCheckLBA);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomVob, CDialog)
	//{{AFX_MSG_MAP(CCustomVob)
	ON_BN_CLICKED(IDDEFAULT, OnDefault)
	ON_BN_CLICKED(IDC_CHECKVIDEOPACK, OnCheckvideopack)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomVob message handlers
BOOL CCustomVob::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bCheckIFrame   = theApp.m_bCheckIFrame;
	m_bCheckVideoPack= theApp.m_bCheckVideoPack;
	m_bCheckAudioPack= theApp.m_bCheckAudioPack;
	m_bCheckNavPack  = theApp.m_bCheckNavPack;
	m_bCheckSubPack  = theApp.m_bCheckSubPack;
	m_bCheckLBA		 = theApp.m_bCheckLBA;

	c_bShowToolTips = TRUE;
	m_wndToolTip.Create(this);
	m_wndToolTip.Activate(c_bShowToolTips);

	m_wndToolTip.AddTool(GetDlgItem(IDOK), "Apply and close dialog");
	m_wndToolTip.AddTool(GetDlgItem(IDCANCEL), "Cancel and close dialog" );
	m_wndToolTip.AddTool(GetDlgItem(IDDEFAULT), "Change to defaults" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKNAVPACK),	 "Include Nav Packs in output file" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKVIDEOPACK), "Include Video Packs in output file" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKAUDIOPACK), "Include Audio Packs in output file" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKSUBPACK),	 "Include Subs Packs in output file" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKIFRAME),	 "Write only the video packs with the first IFrame");
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKLBA),	     "Patch LBA numbers to fit new sector numbers");

	UpdateData(FALSE);

	CheckButtons();

	return TRUE;  // return TRUE  unless you set the focus to a control

}

void CCustomVob::CheckButtons() 
{
	// TODO: Add extra cleanup here
	CWnd * pWnd;

	UpdateData(TRUE);
	if (m_bCheckVideoPack)
	{
		pWnd=GetDlgItem(IDC_CHECKIFRAME);
		pWnd->ShowWindow(SW_SHOW);
	}
	else
	{
		pWnd=GetDlgItem(IDC_CHECKIFRAME);
		pWnd->ShowWindow(SW_HIDE);
	}

}


void CCustomVob::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CCustomVob::OnDefault() 
{
	// TODO: Add your control notification handler code here
	m_bCheckIFrame   = FALSE;
	m_bCheckVideoPack= TRUE;
	m_bCheckAudioPack= TRUE;
	m_bCheckNavPack  = TRUE;
	m_bCheckSubPack  = TRUE;
	m_bCheckLBA      = TRUE;
	UpdateData(FALSE);
	CheckButtons();
		
}

void CCustomVob::OnOK() 
{
	// TODO: Add extra validation here
	CString csAux;
	int iRet;

	UpdateData(TRUE);

	if (m_bCheckIFrame    != FALSE ||
		m_bCheckVideoPack != TRUE  ||
		m_bCheckAudioPack != TRUE  ||
		m_bCheckNavPack	  != TRUE  ||
		m_bCheckLBA       != TRUE  ||
		m_bCheckSubPack   != TRUE  )
	{
		csAux="The selections you have done will create an unplayable VOB.";
		csAux+="\nThe resulting VOB should be used for testing/debugging purposes.";
		csAux+="\n\nDo you want to continue?";
		iRet=MessageBox( csAux, "Question", MB_YESNO | MB_ICONQUESTION );
		if (iRet !=IDYES) return;
	}

	theApp.m_bCheckIFrame   = m_bCheckIFrame;
	theApp.m_bCheckVideoPack= m_bCheckVideoPack;
	theApp.m_bCheckAudioPack= m_bCheckAudioPack;
	theApp.m_bCheckNavPack  = m_bCheckNavPack;
	theApp.m_bCheckSubPack  = m_bCheckSubPack;
	theApp.m_bCheckLBA		= m_bCheckLBA;
	
	CDialog::OnOK();
}

void CCustomVob::OnCheckvideopack() 
{
	// TODO: Add your control notification handler code here
	CheckButtons();
}
