/************************************************************************
*                                                                       *
*   database_error.h --  error code definitions for database module     *
*                                                                       *
*   Copyright (c) Vivotek Corp.  All rights reserved.                   *
*                                                                       *
************************************************************************/

#ifndef _DATABASE_ERROR_
#define _DATABASE_ERROR_

// Severity code 5

//--------------------------------------------------------------------------------
// Database Error Codes
//--------------------------------------------------------------------------------
#define DATABASE_E_FIRST 0x80050000
#define DATABASE_E_LAST  0x8005FFFF
#define DATABASE_S_FIRST 0x00050000
#define DATABASE_S_LAST  0x0005FFFF

//
// MessageId: DATABASE_E_OPENFILE
//
// MessageText:
//
//  The error code of opening file
//
#define DATABASE_E_OPENFILE            0x80050001

//
// MessageId: DATABASE_E_WRITEFILE
//
// MessageText:
//
//  The error code of writing file
//
#define DATABASE_E_WRITEFILE           0x80050002

//
// MessageId: DATABASE_E_READFILE
//
// MessageText:
//
//  The error code of reading file
//
#define DATABASE_E_READFILE            0x80050003

//
// MessageId: DATABASE_E_REMOVEFILE
//
// MessageText:
//
//  The error code of removing file
//
#define DATABASE_E_REMOVEFILE          0x80050004

//
// MessageId: DATABASE_E_RENAMEFILE
//
// MessageText:
//
//  The error code of renaming file
//
#define DATABASE_E_RENAMEFILE          0x80050005

//
// MessageId: DATABASE_E_FILENOTFOUND
//
// MessageText:
//
//  File does not exist.
//
#define DATABASE_E_FILENOTFOUND        0x80050006

//
// MessageId: DATABASE_E_FILEEXIST
//
// MessageText:
//
//  File already exist.
//
#define DATABASE_E_FILEEXIST           0x80050007

//
// MessageId: DATABASE_E_MAKEDIRECTORY
//
// MessageText:
//
//  Make directory failed.
//
#define DATABASE_E_MAKEDIRECTORY       0x80050008

//
// MessageId: DATABASE_E_REMOVEDIR
//
// MessageText:
//
//  The error code of removing directory
//
#define DATABASE_E_REMOVEDIR           0x80050009

//
// MessageId: DATABASE_E_FILEOCCUPIED
//
// MessageText:
//
//  The file cannot be removed and occupied by others
//
#define DATABASE_E_FILEOCCUPIED        0x8005000A

//
// MessageId: DATABASE_E_PARENTPATH
//
// MessageText:
//
//  The parent path does not exist, AP must make the directory first.
//
#define DATABASE_E_PARENTPATH          0x8005000B

//
// MessageId: DATABASE_E_PATHNOTFOUND
//
// MessageText:
//
//  Path does not exist.
//
#define DATABASE_E_PATHNOTFOUND        0x8005000C

//
// MessageId: DATABASE_E_PATHEXIST
//
// MessageText:
//
//  Path already exist.
//
#define DATABASE_E_PATHEXIST           0x8005000D

//
// MessageId: DATABASE_E_BMKFILENOTFOUND
//
// MessageText:
//
//  Bookmark file does not exist.
//
#define DATABASE_E_BMKFILENOTFOUND     0x8005000E

//
// MessageId: DATABASE_E_READFILE
//
// MessageText:
//
//  The string length exceeds regulation.
//
#define DATABASE_E_STRINGTOOLARGE      0x8005000F

//
// MessageId: DATABASE_E_FOLDERNOTEMPTY
//
// MessageText:
//
//  The folder is not empty.
//
#define DATABASE_E_FOLDERNOTEMPTY      0x80050010

//
// MessageId: DATABASE_E_NULLBUFFER
//
// MessageText:
//
//  The Buffer is NULL.
//
#define DATABASE_E_NULLBUFFER          0x80050101

//
// MessageId: DATABASE_E_INVALIDTIMESEGINDEX
//
// MessageText:
//
//  Invalid time segment index
//
#define DATABASE_E_INVALIDTIMESEGINDEX 0x80050102

//
// MessageId: DATABASE_E_NOTSETRETPOSITION
//
// MessageText:
//
//  Must call MatchTime or MatchEventInterval before OutputOneDataPacket
//
#define DATABASE_E_NOTSETRETPOSITION   0x80050201

//
// MessageId: DATABASE_E_NODESCRIPTOR
//
// MessageText:
//
//  Must call GenerateDescriptorFile before calling file-based query and retrieval functions
//
#define DATABASE_E_NODESCRIPTOR        0x80050202

//
// MessageId: DATABASE_E_INVALIDSTREAMTYPE
//
// MessageText:
//
//  Invalid stream type
//
#define DATABASE_E_INVALIDSTREAMTYPE   0x80050301

//
// MessageId: DATABASE_E_INVALIDLOCATIONNUM
//
// MessageText:
//
//  Invalid location number
//
#define DATABASE_E_INVALIDLOCATIONNUM  0x80050302

//
// MessageId: DATABASE_E_INVALIDFILEID
//
// MessageText:
//
//  File ID is invalid.
//
#define DATABASE_E_INVALIDFILEID       0x80050303

//
// MessageId: DATABASE_E_INVALIDDBVERSION
//
// MessageText:
//
//  File format of database is invalid.
//
#define DATABASE_E_INVALIDDBVERSION    0x80050304

//
// MessageId: DATABASE_E_NOTIMATCH
//
// MessageText:
//
//  There is no time interval match the given value.
//
#define DATABASE_E_NOTIMATCH           0x80050305

//
// MessageId: DATABASE_E_NOTEXACTTI
//
// MessageText:
//
//  There are more than one time interval match the given value.
//
#define DATABASE_E_NOTEXACTTI          0x80050306

//
// MessageId: DATABASE_E_INVALIDMEDIATYPE
//
// MessageText:
//
//  The media type is invaild.
//
#define DATABASE_E_INVALIDMEDIATYPE    0x80050307

//
// MessageId: DATABASE_E_NOMEDIAFILE
//
// MessageText:
//
//  There is no media file in the database.
//
#define DATABASE_E_NOMEDIAFILE         0x80050308

//
// MessageId: DATABASE_E_NOIFRAME
//
// MessageText:
//
//  There is no I frame in the interval.
//
#define DATABASE_E_NOIFRAME            0x80050309

//
// MessageId: DATABASE_E_MATCHTIMENOTFOUND
//
// MessageText:
//
//  Can not find match time in given time interval, time segment index, location and database path.
//
#define DATABASE_E_MATCHTIMENOTFOUND   0x8005030A

//
// MessageId: DATABASE_E_INVALIDTIMEINTERVAL
//
// MessageText:
//
//  Can not find intra frame or media in the interval.
//
#define DATABASE_E_INVALIDTIMEINTERVAL 0x8005030B

//
// MessageId: DATABASE_E_NOPACKET
//
// MessageText:
//
//  There is no data packet in the media file.
//
#define DATABASE_E_NOPACKET            0x8005030C

//
// MessageId: DATABASE_E_ENDOFQUERY
//
// MessageText:
//
//  The end of the time query or event query
//
#define DATABASE_E_ENDOFQUERY          0x80050401

//
// MessageId: DATABASE_E_BUFFERDEFICIT
//
// MessageText:
//
//  Given buffer size is not enough.
//
#define DATABASE_E_BUFFERDEFICIT       0x80050402

//
// MessageId: DATABASE_E_ENDOFSTREAM
//
// MessageText:
//
//  End of stream in a time segment
//
#define DATABASE_E_ENDOFSTREAM         0x80050403

//
// MessageId: DATABASE_E_STARTOFSTREAM
//
// MessageText:
//
//  Start of stream in a time segment
//
#define DATABASE_E_STARTOFSTREAM       0x80050404

//
// MessageId: DATABASE_E_LOCATIONNOTEXIST
//
// MessageText:
//
//  Location path does not exist.
//
#define DATABASE_E_LOCATIONNOTEXIST    0x80050501

//
// MessageId: DATABASE_E_EXCEPTION
//
// MessageText:
//
//  An exceptional condition
//
#define DATABASE_E_EXCEPTION           0x80050502

//
// MessageId: DATABASE_E_READONLY
//
// MessageText:
//
//  This location is read-only.
//
#define DATABASE_E_READONLY            0x80050503

//
// MessageId: DATABASE_E_SUBDIRINLOCATION
//
// MessageText:
//
//  There is subdirectory in Location path.
//
#define DATABASE_E_SUBDIRINLOCATION    0x80050504

//
// MessageId: DATABASE_E_BACKUPPROCEEDING
//
// MessageText:
//
//  Another process is executing buckup.
//
#define DATABASE_E_BACKUPPROCEEDING    0x80050505

//
// MessageId: DATABASE_E_CREATESEMAPHORE
//
// MessageText:
//
//  Fail to create a semaphore object.
//
#define DATABASE_E_CREATESEMAPHORE     0x80050601

//
// MessageId: DATABASE_E_FREESEMAPHORE
//
// MessageText:
//
//  Fail to free semaphore.
//
#define DATABASE_E_FREESEMAPHORE       0x80050602

//
// MessageId: DATABASE_E_RELEASESEMAPHORE
//
// MessageText:
//
//  Fail to release a semaphore object.
//
#define DATABASE_E_RELEASESEMAPHORE    0x80050603

//
// MessageId: DATABASE_E_INITIALCS
//
// MessageText:
//
//  Fail to initialize critical section.
//
#define DATABASE_E_INITIALCS           0x80050604

//
// MessageId: DATABASE_E_RELEASECS
//
// MessageText:
//
//  Fail to release critical section.
//
#define DATABASE_E_RELEASECS           0x80050605

//
// MessageId: DATABASE_E_NOSPACE
//
// MessageText:
//
//  The disk space alreay runs out or the file number of subdirectory already reach
//
#define DATABASE_E_NOSPACE			   0x80050701

//
// MessageId: DATABASE_E_NOPERMISSION
//
// MessageText:
//
//  user does not have access right to the specify directory or files
//
#define DATABASE_E_NOPERMISSION		   0x80050702

//
// MessageId: DATABASE_E_OPERATIONABORTED
//
// MessageText:
//
//  The operation is aborted before complete
//
#define DATABASE_E_OPERATIONABORTED		0x80050703

//
// MessageId: DATABASE_E_WOULDBLOCK
//
// MessageText:
//
//  The operation would block, and this is not what user wants
//
#define DATABASE_E_WOULDBLOCK			0x80050704

//
// MessageId: DATABASE_E_NOT_ALLOWED
//
// MessageText:
//
//  When input one media file, the file should not be located in the location directory
//
#define DATABASE_E_NOT_ALLOWED			0x80050705

/////////////////////////////////////////////////////////////////////////////
//  In this group, the database has problem in integrity and file format.  //
//  It must be repaired.                                                   //
/////////////////////////////////////////////////////////////////////////////
//
// MessageId: DATABASE_E_NEEDREPAIR
//
// MessageText:
//
//  Database needs to be repaired.
//
#define DATABASE_E_NEEDREPAIR		   0x80051000

//
// MessageId: DATABASE_E_NEEDREPAIRDATABASE
//
// MessageText:
//
//  The whole Database needs to be repaired.
//
#define DATABASE_E_NEEDREPAIRDATABASE  0x80051100

//
// MessageId: DATABASE_E_NODBS
//
// MessageText:
//
//  Dbs file does not exist.
//
#define DATABASE_E_NODBS               0x80051101

//
// MessageId: DATABASE_E_DBSFILEFORMAT
//
// MessageText:
//
//  File format of dbs file is error.
//
#define DATABASE_E_DBSFILEFORMAT       0x80051102

//
// MessageId: DATABASE_E_NEEDREPAIRLOCATION
//
// MessageText:
//
//  The Location needs to be repaired.
//
#define DATABASE_E_NEEDREPAIRLOCATION  0x80051200

//
// MessageId: DATABASE_E_NODSK
//
// MessageText:
//
//  Dsk file does not exist.
//
#define DATABASE_E_NODSK               0x80051201

//
// MessageId: DATABASE_E_NODSS
//
// MessageText:
//
//  Dss file does not exist.
//
#define DATABASE_E_NODSS               0x80051202

//
// MessageId: DATABASE_E_NOHGD
//
// MessageText:
//
//  Hgd file does not exist.
//
#define DATABASE_E_NOHGD               0x80051203

//
// MessageId: DATABASE_E_NODES
//
// MessageText:
//
//  Des file does not exist.
//
#define DATABASE_E_NODES               0x80051204

//
// MessageId: DATABASE_E_NODIA
//
// MessageText:
//
//  Dia file does not exist.
//
#define DATABASE_E_NODIA               0x80051205

//
// MessageId: DATABASE_E_NOBMK
//
// MessageText:
//
//  Bmk file does not exist.
//
#define DATABASE_E_NOBMK               0x80051206

//
// MessageId: DATABASE_E_DSKFILEFORMAT
//
// MessageText:
//
//  File format of .dsk file is error
//
#define DATABASE_E_DSKFILEFORMAT       0x80051210

//
// MessageId: DATABASE_E_DSSFILEFORMAT
//
// MessageText:
//
//  File format of .dss file is error
//
#define DATABASE_E_DSSFILEFORMAT       0x80051211

//
// MessageId: DATABASE_E_HGDFILEFORMAT
//
// MessageText:
//
//  File format of .hgd file is error
//
#define DATABASE_E_HGDFILEFORMAT       0x80051212

//
// MessageId: DATABASE_E_DESFILEFORMAT
//
// MessageText:
//
//  File format of .des file is error
//
#define DATABASE_E_DESFILEFORMAT       0x80051213

//
// MessageId: DATABASE_E_DIAFILEFORMAT
//
// MessageText:
//
//  File format of .dia file is error
//
#define DATABASE_E_DIAFILEFORMAT       0x80051214

// end of repair group

//
// MessageId: DATABASE_S_NOQUOTA
//
// MessageText:
//
//  There is no quota for backup.
//
#define DATABASE_S_NOQUOTA             0x00050101

//
// MessageId: DATABASE_S_MANUALSTOP
//
// MessageText:
//
//  Stop the process of backup manually.
//
#define DATABASE_S_MANUALSTOP          0x00050102

//
// MessageId: DATABASE_S_LOCATIONBACKUPED
//
// MessageText:
//
//  The location has backuped completely.
//
#define DATABASE_S_LOCATIONBACKUPED    0x00050103

//
// MessageId: DATABASE_S_CHANGEINTERVAL
//
// MessageText:
//
//  Output Data Packet in new interval when using the database retriever.
//
#define DATABASE_S_CHANGEINTERVAL      0x00050201

//
// MessageId: DATABASE_S_NOPREBUFFER
//
// MessageText:
//
//  There is no pre-buffer data in the location.
//
#define DATABASE_S_NOPREBUFFER         0x00050301

//
// MessageId: DATABASE_S_NON_I_VFRAME
//
// MessageText:
//
//  non-I Video frame is dropped until first I frame arrived
//
#define DATABASE_S_NON_I_VFRAME         0x00050302

//
// MessageId: DATABASE_S_INTEGRITY		
//
// MessageText:
//
//  the location need to be recovered, but the opener has readonly accees right,
//  could not recover it now. It might be browsed but might have problem for some
//  file.
//
#define DATABASE_S_INTEGRITY			0x00050303

#endif //_DATABASE_ERROR_


