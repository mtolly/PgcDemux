// PgcDemuxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "io.h"
#include "ToolTipDialog.h"
#include "PgcDemux.h"
#include "PgcDemuxDlg.h"
#include "CustomVob.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CPgcDemuxApp theApp;

extern CString SelectFolder (CString csTitulo, CString csStartFolder);
extern CString FormatDuration(DWORD duration);


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_csVersion;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_csVersion = _T("");
	//}}AFX_DATA_INIT

	m_csVersion.Format(_T("PgcDemux Version %s"),PGCDEMUX_VERSION);
//	SetWindowText(csAux);

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_STATICVER, m_csVersion);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPgcDemuxDlg dialog

CPgcDemuxDlg::CPgcDemuxDlg(CWnd* pParent /*=NULL*/)
	: CToolTipDialog(CPgcDemuxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPgcDemuxDlg)
	m_bCheckAud = TRUE;
	m_bCheckSub = TRUE;
	m_bCheckVid = FALSE;
	m_bCheckVob = FALSE;
	m_bCheckLog = TRUE;
	m_bCheckCellt = TRUE;
	m_iRadioT = 1;
	m_csEditInput = _T("");
	m_csEditOutput = _T("");
	m_csEditCells = _T("");
	m_iRadioMode = 0;
	m_bCheckVob2 = FALSE;
	m_bCheckEndTime = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPgcDemuxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPgcDemuxDlg)
	DDX_Control(pDX, IDC_COMBOANG, m_ComboAng);
	DDX_Control(pDX, IDC_PROGRESS, m_CProgress);
	DDX_Control(pDX, IDC_COMBOPGC, m_ComboPgc);
	DDX_Check(pDX, IDC_CHECKAUD, m_bCheckAud);
	DDX_Check(pDX, IDC_CHECKSUB, m_bCheckSub);
	DDX_Check(pDX, IDC_CHECKVID, m_bCheckVid);
	DDX_Check(pDX, IDC_CHECKVOB, m_bCheckVob);
	DDX_Check(pDX, IDC_CHECKLOG, m_bCheckLog);
	DDX_Check(pDX, IDC_CHECKCELLT, m_bCheckCellt);
	DDX_Radio(pDX, IDC_RADIOM, m_iRadioT);
	DDX_Text(pDX, IDC_EDITINPUT, m_csEditInput);
	DDX_Text(pDX, IDC_EDITOUTPUT, m_csEditOutput);
	DDX_Text(pDX, IDC_EDITCELL, m_csEditCells);
	DDX_Radio(pDX, IDC_RADIOPGC, m_iRadioMode);
	DDX_Check(pDX, IDC_CHECKVOB2, m_bCheckVob2);
	DDX_Check(pDX, IDC_CHECKENDTIME, m_bCheckEndTime);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPgcDemuxDlg, CDialog)
	//{{AFX_MSG_MAP(CPgcDemuxDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTONINPUT, OnButtoninput)
	ON_BN_CLICKED(IDC_BUTTONOUTPUT, OnButtonoutput)
	ON_BN_CLICKED(IDABORT, OnAbort)
	ON_BN_CLICKED(IDC_RADIOM, OnRadiom)
	ON_BN_CLICKED(IDC_RADIOT, OnRadiot)
	ON_CBN_SELCHANGE(IDC_COMBOPGC, OnSelchangeCombopgc)
	ON_BN_CLICKED(IDC_BUTTONCUSTOM, OnButtoncustom)
	ON_BN_CLICKED(IDC_CHECKVOB, OnCheckvob)
	ON_BN_CLICKED(IDC_RADIOPGC, OnRadiopgc)
	ON_BN_CLICKED(IDC_RADIOVOB, OnRadiovob)
	ON_BN_CLICKED(IDC_RADIOCELL, OnRadiocell)
	ON_BN_CLICKED(IDC_BUTTONDELAY, OnButtondelay)
	ON_BN_CLICKED(IDC_CHECKCELLT, OnCheckcellt)
	ON_BN_CLICKED(IDCANCEL2, OnCancel2)
	//}}AFX_MSG_MAP
    ON_WM_DROPFILES()

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPgcDemuxDlg message handlers

BOOL CPgcDemuxDlg::OnInitDialog()
{
	CString csAux;

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	ClearControls();
	CheckEnableButtons();

	m_bCheckAud   = theApp.m_bCheckAud;
	m_bCheckSub   = theApp.m_bCheckSub;
	m_bCheckVid   = theApp.m_bCheckVid;
	m_bCheckVob   = theApp.m_bCheckVob;
	m_bCheckVob2  = theApp.m_bCheckVob2;
	m_bCheckLog   = theApp.m_bCheckLog;
	m_bCheckCellt = theApp.m_bCheckCellt;
	m_bCheckEndTime = theApp.m_bCheckEndTime;
	UpdateData(FALSE);

	c_bShowToolTips = TRUE;
	m_wndToolTip.Create(this);
	m_wndToolTip.Activate(c_bShowToolTips);

	m_wndToolTip.AddTool(GetDlgItem(IDOK), "Use this to start demux");
	m_wndToolTip.AddTool(GetDlgItem(IDCANCEL2), "Use this for...quit" );
	m_wndToolTip.AddTool(GetDlgItem(IDABORT), "Use this for abort the demux process" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKVID), "Demux video stream in a m2v file" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKAUD), "Demux all audio streams, each one in a separate file" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKSUB), "Demux all subpicture streams in sup format, each one in a separate file" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKVOB), "Creates a VOB file with and only with the selected PGC/VOB/CELL" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKVOB2), "Creates one file per VID" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKCELLT), "Generates a Celltimes.txt file. Time is expresed in frames, one row per cell" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKENDTIME), "Includes the end time in Celltimes.txt" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_CHECKLOG), "Generates a simple log file, including audio/video delays" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_RADIOM), "Select this to work in menu domain" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_RADIOT), "Select this to work in titles domain" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_EDITCELL), "Shows the number of cells of the selected PGC" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_COMBOANG), "Selects the angle (if applicable)" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_COMBOPGC), "Selects the PGC in the selected domain" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_EDITINPUT), "Selected IFO input: VTS_XX_0.IFO or VIDEO_TS.IFO" );
	m_wndToolTip.AddTool(GetDlgItem(IDC_EDITOUTPUT), "Output folder for the generated files" );

	csAux.Format(_T("PgcDemux v%s"),PGCDEMUX_VERSION);
	SetWindowText(csAux);

	DragAcceptFiles(TRUE);

	
	if ( __argc == 2 ) 
//			PostMessage(WM_COMMAND, ID_FILE_OPEN, NULL);
	{
		m_csEditInput.Format(_T("%s"),(__argv[1]));
		ReadInputFile();
	}


	return TRUE;  // return TRUE  unless you set the focus to a control

}

void CPgcDemuxDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPgcDemuxDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPgcDemuxDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CPgcDemuxDlg::OnButtoninput() 
{
	// TODO: Add your control notification handler code here
	CString	csFiltro,csTitulo;
	CString	csAux,csAux1,csAux2;

	// String del filtro de ficheros y del titulo
	UpdateData (TRUE);

	csFiltro = "IFO VTS files";
	csFiltro += " (*.IFO)|*.IFO||";
	csTitulo = "Select IFO Video Title Set";
	// OpenFile Dlg construction 
	CFileDialog dlgAbrir(TRUE,"IFO",NULL,OFN_EXPLORER|OFN_FILEMUSTEXIST ,(const char*)csFiltro);

	dlgAbrir.m_ofn.lpstrTitle = (const char*)csTitulo;

	if (dlgAbrir.DoModal() == IDOK)
	{
		m_csEditInput = dlgAbrir.GetPathName(); 
		ReadInputFile();
	}		
}

void CPgcDemuxDlg::ReadInputFile() 
{
	CString csAux,csAux1,csAux2;

	m_csEditInput.MakeUpper();
	csAux=m_csEditInput.Right(12);

	csAux1=csAux.Left(4);
	csAux=m_csEditInput.Right(6);
	csAux2=m_csEditInput.Right(12);
	if ( (csAux!="_0.IFO" || csAux1 != "VTS_" ) && csAux2 !="VIDEO_TS.IFO")
	{
		MessageBox( "Invalid input file!", "Error",  MB_OK | MB_ICONEXCLAMATION );
		m_csEditInput="";
		ClearControls();
		CheckEnableButtons();
		return;
	}
	if (csAux2=="VIDEO_TS.IFO")
	{
		theApp.m_bVMGM=true;
		m_iRadioT=MENUS;
	}
	else 
		theApp.m_bVMGM=false;

	theApp.m_csInputIFO=m_csEditInput;
	theApp.m_csInputPath=m_csEditInput.Left(m_csEditInput.ReverseFind('\\'));
	UpdateData (FALSE);
	if (theApp.ReadIFO()==0)
	{
		FillComboPgc();
		OnSelchangeCombopgc() ;
	}
	else
	{
		MessageBox( "Error reading IFO!", "Error",  MB_OK | MB_ICONEXCLAMATION );
		m_csEditInput="";
		ClearControls();
		UpdateData (FALSE);
	}
	CheckEnableButtons();
}		



void CPgcDemuxDlg::OnButtonoutput() 
{
	// TODO: Add your control notification handler code here

	CString	csPath;
	
	UpdateData (TRUE);
	csPath=SelectFolder ("Select destination Folder",m_csEditOutput);
	if (! csPath.IsEmpty())
	{
		m_csEditOutput= csPath;
		theApp.m_csOutputPath=csPath;
	}
	UpdateData (FALSE);

}

void CPgcDemuxDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	int iRet;

	iRet=MessageBox ("Do you really want to quit?","Question", MB_YESNO|MB_ICONQUESTION );
	if (iRet !=IDYES) return;

	OnCancel2();
}

void CPgcDemuxDlg::OnCancel2() 
{
	// TODO: Add your control notification handler code here
	
	theApp.m_bInProcess=false;
	theApp.m_bAbort=true;
	UpdateProgress(0);
	
	CDialog::OnCancel();
}
void CPgcDemuxDlg::OnAbort() 
{
	// TODO: Add your control notification handler code here
	CString csAux;

	theApp.m_bInProcess=false;
	theApp.m_bAbort=true;
	UpdateProgress(0);

	csAux.Format(_T("PgcDemux v%s"),PGCDEMUX_VERSION);
	SetWindowText(csAux);

}

void CPgcDemuxDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString csOutput,csInput,csAux;
	int nSelPGC,nSelAng;
	int iRet;


	UpdateData (TRUE);
	if (m_csEditOutput.Right(1)=="\\")
	{
		m_csEditOutput=m_csEditOutput.Left(m_csEditOutput.GetLength()-1);
		UpdateData (FALSE);
	}

	theApp.m_csOutputPath=m_csEditOutput;

	csOutput=theApp.m_csOutputPath;
	csInput=theApp.m_csInputPath;
	csOutput.MakeUpper();
	csInput.MakeUpper();
	
	if (FilesAlreadyExist())
	{
		iRet=MessageBox( "There are some files in destination folder\nwhich could be overwritten.\n Do you want to continue?", "Overwrite?", MB_YESNO | MB_ICONQUESTION );
		if (iRet !=IDYES) return;
	}

	if (m_csEditInput.IsEmpty())
	{
		MessageBox( "Fill input file!", "Error",  MB_OK | MB_ICONEXCLAMATION );
		return;
	}

	if (csOutput.IsEmpty())
	{
		MessageBox( "Fill outout folder!", "Error",  MB_OK | MB_ICONEXCLAMATION );
		return;
	}
	if (csOutput==csInput)
	{
		MessageBox( "Output Folder must be different from input one!", "Error",  MB_OK | MB_ICONEXCLAMATION );
		return;
	}

	if (!m_bCheckAud && !m_bCheckVid && !m_bCheckVob && !m_bCheckSub && !m_bCheckLog && !m_bCheckCellt)
	{
		MessageBox( "Check at least one checkbox!", "Error",  MB_OK | MB_ICONEXCLAMATION );
		return;
	}
	nSelPGC=m_ComboPgc.GetCurSel();
	nSelAng=m_ComboAng.GetCurSel();

	if (m_iRadioMode==PGCMODE)
	{
		if ((m_iRadioT==TITLES && theApp.m_nCells[nSelPGC]==0) ||
			(m_iRadioT==MENUS  && theApp.m_nMCells[nSelPGC]==0) )
		{
			MessageBox( "Select a PGC with at least one cell", "Error",  MB_OK | MB_ICONEXCLAMATION );
			return;
		}
	}
	if (m_iRadioMode==VIDMODE)
	{
		if ((m_iRadioT==TITLES && theApp.m_AADT_Cell_list.GetSize()==0) ||
			(m_iRadioT==MENUS  && theApp.m_MADT_Cell_list.GetSize()==0) )
		{
			MessageBox( "Select a VOB id with at least one cell", "Error",  MB_OK | MB_ICONEXCLAMATION );
			return;
		}
	}

	theApp.m_bCheckAud=m_bCheckAud;
	theApp.m_bCheckVid=m_bCheckVid;
	theApp.m_bCheckVob=m_bCheckVob;
	theApp.m_bCheckVob2=m_bCheckVob2;
	theApp.m_bCheckSub=m_bCheckSub;
	theApp.m_bCheckLog=m_bCheckLog;
	theApp.m_bCheckCellt=m_bCheckCellt;
	theApp.m_bCheckEndTime=m_bCheckEndTime;
	theApp.m_iDomain=m_iRadioT;
	theApp.m_iMode=m_iRadioMode;

	theApp.m_bAbort=false;
	theApp.m_bInProcess=true;
	CheckEnableButtons();

	if (m_iRadioMode==PGCMODE)
	{
		if (m_iRadioT==TITLES)
			iRet=theApp.PgcDemux(nSelPGC, nSelAng, this);
		else
			iRet=theApp.PgcMDemux(nSelPGC, this);
	}
	else if (m_iRadioMode==VIDMODE)
	{
		if (m_iRadioT==TITLES)
			iRet=theApp.VIDDemux(nSelPGC, this);
		else
			iRet=theApp.VIDMDemux(nSelPGC, this);
	}
	else if (m_iRadioMode==CIDMODE)
	{
		if (m_iRadioT==TITLES)
			iRet=theApp.CIDDemux(nSelPGC, this);
		else
			iRet=theApp.CIDMDemux(nSelPGC, this);
	}


	theApp.m_bInProcess=false;
	CheckEnableButtons();

	if (theApp.m_bAbort)
		csAux="Aborted";
	else
	{
		if (iRet==0)
			csAux="Finished OK.";
		else
			csAux="Finished with error";
	}

	MessageBox( csAux, "Info",  MB_OK | MB_ICONINFORMATION );
	UpdateProgress(0);

	theApp.m_bAbort=false;

	csAux.Format(_T("PgcDemux v%s"),PGCDEMUX_VERSION);
	SetWindowText(csAux);

	return;

	CDialog::OnOK();

}


void CPgcDemuxDlg::UpdateProgress(int nPerc)
{
	CString csAux;

	if (nPerc <0 ) nPerc=0;
	if (nPerc >100 ) nPerc=100;
	m_CProgress.SetPos(nPerc);

	csAux.Format(_T("%2d %%  PgcDemux"), nPerc);
	SetWindowText(csAux);

//	m_csPcProgress.Format(_T("%d %%"),nPerc);
	UpdateData(FALSE);
}

void CPgcDemuxDlg::ClearControls()
{
	CWnd *pWnd;


	m_ComboPgc.ResetContent();
	m_ComboAng.ResetContent();
	m_csEditCells="";
	pWnd = GetDlgItem(IDC_COMBOANG);
	pWnd->ShowWindow(SW_SHOW);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_RADIOT);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_RADIOM);
	pWnd->EnableWindow(FALSE);
	m_iRadioT=1;
	UpdateData(FALSE);
}

void CPgcDemuxDlg::FillComboPgc()
{
	int k,nLU,nPGC;
	CString csAux;

	m_ComboPgc.ResetContent();

	if (m_iRadioMode==PGCMODE)
	{
		if (m_iRadioT==TITLES)
		{
			for (k=0; k< theApp.m_nPGCs ;k++)
			{
				csAux.Format(_T("PGC # %02d--> "), k+1);
				csAux+= FormatDuration(theApp.m_dwDuration[k]);
				m_ComboPgc.AddString(csAux);
			}
		}
		else
		{
			for (k=0; k< theApp.m_nMPGCs ;k++)
			{
				nLU=theApp.m_nLU_MPGC[k];
				nPGC= k-theApp.m_nIniPGCinLU[nLU];
				csAux.Format(_T("LU %2d # %02d --> "), nLU+1, nPGC+1);
				csAux+= FormatDuration(theApp.m_dwMDuration[k]);
				m_ComboPgc.AddString(csAux);
			}
		}
	}

	if (m_iRadioMode==VIDMODE)
	{
		if (m_iRadioT==TITLES)
		{
			for (k=0; k< theApp.m_AADT_Vid_list.GetSize() ;k++)
			{
				csAux.Format(_T("VID %02d (%02d)--> "), k+1, theApp.m_AADT_Vid_list[k].VID);
				csAux+= FormatDuration(theApp.m_AADT_Vid_list[k].dwDuration);
				m_ComboPgc.AddString(csAux);
			}
		}
		else
		{
			for (k=0; k< theApp.m_MADT_Vid_list.GetSize() ;k++)
			{
				csAux.Format(_T("VID %02d (%02d)--> "), k+1, theApp.m_MADT_Vid_list[k].VID);
				csAux+= FormatDuration(theApp.m_MADT_Vid_list[k].dwDuration);
				m_ComboPgc.AddString(csAux);
			}
		}
	}

	if (m_iRadioMode==CIDMODE)
	{
		if (m_iRadioT==TITLES)
		{
			for (k=0; k< theApp.m_AADT_Cell_list.GetSize() ;k++)
			{
				csAux.Format(_T("%02d (%02d/%02d)--> "), k+1, 
					theApp.m_AADT_Cell_list[k].VID, theApp.m_AADT_Cell_list[k].CID);
				csAux+= FormatDuration(theApp.m_AADT_Cell_list[k].dwDuration);
				m_ComboPgc.AddString(csAux);
			}
		}
		else
		{
			for (k=0; k< theApp.m_MADT_Cell_list.GetSize() ;k++)
			{
				csAux.Format(_T("%02d (%02d/%02d)--> "), k+1,
					theApp.m_MADT_Cell_list[k].VID,theApp.m_MADT_Cell_list[k].CID);
				csAux+= FormatDuration(theApp.m_MADT_Cell_list[k].dwDuration);
				m_ComboPgc.AddString(csAux);
			}
		}
	}

	m_ComboPgc.SetCurSel(0);
	UpdateData(FALSE);
}

void CPgcDemuxDlg::FillAnglePgc()
{
	int k;
	CString csAux;
	CWnd *pWnd;
	int nSelPGC;

	m_ComboAng.ResetContent();
	if (m_iRadioMode!=PGCMODE) return;

	csAux.Format(_T("%d"), 1);
	m_ComboAng.AddString(csAux);

	nSelPGC=m_ComboPgc.GetCurSel();

	if (m_iRadioT==TITLES && nSelPGC >=0 &&	theApp.m_nAngles[nSelPGC] > 1)
		for(k=1;k< theApp.m_nAngles[nSelPGC]; k++)
		{
			csAux.Format(_T("%d"), k+1);
			m_ComboAng.AddString(csAux);
		}

	m_ComboAng.SetCurSel(0);
	UpdateData(FALSE);

	if (m_iRadioT==MENUS)
	{
		pWnd = GetDlgItem(IDC_COMBOANG);
		pWnd->ShowWindow(SW_HIDE);
		pWnd = GetDlgItem(IDC_STATICANG);
		pWnd->ShowWindow(SW_HIDE);
	}
	else
	{
		pWnd = GetDlgItem(IDC_STATICANG);
		pWnd->ShowWindow(SW_SHOW);
		pWnd = GetDlgItem(IDC_COMBOANG);
		pWnd->ShowWindow(SW_SHOW);

		if (m_ComboAng.GetCount() > 1)
			pWnd->EnableWindow(TRUE);
		else
			pWnd->EnableWindow(FALSE);
	}
}

void CPgcDemuxDlg::CheckEnableButtons() 
{
	// TODO: Add your control notification handler code here
	CWnd *pWnd;

	if (theApp.m_bInProcess==true)
	{
		pWnd = GetDlgItem(IDOK);
		pWnd->EnableWindow(FALSE);
		pWnd = GetDlgItem(IDABORT);
		pWnd->EnableWindow(TRUE);
	}
	else
	{
		pWnd = GetDlgItem(IDOK);
		if (m_ComboPgc.GetCount())
			pWnd->EnableWindow(TRUE);
		else
			pWnd->EnableWindow(FALSE);
		pWnd = GetDlgItem(IDABORT);
		pWnd->EnableWindow(FALSE);
	}

	if (m_csEditInput.IsEmpty())
	{
		pWnd = GetDlgItem(IDC_COMBOPGC);
		pWnd->EnableWindow(FALSE);
	}
	else
	{
		pWnd = GetDlgItem(IDC_COMBOPGC);
		pWnd->EnableWindow(TRUE);
	}

	if (m_csEditInput.IsEmpty())
	{
		pWnd = GetDlgItem(IDC_RADIOM);
		pWnd->EnableWindow(FALSE);
		pWnd = GetDlgItem(IDC_RADIOT);
		pWnd->EnableWindow(FALSE);
		pWnd=GetDlgItem(IDC_BUTTONDELAY);
		pWnd->EnableWindow(FALSE);
	}
	else
	{
		pWnd = GetDlgItem(IDC_RADIOM);
		pWnd->EnableWindow(TRUE);
		pWnd = GetDlgItem(IDC_RADIOT);
		if (theApp.m_bVMGM)
			pWnd->EnableWindow(FALSE);
		else
			pWnd->EnableWindow(TRUE);
		pWnd=GetDlgItem(IDC_BUTTONDELAY);
		pWnd->EnableWindow(TRUE);
	}

	if (m_bCheckVob)
	{
		pWnd=GetDlgItem(IDC_BUTTONCUSTOM);
		pWnd->ShowWindow(SW_SHOW);
		pWnd=GetDlgItem(IDC_CHECKVOB2);
		pWnd->ShowWindow(SW_SHOW);
	}
	else
	{
		pWnd=GetDlgItem(IDC_BUTTONCUSTOM);
		pWnd->ShowWindow(SW_HIDE);
		pWnd=GetDlgItem(IDC_CHECKVOB2);
		pWnd->ShowWindow(SW_HIDE);
	}
	if (m_bCheckCellt)
	{
		pWnd=GetDlgItem(IDC_CHECKENDTIME);
		pWnd->ShowWindow(SW_SHOW);
	}
	else
	{
		pWnd=GetDlgItem(IDC_CHECKENDTIME);
		pWnd->ShowWindow(SW_HIDE);
	}

}

void CPgcDemuxDlg::OnCheckvob() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	CheckEnableButtons();

}

void CPgcDemuxDlg::OnCheckcellt() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CheckEnableButtons();
	
}
void CPgcDemuxDlg::OnRadiom() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ShowControls();
	FillComboPgc();
	OnSelchangeCombopgc();
	CheckEnableButtons();
}

void CPgcDemuxDlg::OnRadiot() 
{
	// TODO: Add your control notification handler code here
	OnRadiom();
}
void CPgcDemuxDlg::OnRadiopgc() 
{
	// TODO: Add your control notification handler code here
	OnRadiom();
}

void CPgcDemuxDlg::OnRadiovob() 
{
	// TODO: Add your control notification handler code here
	OnRadiom();
}

void CPgcDemuxDlg::OnRadiocell() 
{
	// TODO: Add your control notification handler code here
	OnRadiom();
}

void CPgcDemuxDlg::OnSelchangeCombopgc() 
{
	// TODO: Add your control notification handler code here
	int 	nSelPGC;

	UpdateData(TRUE);

	nSelPGC=m_ComboPgc.GetCurSel();

	if (m_iRadioMode==PGCMODE)
	{
		if (nSelPGC >=0)
		{
			if (m_iRadioT==TITLES)
				m_csEditCells.Format(_T("%d"),theApp.m_nCells[nSelPGC]);
			else
				m_csEditCells.Format(_T("%d"),theApp.m_nMCells[nSelPGC]);
		}
		else
			m_csEditCells="";
	}
	if (m_iRadioMode==VIDMODE)
	{
		if (nSelPGC >=0)
		{
			if (m_iRadioT==TITLES)
				m_csEditCells.Format(_T("%d"),theApp.m_AADT_Vid_list[nSelPGC].nCells);
			else
				m_csEditCells.Format(_T("%d"),theApp.m_MADT_Vid_list[nSelPGC].nCells);
		}
		else
			m_csEditCells="";
	}
	if (m_iRadioMode==CIDMODE)
		m_csEditCells="";


	UpdateData(FALSE);

	if (m_iRadioMode==PGCMODE)
		FillAnglePgc();
}

void CPgcDemuxDlg::OnDropFiles( HDROP hDropInfo)
{
	UINT nFiles;
	CWinApp* pApp;
	TCHAR szFileName[_MAX_PATH];
	CString csAux;

	SetActiveWindow();      // activate us first !
	POINT ThePoint;

	DragQueryPoint(hDropInfo,&ThePoint);

	nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	pApp = AfxGetApp();
	ASSERT(pApp != NULL);
	if(nFiles> 1)
	{
		MessageBox( "Only one file at a time!.", "Error",  MB_OK | MB_ICONEXCLAMATION );
	}
	else if(nFiles== 1)
	{
		::DragQueryFile(hDropInfo, 0, szFileName, _MAX_PATH);

		csAux =  szFileName; 
		csAux.MakeUpper();
		csAux=csAux.Right(4);

		if (csAux==".IFO")
		{
			m_csEditInput =  szFileName; 
			UpdateData (FALSE);
			ReadInputFile();
		}
		else
			MessageBox( "Please drag & drop an IFO file!.", "Error",  MB_OK | MB_ICONEXCLAMATION );
	}
	::DragFinish(hDropInfo);
}

void CPgcDemuxDlg::OnButtoncustom() 
{
	// TODO: Add your control notification handler code here
	CCustomVob dlg;
	int iRet;
	


	iRet = dlg.DoModal();

	if (iRet == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}

}


void CPgcDemuxDlg::ShowControls()
{
	CWnd * pWnd;


	if (m_iRadioMode==PGCMODE)
		//PGC
	{
		pWnd=GetDlgItem(IDC_STATICCELL);
		pWnd->ShowWindow(SW_SHOW);
		pWnd=GetDlgItem(IDC_EDITCELL);
		pWnd->ShowWindow(SW_SHOW);
		pWnd=GetDlgItem(IDC_STATICANG);
		pWnd->ShowWindow(SW_SHOW);
		pWnd=GetDlgItem(IDC_COMBOANG);
		pWnd->ShowWindow(SW_SHOW);
		pWnd=GetDlgItem(IDC_STATICSEL);
		pWnd->SetWindowText("PGC Selection");


	}
	else if (m_iRadioMode==VIDMODE)
		// VID
	{
		pWnd=GetDlgItem(IDC_STATICCELL);
		pWnd->ShowWindow(SW_SHOW);
		pWnd=GetDlgItem(IDC_EDITCELL);
		pWnd->ShowWindow(SW_SHOW);
		pWnd=GetDlgItem(IDC_STATICANG);
		pWnd->ShowWindow(SW_HIDE);
		pWnd=GetDlgItem(IDC_COMBOANG);
		pWnd->ShowWindow(SW_HIDE);
		pWnd=GetDlgItem(IDC_STATICSEL);
		pWnd->SetWindowText("VOB Id Selection");

	}
	else if (m_iRadioMode==CIDMODE)
		// CID
	{
		pWnd=GetDlgItem(IDC_STATICCELL);
		pWnd->ShowWindow(SW_HIDE);
		pWnd=GetDlgItem(IDC_EDITCELL);
		pWnd->ShowWindow(SW_HIDE);
		pWnd=GetDlgItem(IDC_STATICANG);
		pWnd->ShowWindow(SW_HIDE);
		pWnd=GetDlgItem(IDC_COMBOANG);
		pWnd->ShowWindow(SW_HIDE);
		pWnd=GetDlgItem(IDC_STATICSEL);
		pWnd->SetWindowText("Cell Selection");

	}

}

void CPgcDemuxDlg::OnButtondelay() 
{
	// TODO: Add your control notification handler code here
	int k,AudDelay,nSelPGC;
	int iRet;

	CString csInfo,csAux;

	UpdateData(TRUE);

	nSelPGC=m_ComboPgc.GetCurSel();


	if (m_iRadioMode==PGCMODE)
	{
		if ((m_iRadioT==TITLES && theApp.m_nCells[nSelPGC]==0) ||
			(m_iRadioT==MENUS  && theApp.m_nMCells[nSelPGC]==0) )
		{
			MessageBox( "Select a PGC with at least one cell", "Error",  MB_OK | MB_ICONEXCLAMATION );
			return;
		}
	}
	if (m_iRadioMode==VIDMODE)
	{
		if ((m_iRadioT==TITLES && theApp.m_AADT_Cell_list.GetSize()==0) ||
			(m_iRadioT==MENUS  && theApp.m_MADT_Cell_list.GetSize()==0) )
		{
			MessageBox( "Select a VOB id with at least one cell", "Error",  MB_OK | MB_ICONEXCLAMATION );
			return;
		}
	}

	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));

// Read PTSs from VOB
	if (m_iRadioT==TITLES)
		iRet=theApp.GetAudioDelay(m_iRadioMode,nSelPGC);
	else
		iRet=theApp.GetMAudioDelay(m_iRadioMode,nSelPGC);

	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

	if (iRet!=0) return;
// Now calculate and write the delays
	csInfo="";
	for (k=0;k<8;k++)
	{
		if (theApp.m_iFirstAudPTS[k])
		{
//			AudDelay=theApp.m_iFirstAudPTS[k]-theApp.m_iFirstVidPTS;
			AudDelay=theApp.m_iFirstAudPTS[k]-theApp.m_iFirstNavPTS0;
			
			if (AudDelay <0)
				AudDelay-=44;
			else
				AudDelay+=44;
			AudDelay/=90;
			csAux.Format(_T("Audio_%d: 0x%02X --> %d msecs. "),k+1, theApp.m_iAudIndex[k], AudDelay);
		}
		else
			csAux.Format(_T("Audio_%d: None."),k+1);
	
		csInfo+=csAux+"\r\n";

	}

	MessageBox( csInfo, "Audio/Video delays",  MB_OK | MB_ICONINFORMATION );

}

bool CPgcDemuxDlg::FilesAlreadyExist() 
{
	bool bAudio, bSubs, bVideo, bVob;
	CString csFile,csAux;
	int i;

	bAudio=bSubs=bVideo=bVob=false;

	csFile=m_csEditOutput+ '\\' + "VideoFile.m2v";
	if (_access( csFile, 0 ) != -1 ) bVideo=true;


	for (i=0;i<8 && bAudio==false;i++)
	{
// ac3
		csAux.Format(_T("AudioFile_%02X.ac3"),i+0x80);
		csFile=m_csEditOutput+ '\\' + csAux;
		if (_access( csFile, 0 ) != -1 ) bAudio=true;
//dts
		csAux.Format(_T("AudioFile_%02X.dts"),i+0x88);
		csFile=m_csEditOutput+ '\\' + csAux;
		if (_access( csFile, 0 ) != -1 ) bAudio=true;
//dds
		csAux.Format(_T("AudioFile_%02X.dds"),i+0x90);
		csFile=m_csEditOutput+ '\\' + csAux;
		if (_access( csFile, 0 ) != -1 ) bAudio=true;
//pcm
		csAux.Format(_T("AudioFile_%02X.wav"),i+0xa0);
		csFile=m_csEditOutput+ '\\' + csAux;
		if (_access( csFile, 0 ) != -1 ) bAudio=true;
//mpeg1
		csAux.Format(_T("AudioFile_%02X.mpa"),i+0xc0);
		csFile=m_csEditOutput+ '\\' + csAux;
		if (_access( csFile, 0 ) != -1 ) bAudio=true;
//mpeg2
		csAux.Format(_T("AudioFile_%02X.mpa"),i+0xd0);
		csFile=m_csEditOutput+ '\\' + csAux;
		if (_access( csFile, 0 ) != -1 ) bAudio=true;
	}

	for (i=0;i<32 && bSubs==false;i++)
	{
		csAux.Format(_T("Subpictures_%02X.sup"),i+0x20);
		csFile=m_csEditOutput+ '\\' + csAux;
		if (_access( csFile, 0 ) != -1 ) bSubs=true;
	}

	csFile=m_csEditOutput+ '\\' + "VTS_*.VOB";

	if (AnyFileExists ( csFile )) bVob=true;

	return (bAudio || bSubs || bVideo || bVob);

}


bool CPgcDemuxDlg::AnyFileExists(CString csFile) 
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(csFile, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
		return false;
	else
	{
		FindClose(hFind);
		return true;
	}
}

/*
	char			szTempPath[_MAX_PATH+1];

	strcpy(szTempPath,szFilename);
	// Split the target into bits (used for reconstruction later)
	_splitpath( szTempPath, szDrive, szDir, szFile, szExt );

	// Delete all files matching the criteria
	hSearch = FindFirstFile(szTempPath, &findData);
	bLoop = true;
	while (hSearch != INVALID_HANDLE_VALUE && bLoop == true)
	{
		// Make sure the returned handle is a file and not a directory before we
		// try and do delete type things on it!
		if ( (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
		{
			bFound = true;							// Found at least one match
			
			// Returned data is just the filename, we need to add the path stuff back on
			// The find strcut only returns the file NAME, we need to reconstruct the path!
			strcpy(szTempPath, szDrive);	
			strcat(szTempPath, szDir);
			strcat(szTempPath, findData.cFileName);
			
			if ( DeleteFile(szTempPath) != TRUE )
			{
				FindClose(hSearch);
				return false;						// Error deleting one of the files
			}

		} // End if

		if (FindNextFile(hSearch, &findData) == FALSE)
			bLoop = false;
	}

	FindClose(hSearch);
*/



