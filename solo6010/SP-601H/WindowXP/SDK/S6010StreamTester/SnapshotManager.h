// SnapshotManager.h: interface for the CSnapshotManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNAPSHOTMANAGER_H__17EDA52D_203A_42AD_A76F_3D4887AA151B__INCLUDED_)
#define AFX_SNAPSHOTMANAGER_H__17EDA52D_203A_42AD_A76F_3D4887AA151B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSnapshotManager  
{
public:
	CSnapshotManager();
	virtual ~CSnapshotManager();

	void WriteSnapshotToFile (BYTE *bufYUVImg, int nWidth, int nHeight,
		const char *strFileName);

private:
	void ConvertYUVToBGR (BYTE *bufYUV, int nWidth, int nHeight, BYTE *bufBGR);
	void SaveBGRToBMP (BYTE *bufBGR, int nWidth, int nHeight, const char *strFileName);
};

#endif // !defined(AFX_SNAPSHOTMANAGER_H__17EDA52D_203A_42AD_A76F_3D4887AA151B__INCLUDED_)
