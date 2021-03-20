/*
Module Name:
	Direct3D Window

Abstract:
	Template Direct3D application

Revision History:
	Date: Jan 14, 2019.
	Desc: Started

	Date: Jan 17, 2019.
	Desc: Done
*/

/////////////////////////////////////////////////////////////////////
//	H E A D E R S.
/////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>

#include <d3d11.h>

/////////////////////////////////////////////////////////////////////
//	M A C R O S   &   P R A G M A S 
/////////////////////////////////////////////////////////////////////
#pragma comment (lib, "d3d11.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

/////////////////////////////////////////////////////
// Global Variables declarations and initializations
/////////////////////////////////////////////////////
FILE* g_fLogger_sj;
char g_szLogFileName[] = "LOG.txt";

DWORD g_dwStyle;
HWND g_hwnd = NULL;
WINDOWPLACEMENT g_wpPrev = {sizeof(WINDOWPLACEMENT)};

bool g_bFullScreen = false;
bool g_boActiveWindow = false;
bool g_boEscapeKeyIsPressed = false;

HDC g_hdc = NULL;

float g_ClearColor_sj[4]; // RGBA
IDXGISwapChain *g_pIDXGISwapChain = NULL;
ID3D11Device *g_pID3D11Device = NULL;
ID3D11DeviceContext *g_pID3D11DeviceContext = NULL;
ID3D11RenderTargetView *g_pID3D11RenderTargetView = NULL;

/////////////////////////////////////////////////////////////////
// G L O B A L  F U N C T I O N S  &  D E C L A R A T I O N S
/////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/////////////////////////////////////////////////////////////////////
//	F U N C T I O N  D E F I N I T I O N S.
/////////////////////////////////////////////////////////////////////

int WINAPI
WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszcmdLine,
	int iCmdShow
)
{
	//function declarations
	HRESULT Initialize(void);
	void Display(void);
	void Uninitialize(void);

	// Declarations
	MSG msg_sj;
	HWND hwnd_sj;
	bool bDone_sj = false;
	WNDCLASSEX wndclass_sj;
	TCHAR szAppName_sj[] = TEXT("Direct3D11");

	// code 
	// create log files
	if(0 != fopen_s(&g_fLogger_sj, g_szLogFileName, "w"))
	{
		MessageBox(NULL, TEXT("Log File can not be Created\nExiting ...\n"), TEXT("Error"),
			MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}


	fprintf(g_fLogger_sj, "Log File SuccessFully Created \r\n");
	fclose(g_fLogger_sj);

	wndclass_sj.cbSize = sizeof(WNDCLASSEX);
	wndclass_sj.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;  // Adding owndc means telling OS that dont discard(fixed | discardable | movable)
	wndclass_sj.cbClsExtra = 0;
	wndclass_sj.cbWndExtra = 0;
	wndclass_sj.lpfnWndProc = WndProc;
	wndclass_sj.hInstance = hInstance;
	wndclass_sj.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass_sj.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass_sj.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass_sj.lpszClassName = szAppName_sj;
	wndclass_sj.lpszMenuName = NULL;
	wndclass_sj.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass_sj);
	hwnd_sj = CreateWindow(
			szAppName_sj,
			// WS_EX_APPWINDOW,		//top of taskbar even in multi-monitor scenario
			TEXT("Direct11 Window"),
			WS_OVERLAPPEDWINDOW,
			100,
			100,
			WIN_WIDTH,
			WIN_HEIGHT,
			NULL,
			NULL,
			hInstance,
			NULL);

	g_hwnd = hwnd_sj;

	ShowWindow(hwnd_sj, iCmdShow);
	SetForegroundWindow(hwnd_sj);					// Keep My Window At The First Position in  Z - ORDER
	SetFocus(hwnd_sj);

	// initialize D3D
	HRESULT hr;
	hr = Initialize();
	if (FAILED(hr))
	{
		//Choose PixelFormat
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "Initialize() Failed.\nExiting Now... \n");
		fclose(g_fLogger_sj);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "Initialize Succedded");
		fclose(g_fLogger_sj);
	}

	// message loop
	while (bDone_sj == false)
	{
		if (PeekMessage(&msg_sj, NULL, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT == msg_sj.message)
			{
				bDone_sj = true;
				//break;
			}
			else
			{
				TranslateMessage(&msg_sj);
				DispatchMessage(&msg_sj);
			}
		}
		else
		{
			// render
			Display();

			if (true == g_boActiveWindow)
			{
				if (true == g_boEscapeKeyIsPressed)
				{
					bDone_sj = true;
				}
			}
		}
	}

	// clean up
	Uninitialize();

	return ((int)msg_sj.wParam);
}

LRESULT CALLBACK
WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function declarations;
	HRESULT Resize(int, int);
	void Uninitialize(void);
	void ToggleFullScreen(void);

	HRESULT hr;

	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
		{
			g_boActiveWindow = true;
		}
		else
		{
			g_boActiveWindow = false;
		}

		break;

	case WM_ERASEBKGND:
		return 0; // it tells that we have Display call

	case WM_SETFOCUS:
		g_boActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		g_boActiveWindow = false;
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 'f':
		case 'F':
			ToggleFullScreen();
			break;
		}

		break;

	case WM_SIZE:
		if (g_pID3D11DeviceContext)
		{
			hr = Resize(LOWORD(lParam), HIWORD(lParam));
			if (FAILED(hr))
			{
				fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
				fprintf_s(g_fLogger_sj, "Resize() Failed. \n");
				fclose(g_fLogger_sj);
				return(hr);
			}
			else
			{
				fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
				fprintf_s(g_fLogger_sj, "Resize() Successed \n");
				fclose(g_fLogger_sj);
			}
		}

		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			if(false == g_boEscapeKeyIsPressed)
			{
				g_boEscapeKeyIsPressed = true;
			}

			break;

		case 0x46:
			if (g_bFullScreen)
			{
				ToggleFullScreen();
				g_bFullScreen = true;
			}
			else
			{
				ToggleFullScreen();
				g_bFullScreen = false;
			}

			break;
		default:
			break;

		}

		break;

	case WM_LBUTTONDOWN:
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		Uninitialize();
		PostQuitMessage(0);
		break;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

HRESULT Initialize(void)
{
	// Functions Declarations
	HRESULT Resize(int, int);
	void Uninitialize();

	// Variable Declarations
	HRESULT hr_sj;
	D3D_DRIVER_TYPE d3dDriverType_sj;
	D3D_DRIVER_TYPE d3dDriverTypes_sj[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE };
	D3D_FEATURE_LEVEL d3dFeatureLevel_required_sj = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevel_aquired_sj = D3D_FEATURE_LEVEL_10_0;

	UINT createDeviceFlags_sj = 0;
	UINT numDriverTypes_sj = 0;
	UINT numFeaturesLevels_sj = 1; // based upon d3dFeatureLevel_required

	// Code
	numDriverTypes_sj = sizeof(d3dDriverTypes_sj) / sizeof(d3dDriverTypes_sj[0]);

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc_sj;
	ZeroMemory((void *)&dxgiSwapChainDesc_sj, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc_sj.BufferCount = 1;
	dxgiSwapChainDesc_sj.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc_sj.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc_sj.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc_sj.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc_sj.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc_sj.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc_sj.OutputWindow = g_hwnd;
	dxgiSwapChainDesc_sj.SampleDesc.Count = 1;
	dxgiSwapChainDesc_sj.SampleDesc.Quality = 0;
	dxgiSwapChainDesc_sj.Windowed = TRUE;

	for (size_t driverTypeIndex_sj = 0; driverTypeIndex_sj < numDriverTypes_sj; driverTypeIndex_sj++)
	{
		d3dDriverType_sj = d3dDriverTypes_sj[driverTypeIndex_sj];
		hr_sj = D3D11CreateDeviceAndSwapChain(NULL,
					d3dDriverType_sj,
					NULL,
					createDeviceFlags_sj,
					&d3dFeatureLevel_required_sj,
					numFeaturesLevels_sj,
					D3D11_SDK_VERSION,
					&dxgiSwapChainDesc_sj,
					&g_pIDXGISwapChain,
					&g_pID3D11Device,
					&d3dFeatureLevel_aquired_sj,
					&g_pID3D11DeviceContext);

		if (SUCCEEDED(hr_sj))
			break;
	}

	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "D3D11CreateDeviceAndSwapChain() Failed\n");
		fclose(g_fLogger_sj);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "D3D11CreateDeviceAndSwapChain() Succedded\n");
		fprintf_s(g_fLogger_sj, "The Chosen Driver Is Of ");
		if(d3dDriverType_sj == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(g_fLogger_sj, "Hardware Type\n");
		}
		else if(d3dDriverType_sj == D3D_DRIVER_TYPE_WARP)
		{
			fprintf_s(g_fLogger_sj, "Warp Type\n");
		}
		else if(d3dDriverType_sj == D3D_DRIVER_TYPE_REFERENCE)
		{
			fprintf_s(g_fLogger_sj, "Reference Type. \n");
		}
		else
		{
			fprintf_s(g_fLogger_sj, "Unknown Type. \n");
		}

		fclose(g_fLogger_sj);
	}

	// d2d Clearing Color
	g_ClearColor_sj[0] = 0.0f;
	g_ClearColor_sj[1] = 0.0f;
	g_ClearColor_sj[2] = 1.0f;
	g_ClearColor_sj[3] = 1.0f;

	hr_sj = Resize(WIN_WIDTH, WIN_HEIGHT);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "Resize() Failed\n");
		fclose(g_fLogger_sj);
		return(hr_sj);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "Resize() Succedded\n");
		fclose(g_fLogger_sj);
	}

	return 0;
}



void ToggleFullScreen(void)
{
	// Declarations
	MONITORINFO mi_sj;

	//code
	if (false == g_bFullScreen)
	{
		g_dwStyle = GetWindowLong(g_hwnd, GWL_STYLE);
		if (WS_OVERLAPPEDWINDOW == (WS_OVERLAPPEDWINDOW & g_dwStyle))
		{
			mi_sj = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(g_hwnd, &g_wpPrev) && GetMonitorInfo(MonitorFromWindow(g_hwnd, MONITORINFOF_PRIMARY), &mi_sj))
			{
				SetWindowLong(g_hwnd, GWL_STYLE, g_dwStyle & (~WS_OVERLAPPEDWINDOW));
				SetWindowPos(g_hwnd, HWND_TOP, mi_sj.rcMonitor.left, mi_sj.rcMonitor.top, (mi_sj.rcMonitor.right - mi_sj.rcMonitor.left),
					(mi_sj.rcMonitor.bottom - mi_sj.rcMonitor.top), SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}

		ShowCursor(false);
		g_bFullScreen = true;
	}
	else
	{
		SetWindowLong(g_hwnd, GWL_STYLE, g_dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(g_hwnd, &g_wpPrev);
		SetWindowPos(g_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
		ShowCursor(true);
		g_bFullScreen = FALSE;
	}
}

void Display(void)
{
	g_pID3D11DeviceContext->ClearRenderTargetView(g_pID3D11RenderTargetView, g_ClearColor_sj);
	g_pIDXGISwapChain->Present(0, 0);
}

HRESULT Resize(int iwidth, int iheight)
{
	HRESULT hr;

	// code
	if(g_pID3D11RenderTargetView)
	{
		g_pID3D11RenderTargetView->Release();
		g_pID3D11RenderTargetView = NULL;
	}

	// resize swap chain buffers accordingly
	g_pIDXGISwapChain->ResizeBuffers(1, iwidth, iheight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	//
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer;
	g_pIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pID3D11Texture2D_BackBuffer);

	hr = g_pID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &g_pID3D11RenderTargetView);

	if (FAILED(hr))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "ID3D11Device::g_pID3D11Device->CreateRenderTargetView()\n");
		fclose(g_fLogger_sj);
		return(hr);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "ID3D11Device::g_pID3D11Device->CreateRenderTargetView()\n");
		fclose(g_fLogger_sj);
	}

	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	// set render target view as render target
	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)iwidth;
	d3dViewPort.Height = (float)iheight;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;
	g_pID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	return(hr);
}

void Uninitialize(void)
{
	// code
	if (g_pID3D11RenderTargetView)
	{
		g_pID3D11RenderTargetView->Release();
		g_pID3D11RenderTargetView = NULL;
	}

	if (g_pIDXGISwapChain)
	{
		g_pIDXGISwapChain->Release();
		g_pIDXGISwapChain = NULL;
	}

	if (g_pID3D11DeviceContext)
	{
		g_pID3D11DeviceContext->Release();
		g_pID3D11DeviceContext = NULL;
	}

	if(g_pID3D11Device)
	{
		g_pID3D11Device->Release();
		g_pID3D11Device = NULL;
	}

	if (g_fLogger_sj)
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "Uninitialize() Succedded\n");
		fprintf_s(g_fLogger_sj, "Successfully Created Log File\n");
		fclose(g_fLogger_sj);
	}
}

