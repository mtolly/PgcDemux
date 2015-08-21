; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CPgcDemuxDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "PgcDemux.h"

ClassCount=4
Class1=CPgcDemuxApp
Class2=CPgcDemuxDlg
Class3=CAboutDlg

ResourceCount=6
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_PGCDEMUX_DIALOG
Resource4=IDD_ABOUTBOX (English (U.S.))
Resource5=IDD_PGCDEMUX_DIALOG (English (U.S.))
Class4=CCustomVob
Resource6=IDD_CUSTOMVOB (English (U.S.))

[CLS:CPgcDemuxApp]
Type=0
HeaderFile=PgcDemux.h
ImplementationFile=PgcDemux.cpp
Filter=N

[CLS:CPgcDemuxDlg]
Type=0
HeaderFile=PgcDemuxDlg.h
ImplementationFile=PgcDemuxDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDCANCEL2

[CLS:CAboutDlg]
Type=0
HeaderFile=PgcDemuxDlg.h
ImplementationFile=PgcDemuxDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC

[DLG:IDD_ABOUTBOX]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Class=CAboutDlg


[DLG:IDD_PGCDEMUX_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CPgcDemuxDlg

[DLG:IDD_PGCDEMUX_DIALOG (English (U.S.))]
Type=1
Class=CPgcDemuxDlg
ControlCount=34
Control1=IDC_STATIC,button,1342308359
Control2=IDC_EDITINPUT,edit,1350633600
Control3=IDC_BUTTONINPUT,button,1342242816
Control4=IDC_STATIC,button,1342177287
Control5=IDC_EDITOUTPUT,edit,1350633600
Control6=IDC_BUTTONOUTPUT,button,1342242816
Control7=IDC_STATIC,button,1342308359
Control8=IDC_RADIOPGC,button,1342373897
Control9=IDC_RADIOVOB,button,1342177289
Control10=IDC_RADIOCELL,button,1342177289
Control11=IDC_STATIC,button,1342308359
Control12=IDC_CHECKVID,button,1342373891
Control13=IDC_CHECKAUD,button,1342242819
Control14=IDC_CHECKSUB,button,1342242819
Control15=IDC_CHECKCELLT,button,1342242819
Control16=IDC_CHECKLOG,button,1342242819
Control17=IDC_CHECKVOB,button,1342242819
Control18=IDC_CHECKVOB2,button,1342242819
Control19=IDC_STATICSEL,button,1342308359
Control20=IDC_STATIC,button,1342177287
Control21=IDC_RADIOM,button,1342373897
Control22=IDC_RADIOT,button,1342242825
Control23=IDC_COMBOPGC,combobox,1344471043
Control24=IDC_STATICCELL,static,1342308866
Control25=IDC_EDITCELL,edit,1350641664
Control26=IDC_STATICANG,static,1342308864
Control27=IDC_COMBOANG,combobox,1344339971
Control28=IDC_PROGRESS,msctls_progress32,1350565888
Control29=IDOK,button,1342373889
Control30=IDABORT,button,1342242817
Control31=IDCANCEL2,button,1342242816
Control32=IDC_BUTTONCUSTOM,button,1342242816
Control33=IDC_BUTTONDELAY,button,1342242816
Control34=IDC_CHECKENDTIME,button,1342242819

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATICVER,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308480

[DLG:IDD_CUSTOMVOB (English (U.S.))]
Type=1
Class=CCustomVob
ControlCount=9
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDDEFAULT,button,1342242817
Control4=IDC_CHECKVIDEOPACK,button,1342242819
Control5=IDC_CHECKIFRAME,button,1342242819
Control6=IDC_CHECKAUDIOPACK,button,1342242819
Control7=IDC_CHECKNAVPACK,button,1342242819
Control8=IDC_CHECKSUBPACK,button,1342242819
Control9=IDC_CHECKLBA,button,1342242819

[CLS:CCustomVob]
Type=0
HeaderFile=CustomVob.h
ImplementationFile=CustomVob.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_CHECKLBA

