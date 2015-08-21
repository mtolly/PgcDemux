// PgcDemux.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <Winbase.h>
#include "ToolTipDialog.h"
#include "PgcDemux.h"
#include "PgcDemuxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void MyErrorBox(LPCTSTR text);
extern int readbuffer(uchar* buffer, FILE *in);
extern void writebuffer(uchar* buffer, FILE *out, int nbytes);
extern bool IsNav(uchar* buffer);
extern bool IsPad(uchar* buffer);
extern bool IsSynch (uchar* buffer);
extern bool IsVideo (uchar* buffer);
extern bool IsAudio (uchar* buffer);
extern bool IsAudMpeg (uchar* buffer);
extern bool IsSubs (uchar* buffer);
extern void ModifyCID (uchar* buffer, int VobId, int CellId);
extern int GetNbytes(int nNumber,uchar* address);
extern void Put4bytes(__int64 i64Number,uchar* address);
extern void MyErrorBox(LPCTSTR text);
extern void ModifyLBA (uchar* buffer, __int64 m_i64OutputLBA);
extern int readpts(uchar *buf);
extern int DurationInFrames(DWORD dwDuration);
extern DWORD AddDuration(DWORD dwDuration1, DWORD dwDuration2);
extern int getAudId(uchar * buffer);



uchar pcmheader[44] = {
	0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,	0x66, 0x6D, 0x74, 0x20,
	0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,	0x80, 0xBB, 0x00, 0x00, 0x70, 0x17, 0x00, 0x00,
	0x04, 0x00, 0x10, 0x00,	0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00 };
/*
uchar pcmheader2[58] = {
	0x52, 0x49, 0x46, 0x46, 0x58, 0x08, 0x2C, 0x00, 0x57, 0x41, 0x56, 0x45,	0x66, 0x6D, 0x74, 0x20,
	0x12, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,	0x80, 0xBB, 0x00, 0x00, 0x00, 0x65, 0x04, 0x00,
	0x06, 0x00, 0x18, 0x00,	0x00, 0x00, 0x66, 0x61, 0x63, 0x74, 0x04, 0x00, 0x00, 0x00, 0x05, 0x56, 
	0x07, 0x00, 0x64, 0x61, 0x74, 0x61, 0x1E, 0x04, 0x2C, 0x00 };
*/

////////////// AUX nibble functions used for code cleaning ///// 
#define hi_nib(a)	((a>>4) & 0x0f)
#define lo_nib(a)	(a & 0x0f)

/*
inline uchar hi_nib(uchar ch)
{
  return ((ch>>4) & 0x0f);
}

inline uchar lo_nib(uchar ch)
{
  return (ch & 0x0f);
}
*/
/////////////////////////////////////////////////////////////////////////////
// CPgcDemuxApp

BEGIN_MESSAGE_MAP(CPgcDemuxApp, CWinApp)
	//{{AFX_MSG_MAP(CPgcDemuxApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPgcDemuxApp construction

CPgcDemuxApp::CPgcDemuxApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPgcDemuxApp object

CPgcDemuxApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPgcDemuxApp initialization

BOOL CPgcDemuxApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif


	int i,k;
	int nSelVid,nSelCid;

	m_pIFO = NULL;

//	SetRegistryKey( "jsoto's tools" );
//	WriteProfileInt("MySection", "My Key",123);

	for (i=0;i<32;i++) fsub[i]=NULL;
	for (i=0;i<8;i++) 
		faud[i]=NULL;
	fvob=fvid=NULL;

	m_bInProcess=false;
	m_bCLI=false;
	m_bAbort=false;
	m_bVMGM=false;

	m_bCheckAud=m_bCheckSub=m_bCheckLog=m_bCheckCellt=TRUE;
	m_bCheckVid=m_bCheckVob=m_bCheckVob2=m_bCheckEndTime=FALSE;

	m_bCheckIFrame=FALSE;
	m_bCheckLBA=m_bCheckVideoPack=m_bCheckAudioPack=m_bCheckNavPack=m_bCheckSubPack=TRUE;


	if ( __argc > 2 ) 
		// CLI mode
	{
		m_bCLI=true;
		if (ParseCommandLine() ==TRUE)
		{
			m_bInProcess=true;
			m_iRet=ReadIFO();
			if (m_iRet==0)
			{
				if (m_iMode==PGCMODE)
				{
// Check if PGC exists in done in PgcDemux
					if (m_iDomain==TITLES)
						m_iRet=PgcDemux (m_nSelPGC,m_nSelAng, NULL);
					else
						m_iRet=PgcMDemux(m_nSelPGC,NULL);
				}
				if (m_iMode==VIDMODE)
				{
// Look for nSelVid
					nSelVid=-1;
					if (m_iDomain==TITLES)
					{
						for (k=0;k<m_AADT_Vid_list.GetSize() && nSelVid==-1; k++)
							if (m_AADT_Vid_list[k].VID==m_nVid)
								nSelVid=k;
					}
					else
					{
						for (k=0;k<m_MADT_Vid_list.GetSize() && nSelVid==-1; k++)
							if (m_MADT_Vid_list[k].VID==m_nVid)
								nSelVid=k;
					}
					if ( nSelVid==-1) 
					{
						m_iRet=-1;
						MyErrorBox( "Selected Vid not found!");
					}
					if (m_iRet==0)
					{
						if (m_iDomain==TITLES)
							m_iRet=VIDDemux(nSelVid, NULL);
						else
							m_iRet=VIDMDemux(nSelVid,NULL);
					}
				}
				if (m_iMode==CIDMODE)
				{
// Look for nSelVid
					nSelCid=-1;
					if (m_iDomain==TITLES)
					{
						for (k=0;k<m_AADT_Cell_list.GetSize() && nSelCid==-1; k++)
							if (m_AADT_Cell_list[k].VID==m_nVid && m_AADT_Cell_list[k].CID==m_nCid)
								nSelCid=k;
					}
					else
					{
						for (k=0;k<m_MADT_Cell_list.GetSize() && nSelCid==-1; k++)
							if (m_MADT_Cell_list[k].VID==m_nVid && m_MADT_Cell_list[k].CID==m_nCid)
								nSelCid=k;
					}
					if ( nSelCid==-1) 
					{
						m_iRet=-1;
						MyErrorBox( "Selected Vid/Cid not found!");
					}
					if (m_iRet==0)
					{
						if (m_iDomain==TITLES)
							m_iRet=CIDDemux(nSelCid, NULL);
						else
							m_iRet=CIDMDemux(nSelCid,NULL);
					}
				}


			}
			m_bInProcess=false;
		}
		else
			m_iRet=-1;

		//  return FALSE so that we exit the
		//  application, rather than start the application's message pump.
		return FALSE;
	}



// Dlg mode	
	CPgcDemuxDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CPgcDemuxApp::UpdateProgress( CWnd* pDlg, int nPerc)
{
	MSG msg;
	static int oldPerc=-1;

	if ( pDlg )
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (oldPerc!=nPerc) 
		{
			oldPerc=nPerc;
			((CPgcDemuxDlg *)pDlg)->UpdateProgress(nPerc);
		}
	}
}


BOOL CPgcDemuxApp::ParseCommandLine()
{
/*
PgcDemux [option1] [option2] ... [option12] <ifo_input_file> <destination_folder>
option1: [-pgc, <pgcnumber>].      Selects the PGC number (from 1 to nPGCs). Default 1
option2: [-ang, <angnumber>].      Selects the Angle number (from 1 to n). Default 1
option3: [-vid, <vobid>].          Selects the Angle number (from 1 to n). Default 1
option4: [-cid, <vobid> <cellid>]. Selects a cell vobid (from 1 to n). Default 1
option5: {-m2v, -nom2v}. Extracts/No extracts video file. Default NO
option6: {-aud, -noaud}. Extracts/No extracts audio streams. Default YES
option7: {-sub, -nosub}. Extracts/No extracts subs streams. Default YES
option8: {-vob, -novob}. Generates a single PGC VOB. Default NO
option9: {-customvob <flags>}. Generates a custom VOB file. Flags:
           b: split VOB: one file per vob_id
		   n: write nav packs
		   v: write video packs
		   a: write audio packs
		   s: write subs packs
		   i: only first Iframe
		   l: patch LBA number
option10:{-cellt, -nocellt}. Generates a Celltimes.txt file. Default YES
option10:{-endt, -noendt}. Includes Last end time in Celltimes.txt. Default NO
option11:{-log, -nolog}. Generates a log file. Default YES
option12:{-menu, -title}. Domain. Default Title (except if filename is VIDEO_TS.IFO)
*/
	int i;
	CString csPar,csPar2;
	BOOL bRet;
	CString csAux,csAux1,csAux2;


	bRet=TRUE;

//	m_bCheckAud=m_bCheckSub=m_bCheckLog=m_bCheckCellt=TRUE;
//	m_bCheckVid=m_bCheckVob=m_bCheckVob2=m_bCheckEndTime=FALSE;

//	m_bCheckIFrame=FALSE;
//	m_bCheckLBA=m_bCheckVideoPack=m_bCheckAudioPack=m_bCheckNavPack=m_bCheckSubPack=TRUE;


	m_nSelPGC=m_nSelAng=0;
	m_iMode=PGCMODE;
	m_iDomain=TITLES;

	if (__argc < 3) return -1;
	
	for (i =1 ; i<(__argc)-1 ; i++)
	{
		csPar.Format(_T("%s"),(__argv[i]));
		csPar.MakeLower();

		if ( csPar=="-pgc" && __argc >i+1 )	
		{
			sscanf ( __argv[i+1], "%d", &m_nSelPGC);
			if (m_nSelPGC <1 || m_nSelPGC >255)
			{
				MyErrorBox( "Invalid pgc number!");
				return FALSE;
			}
			m_iMode=PGCMODE;
			i++;
			m_nSelPGC--; // internally from 0 to nPGCs-1.
		}
		else if ( csPar=="-ang" && __argc >i+1 )	
		{
			sscanf ( __argv[i+1], "%d", &m_nSelAng);
			if (m_nSelAng <1 || m_nSelAng >9)
			{
				MyErrorBox( "Invalid angle number!");
				return FALSE;
			}
			i++;
			m_nSelAng--; // internally from 0 to nAngs-1.
		}
		else if ( csPar=="-vid" && __argc >i+1 )	
		{
			sscanf ( __argv[i+1], "%d", &m_nVid);
			if (m_nVid <1 || m_nVid >32768)
			{
				MyErrorBox( "Invalid Vid number!");
				return FALSE;
			}
			m_iMode=VIDMODE;
			i++;
		}
		else if ( csPar=="-cid" && __argc >i+2 )	
		{
			sscanf ( __argv[i+1], "%d", &m_nVid);
			sscanf ( __argv[i+2], "%d", &m_nCid);
			if (m_nVid <1 || m_nVid >32768)
			{
				MyErrorBox( "Invalid Vid number!");
				return FALSE;
			}
			if (m_nCid <1 || m_nCid >255)
			{
				MyErrorBox( "Invalid Cid number!");
				return FALSE;
			}

			m_iMode=CIDMODE;
			i+=2;
		}
		else if ( csPar=="-customvob" && __argc >i+1 )	
		{
			csPar2.Format(_T("%s"),(__argv[i+1]));
			csPar2.MakeLower();

			m_bCheckVob=TRUE;
// n: write nav packs
// v: write video packs
// a: write audio packs
// s: write subs packs
// i: only first Iframe
// b: split file per vob_id
// l: Patch LBA number

			if (csPar2.Find('b')!=-1)  m_bCheckVob2=TRUE;
			else m_bCheckVob2=FALSE;
			if (csPar2.Find('v')!=-1)  m_bCheckVideoPack=TRUE;
			else m_bCheckVideoPack=FALSE;
			if (csPar2.Find('a')!=-1)  m_bCheckAudioPack=TRUE;
			else m_bCheckAudioPack=FALSE;
			if (csPar2.Find('n')!=-1)  m_bCheckNavPack=TRUE;
			else m_bCheckNavPack=FALSE;
			if (csPar2.Find('s')!=-1)  m_bCheckSubPack=TRUE;
			else m_bCheckSubPack=FALSE;
			if (csPar2.Find('i')!=-1)  m_bCheckIFrame=TRUE;
			else m_bCheckIFrame=FALSE;
			if (csPar2.Find('l')!=-1)  m_bCheckLBA=TRUE;
			else m_bCheckLBA=FALSE;
			i++;
		}
		else if ( csPar=="-m2v" )  m_bCheckVid=1;   
		else if ( csPar=="-vob" )  m_bCheckVob=1;   
		else if ( csPar=="-aud" )  m_bCheckAud=1;   
		else if ( csPar=="-sub" )  m_bCheckSub=1;   
		else if ( csPar=="-log" )  m_bCheckLog=1;   
		else if ( csPar=="-cellt" )  m_bCheckCellt=1;   
		else if ( csPar=="-endt" )  m_bCheckEndTime=1;   
		else if ( csPar=="-nom2v" )  m_bCheckVid=0;   
		else if ( csPar=="-novob" )  m_bCheckVob=0;   
		else if ( csPar=="-noaud" )  m_bCheckAud=0;   
		else if ( csPar=="-nosub" )  m_bCheckSub=0;   
		else if ( csPar=="-nolog" )  m_bCheckLog=0;   
		else if ( csPar=="-nocellt" )  m_bCheckCellt=0;   
		else if ( csPar=="-noendt" )  m_bCheckEndTime=0;   
		else if ( csPar=="-menu" )	m_iDomain=MENUS;   
		else if ( csPar=="-title" )  m_iDomain=TITLES;   
	}
	m_csInputIFO=__argv[(__argc) -2];
	m_csOutputPath=__argv[(__argc) -1];

	m_csInputPath=m_csInputIFO.Left(m_csInputIFO.ReverseFind('\\'));

	m_csInputIFO.MakeUpper();
	m_csOutputPath.MakeUpper();
	m_csInputPath.MakeUpper();

	csAux=m_csInputIFO.Right(m_csInputIFO.GetLength()-m_csInputIFO.ReverseFind('\\')-1);
	csAux1=csAux.Left(4);
	csAux=m_csInputIFO.Right(6);
	csAux2=m_csInputIFO.Right(12);
	if ( (csAux!="_0.IFO" || csAux1 != "VTS_" ) && csAux2 !="VIDEO_TS.IFO")
	{
		MyErrorBox( "Invalid input file!");
		return FALSE;
	}
	
	if (csAux2=="VIDEO_TS.IFO")
	{
		m_bVMGM=true;
		m_iDomain=MENUS;
	}
	else m_bVMGM=false;

	return TRUE;
}



// Exit Value
int CPgcDemuxApp::ExitInstance()
{
	int ret;
	
	if (m_pIFO!=NULL)  delete[]  m_pIFO;

	ret= CWinApp::ExitInstance();

    
	if (__argc == 1)
		return ret;
	else
		return m_iRet;

}


///////////////////////////////////////////////////////////////////////
///////////////  MAIN CODE ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

int CPgcDemuxApp::InsertCell (ADT_CELL_LIST myADT_Cell, int iDomain)
{
	int iArraysize,i,ii;
	bool bIsHigher;

	if (iDomain==TITLES)
	{
		iArraysize=m_AADT_Cell_list.GetSize();
		ii=iArraysize;
		for (i=0,bIsHigher=true; i<iArraysize && bIsHigher ; i++)
		{
			if (myADT_Cell.VID < m_AADT_Cell_list[i].VID )  {ii=i; bIsHigher=false;}
			else if (myADT_Cell.VID > m_AADT_Cell_list[i].VID )  bIsHigher=true;
			else
			{
				if (myADT_Cell.CID < m_AADT_Cell_list[i].CID ) {ii=i; bIsHigher=false;}
				else if (myADT_Cell.CID > m_AADT_Cell_list[i].CID )  bIsHigher=true;
			}

		}
		m_AADT_Cell_list.InsertAt(ii,myADT_Cell);
	}
	if (iDomain==MENUS)
	{
		iArraysize=m_MADT_Cell_list.GetSize();
		ii=iArraysize;
		for (i=0,bIsHigher=true; i<iArraysize && bIsHigher ; i++)
		{
			if (myADT_Cell.VID < m_MADT_Cell_list[i].VID ) {ii=i; bIsHigher=false;}
			else if (myADT_Cell.VID > m_MADT_Cell_list[i].VID )  bIsHigher=true;
			else
			{
				if (myADT_Cell.CID < m_MADT_Cell_list[i].CID )   {ii=i; bIsHigher=false;}
				else if (myADT_Cell.CID > m_MADT_Cell_list[i].CID )  bIsHigher=true;
			}

		}
//		if (i>0 && bIsHigher) i--;
		m_MADT_Cell_list.InsertAt(ii,myADT_Cell);
	}
	return ii;
}

void CPgcDemuxApp::FillDurations()
{
	int iArraysize;
	int i,j,k;
	int VIDa,CIDa,VIDb,CIDb;
	bool bFound;
	int iVideoAttr, iFormat; 


	iArraysize=m_AADT_Cell_list.GetSize();

	for (i=0; i<iArraysize; i++)
	{
		VIDb=m_AADT_Cell_list[i].VID;
		CIDb=m_AADT_Cell_list[i].CID;
		for (j=0,bFound=false;j<m_nPGCs && !bFound; j++)
		{
			for (k=0;k<m_nCells[j];k++)
			{
				VIDa=GetNbytes(2,&m_pIFO[m_C_POST[j]+k*4]);
				CIDa=m_pIFO[m_C_POST[j]+k*4+3];
				if (VIDa==VIDb && CIDa==CIDb)
				{
					bFound=true;
					m_AADT_Cell_list[i].dwDuration=GetNbytes(4,&m_pIFO[m_C_PBKT[j]+0x18*k+4]);
				}
			}
		}
		if (!bFound)
		{
			iVideoAttr=m_pIFO[0x200]*256+m_pIFO[0x201];
			iFormat=(iVideoAttr & 0x1000) >> 12;
			if (iFormat == 0 ) // NTSC
				m_AADT_Cell_list[i].dwDuration=0xC0;
			else // PAL
				m_AADT_Cell_list[i].dwDuration=0x40;
		}
	}

	iArraysize=m_MADT_Cell_list.GetSize();

	for (i=0; i<iArraysize; i++)
	{
		VIDb=m_MADT_Cell_list[i].VID;
		CIDb=m_MADT_Cell_list[i].CID;
		for (j=0,bFound=false;j<m_nMPGCs && !bFound; j++)
		{
			for (k=0;k<m_nMCells[j];k++)
			{
				VIDa=GetNbytes(2,&m_pIFO[m_M_C_POST[j]+k*4]);
				CIDa=m_pIFO[m_M_C_POST[j]+k*4+3];
				if (VIDa==VIDb && CIDa==CIDb)
				{
					bFound=true;
					m_MADT_Cell_list[i].dwDuration=GetNbytes(4,&m_pIFO[m_M_C_PBKT[j]+0x18*k+4]);
				}
			}
		}
		if (!bFound)
		{
			iVideoAttr=m_pIFO[0x100]*256+m_pIFO[0x101];
			iFormat=(iVideoAttr & 0x1000) >> 12;
			if (iFormat == 0 ) // NTSC
				m_MADT_Cell_list[i].dwDuration=0xC0;
			else // PAL
				m_MADT_Cell_list[i].dwDuration=0x40;
		}
	}

}
	
////////////////////////////////////////////////////////////////////////////////
/////////////  ReadIFO /////
////////////////////////////////////////////////////////////////////////////////
int CPgcDemuxApp::ReadIFO()
{
	CString csAux,csAux2;
	int i,j,k,kk,nCell,nVIDs;
	ADT_CELL_LIST myADT_Cell;
	ADT_VID_LIST myADT_Vid;
	int nTotADT, nADT, VidADT,CidADT;
	int iArraysize;
	bool bAlready, bEndAngle;
	FILE * in;
	int iIniSec,iEndSec;
	struct  _stati64 statbuf;
	int iSize,iCat;
	int iIFOSize;


	if (_stati64 ( m_csInputIFO, &statbuf)==0)
		iIFOSize= (int) statbuf.st_size;
	
	if ( iIFOSize > MAXLENGTH)
	{
		csAux.Format(_T("IFO too big %s"),m_csInputIFO);
		MyErrorBox (csAux);
		return -1;
	}

	in=fopen(m_csInputIFO,"rb");
	if (in == NULL)
	{
		csAux.Format(_T("Unable to open %s"),m_csInputIFO);
		MyErrorBox (csAux);
		return -1;
	}

	if (m_pIFO!=NULL)  delete[]  m_pIFO;

	m_pIFO = new uchar[iIFOSize+2048];

// Read IFO


	for (i=0;!feof(in) && i< MAXLENGTH ;i++)
		m_pIFO[i]=fgetc(in);
	m_iIFOlen=i-1;
	fclose (in);

	m_AADT_Cell_list.RemoveAll();
	m_MADT_Cell_list.RemoveAll();
	m_AADT_Vid_list.RemoveAll();
	m_MADT_Vid_list.RemoveAll();


// Get Title Cells
	if (m_bVMGM) 
	{
		m_iVTS_PTT_SRPT=   0;
		m_iVTS_PGCI=       0;
		m_iVTSM_PGCI=      2048*GetNbytes(4,&m_pIFO[0xC8]);
		m_iVTS_TMAPTI=     0;
		m_iVTSM_C_ADT=     2048*GetNbytes(4,&m_pIFO[0xD8]);
		m_iVTSM_VOBU_ADMAP=2048*GetNbytes(4,&m_pIFO[0xDC]);
		m_iVTS_C_ADT=      0;
		m_iVTS_VOBU_ADMAP= 0;
	}
	else
	{
		m_iVTS_PTT_SRPT=   2048*GetNbytes(4,&m_pIFO[0xC8]);
		m_iVTS_PGCI=       2048*GetNbytes(4,&m_pIFO[0xCC]);
		m_iVTSM_PGCI=      2048*GetNbytes(4,&m_pIFO[0xD0]);
		m_iVTS_TMAPTI=     2048*GetNbytes(4,&m_pIFO[0xD4]);
		m_iVTSM_C_ADT=     2048*GetNbytes(4,&m_pIFO[0xD8]);
		m_iVTSM_VOBU_ADMAP=2048*GetNbytes(4,&m_pIFO[0xDC]);
		m_iVTS_C_ADT=      2048*GetNbytes(4,&m_pIFO[0xE0]);
		m_iVTS_VOBU_ADMAP= 2048*GetNbytes(4,&m_pIFO[0xE4]);
	}
	if (m_bVMGM) 
		m_nPGCs=0;
	else
		m_nPGCs=GetNbytes(2,&m_pIFO[m_iVTS_PGCI]);


// Title PGCs	
	if (m_nPGCs > MAX_PGC)
	{
		csAux.Format(_T("ERROR: Max PGCs limit (%d) has been reached."),MAX_PGC);
		MyErrorBox (csAux);
		return -1;
	}
	for (k=0; k<m_nPGCs;k++)
	{
		m_iVTS_PGC[k]=GetNbytes(4,&m_pIFO[m_iVTS_PGCI+0x04+(k+1)*8])+m_iVTS_PGCI;
		m_dwDuration[k]=(DWORD)GetNbytes(4,&m_pIFO[m_iVTS_PGC[k]+4]);

		m_C_PBKT[k]=GetNbytes(2,&m_pIFO[m_iVTS_PGC[k]+0xE8]);
		if (m_C_PBKT[k]!=0  ) m_C_PBKT[k]+=m_iVTS_PGC[k];

		m_C_POST[k]=GetNbytes(2,&m_pIFO[m_iVTS_PGC[k]+0xEA]);
		if (m_C_POST[k]!=0  ) m_C_POST[k]+=m_iVTS_PGC[k];

		m_nCells[k]=m_pIFO[m_iVTS_PGC[k]+3];


		m_nAngles[k]=1;

		for (nCell=0,bEndAngle=false; nCell<m_nCells[k] && bEndAngle==false; nCell++)
		{
			iCat=GetNbytes(1,&m_pIFO[m_C_PBKT[k]+24*nCell]);
			iCat=iCat & 0xF0;
//			0101=First; 1001=Middle ;	1101=Last
			if      (iCat == 0x50)
				m_nAngles[k]=1;
			else if (iCat == 0x90)
				m_nAngles[k]++;
			else if (iCat == 0xD0)
			{
				m_nAngles[k]++;
				bEndAngle=true;
			}
		}
	}


// Menu PGCs
	if( m_iVTSM_PGCI==0 )
		m_nLUs=0;
	else
		m_nLUs=GetNbytes(2,&m_pIFO[m_iVTSM_PGCI]);

	m_nMPGCs=0;
	if (m_nLUs > MAX_LU)
	{
		csAux.Format(_T("ERROR: Max LUs limit (%d) has been reached."),MAX_LU);
		MyErrorBox (csAux);
		return -1;
	}

	for (nLU=0; nLU<m_nLUs;nLU++)
	{
  		m_iVTSM_LU[nLU]=   GetNbytes(4,&m_pIFO[m_iVTSM_PGCI+0x04+(nLU+1)*8])+m_iVTSM_PGCI;
		m_nPGCinLU[nLU]=   GetNbytes(2,&m_pIFO[m_iVTSM_LU[nLU]]);
		m_nIniPGCinLU[nLU]= m_nMPGCs;

		for (j=0; j < m_nPGCinLU[nLU]; j++)
		{
			if ((m_nMPGCs + m_nPGCinLU[nLU]) > MAX_MPGC)
			{
				csAux.Format(_T("ERROR: Max MPGCs limit (%d) has been reached."),MAX_MPGC);
				MyErrorBox (csAux);
				return -1;
			}
			nAbsPGC=j+m_nMPGCs;
			m_nLU_MPGC[nAbsPGC]=nLU;
			m_iMENU_PGC[nAbsPGC]= GetNbytes(4,&m_pIFO[m_iVTSM_LU[nLU]+0x04+(j+1)*8])+m_iVTSM_LU[nLU];

			m_M_C_PBKT[nAbsPGC]  =GetNbytes(2,&m_pIFO[m_iMENU_PGC[nAbsPGC]+0xE8]);
			if (m_M_C_PBKT[nAbsPGC] !=0) m_M_C_PBKT[nAbsPGC] += m_iMENU_PGC[nAbsPGC];
			m_M_C_POST[nAbsPGC]  =GetNbytes(2,&m_pIFO[m_iMENU_PGC[nAbsPGC]+0xEA]);
			if (m_M_C_POST[nAbsPGC] !=0) m_M_C_POST[nAbsPGC] +=m_iMENU_PGC[nAbsPGC];

			m_nMCells[nAbsPGC]=m_pIFO[m_iMENU_PGC[nAbsPGC]+3];

			if ( (m_M_C_PBKT[nAbsPGC]==0 || m_M_C_POST[nAbsPGC] ==0) &&  m_nMCells[nAbsPGC]!=0)
// There is something wrong...
			{
				m_nMCells[nAbsPGC]=0;
				csAux.Format(_T("ERROR: There is something wrong in number of cells in LU %02d, Menu PGC %02d."),nLU, j);
				MyErrorBox (csAux);
				return -1;
			}
			m_dwMDuration[nAbsPGC]=(DWORD)GetNbytes(4,&m_pIFO[m_iMENU_PGC[nAbsPGC]+4]);

		} // For PGCs
		m_nMPGCs+=m_nPGCinLU[nLU];
	}


///////////// VTS_C_ADT  ///////////////////////
	if (m_iVTS_C_ADT==0) nTotADT=0;
	else
	{
		nTotADT=GetNbytes(4,&m_pIFO[m_iVTS_C_ADT+4]);
		nTotADT=(nTotADT-7)/12;
	}

//Cells
	for (nADT=0; nADT <nTotADT; nADT++)
	{
		VidADT=GetNbytes(2,&m_pIFO[m_iVTS_C_ADT+8+12*nADT]);
		CidADT=m_pIFO[m_iVTS_C_ADT+8+12*nADT+2];

		iArraysize=m_AADT_Cell_list.GetSize();
		for (k=0,bAlready=false; k< iArraysize ;k++)
		{
			if (CidADT==m_AADT_Cell_list[k].CID &&
				VidADT==m_AADT_Cell_list[k].VID )
			{
				bAlready=true;
				kk=k;
			}
		}
		if (!bAlready)
		{
			myADT_Cell.CID=CidADT;
			myADT_Cell.VID=VidADT;
			myADT_Cell.iSize=0;
			myADT_Cell.iIniSec=0x7fffffff;
			myADT_Cell.iEndSec=0;
			kk=InsertCell (myADT_Cell, TITLES);
//			m_AADT_Cell_list.SetAtGrow(iArraysize,myADT_Cell);
//			kk=iArraysize;
		}
		iIniSec=GetNbytes(4,&m_pIFO[m_iVTS_C_ADT+8+12*nADT+4]);
		iEndSec=GetNbytes(4,&m_pIFO[m_iVTS_C_ADT+8+12*nADT+8]);
		if (iIniSec < m_AADT_Cell_list[kk].iIniSec) m_AADT_Cell_list[kk].iIniSec=iIniSec;
		if (iEndSec > m_AADT_Cell_list[kk].iEndSec) m_AADT_Cell_list[kk].iEndSec=iEndSec;
		iSize=(iEndSec-iIniSec+1);
		m_AADT_Cell_list[kk].iSize+=(iEndSec-iIniSec+1);
	}

///////////// VTSM_C_ADT  ///////////////////////
	if (m_iVTSM_C_ADT==0) nTotADT=0;
	else
	{
		nTotADT=GetNbytes(4,&m_pIFO[m_iVTSM_C_ADT+4]);
		nTotADT=(nTotADT-7)/12;
	}

// Cells
	for (nADT=0; nADT <nTotADT; nADT++)
	{
		VidADT=GetNbytes(2,&m_pIFO[m_iVTSM_C_ADT+8+12*nADT]);
		CidADT=m_pIFO[m_iVTSM_C_ADT+8+12*nADT+2];

		iArraysize=m_MADT_Cell_list.GetSize();
		for (k=0,bAlready=false; k< iArraysize ;k++)
		{
			if (CidADT==m_MADT_Cell_list[k].CID &&
				VidADT==m_MADT_Cell_list[k].VID )
			{
				bAlready=true;
				kk=k;
			}
		}
		if (!bAlready)
		{
			myADT_Cell.CID=CidADT;
			myADT_Cell.VID=VidADT;
			myADT_Cell.iSize=0;
			myADT_Cell.iIniSec=0x7fffffff;
			myADT_Cell.iEndSec=0;
			kk=InsertCell (myADT_Cell, MENUS);
//			m_MADT_Cell_list.SetAtGrow(iArraysize,myADT_Cell);
//			kk=iArraysize;
		}
		iIniSec=GetNbytes(4,&m_pIFO[m_iVTSM_C_ADT+8+12*nADT+4]);
		iEndSec=GetNbytes(4,&m_pIFO[m_iVTSM_C_ADT+8+12*nADT+8]);
		if (iIniSec < m_MADT_Cell_list[kk].iIniSec) m_MADT_Cell_list[kk].iIniSec=iIniSec;
		if (iEndSec > m_MADT_Cell_list[kk].iEndSec) m_MADT_Cell_list[kk].iEndSec=iEndSec;
		iSize=(iEndSec-iIniSec+1);
		m_MADT_Cell_list[kk].iSize+=(iEndSec-iIniSec+1);
	}

	FillDurations();

//////////////////////////////////////////////////////////////	
/////////////   VIDs
// VIDs in Titles
	iArraysize=m_AADT_Cell_list.GetSize();
	for (i=0; i <iArraysize; i++)
	{
		VidADT=m_AADT_Cell_list[i].VID;

		nVIDs=m_AADT_Vid_list.GetSize();
		for (k=0,bAlready=false; k< nVIDs ;k++)
		{
			if (VidADT==m_AADT_Vid_list[k].VID )
			{
				bAlready=true;
				kk=k;
			}
		}
		if (!bAlready)
		{
			myADT_Vid.VID=VidADT;
			myADT_Vid.iSize=0;
			myADT_Vid.nCells=0;
			myADT_Vid.dwDuration=0;
			m_AADT_Vid_list.SetAtGrow(nVIDs,myADT_Vid);
			kk=nVIDs;
		}
		m_AADT_Vid_list[kk].iSize+=m_AADT_Cell_list[i].iSize;
		m_AADT_Vid_list[kk].nCells++;
		m_AADT_Vid_list[kk].dwDuration=AddDuration(m_AADT_Cell_list[i].dwDuration,m_AADT_Vid_list[kk].dwDuration);
	}
	
// VIDs in Menus
	iArraysize=m_MADT_Cell_list.GetSize();
	for (i=0; i <iArraysize; i++)
	{
		VidADT=m_MADT_Cell_list[i].VID;

		nVIDs=m_MADT_Vid_list.GetSize();
		for (k=0,bAlready=false; k< nVIDs ;k++)
		{
			if (VidADT==m_MADT_Vid_list[k].VID )
			{
				bAlready=true;
				kk=k;
			}
		}
		if (!bAlready)
		{
			myADT_Vid.VID=VidADT;
			myADT_Vid.iSize=0;
			myADT_Vid.nCells=0;
			myADT_Vid.dwDuration=0;
			m_MADT_Vid_list.SetAtGrow(nVIDs,myADT_Vid);
			kk=nVIDs;
		}
		m_MADT_Vid_list[kk].iSize+=m_MADT_Cell_list[i].iSize;
		m_MADT_Vid_list[kk].nCells++;
		m_MADT_Vid_list[kk].dwDuration=AddDuration(m_MADT_Cell_list[i].dwDuration,m_MADT_Vid_list[kk].dwDuration);
	}
	
// Fill VOB file size
	if (m_bVMGM)
	{
		m_nVobFiles=0;

		for (k=0; k<10; k++)
			m_i64VOBSize[k]=0;

		csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-3);
		csAux=csAux2+"VOB";
		if (_stati64 ( csAux, &statbuf)==0)
			m_i64VOBSize[0]= statbuf.st_size;
	}
	else
	{
		for (k=0; k<10; k++)
		{
			csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
			csAux.Format(_T("%d.VOB"), k);
			csAux=csAux2+csAux;
			if (_stati64 ( csAux, &statbuf)==0)
			{
				m_i64VOBSize[k]= statbuf.st_size;
				m_nVobFiles=k;
			}
			else 
				m_i64VOBSize[k]=0;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
/////////////   Demuxing Code  : Process Pack, called by all demuxing methods
/////////////                    bWrite: If true Write files, if not used for delay checking
////////////////////////////////////////////////////////////////////////////////

int CPgcDemuxApp::ProcessPack(bool bWrite)
{
	int sID;
	static int nPack=0;
	static int nFirstRef=0;
	bool bFirstAud;
	int nBytesOffset;

	if ( bWrite && m_bCheckVob ) 
	{
		if (IsNav(m_buffer)) 
		{
			if (m_bCheckLBA) ModifyLBA (m_buffer,m_i64OutputLBA);
			m_nVidout= (int) GetNbytes(2,&m_buffer[0x41f]);
			m_nCidout= (int) m_buffer[0x422];
			nFirstRef= (int) GetNbytes(4,&m_buffer[0x413]);
			nPack=0;

			bNewCell=false;
			if (m_nVidout!= m_nLastVid || m_nCidout!= m_nLastCid) 
			{
				bNewCell=true;
				m_nLastVid=m_nVidout;
				m_nLastCid=m_nCidout;
			}
		}
		else
			nPack++;
		if ( (IsNav(m_buffer)   && m_bCheckNavPack)   ||
		     (IsAudio(m_buffer) && m_bCheckAudioPack) ||
		     (IsSubs (m_buffer) && m_bCheckSubPack)   )
				WritePack(m_buffer);
		else if ( IsVideo(m_buffer) && m_bCheckVideoPack) 
		{
			if (!m_bCheckIFrame)
				WritePack(m_buffer);
			else
			{
//				if (nFirstRef == nPack)  
//					if ( ! PatchEndOfSequence(m_buffer))
//						WritePack (Pad_pack);
				if (bNewCell && nFirstRef >= nPack ) WritePack(m_buffer);
			}
		}

	}
	if (IsNav(m_buffer))
	{
// do nothing
		m_nNavPacks++;
		m_iNavPTS0= (int) GetNbytes(4,&m_buffer[0x39]);
		m_iNavPTS1= (int) GetNbytes(4,&m_buffer[0x3d]);
		if (m_iFirstNavPTS0==0) m_iFirstNavPTS0=m_iNavPTS0;
		if (m_iNavPTS1_old> m_iNavPTS0) 
		{
// Discontinuity, so add the offset 
			m_iOffsetPTS+=(m_iNavPTS1_old-m_iNavPTS0);
		}
		m_iNavPTS0_old=m_iNavPTS0;
		m_iNavPTS1_old=m_iNavPTS1;
	}
	else if (IsVideo(m_buffer))
	{
		m_nVidPacks++;
		if (m_buffer[0x15] & 0x80) 
		{
			m_iVidPTS= readpts(&m_buffer[0x17]);
			if (m_iFirstVidPTS==0) m_iFirstVidPTS=m_iVidPTS;
		}
		if ( bWrite && m_bCheckVid) demuxvideo(m_buffer);
	}
	else if (IsAudio(m_buffer))
	{
		m_nAudPacks++;
		nBytesOffset=0;

		sID=getAudId(m_buffer) & 0x07;

		bFirstAud=false;

		if (m_buffer[0x15] & 0x80) 
		{
			if (m_iFirstAudPTS[sID]==0)
			{
				bFirstAud=true;
				m_iAudPTS= readpts(&m_buffer[0x17]);
				m_iFirstAudPTS[sID]=m_iAudPTS;
//				m_iAudIndex[sID]=m_buffer[0x17+m_buffer[0x16]];
				m_iAudIndex[sID]=getAudId (m_buffer);
			}
		}
		if (bFirstAud)
		{
			nBytesOffset=GetAudHeader(m_buffer);
			if (nBytesOffset<0)
// This pack does not have an Audio Frame Header, so its PTS is  not valid.
				m_iFirstAudPTS[sID]=0;
		}

		if ( bWrite && m_bCheckAud && m_iFirstAudPTS[sID]!=0)
		{
			demuxaudio(m_buffer,nBytesOffset);
		}
	}
	else if (IsSubs(m_buffer))
	{
		m_nSubPacks++;
		sID=m_buffer[0x17+m_buffer[0x16]] & 0x1F;
		
		if (m_buffer[0x15] & 0x80) 
		{
			m_iSubPTS= readpts(&m_buffer[0x17]);
			if (m_iFirstSubPTS[sID]==0)
				m_iFirstSubPTS[sID]=m_iSubPTS;
		}
		if (bWrite && m_bCheckSub) demuxsubs(m_buffer);
	}
	else if (IsPad(m_buffer))
	{
		m_nPadPacks++;
	}
	else
	{
		m_nUnkPacks++;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
/////////////   Audio Delay  Code: 
////////////////////////////////////////////////////////////////////////////////
int CPgcDemuxApp::GetAudHeader(uchar* buffer)
// Returns the number of bytes from audio start until first header
// If no header found  returns -1
{
	int i,start,nbytes;
	uchar streamID;
	int firstheader,nHeaders;
	bool bFound;

	start=0x17+buffer[0x16];
	nbytes=buffer[0x12]*256+buffer[0x13]+0x14;
	if (IsAudMpeg(buffer))
		streamID=buffer[0x11];
	else
	{
		streamID=buffer[start];
		start+=4;
	}

	firstheader=0;

// Check if PCM
	if ( streamID>=0xa0 && streamID<= 0xa7 ) return 0;
	if ( streamID>=0x80 && streamID<= 0x8f ) 
	{
// Stream is AC3 or DTS...
		nHeaders=buffer[start-3];
		if (nHeaders !=0)
		{
			bFound=true;
			firstheader= buffer[start-2]*256+ buffer[start-1]-1;
		}
		else
			bFound=false;
	}
	else if ( streamID>=0xc0 && streamID<= 0xc7 ) 
	{
// Stream is MPEG ...
		for (i=start, bFound=false ; i< (nbytes-1) && bFound==false ; i++)
		{
//			if ( buffer[start+i] == 0xFF && (buffer[start+1+i] & 0xF0 )== 0xF0 )
			if ( buffer[i] == 0xFF && (buffer[i+1] & 0xF0 )== 0xF0 )
			{
				bFound=true;
				firstheader=i-start;
			}
		}
	}

	if ((start+firstheader) >= nbytes) bFound=false;
	
	if (bFound)
		return firstheader;
	else 
		return -1;

}

int CPgcDemuxApp::GetAudioDelay(int iMode, int nSelection)
{
	int VID,CID;
	int k,nCell;
	__int64 i64IniSec,i64EndSec;
	__int64 i64sectors;
	int nVobin;
	CString csAux,csAux2;
	FILE *in;
	__int64 i64;
	bool bMyCell;
	int iRet;

	IniDemuxGlobalVars();

	if (iMode==PGCMODE)
	{
		if (nSelection >= m_nPGCs)
		{
			MyErrorBox("Error: PGC does not exist");
			return -1;
		}
		nCell=0;
		VID=GetNbytes(2,&m_pIFO[m_C_POST[nSelection]+4*nCell]);
		CID=m_pIFO[m_C_POST[nSelection]+3+4*nCell];
	}
	else if (iMode==VIDMODE)
	{
		if (nSelection >= m_AADT_Vid_list.GetSize())
		{
			MyErrorBox("Error: VID does not exist");
			return -1;
		}
		VID=m_AADT_Vid_list[nSelection].VID;
		CID=-1;
		for (k=0;k<m_AADT_Cell_list.GetSize() && CID==-1; k++)
		{
			if (VID==m_AADT_Cell_list[k].VID)
				CID=m_AADT_Cell_list[k].CID;
		}

	}
	else if (iMode==CIDMODE)
	{
		if (nSelection >= m_AADT_Cell_list.GetSize())
		{
			MyErrorBox("Error: CID does not exist");
			return -1;
		}
		VID=m_AADT_Cell_list[nSelection].VID;
		CID=m_AADT_Cell_list[nSelection].CID;
	}

	for (k=0,nCell=-1; k < m_AADT_Cell_list.GetSize() && nCell==-1; k++)
	{
		if (VID==m_AADT_Cell_list[k].VID &&	
			CID==m_AADT_Cell_list[k].CID)
			nCell=k;
	}

	if (nCell<0)
	{
		MyErrorBox("Error: VID/CID not found!.");
		return -1;
	}
//
// Now we have VID; CID; and the index in Cell Array "nCell".
// So we are going to open the VOB and read the delays using ProcessPack(false)
	i64IniSec=m_AADT_Cell_list[nCell].iIniSec;
	i64EndSec=m_AADT_Cell_list[nCell].iEndSec;

	iRet=0;
	for (k=1,i64sectors=0;k<10;k++)
	{
		i64sectors+=(m_i64VOBSize[k]/2048);
		if (i64IniSec<i64sectors)
		{
			i64sectors-=(m_i64VOBSize[k]/2048);
			nVobin=k;
			k=20;
		}
	}
	csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
	csAux.Format(_T("%d.VOB"), nVobin);
	csAux=csAux2+csAux;
	in=fopen(csAux,"rb");
	if (in ==NULL)
	{
		MyErrorBox("Error opening input VOB: "+csAux);
		iRet=-1;
	}
	if (iRet==0) fseek(in, (long) ((i64IniSec-i64sectors)*2048), SEEK_SET);

	for (i64=0,bMyCell=true; iRet==0 && i64< (i64EndSec-i64IniSec+1) && i64< MAXLOOKFORAUDIO;i64++)
	{
	//readpack
		if (readbuffer(m_buffer,in)!=2048)
		{
			if (in!=NULL) fclose (in);
			nVobin++;
			csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
			csAux.Format(_T("%d.VOB"), nVobin);
			csAux=csAux2+csAux;
			in=fopen(csAux,"rb");
			if (readbuffer(m_buffer,in)!=2048)
			{
				MyErrorBox("Input error: Reached end of VOB too early");
				iRet=-1;
			}
		}

		if (iRet==0)
		{
			if (IsSynch(m_buffer) != true)
			{
				MyErrorBox("Error reading input VOB: Unsynchronized");
				iRet=-1;
			}
			if ((iRet==0) && IsNav(m_buffer))
			{
				if (m_buffer[0x420]==(uchar)(VID%256) &&
					m_buffer[0x41F]==(uchar)(VID/256) &&
					m_buffer[0x422]==(uchar) CID)
					bMyCell=true;
				else
					bMyCell=false;
			}
			
			if (iRet==0 && bMyCell)
			{
				iRet=ProcessPack(false);
			}
		}
	} // For readpacks
	if (in!=NULL) fclose (in);
	in=NULL;

	return iRet;
}


int CPgcDemuxApp::GetMAudioDelay(int iMode, int nSelection)
{
	int VID,CID;
	int k,nCell;
	__int64 i64IniSec,i64EndSec;
	CString csAux,csAux2;
	FILE *in;
	__int64 i64;
	bool bMyCell;
	int iRet;

	IniDemuxGlobalVars();

	if (iMode==PGCMODE)
	{
		if (nSelection >= m_nMPGCs)
		{
			MyErrorBox("Error: PGC does not exist");
			return -1;
		}
		nCell=0;
		VID=GetNbytes(2,&m_pIFO[m_M_C_POST[nSelection]+4*nCell]);
		CID=m_pIFO[m_M_C_POST[nSelection]+3+4*nCell];
	}
	else if (iMode==VIDMODE)
	{
		if (nSelection >= m_MADT_Vid_list.GetSize())
		{
			MyErrorBox("Error: VID does not exist");
			return -1;
		}
		VID=m_MADT_Vid_list[nSelection].VID;
		CID=-1;
		for (k=0;k<m_MADT_Cell_list.GetSize() && CID==-1; k++)
		{
			if (VID==m_MADT_Cell_list[k].VID)
				CID=m_MADT_Cell_list[k].CID;
		}

	}
	else if (iMode==CIDMODE)
	{
		if (nSelection >= m_MADT_Cell_list.GetSize())
		{
			MyErrorBox("Error: CID does not exist");
			return -1;
		}
		VID=m_MADT_Cell_list[nSelection].VID;
		CID=m_MADT_Cell_list[nSelection].CID;
	}

	for (k=0,nCell=-1; k < m_MADT_Cell_list.GetSize() && nCell==-1; k++)
	{
		if (VID==m_MADT_Cell_list[k].VID &&	
			CID==m_MADT_Cell_list[k].CID)
			nCell=k;
	}

	if (nCell<0)
	{
		MyErrorBox("Error: VID/CID not found!.");
		return -1;
	}
//
// Now we have VID; CID; and the index in Cell Array "nCell".
// So we are going to open the VOB and read the delays using ProcessPack(false)
	i64IniSec=m_MADT_Cell_list[nCell].iIniSec;
	i64EndSec=m_MADT_Cell_list[nCell].iEndSec;

	iRet=0;

	if (m_bVMGM)
	{
		csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-3);
		csAux=csAux2+"VOB";
	}
	else
	{
		csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
		csAux=csAux2+"0.VOB";
	}
	in=fopen(csAux,"rb");
	if (in ==NULL)
	{
		MyErrorBox("Error opening input VOB: "+csAux);
		iRet=-1;
	}
	if (iRet==0) fseek(in, (long) ((i64IniSec)*2048), SEEK_SET);

	for (i64=0,bMyCell=true; iRet==0 && i64< (i64EndSec-i64IniSec+1) && i64< MAXLOOKFORAUDIO;i64++)
	{
//readpack
		if (readbuffer(m_buffer,in)!=2048)
		{
			MyErrorBox("Input error: Reached end of VOB too early");
			iRet=-1;
		}

		if (iRet==0)
		{
			if (IsSynch(m_buffer) != true)
			{
				MyErrorBox("Error reading input VOB: Unsynchronized");
				iRet=-1;
			}
			if ((iRet==0) && IsNav(m_buffer))
			{
				if (m_buffer[0x420]==(uchar)(VID%256) &&
					m_buffer[0x41F]==(uchar)(VID/256) &&
					m_buffer[0x422]==(uchar) CID)
					bMyCell=true;
				else
					bMyCell=false;
			}
			
			if (iRet==0 && bMyCell)
			{
				iRet=ProcessPack(false);
			}
		}
	} // For readpacks
	if (in!=NULL) fclose (in);
	in=NULL;

	return iRet;
}

////////////////////////////////////////////////////////////////////////////////
/////////////   Demuxing Code : DEMUX BY PGC
////////////////////////////////////////////////////////////////////////////////
void CPgcDemuxApp::IniDemuxGlobalVars()
{
	int k;
	CString csAux;

	// clear PTS
	for (k=0;k<32;k++)
		m_iFirstSubPTS[k]=0;
	for (k=0;k<8;k++)
	{
		m_iFirstAudPTS[k]=0;
		nchannels[k]=-1;
		nbitspersample[k]=-1;
		fsample[k]=-1;
	}
	m_iFirstVidPTS=0;
	m_iFirstNavPTS0=0;
	m_iNavPTS0_old=m_iNavPTS0=0;
	m_iNavPTS1_old=m_iNavPTS1=0;

	m_nNavPacks=m_nVidPacks=m_nAudPacks=m_nSubPacks=m_nUnkPacks=m_nPadPacks=0;
	m_i64OutputLBA=0;
	m_nVobout=m_nVidout=m_nCidout=0;
	m_nLastVid,m_nLastCid=0;

	m_nCurrVid=0;
	m_iOffsetPTS=0;
	bNewCell=false;
}

int CPgcDemuxApp::OpenVideoFile()
{
	CString csAux;

	if (m_bCheckVid)
	{
		csAux=m_csOutputPath+ '\\' + "VideoFile.m2v";
		fvid=fopen(csAux,"wb");
		if (fvid==NULL) return -1;
	}

	return 0;
}


int CPgcDemuxApp::PgcDemux(int nPGC, int nAng, CWnd* pDlg)
{
	int nTotalSectors;
	int nSector,nCell;
	int k,iArraysize;
	int CID,VID;
	__int64 i64IniSec,i64EndSec;
	__int64 i64sectors;
	int nVobin;
	CString csAux,csAux2;
	FILE *in, *fout;
	__int64 i64;
	bool bMyCell;
	int iRet;
	DWORD dwCellDuration;
	int nFrames;
	int nCurrAngle, iCat;

	if (nPGC >= theApp.m_nPGCs)
	{
		MyErrorBox("Error: PGC does not exist");
		m_bInProcess=false;
		return -1;
	}

	IniDemuxGlobalVars();
	if (OpenVideoFile()) return -1;
	m_bInProcess=true;

// Calculate  the total number of sectors
	nTotalSectors=0;
	iArraysize=m_AADT_Cell_list.GetSize();
	for (nCell=nCurrAngle=0; nCell<m_nCells[nPGC]; nCell++)
	{
		VID=GetNbytes(2,&m_pIFO[m_C_POST[nPGC]+4*nCell]);
		CID=m_pIFO[m_C_POST[nPGC]+3+4*nCell];

		iCat=m_pIFO[m_C_PBKT[nPGC]+24*nCell];
		iCat=iCat & 0xF0;
//		0101=First; 1001=Middle ;	1101=Last
		if (iCat == 0x50)
			nCurrAngle=1;
		else if ((iCat == 0x90 || iCat == 0xD0) && nCurrAngle!=0)
			nCurrAngle++;
		if (iCat==0 || (nAng+1) == nCurrAngle)
		{
			for (k=0; k< iArraysize ;k++)
			{
				if (CID==m_AADT_Cell_list[k].CID &&
					VID==m_AADT_Cell_list[k].VID )
				{
					nTotalSectors+= m_AADT_Cell_list[k].iSize;
				}
			}
		}
		if (iCat == 0xD0)	nCurrAngle=0;
	}

	nSector=0;
	iRet=0;
	for (nCell=nCurrAngle=0; nCell<m_nCells[nPGC] && m_bInProcess==true; nCell++)
	{
		iCat=m_pIFO[m_C_PBKT[nPGC]+24*nCell];
		iCat=iCat & 0xF0;
//		0101=First; 1001=Middle ;	1101=Last
		if (iCat == 0x50)
			nCurrAngle=1;
		else if ((iCat == 0x90 || iCat == 0xD0) && nCurrAngle!=0)
			nCurrAngle++;
		if (iCat==0 || (nAng+1) == nCurrAngle)
		{

			VID=GetNbytes(2,&m_pIFO[m_C_POST[nPGC]+4*nCell]);
			CID=m_pIFO[m_C_POST[nPGC]+3+4*nCell];

			i64IniSec=GetNbytes(4,&m_pIFO[m_C_PBKT[nPGC]+nCell*24+8]);
			i64EndSec=GetNbytes(4,&m_pIFO[m_C_PBKT[nPGC]+nCell*24+0x14]);
			for (k=1,i64sectors=0;k<10;k++)
			{
				i64sectors+=(m_i64VOBSize[k]/2048);
				if (i64IniSec<i64sectors)
				{
					i64sectors-=(m_i64VOBSize[k]/2048);
					nVobin=k;
					k=20;
				}
			}
			csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
			csAux.Format(_T("%d.VOB"), nVobin);
			csAux=csAux2+csAux;
			in=fopen(csAux,"rb");
			if (in ==NULL)
			{
				MyErrorBox("Error opening input VOB: "+csAux);
				m_bInProcess=false;
				iRet=-1;
			}
			if (m_bInProcess) fseek(in, (long) ((i64IniSec-i64sectors)*2048), SEEK_SET);

			for (i64=0,bMyCell=true;i64< (i64EndSec-i64IniSec+1) && m_bInProcess==true;i64++)
			{
			//readpack
				if ((i64%MODUPDATE) == 0) UpdateProgress(pDlg, (int)((100*nSector)/nTotalSectors) );
				if (readbuffer(m_buffer,in)!=2048)
				{
					if (in!=NULL) fclose (in);
					nVobin++;
					csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
					csAux.Format(_T("%d.VOB"), nVobin);
					csAux=csAux2+csAux;
					in=fopen(csAux,"rb");
					if (readbuffer(m_buffer,in)!=2048)
					{
						MyErrorBox("Input error: Reached end of VOB too early");
						m_bInProcess=false;
						iRet=-1;
					}
				}

				if (m_bInProcess==true)
				{
					if (IsSynch(m_buffer) != true)
					{
						MyErrorBox("Error reading input VOB: Unsynchronized");
						m_bInProcess=false;
						iRet=-1;
					}
					if (IsNav(m_buffer))
					{
						if (m_buffer[0x420]==(uchar)(VID%256) &&
							m_buffer[0x41F]==(uchar)(VID/256) &&
							m_buffer[0x422]==(uchar) CID)
							bMyCell=true;
						else
							bMyCell=false;
					}
			
					if (bMyCell)
					{
						nSector++;
						iRet=ProcessPack(true);
					}

				}
			} // For readpacks
			if (in!=NULL) fclose (in);
			in=NULL;
		}  // if (iCat==0 || (nAng+1) == nCurrAngle)
		if (iCat == 0xD0) nCurrAngle=0;
	}	// For Cells 

	CloseAndNull();
	nFrames=0;

	if (m_bCheckCellt && m_bInProcess==true)
	{
		csAux=m_csOutputPath+ '\\' + "Celltimes.txt";
		fout=fopen(csAux,"w");
		for (nCell=0,nCurrAngle=0; nCell<m_nCells[nPGC] && m_bInProcess==true; nCell++)
		{
			dwCellDuration=GetNbytes(4,&m_pIFO[m_C_PBKT[nPGC]+24*nCell+4]);

			iCat=m_pIFO[m_C_PBKT[nPGC]+24*nCell];
			iCat=iCat & 0xF0;
//			0101=First; 1001=Middle ;	1101=Last
			if (iCat == 0x50)
				nCurrAngle=1;
			else if ((iCat == 0x90 || iCat == 0xD0) && nCurrAngle!=0)
				nCurrAngle++;
			if (iCat==0 || (nAng+1) == nCurrAngle)
			{
				nFrames+=DurationInFrames(dwCellDuration);
				if (nCell!=(m_nCells[nPGC]-1) || m_bCheckEndTime ) 
					fprintf(fout,"%d\n",nFrames);
			}

			if (iCat == 0xD0) nCurrAngle=0;
		}
		fclose(fout);
	}

	m_nTotalFrames=nFrames;

	if (m_bCheckLog && m_bInProcess==true) OutputLog(nPGC, nAng, TITLES);

	return iRet;
}

int CPgcDemuxApp::PgcMDemux(int nPGC, CWnd* pDlg)
{
	int nTotalSectors;
	int nSector,nCell;
	int k,iArraysize;
	int CID,VID;
	__int64 i64IniSec,i64EndSec;
	CString csAux,csAux2;
	FILE *in,*fout;
	__int64 i64;
	bool bMyCell;
	int iRet;
	DWORD dwCellDuration;
	int nFrames;


	if (nPGC >= theApp.m_nMPGCs)
	{
		MyErrorBox("Error: PGC does not exist");
		m_bInProcess=false;
		return -1;
	}

	IniDemuxGlobalVars();
	if (OpenVideoFile()) return -1;
	m_bInProcess=true;

// Calculate  the total number of sectors
	nTotalSectors=0;
	iArraysize=m_MADT_Cell_list.GetSize();
	for (nCell=0; nCell<m_nMCells[nPGC]; nCell++)
	{
		VID=GetNbytes(2,&m_pIFO[m_M_C_POST[nPGC]+4*nCell]);
		CID=m_pIFO[m_M_C_POST[nPGC]+3+4*nCell];
		for (k=0; k< iArraysize ;k++)
		{
			if (CID==m_MADT_Cell_list[k].CID &&
				VID==m_MADT_Cell_list[k].VID )
			{
				nTotalSectors+= m_MADT_Cell_list[k].iSize;
			}
		}
	}

	nSector=0;
	iRet=0;

	for (nCell=0; nCell<m_nMCells[nPGC] && m_bInProcess==true; nCell++)
	{
		VID=GetNbytes(2,&m_pIFO[m_M_C_POST[nPGC]+4*nCell]);
		CID=m_pIFO[m_M_C_POST[nPGC]+3+4*nCell];

		i64IniSec=GetNbytes(4,&m_pIFO[m_M_C_PBKT[nPGC]+nCell*24+8]);
		i64EndSec=GetNbytes(4,&m_pIFO[m_M_C_PBKT[nPGC]+nCell*24+0x14]);

		if (m_bVMGM)
		{
			csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-3);
			csAux=csAux2+"VOB";
		}
		else
		{
			csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
			csAux=csAux2+"0.VOB";
		}
		in=fopen(csAux,"rb");
		if (in ==NULL)
		{
			MyErrorBox("Error opening input VOB: "+csAux);
			m_bInProcess=false;
			iRet=-1;
		}
		if (m_bInProcess) fseek(in, (long) ((i64IniSec)*2048), SEEK_SET);

		for (i64=0,bMyCell=true;i64< (i64EndSec-i64IniSec+1) && m_bInProcess==true;i64++)
		{
			//readpack
			if ((i64%MODUPDATE) == 0) UpdateProgress(pDlg, (int)((100*nSector)/nTotalSectors) );
			if (readbuffer(m_buffer,in)!=2048)
			{
				if (in!=NULL) fclose (in);
				MyErrorBox("Input error: Reached end of VOB too early");
				m_bInProcess=false;
				iRet=-1;
			}

			if (m_bInProcess==true)
			{
				if (IsSynch(m_buffer) != true)
				{
					MyErrorBox("Error reading input VOB: Unsynchronized");
					m_bInProcess=false;
					iRet=-1;
				}
				if (IsNav(m_buffer))
				{
					if (m_buffer[0x420]==(uchar)(VID%256) &&
						m_buffer[0x41F]==(uchar)(VID/256) &&
						m_buffer[0x422]== (uchar) CID)
						bMyCell=true;
					else
						bMyCell=false;
				}

				if (bMyCell)
				{
					nSector++;
					iRet=ProcessPack(true);
				}
			}
		} // For readpacks
		if (in!=NULL) fclose (in);
		in=NULL;
	}	// For Cells 

	CloseAndNull();

	nFrames=0;

	if (m_bCheckCellt && m_bInProcess==true)
	{
		csAux=m_csOutputPath+ '\\' + "Celltimes.txt";
		fout=fopen(csAux,"w");
		for (nCell=0; nCell<m_nMCells[nPGC] && m_bInProcess==true; nCell++)
		{
			dwCellDuration=GetNbytes(4,&m_pIFO[m_M_C_PBKT[nPGC]+24*nCell+4]);
			nFrames+=DurationInFrames(dwCellDuration);
			if (nCell!=(m_nMCells[nPGC]-1) || m_bCheckEndTime ) 
				fprintf(fout,"%d\n",nFrames);
		}
		fclose(fout);
	}

	m_nTotalFrames=nFrames;

	if (m_bCheckLog && m_bInProcess==true) OutputLog(nPGC, 1 , MENUS);

	return iRet;
}

////////////////////////////////////////////////////////////////////////////////
/////////////   Demuxing Code : DEMUX BY VOBID
////////////////////////////////////////////////////////////////////////////////

int CPgcDemuxApp::VIDDemux(int nVid, CWnd* pDlg)
{
	int nTotalSectors;
	int nSector,nCell;
	int k,iArraysize;
	int CID,VID,nDemuxedVID;
	__int64 i64IniSec,i64EndSec;
	__int64 i64sectors;
	int nVobin;
	CString csAux,csAux2;
	FILE *in, *fout;
	__int64 i64;
	bool bMyCell;
	int iRet;
	int nFrames;
	int nLastCell;

	if (nVid >= m_AADT_Vid_list.GetSize())
	{
		MyErrorBox("Error: Selected Vid does not exist");
		m_bInProcess=false;
		return -1;
	}
	
	IniDemuxGlobalVars();
	if (OpenVideoFile()) return -1;
	m_bInProcess=true;

// Calculate  the total number of sectors
	nTotalSectors= m_AADT_Vid_list[nVid].iSize;
	nSector=0;
	iRet=0;
	nDemuxedVID=m_AADT_Vid_list[nVid].VID; 

	iArraysize=m_AADT_Cell_list.GetSize();
	for (nCell=0; nCell<iArraysize && m_bInProcess==true; nCell++)
	{
		VID=m_AADT_Cell_list[nCell].VID;
		CID=m_AADT_Cell_list[nCell].CID;

		if (VID==nDemuxedVID)
		{
			i64IniSec=m_AADT_Cell_list[nCell].iIniSec;
			i64EndSec=m_AADT_Cell_list[nCell].iEndSec;
			for (k=1,i64sectors=0;k<10;k++)
			{
				i64sectors+=(m_i64VOBSize[k]/2048);
				if (i64IniSec<i64sectors)
				{
					i64sectors-=(m_i64VOBSize[k]/2048);
					nVobin=k;
					k=20;
				}
			}
			csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
			csAux.Format(_T("%d.VOB"), nVobin);
			csAux=csAux2+csAux;
			in=fopen(csAux,"rb");
			if (in ==NULL)
			{
				MyErrorBox("Error opening input VOB: "+csAux);
				m_bInProcess=false;
				iRet=-1;
			}
			if (m_bInProcess) fseek(in, (long) ((i64IniSec-i64sectors)*2048), SEEK_SET);

			for (i64=0,bMyCell=true;i64< (i64EndSec-i64IniSec+1) && m_bInProcess==true;i64++)
			{
			//readpack
				if ((i64%MODUPDATE) == 0) UpdateProgress(pDlg, (int)((100*nSector)/nTotalSectors) );
				if (readbuffer(m_buffer,in)!=2048)
				{
					if (in!=NULL) fclose (in);
					nVobin++;
					csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
					csAux.Format(_T("%d.VOB"), nVobin);
					csAux=csAux2+csAux;
					in=fopen(csAux,"rb");
					if (readbuffer(m_buffer,in)!=2048)
					{
						MyErrorBox("Input error: Reached end of VOB too early");
						m_bInProcess=false;
						iRet=-1;
					}
				}

				if (m_bInProcess==true)
				{
					if (IsSynch(m_buffer) != true)
					{
						MyErrorBox("Error reading input VOB: Unsynchronized");
						m_bInProcess=false;
						iRet=-1;
					}
					if (IsNav(m_buffer))
					{
						if (m_buffer[0x420]==(uchar)(VID%256) &&
							m_buffer[0x41F]==(uchar)(VID/256) &&
							m_buffer[0x422]==(uchar) CID)
							bMyCell=true;
						else
							bMyCell=false;
					}
			
					if (bMyCell)
					{
						nSector++;
						iRet=ProcessPack(true);
					}
				}
			} // For readpacks
			if (in!=NULL) fclose (in);
			in=NULL;
		}  // if (VID== DemuxedVID)
	}	// For Cells 

	CloseAndNull();
	nFrames=0;

	if (m_bCheckCellt && m_bInProcess==true)
	{
		csAux=m_csOutputPath+ '\\' + "Celltimes.txt";
		fout=fopen(csAux,"w");

		nDemuxedVID=m_AADT_Vid_list[nVid].VID; 

		iArraysize=m_AADT_Cell_list.GetSize();
		for (nCell=nLastCell=0; nCell<iArraysize && m_bInProcess==true; nCell++)
		{
			VID=m_AADT_Cell_list[nCell].VID;
			if (VID==nDemuxedVID)
				nLastCell=nCell;
		}

		for (nCell=0; nCell<iArraysize && m_bInProcess==true; nCell++)
		{
			VID=m_AADT_Cell_list[nCell].VID;

			if (VID==nDemuxedVID)
			{
				nFrames+=DurationInFrames(m_AADT_Cell_list[nCell].dwDuration);
				if (nCell!=nLastCell || m_bCheckEndTime ) 
					fprintf(fout,"%d\n",nFrames);
			}
		}
		fclose(fout);
	}

	m_nTotalFrames=nFrames;

	if (m_bCheckLog && m_bInProcess==true) OutputLog(nVid, 1, TITLES);

	return iRet;
}

int CPgcDemuxApp::VIDMDemux(int nVid, CWnd* pDlg)
{
	int nTotalSectors;
	int nSector,nCell;
	int iArraysize;
	int CID,VID,nDemuxedVID;
	__int64 i64IniSec,i64EndSec;
	CString csAux,csAux2;
	FILE *in, *fout;
	__int64 i64;
	bool bMyCell;
	int iRet;
	int nFrames;
	int nLastCell;

	if (nVid >= m_MADT_Vid_list.GetSize())
	{
		MyErrorBox("Error: Selected Vid does not exist");
		m_bInProcess=false;
		return -1;
	}
	
	IniDemuxGlobalVars();
	if (OpenVideoFile()) return -1;
	m_bInProcess=true;

// Calculate  the total number of sectors
	nTotalSectors= m_MADT_Vid_list[nVid].iSize;
	nSector=0;
	iRet=0;
	nDemuxedVID=m_MADT_Vid_list[nVid].VID; 

	iArraysize=m_MADT_Cell_list.GetSize();
	for (nCell=0; nCell<iArraysize && m_bInProcess==true; nCell++)
	{
		VID=m_MADT_Cell_list[nCell].VID;
		CID=m_MADT_Cell_list[nCell].CID;

		if (VID==nDemuxedVID)
		{
			i64IniSec=m_MADT_Cell_list[nCell].iIniSec;
			i64EndSec=m_MADT_Cell_list[nCell].iEndSec;
			if (m_bVMGM)
			{
				csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-3);
				csAux=csAux2+"VOB";
			}
			else
			{
				csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
				csAux=csAux2+"0.VOB";
			}
			in=fopen(csAux,"rb");
			if (in ==NULL)
			{
				MyErrorBox("Error opening input VOB: "+csAux);
				m_bInProcess=false;
				iRet=-1;
			}
			if (m_bInProcess) fseek(in, (long) ((i64IniSec)*2048), SEEK_SET);

			for (i64=0,bMyCell=true;i64< (i64EndSec-i64IniSec+1) && m_bInProcess==true;i64++)
			{
	//readpack
				if ((i64%MODUPDATE) == 0) UpdateProgress(pDlg, (int)((100*nSector)/nTotalSectors) );
				if (readbuffer(m_buffer,in)!=2048)
				{
					if (in!=NULL) fclose (in);
					MyErrorBox("Input error: Reached end of VOB too early");
					m_bInProcess=false;
					iRet=-1;
				}

				if (m_bInProcess==true)
				{
					if (IsSynch(m_buffer) != true)
					{
						MyErrorBox("Error reading input VOB: Unsynchronized");
						m_bInProcess=false;
						iRet=-1;
					}
					if (IsNav(m_buffer))
					{
						if (m_buffer[0x420]==(uchar)(VID%256) &&
							m_buffer[0x41F]==(uchar)(VID/256) &&
							m_buffer[0x422]== (uchar) CID)
							bMyCell=true;
						else
							bMyCell=false;
					}

					if (bMyCell)
					{
						nSector++;
						iRet=ProcessPack(true);
					}
				}
			} // For readpacks
			if (in!=NULL) fclose (in);
			in=NULL;
		} // If (VID==DemuxedVID)
	}	// For Cells 

	CloseAndNull();

	nFrames=0;

	if (m_bCheckCellt && m_bInProcess==true)
	{
		csAux=m_csOutputPath+ '\\' + "Celltimes.txt";
		fout=fopen(csAux,"w");

		nDemuxedVID=m_MADT_Vid_list[nVid].VID; 

		iArraysize=m_MADT_Cell_list.GetSize();

		for (nCell=nLastCell=0; nCell<iArraysize && m_bInProcess==true; nCell++)
		{
			VID=m_MADT_Cell_list[nCell].VID;
			if (VID==nDemuxedVID) nLastCell=nCell;
		}


		for (nCell=0; nCell<iArraysize && m_bInProcess==true; nCell++)
		{
			VID=m_MADT_Cell_list[nCell].VID;

			if (VID==nDemuxedVID)
			{
				nFrames+=DurationInFrames(m_MADT_Cell_list[nCell].dwDuration);
				if (nCell!=nLastCell || m_bCheckEndTime ) 
					fprintf(fout,"%d\n",nFrames);
			}
		}
		fclose(fout);
	}

	m_nTotalFrames=nFrames;

	if (m_bCheckLog && m_bInProcess==true) OutputLog(nVid, 1 , MENUS);

	return iRet;
}

////////////////////////////////////////////////////////////////////////////////
/////////////   Demuxing Code : DEMUX BY CELLID
////////////////////////////////////////////////////////////////////////////////
int CPgcDemuxApp::CIDDemux(int nCell, CWnd* pDlg)
{
	int nTotalSectors;
	int nSector;
	int k;
	int CID,VID;
	__int64 i64IniSec,i64EndSec;
	__int64 i64sectors;
	int nVobin;
	CString csAux,csAux2;
	FILE *in, *fout;
	__int64 i64;
	bool bMyCell;
	int iRet;
	int nFrames;

	if (nCell >= m_AADT_Cell_list.GetSize())
	{
		MyErrorBox("Error: Selected Cell does not exist");
		m_bInProcess=false;
		return -1;
	}
	
	IniDemuxGlobalVars();
	if (OpenVideoFile()) return -1;
	m_bInProcess=true;

// Calculate  the total number of sectors
	nTotalSectors= m_AADT_Cell_list[nCell].iSize;
	nSector=0;
	iRet=0;

	VID=m_AADT_Cell_list[nCell].VID;
	CID=m_AADT_Cell_list[nCell].CID;

	i64IniSec=m_AADT_Cell_list[nCell].iIniSec;
	i64EndSec=m_AADT_Cell_list[nCell].iEndSec;
	for (k=1,i64sectors=0;k<10;k++)
	{
		i64sectors+=(m_i64VOBSize[k]/2048);
		if (i64IniSec<i64sectors)
		{
			i64sectors-=(m_i64VOBSize[k]/2048);
			nVobin=k;
			k=20;
		}
	}
	csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
	csAux.Format(_T("%d.VOB"), nVobin);
	csAux=csAux2+csAux;
	in=fopen(csAux,"rb");
	if (in ==NULL)
	{
		MyErrorBox("Error opening input VOB: "+csAux);
		m_bInProcess=false;
		iRet=-1;
	}
	if (m_bInProcess) fseek(in, (long) ((i64IniSec-i64sectors)*2048), SEEK_SET);

	for (i64=0,bMyCell=true;i64< (i64EndSec-i64IniSec+1) && m_bInProcess==true;i64++)
	{
	//readpack
		if ((i64%MODUPDATE) == 0) UpdateProgress(pDlg, (int)((100*nSector)/nTotalSectors) );
		if (readbuffer(m_buffer,in)!=2048)
		{
			if (in!=NULL) fclose (in);
			nVobin++;
			csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
			csAux.Format(_T("%d.VOB"), nVobin);
			csAux=csAux2+csAux;
			in=fopen(csAux,"rb");
			if (readbuffer(m_buffer,in)!=2048)
			{
				MyErrorBox("Input error: Reached end of VOB too early");
				m_bInProcess=false;
				iRet=-1;
			}
		}

		if (m_bInProcess==true)
		{
			if (IsSynch(m_buffer) != true)
			{
				MyErrorBox("Error reading input VOB: Unsynchronized");
				m_bInProcess=false;
				iRet=-1;
			}
			if (IsNav(m_buffer))
			{
				if (m_buffer[0x420]==(uchar)(VID%256) &&
					m_buffer[0x41F]==(uchar)(VID/256) &&
					m_buffer[0x422]==(uchar) CID)
					bMyCell=true;
				else
					bMyCell=false;
			}
			
			if (bMyCell)
			{
				nSector++;
				iRet=ProcessPack(true);
			}
		}
	} // For readpacks
	if (in!=NULL) fclose (in);
	in=NULL;
	
	CloseAndNull();

	nFrames=0;

	if (m_bCheckCellt && m_bInProcess==true)
	{
		csAux=m_csOutputPath+ '\\' + "Celltimes.txt";
		fout=fopen(csAux,"w");
		nFrames=DurationInFrames(m_AADT_Cell_list[nCell].dwDuration);
		if (m_bCheckEndTime ) 
			fprintf(fout,"%d\n",nFrames);
		fclose(fout);
	}

	m_nTotalFrames=nFrames;

	if (m_bCheckLog && m_bInProcess==true) OutputLog(nCell, 1, TITLES);

	return iRet;
}

int CPgcDemuxApp::CIDMDemux(int nCell, CWnd* pDlg)
{
	int nTotalSectors;
	int nSector;
	int CID,VID;
	__int64 i64IniSec,i64EndSec;
	CString csAux,csAux2;
	FILE *in, *fout;
	__int64 i64;
	bool bMyCell;
	int iRet;
	int nFrames;

	if (nCell >= m_MADT_Cell_list.GetSize())
	{
		MyErrorBox("Error: Selected Cell does not exist");
		m_bInProcess=false;
		return -1;
	}
	
	IniDemuxGlobalVars();
	if (OpenVideoFile()) return -1;
	m_bInProcess=true;

// Calculate  the total number of sectors
	nTotalSectors= m_MADT_Cell_list[nCell].iSize;
	nSector=0;
	iRet=0;

	VID=m_MADT_Cell_list[nCell].VID;
	CID=m_MADT_Cell_list[nCell].CID;

	i64IniSec=m_MADT_Cell_list[nCell].iIniSec;
	i64EndSec=m_MADT_Cell_list[nCell].iEndSec;
	if (m_bVMGM)
	{
		csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-3);
		csAux=csAux2+"VOB";
	}
	else
	{
		csAux2=m_csInputIFO.Left(m_csInputIFO.GetLength()-5);
		csAux=csAux2+"0.VOB";
	}
	in=fopen(csAux,"rb");
	if (in ==NULL)
	{
		MyErrorBox("Error opening input VOB: "+csAux);
		m_bInProcess=false;
		iRet=-1;
	}
	if (m_bInProcess) fseek(in, (long) ((i64IniSec)*2048), SEEK_SET);

	for (i64=0,bMyCell=true;i64< (i64EndSec-i64IniSec+1) && m_bInProcess==true;i64++)
	{
	//readpack
		if ((i64%MODUPDATE) == 0) UpdateProgress(pDlg, (int)((100*nSector)/nTotalSectors) );
		if (readbuffer(m_buffer,in)!=2048)
		{
			if (in!=NULL) fclose (in);
			MyErrorBox("Input error: Reached end of VOB too early");
			m_bInProcess=false;
			iRet=-1;
		}
		if (m_bInProcess==true)
		{
			if (IsSynch(m_buffer) != true)
			{
				MyErrorBox("Error reading input VOB: Unsynchronized");
				m_bInProcess=false;
				iRet=-1;
			}
			if (IsNav(m_buffer))
			{
				if (m_buffer[0x420]==(uchar)(VID%256) &&
					m_buffer[0x41F]==(uchar)(VID/256) &&
					m_buffer[0x422]== (uchar) CID)
					bMyCell=true;
				else
					bMyCell=false;
			}

			if (bMyCell)
			{
				nSector++;
				iRet=ProcessPack(true);
			}
		}
	} // For readpacks
	if (in!=NULL) fclose (in);
	in=NULL;

	CloseAndNull();

	nFrames=0;

	if (m_bCheckCellt && m_bInProcess==true)
	{
		csAux=m_csOutputPath+ '\\' + "Celltimes.txt";
		fout=fopen(csAux,"w");
		nFrames=DurationInFrames(m_MADT_Cell_list[nCell].dwDuration);
		if (m_bCheckEndTime ) 
			fprintf(fout,"%d\n",nFrames);
		fclose(fout);
	}

	m_nTotalFrames=nFrames;

	if (m_bCheckLog && m_bInProcess==true) OutputLog(nCell, 1 , MENUS);

	return iRet;
}



////////////////////////////////////////////////////////////////////////////////
/////////////   Aux Code : Log 
////////////////////////////////////////////////////////////////////////////////
void CPgcDemuxApp::OutputLog(int nItem, int nAng, int iDomain)
{
	CString csFilePath, csAux, csAux1,csAux2;
	int k;
	int AudDelay;

	csFilePath=m_csOutputPath+ '\\' + "LogFile.txt";

	DeleteFile(csFilePath);

	csAux.Format(_T("%d"),m_nPGCs);
	WritePrivateProfileString("General", "Total Number of PGCs   in Titles", csAux, csFilePath);
	csAux.Format(_T("%d"),m_nMPGCs);
	WritePrivateProfileString("General", "Total Number of PGCs   in  Menus", csAux, csFilePath);

	csAux.Format(_T("%d"),m_AADT_Vid_list.GetSize());
	WritePrivateProfileString("General", "Total Number of VobIDs in Titles", csAux, csFilePath);
	csAux.Format(_T("%d"),m_MADT_Vid_list.GetSize());
	WritePrivateProfileString("General", "Total Number of VobIDs in  Menus", csAux, csFilePath);

	csAux.Format(_T("%d"),m_AADT_Cell_list.GetSize());
	WritePrivateProfileString("General", "Total Number of Cells  in Titles", csAux, csFilePath);
	csAux.Format(_T("%d"),m_MADT_Cell_list.GetSize());
	WritePrivateProfileString("General", "Total Number of Cells  in  Menus", csAux, csFilePath);

	if (m_iMode==PGCMODE) csAux="by PGC";
	else if (m_iMode==VIDMODE) csAux="by VOB Id";
	else if (m_iMode==CIDMODE) csAux="Single Cell";
	WritePrivateProfileString("General", "Demuxing   Mode", csAux, csFilePath);

	if (iDomain==TITLES) csAux="Titles";
	else  csAux="Menus";
	WritePrivateProfileString("General", "Demuxing Domain", csAux, csFilePath);

	csAux.Format(_T("%d"),m_nTotalFrames);
	WritePrivateProfileString("General", "Total Number of Frames", csAux, csFilePath);

	if (m_iMode==PGCMODE)
	{
		csAux.Format(_T("%d"),nItem+1);
		WritePrivateProfileString("General", "Selected PGC", csAux, csFilePath);

		if (iDomain==TITLES)
			csAux.Format(_T("%d"),m_nCells[nItem]);
		else
			csAux.Format(_T("%d"),m_nMCells[nItem]);
	
		WritePrivateProfileString("General", "Number of Cells in Selected PGC", csAux, csFilePath);
		WritePrivateProfileString("General", "Selected VOBID", "None", csFilePath);
		WritePrivateProfileString("General", "Number of Cells in Selected VOB", "None", csFilePath);

	}
	if (m_iMode==VIDMODE)
	{
		if (iDomain==TITLES)
			csAux.Format(_T("%d"),m_AADT_Vid_list[nItem].VID);
		else
			csAux.Format(_T("%d"),m_MADT_Vid_list[nItem].VID);
	
		WritePrivateProfileString("General", "Selected VOBID", csAux, csFilePath);

		if (iDomain==TITLES)
			csAux.Format(_T("%d"),m_AADT_Vid_list[nItem].nCells);
		else
			csAux.Format(_T("%d"),m_MADT_Vid_list[nItem].nCells);
	
		WritePrivateProfileString("General", "Number of Cells in Selected VOB", csAux, csFilePath);
		WritePrivateProfileString("General", "Selected PGC", "None", csFilePath);
		WritePrivateProfileString("General", "Number of Cells in Selected PGC", "None", csFilePath);
	}
	if (m_iMode==CIDMODE)
	{
		WritePrivateProfileString("General", "Selected VOBID", "None", csFilePath);
		WritePrivateProfileString("General", "Number of Cells in Selected VOB", "None", csFilePath);
		WritePrivateProfileString("General", "Selected PGC", "None", csFilePath);
		WritePrivateProfileString("General", "Number of Cells in Selected PGC", "None", csFilePath);
	}

	csAux.Format(_T("%d"),m_nVidPacks);
	WritePrivateProfileString("Demux", "Number of Video Packs", csAux, csFilePath);
	csAux.Format(_T("%d"),m_nAudPacks);
	WritePrivateProfileString("Demux", "Number of Audio Packs", csAux, csFilePath);
	csAux.Format(_T("%d"),m_nSubPacks);
	WritePrivateProfileString("Demux", "Number of Subs  Packs", csAux, csFilePath);
	csAux.Format(_T("%d"),m_nNavPacks);
	WritePrivateProfileString("Demux", "Number of Nav   Packs", csAux, csFilePath);
	csAux.Format(_T("%d"),m_nPadPacks);
	WritePrivateProfileString("Demux", "Number of Pad   Packs", csAux, csFilePath);
	csAux.Format(_T("%d"),m_nUnkPacks);
	WritePrivateProfileString("Demux", "Number of Unkn  Packs", csAux, csFilePath);

	for (k=0;k<8;k++)
	{
		csAux.Format(_T("Audio_%d"),k+1);
		if (m_iFirstAudPTS[k])
			csAux1.Format(_T("0x%02X"),m_iAudIndex[k]);
		else
			csAux1="None";
		WritePrivateProfileString("Audio Streams",csAux, csAux1, csFilePath);
		if (m_iFirstAudPTS[k])
		{
//			AudDelay=m_iFirstAudPTS[k]-m_iFirstVidPTS;
			AudDelay=m_iFirstAudPTS[k]-m_iFirstNavPTS0;
			
			if (AudDelay <0)
				AudDelay-=44;
			else
				AudDelay+=44;
			AudDelay/=90;
			csAux2.Format(_T("%d"),AudDelay);
			WritePrivateProfileString("Audio Delays",csAux, csAux2, csFilePath);
		}
	}
	for (k=0;k<32;k++)
	{
		csAux.Format(_T("Subs_%02d"),k+1);
		if (m_iFirstSubPTS[k])
			csAux1.Format(_T("0x%02X"),k+0x20);
		else
			csAux1="None";
		WritePrivateProfileString("Subs Streams",csAux, csAux1, csFilePath);
	}
}

void CPgcDemuxApp::WritePack(uchar* buffer)
{
	CString csAux;

	if (m_bInProcess==true)
	{
		if (m_bCheckVob2)
		{
			if  (fvob==NULL || m_nVidout != m_nCurrVid )
			{
				m_nCurrVid=m_nVidout;
				if (fvob != NULL) fclose(fvob);
				if (m_iDomain==TITLES)
					csAux.Format(_T("VTS_01_1_%03d.VOB"), m_nVidout);
				else
					csAux.Format(_T("VTS_01_0_%03d.VOB"), m_nVidout);
				csAux=m_csOutputPath+ '\\' +csAux;
				fvob=fopen(csAux,"wb");
			}
		}
		else
		{
			if  (fvob==NULL || ((m_i64OutputLBA)%(512*1024-1))==0 )
			{
				if (fvob != NULL) fclose(fvob);
				if (m_iDomain==TITLES)
				{
					m_nVobout++;
					csAux.Format(_T("VTS_01_%d.VOB"), m_nVobout);
				}
				else
					csAux="VTS_01_0.VOB";

				csAux=m_csOutputPath+ '\\' +csAux;
				fvob=fopen(csAux,"wb");
			}
		}

		if (fvob !=NULL) writebuffer(buffer,fvob, 2048);
		m_i64OutputLBA++;
	}

}


void  CPgcDemuxApp::CloseAndNull()
{
	int i;
	unsigned int byterate, nblockalign;
	struct  _stati64 statbuf;
	__int64 i64size;


	if (fvob !=NULL)
	{
		fclose(fvob);
		fvob=NULL;
	}
	if (fvid !=NULL)
	{
		fclose(fvid);
		fvid=NULL;
	}
	for (i=0;i<32;i++)
		if (fsub[i]!=NULL)
		{
			fclose(fsub[i]);
			fsub[i]=NULL;
		}
	for (i=0;i<8;i++)
		if (faud[i]!=NULL)
		{
			if (m_audfmt[i]==WAV)
			{
				i64size=0;
				fclose(faud[i]);
	
				if (_stati64 ( m_csAudname[i], &statbuf)==0)
					i64size= statbuf.st_size;

				if (i64size >= 8) i64size-=8;

				faud[i]=fopen(m_csAudname[i],"r+b");

				fseek(faud[i],4,SEEK_SET);
				fputc ((uchar)(i64size%256),faud[i]);
				fputc ((uchar)((i64size>>8) %256),faud[i]);
				fputc ((uchar)((i64size>>16)%256),faud[i]);
				fputc ((uchar)((i64size>>24)%256),faud[i]);

//				# of channels (2 bytes!!)
				fseek(faud[i],22,SEEK_SET);
				fputc ((uchar)(nchannels[i]%256),faud[i]);

//				Sample rate ( 48k / 96k in DVD)
				fseek(faud[i],24,SEEK_SET);
				fputc ((uchar)(fsample[i]%256),faud[i]);
				fputc ((uchar)((fsample[i]>>8) %256),faud[i]);
				fputc ((uchar)((fsample[i]>>16)%256),faud[i]);
				fputc ((uchar)((fsample[i]>>24)%256),faud[i]);

//				Byte rate ( 4 bytes)== SampleRate * NumChannels * BitsPerSample/8
//                    6000* NumChannels * BitsPerSample
				byterate=(fsample[i]/8)*nchannels[i]*nbitspersample[i];
				fseek(faud[i],28,SEEK_SET);
				fputc ((uchar)(byterate%256),faud[i]);
				fputc ((uchar)((byterate>>8) %256),faud[i]);
				fputc ((uchar)((byterate>>16)%256),faud[i]);
				fputc ((uchar)((byterate>>24)%256),faud[i]);


//				Block align ( 2 bytes)== NumChannels * BitsPerSample/8
				nblockalign=nbitspersample[i]*nchannels[i]/8;
				fseek(faud[i],32,SEEK_SET);
				fputc ((uchar)(nblockalign%256),faud[i]);
				fputc ((uchar)((nblockalign>>8) %256),faud[i]);

//				Bits per sample ( 2 bytes)
				fseek(faud[i],34,SEEK_SET);
				fputc ((uchar)(nbitspersample[i]%256),faud[i]);

				if (i64size >= 36) i64size-=36;
				fseek(faud[i],40,SEEK_SET);
//				fseek(faud[i],54,SEEK_SET);
				fputc ((uchar)(i64size%256),faud[i]);
				fputc ((uchar)((i64size>>8) %256),faud[i]);
				fputc ((uchar)((i64size>>16)%256),faud[i]);
				fputc ((uchar)((i64size>>24)%256),faud[i]);
			}
			fclose(faud[i]);
			faud[i]=NULL;
		}

}

int CPgcDemuxApp::check_sub_open (uchar i)
{
	CString csAux;

	i-=0x20;

	if ( i> 31) return -1;
	
	if (fsub[i]==NULL)
	{
		csAux.Format(_T("Subpictures_%02X.sup"),i+0x20);
		csAux=m_csOutputPath+ '\\' +csAux;
        if ((fsub[i]=fopen(csAux,"wb"))==NULL)
		{
			MyErrorBox("Error opening output subs file:" + csAux);
			m_bInProcess=false;
			return 1;
		}
		else return 0;
	}
	else
	  return 0;
}

int CPgcDemuxApp::check_aud_open (uchar i)
{
	CString csAux;
	uchar ii;
/*
0x80-0x87: ac3  --> ac3
0x88-0x8f: dts  --> dts
0x90-0x97: sdds --> dds
0x98-0x9f: unknown
0xa0-0xa7: lpcm  -->wav
0xa8-0xaf: unknown
0xb0-0xbf: unknown
0xc0-0xc8: mpeg1 --> mpa
0xc8-0xcf: unknown 
0xd0-0xd7: mpeg2 --> mpb
0xd8-0xdf: unknown 
---------------------------------------------
SDSS   AC3   DTS   LPCM   MPEG-1   MPEG-2

 90    80    88     A0     C0       D0
 91    81    89     A1     C1       D1
 92    82    8A     A2     C2       D2
 93    83    8B     A3     C3       D3
 94    84    8C     A4     C4       D4
 95    85    8D     A5     C5       D5
 96    86    8E     A6     C6       D6
 97    87    8F     A7     C7       D7
---------------------------------------------
*/
	
	ii=i;

	if (ii <0x80) return -1;

	i=i&0x7;
	
	if (faud[i]==NULL)
	{
		if (ii >= 0x80 &&  ii <= 0x87)
		{
			csAux.Format(_T("AudioFile_%02X.ac3"),i+0x80);
			m_audfmt[i]=AC3;
		}
		else if (ii >= 0x88 &&  ii <= 0x8f )
		{
			csAux.Format(_T("AudioFile_%02X.dts"),i+0x88);
			m_audfmt[i]=DTS;
		}
		else if (ii >= 0x90 &&  ii <= 0x97)
		{
			csAux.Format(_T("AudioFile_%02X.dds"),i+0x90);
			m_audfmt[i]=DDS;
		}
		else if (ii >= 0xa0 &&  ii <= 0xa7)
		{
			csAux.Format(_T("AudioFile_%02X.wav"),i+0xa0);
			m_audfmt[i]=WAV;
		}
		else if (ii >= 0xc0 &&  ii <= 0xc7)
		{
			csAux.Format(_T("AudioFile_%02X.mpa"),i+0xc0);
			m_audfmt[i]=MP1;
		}
		else if (ii >= 0xd0 &&  ii <= 0xd7)
		{
			csAux.Format(_T("AudioFile_%02X.mpa"),i+0xd0);
			m_audfmt[i]=MP2;
		}
		else 
		{
			csAux.Format(_T("AudioFile_%02X.unk"),ii);
			m_audfmt[i]=UNK;
		}

		csAux=m_csOutputPath+ '\\' +csAux;
		m_csAudname[i]=csAux;

        if ((faud[i]=fopen(csAux,"wb"))==NULL)
		{
			MyErrorBox("Error opening output audio file:" + csAux);
			m_bInProcess=false;
			return 1;
		}

		if (m_audfmt[i]==WAV)
		{
			fwrite(pcmheader,sizeof(uchar),44,faud[i]);
		}

		return 0;
	}
	else
	  return 0;
}



void  CPgcDemuxApp::demuxvideo(uchar* buffer)
{

	int start,nbytes;

	start=0x17+buffer[0x16];
	nbytes=buffer[0x12]*256+buffer[0x13]+0x14;

	writebuffer(&buffer[start],fvid,nbytes-start);

}

void  CPgcDemuxApp::demuxaudio(uchar* buffer, int nBytesOffset)
{
	int start,nbytes,i,j;
	int nbit,ncha;
	uchar streamID;
	uchar mybuffer[2050];

	start=0x17+buffer[0x16];
	nbytes=buffer[0x12]*256+buffer[0x13]+0x14;
	if (IsAudMpeg(buffer))
		streamID=buffer[0x11];
	else
	{
		streamID=buffer[start];
		start+=4;
	}

// Open File descriptor if it isn't open
	if (check_aud_open(streamID)==1)
	    return;

// Check if PCM
	if ( streamID>=0xa0 && streamID<= 0xa7 ) 
	{
		start +=3;

		if (nchannels[streamID & 0x7] == -1)
			nchannels[streamID & 0x7]=(buffer[0x17+buffer[0x16]+5] & 0x7) +1;

		nbit=(buffer[0x17+buffer[0x16]+5] >> 6) & 0x3;

		if (nbit==0) nbit=16;
		else if (nbit==1) nbit=20;
		else if (nbit==2) nbit=24;
		else nbit=0;

		if (nbitspersample[streamID & 0x7] ==-1)
			nbitspersample[streamID & 0x7]=nbit;
		if (nbitspersample[streamID & 0x7]!=nbit)
			nbit=nbitspersample[streamID & 0x7];

		if (fsample[streamID & 0x7] ==-1)
		{
			fsample[streamID & 0x7]=(buffer[0x17+buffer[0x16]+5] >> 4) & 0x3;
			if  (fsample[streamID & 0x7]==0 ) fsample[streamID & 0x7]=48000;
			else  fsample[streamID & 0x7]=96000;
		}

		ncha=nchannels[streamID & 0x7];
		if (nbit==24)
		{
			for (j=start; j< (nbytes-6*ncha+1) ; j+=(6*ncha))
			{
				for (i=0; i<2*ncha; i++)
				{
					mybuffer[j+3*i+2]=buffer[j+2*i];
					mybuffer[j+3*i+1]=buffer[j+2*i+1];
					mybuffer[j+3*i]=  buffer[j+4*ncha+i];
				}
			}

		}
		else if ( nbit==16 )
		{
			for (i=start; i< (nbytes-1) ; i+=2)
			{
				mybuffer[i]=buffer[i+1];
				mybuffer[i+1]=buffer[i];
			}
		}
		else if (nbit==20)
		{
			for (j=start; j< (nbytes-5*ncha+1) ; j+=(5*ncha))
			{
				for (i=0; i<ncha; i++)
				{
					mybuffer[j+5*i+0] = (hi_nib(buffer[j+4*ncha+i])<<4) + hi_nib(buffer[j+4*i+1]);
					mybuffer[j+5*i+1] = (lo_nib(buffer[j+4*i+1])<<4) + hi_nib(buffer[j+4*i+0]);
					mybuffer[j+5*i+2] = (lo_nib(buffer[j+4*i+0])<<4) + lo_nib(buffer[j+4*ncha+i]);
					mybuffer[j+5*i+3] = buffer[j+4*i+3];
					mybuffer[j+5*i+4] = buffer[j+4*i+2];
				}
			}
		}

		if ((nbit==16 && ((nbytes-start)%2)) ||
			(nbit==24 && ((nbytes-start)%(6*ncha))) ||
			(nbit==20 && ((nbytes-start)%(5*ncha))) ) 

			AfxMessageBox( "Error: Uncompleted PCM sample", MB_OK | MB_ICONEXCLAMATION,0 );

// if PCM do not take into account nBytesOffset
		writebuffer(&mybuffer[start],faud[streamID & 0x7],nbytes-start);
	}
	else
	{
// Very easy, no process at all, but take into account nBytesOffset...
		start+=nBytesOffset;
		writebuffer(&buffer[start],faud[streamID & 0x7],nbytes-start);

	}

}


void  CPgcDemuxApp::demuxsubs(uchar* buffer)
{
	int start,nbytes;
	uchar streamID;
	int k;
	uchar mybuff[10];
	int iPTS;

	start=0x17+buffer[0x16];
	nbytes=buffer[0x12]*256+buffer[0x13]+0x14;
	streamID=buffer[start];

	if (check_sub_open(streamID)==1)
	    return;
	if ((buffer[0x16]==0) || (m_buffer[0x15] & 0x80) != 0x80) 
		writebuffer(&buffer[start+1],fsub[streamID & 0x1F],nbytes-start-1);
	else
	{
     // fill 10 characters
	    for (k=0; k<10;k++)
			mybuff[k]=0;

		iPTS=m_iSubPTS-m_iFirstNavPTS0+m_iOffsetPTS;

		mybuff[0]=0x53;
		mybuff[1]=0x50;
		mybuff[2]=iPTS%256;
		mybuff[3]=(iPTS >> 8)%256;
		mybuff[4]=(iPTS >> 16)%256;
		mybuff[5]=(iPTS >> 24)%256;

		writebuffer(mybuff,fsub[streamID & 0x1F],10);
		writebuffer(&buffer[start+1],fsub[streamID & 0x1F],nbytes-start-1);
	}
}