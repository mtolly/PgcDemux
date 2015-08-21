#include "stdafx.h"

typedef unsigned char uchar;
//#include "Languages.h"

#define PAL 1
#define NTSC 0
/*
CString lang_long(char * szLang)
{
	int j;
	CString csAux;

	for (j=0,csAux="Unknown";LangTable[j].descr[0]!=0 && csAux=="Unknown";j++)
	{
		if (szLang[0]==LangTable[j].descr[0] &&
			szLang[1]==LangTable[j].descr[1])
			 csAux=  LangTable[j].lang_long;
	}

	return csAux;
}

*/

void MyErrorBox(LPCTSTR text)
{
	AfxMessageBox(text,MB_OK|MB_ICONEXCLAMATION,0);
}

void MyInfoBox(LPCTSTR text)
{
	AfxMessageBox(text,MB_OK|MB_ICONINFORMATION,0);
}

int readpts(uchar *buf)
{
    int a1,a2,a3;
	int pts;

    a1=(buf[0]&0xe)>>1;
    a2=((buf[1]<<8)|buf[2])>>1;
    a3=((buf[3]<<8)|buf[4])>>1;
    pts= (int) ((( (__int64) a1)<<30) | (a2<<15) | a3);
    return pts;
}


/*
void writepts(uchar *buf,ulong pts)
{
    buf[0]=((pts>>29)&0xe)|(buf[0]&0xf1); // this preserves the PTS / DTS / PTSwDTS top bits
    write2(buf+1,(pts>>14)|1);
    write2(buf+3,(pts<<1)|1);
}
*/

CString Size_MBytes(__int64 i64size)
{
	CString csAux;

	if (i64size < 9999)
		csAux.Format(_T("%I64d By"),i64size);
	else if (i64size < 9999*1024)
		csAux.Format(_T("%I64d KB"),i64size/1024);
	else if (i64size < (__int64)9999*1024*1024)
		csAux.Format(_T("%I64d MB"),i64size/1024/1024);
	else
		csAux.Format(_T("%I64d MB"),i64size/1024/1024/1024);
	
	return csAux;


}

CString FormatDuration(DWORD duration)
{
	CString csAux;

	if (duration<0)
		csAux="Unknown";
	else
		csAux.Format(_T("%02X:%02X:%02X.%02X"),
			duration/(256*256*256),(duration/(256*256))%256,(duration/256)%256,(duration%256) & 0x3f);
	return csAux;
}

int GetNbytes(int nNumber,uchar* address)
{
	int ret,i;

	for (i=ret=0;i<nNumber;i++)
		ret=ret*256+address[i];
	return ret;
}

void Put4bytes(__int64 i64Number,uchar* address)
{

	address[0]= (uchar) (i64Number/(256*256*256));
	address[1]= (uchar) ((i64Number/(256*256))%256);
	address[2]= (uchar) ((i64Number/256)%256);
	address[3]= (uchar) (i64Number%256);

}

void Put2bytes(__int64 i64Number,uchar* address)
{

	address[0]= (uchar) ((i64Number/256)%256);
	address[1]= (uchar) (i64Number%256);

}

int BCD2Dec ( int BCD)
{
	int ret;
	ret = (BCD/0x10)*10 + (BCD%0x10);
	return ret;
}

int Dec2BCD ( int Dec)
{
	int ret;
	ret = (Dec/10)*0x10 + (Dec%10);
	return ret;
}

int DurationInFrames(DWORD dwDuration)
{

	int ifps,ret;
	__int64 i64Dur;

	if ( ((dwDuration%256) & 0x0c0) == 0x0c0 )
		ifps=30;
	else  
		ifps=25;
	
	i64Dur=  (BCD2Dec( (dwDuration%256) & 0x3f));
	i64Dur+= (BCD2Dec( (dwDuration/256)%256 )*ifps);
	i64Dur+= (BCD2Dec( (dwDuration/(256*256))%256 )*ifps*60);
	i64Dur+= (BCD2Dec(  dwDuration/(256*256*256)  )*ifps*60*60);

	ret=(int)(i64Dur);

	return ret;
}

int DurationInSecs(DWORD dwDuration)
{

	int ifps,ret;
	__int64 i64Dur;

	if ( ((dwDuration%256) & 0x0c0) == 0x0c0 )
		ifps=30;
	else  
		ifps=25;
	
	i64Dur=  (BCD2Dec( (dwDuration%256) & 0x3f));
	i64Dur+= (BCD2Dec( (dwDuration/256)%256 )*ifps);
	i64Dur+= (BCD2Dec( (dwDuration/(256*256))%256 )*ifps*60);
	i64Dur+= (BCD2Dec(  dwDuration/(256*256*256)  )*ifps*60*60);

	ret=(int)(i64Dur/ifps);

	return ret;
}

DWORD AddDuration(DWORD dwDuration1, DWORD dwDuration2)
{
	DWORD ret;
	int ifps,hh,mm,ss,ff;
	__int64 i64Dur1, i64Dur2, i64DurT;

	if ( ((dwDuration1%256) & 0x0c0) == 0x0c0 )
		ifps=30;
	else  
		ifps=25;
	
	i64Dur1=  BCD2Dec( (dwDuration1%256) & 0x3f);
	i64Dur1+= BCD2Dec( (dwDuration1/256)%256 )*ifps;
	i64Dur1+= BCD2Dec( (dwDuration1/(256*256))%256 )*ifps*60;
	i64Dur1+= BCD2Dec( dwDuration1/(256*256*256) )*ifps*60*60;

	i64Dur2=  BCD2Dec( (dwDuration2%256) & 0x3f);
	i64Dur2+= BCD2Dec( (dwDuration2/256)%256 )*ifps;
	i64Dur2+= BCD2Dec( (dwDuration2/(256*256))%256 )*ifps*60;
	i64Dur2+= BCD2Dec( dwDuration2/(256*256*256) )*ifps*60*60;

	i64DurT=i64Dur2+i64Dur1;

	ff=Dec2BCD( (int)(i64DurT%ifps)     );
	ss=Dec2BCD( (int)((i64DurT/ifps)%60)   );
	mm=Dec2BCD( (int)((i64DurT/ifps/60)%60) );
	hh=Dec2BCD( (int)(i64DurT/ifps/60/60)  );

	ret=ff + ss*256 + mm*256*256 + hh*256*256*256;

	if (ifps==30)
		ret+= 0x0c0;
	else
		ret+= 0x040;

	return ret;
}

DWORD SubDuration(DWORD dwDuration1, DWORD dwDuration2)
{
	DWORD ret;
	int ifps,hh,mm,ss,ff;
	__int64 i64Dur1, i64Dur2, i64DurT;

	if ( ((dwDuration1%256) & 0x0c0) == 0x0c0 )
		ifps=30;
	else  
		ifps=25;
	
	i64Dur1=  BCD2Dec( (dwDuration1%256) & 0x3f);
	i64Dur1+= BCD2Dec( (dwDuration1/256)%256 )*ifps;
	i64Dur1+= BCD2Dec( (dwDuration1/(256*256))%256 )*ifps*60;
	i64Dur1+= BCD2Dec( dwDuration1/(256*256*256) )*ifps*60*60;

	i64Dur2=  BCD2Dec( (dwDuration2%256) & 0x3f);
	i64Dur2+= BCD2Dec( (dwDuration2/256)%256 )*ifps;
	i64Dur2+= BCD2Dec( (dwDuration2/(256*256))%256 )*ifps*60;
	i64Dur2+= BCD2Dec( dwDuration2/(256*256*256) )*ifps*60*60;

	i64DurT=i64Dur1-i64Dur2;

	ff=Dec2BCD( (int)(i64DurT%ifps)     );
	ss=Dec2BCD( (int)((i64DurT/ifps)%60)   );
	mm=Dec2BCD( (int)((i64DurT/ifps/60)%60) );
	hh=Dec2BCD( (int)(i64DurT/ifps/60/60)  );

	ret=ff + ss*256 + mm*256*256 + hh*256*256*256;

	if (ifps==30)
		ret+= 0x0c0;
	else
		ret+= 0x040;

	return ret;
}

// Sorry, a dirty trick to pass the folder 
CString csGlobalStartFolder;

int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];

         switch(uMsg)
		 {
            case BFFM_INITIALIZED: {
               //if (GetCurrentDirectory(sizeof(szDir)/sizeof(TCHAR), szDir))
				if (csGlobalStartFolder.IsEmpty())
                  strcpy(szDir,"C:\\");
				else
                  strcpy(szDir,csGlobalStartFolder);
			   {
                  // WParam is TRUE since you are passing a path.
                  // It would be FALSE if you were passing a pidl.
                  SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)szDir);
               }
               break;
            }
            case BFFM_SELCHANGED:
			{
               // Set the status window to the currently selected path.
               if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
			   {
                  SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
               }
             break;
            }
            default:
             break;
         }
         return 0;
}


CString SelectFolder (CString csTitulo, CString csStartFolder)
{
	BROWSEINFO bi;
    LPITEMIDLIST pidlBrowse;    
	char	buffer[_MAX_PATH];
	char	path[_MAX_PATH];

	csGlobalStartFolder=csStartFolder;
	bi.hwndOwner = AfxGetMainWnd()->m_hWnd;     
	bi.pidlRoot = NULL; 
	bi.pszDisplayName = buffer;     
	bi.lpszTitle = (const char*)csTitulo; 
	bi.ulFlags = BIF_EDITBOX|  0x40 | BIF_RETURNONLYFSDIRS ;
	//BIF_USENEWUI;     
    bi.lpfn = BrowseCallbackProc;
//	bi.lpfn = NULL;     
	bi.lParam = 0;  
	bi.iImage = 0;
	pidlBrowse = SHBrowseForFolder(&bi);     
	if (pidlBrowse != NULL) 
	{ 
		SHGetPathFromIDList(pidlBrowse,path);
		return CString(path);
	}
	return "";
}

int readbuffer(uchar *caracter, FILE *in)
{
	int j;

	if (in == NULL) return -1;
	j =fread (caracter,sizeof (uchar), 2048, in);

	return j;
}


void writebuffer(uchar *caracter, FILE *out, int nbytes)
{

	fwrite (caracter,sizeof(uchar),nbytes,out);

	return;
}

bool IsPad(uchar* buffer)
{

	int startcode;

	startcode=GetNbytes(4, &buffer[14]);

	if (startcode==446) return true;
	else return false;

}

bool IsNav(uchar* buffer)
{

	int startcode;

	startcode=GetNbytes(4, &buffer[14]);

	if (startcode==443) return true;
	else return false;

}

bool IsVideo (uchar* buffer)
{

	int startcode;

	startcode=GetNbytes(4, &buffer[14]);

	if (startcode==480) return true;
	else return false;

}

bool IsGOP (uchar* buffer)
{

	int startcode, startmpeg;

	startmpeg=0x17+buffer[0x16];
	startcode=GetNbytes(4, &buffer[startmpeg]);

	if (startcode==0x1b3)
	{
		startcode=GetNbytes(4, &buffer[startmpeg+0x58]);
		if (startcode==0x1b8)
			return true;
	}
	
	return false;
}

bool IsClosedGOP (uchar* buffer)
{

	int startcode, startmpeg;

	startmpeg=0x17+buffer[0x16];
	startcode=GetNbytes(4, &buffer[startmpeg]);

	if (startcode==0x1b3)
	{
		startcode=GetNbytes(4, &buffer[startmpeg+0x58]);
		if (startcode==0x1b8)
		{
			if (buffer[startmpeg+0x5f] & 0x40) 
				return true;
		}
	}
	return false;
}

bool IsAudio (uchar* buffer)
{
	int startcode,st_i;

	startcode=GetNbytes(4, &buffer[14]);
	st_i=0x17+buffer[0x16];
/*
0x80-0x87: ac3
0x88-0x8f: dts
0x90-0x97: dds
0x98-0x9f: unknown
0xa0-0xa7: lpcm

--------------------------------------------------------------------------------
SDSS   AC3   DTS   LPCM   MPEG-1   MPEG-2

 90    80    88     A0     C0       D0
 91    81    89     A1     C1       D1
 92    82    8A     A2     C2       D2
 93    83    8B     A3     C3       D3
 94    84    8C     A4     C4       D4
 95    85    8D     A5     C5       D5
 96    86    8E     A6     C6       D6
 97    87    8F     A7     C7       D7
--------------------------------------------------------------------------------
*/
	if ((startcode==445 && buffer[st_i] >0x7f && buffer[st_i] < 0x98) || 
	    (startcode==445 && buffer[st_i] >0x9f && buffer[st_i] < 0xa8) || 
		(startcode>=0x1c0 && startcode<=0x1c7)  ||
		(startcode>=0x1d0 && startcode<=0x1d7)  ) 
			return true;
	else return false;

}

bool IsAudMpeg (uchar* buffer)
{

	int startcode;

	startcode=GetNbytes(4, &buffer[14]);

	if ((startcode>=0x1c0 && startcode<=0x1c7) ||
		(startcode>=0x1d0 && startcode<=0x1d7))
		return true;
	else return false;

}

bool IsSubs (uchar* buffer)
{

	int startcode,st_i;

	startcode=GetNbytes(4, &buffer[14]);
	st_i=0x17+buffer[0x16];


	if (startcode==445 && buffer[st_i] >0x1f && buffer[st_i] < 0x40)
			return true;
	else return false;

}


bool IsSynch (uchar *buffer)
{

	int startcode;

	startcode=GetNbytes(4, &buffer[0]);

	if (startcode==0x1BA) return true;
	else return false;

}
int getAudId  (uchar *buffer)
{
	int AudId;


	if (!IsAudio(buffer)) return -1;

	if (IsAudMpeg(buffer))
		AudId=buffer[0x11];
	else
		AudId=buffer[0x17+buffer[0x16]];

	return AudId;
}

int getSubId  (uchar *buffer)
{
	int SubId;

	if (!IsSubs(buffer)) return -1;

	SubId=buffer[0x17+buffer[0x16]];

	return SubId;
}

__int64 MPEGVideoAttr (uchar *buffer)
{
	__int64 ret;


	if (GetNbytes(4,&buffer[0x17+buffer[0x16]]) != 0x000001b3 )
		return 0;

	ret=GetNbytes(4,&buffer[0x17+buffer[0x16]+4]);

	return ret;

}

int MPEGvert(__int64 i64Attr)
{
   return (int) (i64Attr & 0x000FFF00) >>8;
}

int MPEGhoriz(__int64 i64Attr)
{
   return (int) (i64Attr & 0xFFF00000) >> 20;
}

int MPEGDAR(__int64 i64Attr)
{
   return (int) (i64Attr & 0x0F0) >> 4;

   // 0= forbidden
   // 1= 1:1
   // 2= 4:3
   // 3= 16:9
   // 4= 2,21:1
}

void ModifyLBA (uchar* buffer, __int64 m_i64OutputLBA)
{
// 1st lba number
	buffer[0x30]= (uchar)(m_i64OutputLBA%256);
	buffer[0x2F]= (uchar)((m_i64OutputLBA/256)%256);
	buffer[0x2E]= (uchar)((m_i64OutputLBA/256/256)%256);
	buffer[0x2D]= (uchar)(m_i64OutputLBA/256/256/256);

// 2nd lba number
	buffer[0x040E]=buffer[0x30];
	buffer[0x040D]=buffer[0x2F];
	buffer[0x040C]=buffer[0x2E];
	buffer[0x040B]=buffer[0x2D];
}

void ModifyCID (uchar* buffer, int VobId, int CellId)
{
// VobID
	buffer[0x420]=(uchar)(VobId%256);
	buffer[0x41F]=(uchar)(VobId/256);
//CellID
	buffer[0x422]= (uchar) CellId;
}

void CleanILV (uchar* buffer)
{
// Flags
	buffer[0x427]=buffer[0x428]=(uchar)0;
//End Address
	buffer[0x429]=buffer[0x42a]=buffer[0x42b]=buffer[0x42c]=(uchar)0;
//Next start
	buffer[0x42d]=buffer[0x42e]=buffer[0x42f]=buffer[0x430]=(uchar)0;
// Size
	buffer[0x431]=buffer[0x432]=(uchar)0;
}


void ModifyPUOPS (uchar* buffer)
{
	buffer[0x35]=0;
	buffer[0x36]=0;
	buffer[0x37]=0;
	buffer[0x38]=0;
}



unsigned int ac3bitrate[64]=   {32,32,40,40,48,48,56,56,64,64,
        80,80,96,96,112,112,128,128,160,160,192,192,224,224,256,256,
        320,320,384,384,448,448,512,512,576,576,640,640,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64 };

unsigned int dtsbitrate[32]=   {32,56,64,96,112,128,192,224,256,320,384,448,512,576,640,768,
		960,1024,1152,1280,1344,1408,1411,1472,1536,1920,2048,3072,3840,0,0,0};

unsigned long int p_bit;
////////////////////////////////////////////////////////////////////////
//      getbits
////////////////////////////////////////////////////////////////////////
unsigned int getbits (int number, unsigned char *p_frame)
{
	unsigned int bit_ini,byte_ini,bit_end,byte_end, output;

	byte_ini=p_bit/8;
	bit_ini=p_bit%8;

	p_bit+=number;

	byte_end=p_bit/8;
	bit_end=p_bit%8;

	if (byte_end==byte_ini)
		output= p_frame[byte_end];
	else if (byte_end==byte_ini+1)
		output= p_frame[byte_end-1]*256 + p_frame[byte_end];
	else
	    output= p_frame[byte_end-2]*256*256+p_frame[byte_end-1]*256+
		      p_frame[byte_end];
		  
	output=(output)>>(8-bit_end);
	output=output & ((1 << number)-1);
    
	return output;
}

int getac3rate(uchar* buffer)
{
	int first_acc_p,rate;

	p_bit=0;

	first_acc_p=GetNbytes(2,&buffer[0x19+buffer[0x16]]);
	first_acc_p+=(0x1A+buffer[0x16]);

	if (buffer[first_acc_p+0]!= 0x0B || buffer[first_acc_p+1]!=0x77)
		return 0;

	rate=ac3bitrate[buffer[first_acc_p+4] & 0x3F];
	return rate;

}

int getdtsrate( uchar* buffer)
{

	int first_acc_p,rate;
	int unused;

	p_bit=0;

	first_acc_p=GetNbytes(2,&buffer[0x19+buffer[0x16]]);
	first_acc_p+=(0x1A+buffer[0x16]);

	if (buffer[first_acc_p+0]!= 0x7F || buffer[first_acc_p+1]!=0xFE ||
		buffer[first_acc_p+2]!= 0x80 || buffer[first_acc_p+3]!=0x01 )
		return 0;

	unused=   getbits (32, &buffer[first_acc_p]); 
	unused=   getbits (1, &buffer[first_acc_p]);
	unused=   getbits (5, &buffer[first_acc_p]);
	unused=   getbits (1, &buffer[first_acc_p]);
	unused=   getbits (7, &buffer[first_acc_p]);
	unused=   getbits (14,&buffer[first_acc_p]);
	unused=   getbits (6, &buffer[first_acc_p]); 
	unused=   getbits (4, &buffer[first_acc_p]);

	rate=     getbits (5, &buffer[first_acc_p]);
	rate=dtsbitrate[rate];


	return rate;


}


// ATENCION Streams conocidos son:
/*

En el startcode: (14 al 17)
000001E0----------> Video stream
000001E1 al 1EF---> Video stream, illegal in DVDs
000001BE----------> Padding Stream
000001BF----------> Private stream 2--> Nav Pack
000001C0-1C7------> Audio Mpeg1 streams 
000001C8-1CF------> Audio Mpeg2 streams 
000001BD----------> Private stream 1--> Subs & audio (ac3 & dts & lpcm)
Y, en este caso, en el StreamID=buffer[0x17+buffer[0x16]];
  0x20-0x3F--> 32 tracks of subs
  0x80-0x87--> ac3
  0x88-0x8f--> dts 
  0xa0-0xa7-->lpcm


0x80-0x87: ac3
0x88-0x8f: dts
0x90-0x9f: unknown
0xa0-0xa7: lpcm
0xa8-0xaf: unknown
0xb0-0xbf: unknown
0xc0-0xc8: mpeg1
0xc9-0xcf: mpeg2
0xd0-0xdf: unknown
0xe0-0xef: unknown
0xf0-0xff: unknown
*/

