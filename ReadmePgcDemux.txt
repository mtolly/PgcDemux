========================Terms=====================================
    PgcDemux app. Streams demuxer.  Copyright (C) <2005>

    PgcDemux is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    PgcDemux is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

http://www.videohelp.com/~jsoto
http://jsoto.posunplugged.com/

Contact me at doom9 or
email: jesus_soto_viso(at)terra(dot)es

===============================Changelog==============================
Vers 1.2.0.5 (28-11-2005)
- BugFix:   Demuxing by CellId in CLI mode does not always work 

Vers 1.2.0.4 (19-04-2005)
- BugFix:   Demuxing audio was completely broken in 1.2.0.3 (Sorry :) )
- Added:    Confirmation dialog when quiting, except in the case of using Quit button 

Vers 1.2.0.3 (10-04-2005)
- BugFix:   Demuxed audio now starts in a frame header
- Added:    Option to do not include last celltime 
- Changed:  Special VOB contents requested by Zeul. Only the first I frame per cell

Vers 1.2.0.2 (08-03-2005)
- BugFix:   Crash opening About dialog
- BugFix:   Unreferenced material. Duration of unreferenced cells was not 
            initialized. Currently computed as zero (not true, but because
            VOB is not opened in this moment there is no way to get this info).	
- BugFix:   It was allowed to check a/v delay in PGCs without cells
- BugFix:   A/V delay failed if the first encoded frame is not temporal sequence number 0, 
            that is when vobu_s_ptm  is different from first video pts value. Thanks to
            mpucoder for the clarification.
- BugFix:   Wrong audio index in logfile if mpeg audio.


Vers 1.2.0.1 (27-02-2005)
- Added:    Demuxing by VOBid and CellId
- Added:    Button to check audio/video delay
- Added:    Percentaje of completion in title bar
- Added:    Change VOB File name between VOBids
- Added:    Customizable VOB file contents
- Added:    Special VOB contents requested by Zeul
- Added:    Number of VIDs in Log
- Added:    Warning if files already exist
- Change:   Button label OK to Process
- Change:   CLI snytax has been modified to support new demux modes 
- BugFix:   CLI was completely broken.

Vers 1.1.0.7 (26-01-2005)
- Added:    Minimize box in title bar
- BugFix:   PGC demuxing in menu domain failed if # language units > 1
 
Vers 1.1.0.6 (07-01-2005)
- Added:    20 bit LPCM full support
- BugFix:   Wrong RIFF chunk size (8 bytes lower than the right value) 

Vers 1.1.0.5 (21-12-2004)
- Added:    24 bit LPCM full support 
- Change:    Unlimited PGCs
- Note:     20 bits still not supported

Vers 1.1.0.4 (19-12-2004)
- Added:    24 bit LPCM support (1, 2 and 4 channels tracks)
- Added:    96 kHz sampling frequency LPCM support
- Note:     20 bits is not supported

Vers 1.1.0.3 (08-11-2004)
- Added:    Shell integration: "Open with" and "drag & drop"
- Change:   Change of file extensions to wav (pcm) and mpa (mpeg)
- BugFix:   Error when loading IFOs without menus.
- BugFix:   Padding packs were not processed.
- BugFix:   LPCM audios extraction were not correct. 
- BugFix:   MPEG audios extraction were not correct.

Note: I'm still unsure on LPCM extraction, mainly because I do not own
DVDs with LPCM audios.


Vers 1.1.0.2 (02-11-2004)
- New build with no changes but with version information OK
 
Vers 1.1.0.1 (01-11-2004)
- Added:    Multiangle support
- Added:    Menu support
- Added:    Logfile creation (including audio/video delays)
- Added:    Celltimes.txt creation
- Added:    Tooltips
- BugFix:   Bug in subpictures timestamp calculation (in second and
            sucesive discontinuities)
- BugFix:   Bug in subpictures extraction when buffer[0x16]!=0


Vers 1.0.0.2 (25-10-2004)
- BugFix:   Bug in VOB generation.
- Changed:  Extension of videofile to m2v
- Added:    MessageBox confirmation at the end of the process.


Vers 1.0.0.1 (24-10-2004)
- First public release


=================================================================
Command line support
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
option10:{-cellt, -nocellt}. Generates a Celltimes.txt file. Only in PGC/VID mode. Default YES
option11:{-log, -nolog}. Generates a log file. Default YES
option12:{-menu, -title}. Domain. Default Title (except if filename is VIDEO_TS.IFO)

