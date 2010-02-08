#pragma once

namespace DADP
{
	typedef struct structural_D_RENDER_INFO
	{
		HWND  hWnd;
		RECT  rect;
	} D_RENDER_INFO;


	class __declspec(dllexport) DADPClass
	{
	public:
		DADPClass();
		~DADPClass();

		void	DEnableFullScreen( bool bFullScreen );
		void	DSetStretchMode( bool bStretch );
		DWORD	DNotifyFullScreenWindow( UINT message, WPARAM wParam, LPARAM lParam );
		void	DSetRenderInfo( D_RENDER_INFO* Renderinfo );
		void	DRender( LPBITMAPINFO lpBMPInfo, int nWidth, int nHeight );
		void	DSetTextOut( int nIndex, int nX, int nY, char* Text, int nTextLen );
		void	DEPTZRender( LPBITMAPINFO lpBMPInfo, int nX, int nY, int nWidth, int nHeight );
		void	DSetImageUpToDown( bool bUpToDown );
		void	DSetImageLeftToRight( bool bLeftToRight );
		void	DDisplayChildScreen( bool bDisplay, int nX, int nY, int nWidth, int nHeight );
	private:
		void *m_pObj;
	};


	extern "C" __declspec(dllexport) void	DGetVersion( char* VersionInfo );
	extern "C" __declspec(dllexport) HANDLE DInit();
	extern "C" __declspec(dllexport) void	DEnableFullScreen( HANDLE h, bool bFullScreen );
	extern "C" __declspec(dllexport) void	DSetStretchMode( HANDLE h, bool bStretch );
	extern "C" __declspec(dllexport) DWORD	DNotifyFullScreenWindow( HANDLE h, UINT message, WPARAM wParam, LPARAM lParam );
	extern "C" __declspec(dllexport) void	DSetRenderInfo( HANDLE h, void* Renderinfo );
	extern "C" __declspec(dllexport) void	DRender( HANDLE h, LPBITMAPINFO lpBMPInfo, int nWidth, int nHeight );
	extern "C" __declspec(dllexport) void	DEPTZRender( HANDLE h, LPBITMAPINFO lpBMPInfo, int nX, int nY, int nWidth, int nHeight );
	extern "C" __declspec(dllexport) void	DSetTextOut( HANDLE h, int nIndex, int nX, int nY, char* Text, int nTextLen );
	extern "C" __declspec(dllexport) void	DSetImageUpToDown( HANDLE h,bool bUpToDown );
	extern "C" __declspec(dllexport) void	DSetImageLeftToRight( HANDLE h,bool bLeftToRight );
	extern "C" __declspec(dllexport) void	DExit( HANDLE h );
};
