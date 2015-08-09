// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		WinAPIプログラムヘッダファイル
// 
// 				Ver 3.14f
// 
// -------------------------------------------------------------------------------

#ifndef __DXWINAPI_H__
#define __DXWINAPI_H__

// インクルード ------------------------------------------------------------------
#include "../DxCompileConfig.h"
#include <windows.h>
#include <winsock.h>
#include "../DxNetwork.h"

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 --------------------------------------------------------------------

// 構造体定義 --------------------------------------------------------------------

#ifndef DX_NON_NETWORK

// WinSock の DLL のポインタや中のAPIのポインタなど
struct WINSOCKFUNCTION
{
	HMODULE					WinSockDLL ;						// WinSockDLL
	int						( WINAPI *WSAGetLastErrorFunc )( void ) ;
	int						( WINAPI *WSAStartupFunc )( WORD wVersionRequested, LPWSADATA lpWSAData ) ;
	int						( WINAPI *WSACleanupFunc )( void ) ;
	int						( WINAPI *WSAAsyncSelectFunc )( SOCKET s, HWND hWnd, unsigned int wMsg, long lEvent ) ;
	int						( WINAPI *getaddrinfoFunc )( const char *nodename, const char *servname, const _addrinfo *hints, _addrinfo **res ) ;
	struct hostent*			( WINAPI *gethostbyaddrFunc )( const char *addr, int len, int type ) ;
	struct hostent*			( WINAPI *gethostbynameFunc )( const char *name ) ;
	int						( WINAPI *gethostnameFunc )( char *name, int namelen ) ;
	u_short					( WINAPI *ntohsFunc )( u_short netshort ) ;
	u_short					( WINAPI *htonsFunc )( u_short hostshort ) ;
	int						( WINAPI *connectFunc )( SOCKET s, const struct sockaddr *name, int namelen ) ;
	SOCKET					( WINAPI *socketFunc )( int af, int type, int protocol ) ;
	int						( WINAPI *sendFunc )( SOCKET s, const char *buf, int len, int flags ) ;
	int						( WINAPI *sendtoFunc )( SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen ) ;
	int						( WINAPI *recvfromFunc )( SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen ) ;
	SOCKET					( WINAPI *acceptFunc )( SOCKET s, struct sockaddr *addr, int *addrlen ) ;
	int						( WINAPI *closesocketFunc )( SOCKET s ) ;
	int						( WINAPI *shutdownFunc )( SOCKET s, int how ) ;
	int						( WINAPI *listenFunc )( SOCKET s, int backlog ) ;
	int						( WINAPI *bindFunc )( SOCKET s, const struct sockaddr *name, int namelen ) ;
	unsigned long			( WINAPI *inet_addrFunc )( const char *cp ) ;
	int						( WINAPI *recvFunc )( SOCKET s, char *buf, int len, int flags ) ;
} ;

#endif // DX_NON_NETWORK

// Input Method Manager DLL のポインタや中のAPIのポインタなど
struct IMMFUNCTION
{
	HMODULE					Imm32DLL ;
	HIMC					( WINAPI *ImmGetContextFunc )( HWND hWnd );
	BOOL					( WINAPI *ImmReleaseContextFunc )( HWND hWnd, HIMC hIMC ) ;
	BOOL					( WINAPI *ImmGetOpenStatusFunc )( HIMC hIMC ) ;
	BOOL					( WINAPI *ImmGetConversionStatusFunc )( HIMC hIMC, LPDWORD lpfdwConversion, LPDWORD lpfdwSentence ) ;
	BOOL					( WINAPI *ImmNotifyIMEFunc )( HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue ) ;
	BOOL					( WINAPI *ImmSetOpenStatusFunc )( HIMC hIMC, BOOL fOpen ) ;

	DWORD					( WINAPI *ImmGetCandidateListFunc )( HIMC hIMC, DWORD deIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen ) ;
	DWORD					( WINAPI *ImmGetCandidateListCountFunc )( HIMC hIMC, LPDWORD lpdwListCount ) ;
	LONG					( WINAPI *ImmGetCompositionStringFunc )( HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen ) ;
	BOOL					( WINAPI *ImmSetCompositionStringFunc )( HIMC hIMC, DWORD dwIndex, LPCVOID lpComp, DWORD dwCompLen, LPCVOID lpRead, DWORD dwReadLen ) ;
} ;

// Win32 API DLL のポインタや API のポインタなど
struct WIN32APIFUNCTION
{
	HMODULE					WinMMDLL ;
	MMRESULT				( WINAPI *timeSetEventFunc )( UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent ) ;
	MMRESULT				( WINAPI *timeKillEventFunc )( UINT uTimerID ) ;
	MMRESULT				( WINAPI *timeBeginPeriodFunc )( UINT uPeriod ) ;
	MMRESULT				( WINAPI *timeEndPeriodFunc )( UINT uPeriod ) ;
	DWORD					( WINAPI *timeGetTimeFunc )( VOID ) ;
	MMRESULT				( WINAPI *timeGetDevCapsFunc )( LPTIMECAPS ptc, UINT cbtc ) ;
	MMRESULT				( WINAPI *joyGetPosExFunc )( UINT uJoyID, LPJOYINFOEX pji ) ;

	MMRESULT				( WINAPI *joyGetDevCapsFunc )( UINT uJoyID, LPJOYCAPSW pjc, UINT cbjc ) ;
	MCIERROR				( WINAPI *mciSendCommandFunc )( MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam ) ;


	HMODULE					Kernel32DLL ;
	HMODULE					( WINAPI *GetModuleHandleWFunc )( LPCWSTR lpModuleName ) ;


	HMODULE					Old32DLL ;
	HRESULT					( WINAPI *CoCreateInstanceFunc )( REFCLSID rclsid, IUnknown * pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv ) ;
	LPVOID					( WINAPI *CoTaskMemAllocFunc )( SIZE_T cb ) ;
	void					( WINAPI *CoTaskMemFreeFunc )( LPVOID pv ) ;
	HRESULT					( WINAPI *CoInitializeExFunc )( LPVOID pvReserved, DWORD dwCoInit ) ;
	void					( WINAPI *CoFreeUnusedLibrariesFunc )( void ) ;
	void					( WINAPI *CoUninitializeFunc )( void ) ;


	HMODULE					Comctl32DLL ;
	void					( WINAPI *InitCommonControlsFunc )( VOID ) ;


	HMODULE					User32DLL ;
	BOOL					( WINAPI *WINNLSEnableIME_Func )( HWND hwnd, BOOL bFlag ) ;	// WINNLSEnableIME APIのアドレス
	BOOL					( WINAPI *UpdateLayeredWindow )( HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD ) ;		// UpdateLayeredWindow のＡＰＩポインタ
	HWND					( WINAPI *CreateWindowExWFunc )( DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam ) ;
	BOOL					( WINAPI *EnumDisplayDevicesWFunc )( LPCWSTR lpDevice, DWORD iDevNum, PDISPLAY_DEVICEW lpDisplayDevice, DWORD dwFlags ) ;

} ;

// WinAPI 情報構造体
struct WINAPIDATA
{
#ifndef DX_NON_NETWORK
	WINSOCKFUNCTION			WinSockFunc ;						// WinSock API 関係のデータ
#endif

#ifndef DX_NON_KEYEX
	IMMFUNCTION				ImmFunc ;							// IMM API 関係のデータ
#endif

	WIN32APIFUNCTION		Win32Func ;							// Win32 API 関係のデータ

	HMODULE					DwmApiDLL ;							// Desktop Window Manager API DLL
	HRESULT					( WINAPI *DF_DwmEnableComposition )( UINT uCompositionAction ) ;	// DwmEnableComposition API のアドレス
} ;

// 内部大域変数宣言 --------------------------------------------------------------

extern WINAPIDATA WinAPIData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

extern int LoadWinAPI( void ) ;				// WindowsOS の DLL を読み込む
extern int ReleaseWinAPI( void ) ;			// WindowsOS の DLL を解放する

extern HRESULT WinAPI_CoCreateInstance_ASync( REFCLSID rclsid, IUnknown * pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv, int ASyncThread = FALSE ) ;

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // __DXWINAPI_H__
