/*
Module Name:
	Direct3D Window

Abstract:
	Direct3D Application to show triangle in Perspective View
	This Triangle Rotates around Y axis

Revision History:
	Date: Jan 26, 2019.
	Desc: Started

	Date: Jan 26, 2019.
	Desc: Done
*/

/////////////////////////////////////////////////////////////////////
//	H E A D E R S.
/////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>

#include <d3d11.h>
#include <d3dcompiler.h>

// need to comment warning
#include "XNAMath/xnamath.h"

/////////////////////////////////////////////////////////////////////
//	M A C R O S   &   P R A G M A S 
/////////////////////////////////////////////////////////////////////
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "D3dcompiler.lib")

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
ID3D11Device *g_pID3D11Device_sj = NULL;
IDXGISwapChain *g_pIDXGISwapChain = NULL;
ID3D11DeviceContext *g_pID3D11Device_Context_sj = NULL;
ID3D11RenderTargetView *g_pID3D11RenderTargetView_sj = NULL;

ID3D11PixelShader  *g_pID3D11PixelShader_sj = NULL;
ID3D11VertexShader *g_pID3D11VertexShader_sj = NULL;
ID3D11Buffer       *g_pID3D11Buffer_Pyramid_PosVertexBuffer_sj = NULL;
ID3D11Buffer       *g_pID3D11Buffer_VertexBuffer_Colors_Triangle_sj = NULL;
ID3D11Buffer       *g_pID3D11Buffer_VertexBuffer_Position_Cube_sj = NULL;
ID3D11Buffer       *g_pID3D11Buffer_VertexBuffer_Colors_Cube_sj = NULL;
ID3D11InputLayout  *g_pID3D11InputLayout_sj = NULL;
ID3D11Buffer       *g_pID3D11Buffer_ConstantBuffer_sj = NULL;

ID3D11RasterizerState *gpID3D11RasterizerState = NULL;
ID3D11DepthStencilView * g_ID3D11DepthStensilView = NULL;

struct CBUFFER
{
	XMMATRIX WorldViewProjectionMatrix_sj;
};

XMMATRIX g_PerspectiveProjectionMatrix_sj;

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
		if (g_pID3D11Device_Context_sj)
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

HRESULT
Initialize(void)
{
	// Functions Declarations
	HRESULT Resize(int, int);
	void Uninitialize();

	// Variable Declarations
	HRESULT hr_sj;
	D3D_DRIVER_TYPE d3dDriverType_sj;
	D3D_DRIVER_TYPE d3dDriverTypes_sj[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
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
					&g_pID3D11Device_sj,
					&d3dFeatureLevel_aquired_sj,
					&g_pID3D11Device_Context_sj);

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

		fprintf_s(g_fLogger_sj, "The Supported Highest Feature Level Is ");
		if(D3D_FEATURE_LEVEL_11_0 == d3dFeatureLevel_aquired_sj)
		{
			fprintf_s(g_fLogger_sj, "11.0\n");
		}
		else if(D3D_FEATURE_LEVEL_10_1 == d3dFeatureLevel_aquired_sj)
		{
			fprintf_s(g_fLogger_sj, "10.1\n");
		}
		else if (D3D_FEATURE_LEVEL_10_0 == d3dFeatureLevel_aquired_sj)
		{
			fprintf_s(g_fLogger_sj, "10.0\n");
		}
		else
		{
			fprintf_s(g_fLogger_sj, "Unknown\n");
		}

		fclose(g_fLogger_sj);
	}

	// initialize shaders, input layouts, constant buffers
	const char *vertextShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 WorldViewProjectionMatrix_sj;" \
		"}" \

		"struct Vertex_Output{" \
		"float4 position: SV_POSITION;" \
		"float4 color: COLOR;" \
		"};" \

		"Vertex_Output main(float4 pos: POSITION, float4 color: COLOR)" \
		"{" \
		"Vertex_Output output;" \
		"output.position = mul(WorldViewProjectionMatrix_sj, pos);" \
		"output.color = color;" \
		"return(output);" \
		"}";

	ID3DBlob *pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob *p_ID3DBlob_Error = NULL;

	hr_sj = D3DCompile(vertextShaderSourceCode,
				lstrlenA(vertextShaderSourceCode) + 1,
				"VS",
				NULL,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main",
				"vs_5_0",
				0,
				0,
				&pID3DBlob_VertexShaderCode,
				&p_ID3DBlob_Error);

	if (FAILED(hr_sj))
	{
		if (NULL != p_ID3DBlob_Error)
		{
			fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
			fprintf(g_fLogger_sj, "D3DCompile() Failed For Vertex Shader: %s\n",
						(char *)p_ID3DBlob_Error->GetBufferPointer());
			fclose(g_fLogger_sj);
			p_ID3DBlob_Error->Release();
			p_ID3DBlob_Error = NULL;
			return(hr_sj);
		}
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "D3DCompile() Successed For Vertex Shader\n");
		fclose(g_fLogger_sj);
	}

	hr_sj = g_pID3D11Device_sj->CreateVertexShader(
				pID3DBlob_VertexShaderCode->GetBufferPointer(),
				pID3DBlob_VertexShaderCode->GetBufferSize(),
				NULL,
				&g_pID3D11VertexShader_sj);
	if(FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateVertexShader() Failed\n");
		fclose(g_fLogger_sj);
		return(hr_sj);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateVertexShader() Succeded\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->VSSetShader(g_pID3D11VertexShader_sj, 0, 0);

	const char * pixelShaderSourceCode =
		"float4 main(float4 pos:SV_POSITION, float4 color: COLOR) : SV_TARGET" \
		"{" \
			"return(color);" \
		"}";

	ID3DBlob *pID3DBlob_PixelShaderCode = NULL;
	p_ID3DBlob_Error = NULL;

	hr_sj = D3DCompile(pixelShaderSourceCode,
		lstrlenA(pixelShaderSourceCode) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode,
		&p_ID3DBlob_Error);
	if (FAILED(hr_sj))
	{
		if (NULL != p_ID3DBlob_Error)
		{
			fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
			fprintf_s(g_fLogger_sj, "D3DCompile() failed for Pixel Shader %s\n", (char *)p_ID3DBlob_Error->GetBufferPointer());
			fclose(g_fLogger_sj);
			return(hr_sj);
		}
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "D3DCompile() failed for Pixel Shader\n");
		fclose(g_fLogger_sj);
	}

	hr_sj = g_pID3D11Device_sj->CreatePixelShader(pID3DBlob_PixelShaderCode->GetBufferPointer(),
		pID3DBlob_PixelShaderCode->GetBufferSize(), NULL, &g_pID3D11PixelShader_sj);
	if(FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName,"a+");
		fprintf_s(g_fLogger_sj, "ID3D11Device::CreatePixelShader() Failed %s \n",
			(char *)p_ID3DBlob_Error->GetBufferPointer());
		fclose(g_fLogger_sj);
		p_ID3DBlob_Error->Release();
		p_ID3DBlob_Error = NULL;
		return(hr_sj);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreatePixelShader() Succedded\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->PSSetShader(g_pID3D11PixelShader_sj, 0, 0);

	//
	// ------------------------ I N P U T   L A Y O U T S--------------------------
	//
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	inputElementDesc[1].SemanticName = "COLOR";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

	hr_sj = g_pID3D11Device_sj->CreateInputLayout(inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		&g_pID3D11InputLayout_sj);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateInputLayout() Failed\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateInputLayout() Succeded\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->IASetInputLayout(g_pID3D11InputLayout_sj);
	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_VertexShaderCode = NULL;
	pID3DBlob_PixelShaderCode->Release();
	pID3DBlob_PixelShaderCode = NULL;


	//
	// ---------------------- V E R T I C E S   B U F F E R S --------------------------
	//

	float fverticesPyramid[] =
	{
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,

		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f
	};

	float fcolorsPyramid[] =
	{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f,

		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f
	};

	float fverticesCube[] =
	{
		// Side 1 (Top)
		// triangle 1
		-1.0f, 1.0f, 1.0f,
		1.0f,  1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		// triangle 2
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, +1.0f,
		1.0f, 1.0f, -1.0f,

		// side 2 (Bottom)
		// triangle 1
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,

		// triangle 2
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		// side 3 (Front)
		// triangle 1
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f,  -1.0f,
		-1.0f, -1.0f, -1.0f,

		// triangle 2
		-1.0f, -1.0f, -1.0f,
		1.0f, 1.0f,  -1.0f,
		1.0f, -1.0f, -1.0f,

		// side 4 (Back)
		// triangle 1
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f,  1.0f,
		-1.0f, -1.0f,1.0f,

		// triangle 2
		-1.0f, -1.0f, 1.0f,
		1.0f,  1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// Side 5
		// triangle 1
		-1.0f,   1.0f,  1.0f,
		-1.0f,   1.0f, -1.0f,
		-1.0f, -1.0f,   1.0f,

		// triangle 2
		-1.0f, -1.0f,   1.0f,
		-1.0f,   1.0f, -1.0f,
		-1.0f, -1.0f,  -1.0f,

		// Side 6
		// triangle 1
		1.0f,  -1.0f,  -1.0f,
		1.0f,   1.0f,  -1.0f,
		1.0f,  -1.0f,  1.0f,

		// triangle 2
		1.0f,  -1.0f,  1.0f,
		1.0f,   1.0f,  -1.0f,
		1.0f,   1.0f,  1.0f
	};

	float fcolorsCube[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};

	//
	// Triangle
	// Triangle Positions
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(fverticesPyramid);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr_sj = g_pID3D11Device_sj->CreateBuffer(&bufferDesc, NULL, &g_pID3D11Buffer_Pyramid_PosVertexBuffer_sj);

	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() failed\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() Succeded For Vertex Buffer()\n");
		fclose(g_fLogger_sj);
	}

	//
	// copy
	//
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	g_pID3D11Device_Context_sj->Map(g_pID3D11Buffer_Pyramid_PosVertexBuffer_sj, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, fverticesPyramid, sizeof(fverticesPyramid));
	g_pID3D11Device_Context_sj->Unmap(g_pID3D11Buffer_Pyramid_PosVertexBuffer_sj, NULL);

	// T R I A N G L E    C O L O R S 
	D3D11_BUFFER_DESC bufferDesc_color;
	ZeroMemory(&bufferDesc_color, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_color.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_color.ByteWidth = sizeof(float) * _ARRAYSIZE(fcolorsPyramid);
	bufferDesc_color.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc_color.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr_sj = g_pID3D11Device_sj->CreateBuffer(&bufferDesc_color, NULL,
		&g_pID3D11Buffer_VertexBuffer_Colors_Triangle_sj);

	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "D3D11Initialize::CreateBuffer() Failed For Triangle Color\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "D3D11Initialize::CreateBuffer() Succeded For Triangle Color()\n");
		fclose(g_fLogger_sj);
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	g_pID3D11Device_Context_sj->Map(g_pID3D11Buffer_VertexBuffer_Colors_Triangle_sj, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, fcolorsPyramid, sizeof(fcolorsPyramid));
	g_pID3D11Device_Context_sj->Unmap(g_pID3D11Buffer_VertexBuffer_Colors_Triangle_sj, NULL);

	// Cube
	D3D11_BUFFER_DESC bufferDescRectangle;
	ZeroMemory(&bufferDescRectangle, sizeof(D3D11_BUFFER_DESC));
	bufferDescRectangle.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescRectangle.ByteWidth = sizeof(float) * _ARRAYSIZE(fverticesCube);
	bufferDescRectangle.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescRectangle.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr_sj = g_pID3D11Device_sj->CreateBuffer(&bufferDescRectangle,
		NULL,
		&g_pID3D11Buffer_VertexBuffer_Position_Cube_sj);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() failed for Rectangle Vertex Buffer\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() Succeded For Rectangle Vertex Buffer\n");
		fclose(g_fLogger_sj);
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	g_pID3D11Device_Context_sj->Map(g_pID3D11Buffer_VertexBuffer_Position_Cube_sj, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, fverticesCube, sizeof(fverticesCube));
	g_pID3D11Device_Context_sj->Unmap(g_pID3D11Buffer_VertexBuffer_Position_Cube_sj, NULL);

	// Cube Colors
	D3D11_BUFFER_DESC bufferDesc_color_rectangle;
	ZeroMemory(&bufferDesc_color_rectangle, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_color_rectangle.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_color_rectangle.ByteWidth = sizeof(float) * _ARRAYSIZE(fcolorsCube);
	bufferDesc_color_rectangle.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc_color_rectangle.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr_sj = g_pID3D11Device_sj->CreateBuffer(&bufferDesc_color_rectangle, NULL,
		&g_pID3D11Buffer_VertexBuffer_Colors_Cube_sj);

	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "D3D11Initialize::CreateBuffer() Failed For Rectangle Color\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "D3D11Initialize::CreateBuffer() Succeded For Rectangle Color()\n");
		fclose(g_fLogger_sj);
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	g_pID3D11Device_Context_sj->Map(g_pID3D11Buffer_VertexBuffer_Colors_Cube_sj, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, fcolorsCube, sizeof(fcolorsCube));
	g_pID3D11Device_Context_sj->Unmap(g_pID3D11Buffer_VertexBuffer_Colors_Cube_sj, NULL);

	// ------------------------------------------------------------------------------------
	// define and set the constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);

	hr_sj = g_pID3D11Device_sj->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &g_pID3D11Buffer_ConstantBuffer_sj);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() Failed for constant Buffer\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() Succeded For Constant Buffer\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->VSSetConstantBuffers(0, 1, &g_pID3D11Buffer_ConstantBuffer_sj);

	// setting rasterization state
	// In D3D backface culling is on dafaultly

	D3D11_RASTERIZER_DESC rasterizerDescriptor;
	ZeroMemory((void *)&rasterizerDescriptor, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDescriptor.AntialiasedLineEnable = FALSE;
	rasterizerDescriptor.CullMode = D3D11_CULL_NONE;
	rasterizerDescriptor.DepthBias = 0;
	rasterizerDescriptor.DepthBiasClamp = 0.0f;
	rasterizerDescriptor.DepthClipEnable = TRUE;
	rasterizerDescriptor.SlopeScaledDepthBias = 0.0f;
	rasterizerDescriptor.FillMode = D3D11_FILL_SOLID;
	rasterizerDescriptor.FrontCounterClockwise = FALSE;
	rasterizerDescriptor.MultisampleEnable = FALSE;
	rasterizerDescriptor.ScissorEnable = FALSE;

	hr_sj = g_pID3D11Device_sj->CreateRasterizerState(&rasterizerDescriptor, &gpID3D11RasterizerState);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateRasterizerState() Failed For Culling\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateRasterizerState() Succeded For Culling\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->RSSetState(gpID3D11RasterizerState);

	// d2d Clearing Color
	g_ClearColor_sj[0] = 0.0f;
	g_ClearColor_sj[1] = 0.0f;
	g_ClearColor_sj[2] = 0.0f;
	g_ClearColor_sj[3] = 1.0f;

	g_PerspectiveProjectionMatrix_sj = XMMatrixIdentity();
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
		fprintf_s(g_fLogger_sj, "Resize() Succeeded\n");
		fclose(g_fLogger_sj);
	}

	return (S_OK);
}

void Display(void)
{
	// initialization & declarations
	static float fAngle = 0.0f;

	XMMATRIX wvpMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX worldMatrix;
	XMMATRIX rotationMatrix;
	XMMATRIX translationMatrix;

	// code
	// clear render target view to a chosen color
	g_pID3D11Device_Context_sj->ClearRenderTargetView(g_pID3D11RenderTargetView_sj, g_ClearColor_sj);
	g_pID3D11Device_Context_sj->ClearDepthStencilView(g_ID3D11DepthStensilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// -----------TRIANGLE----------------------
	// select which vertex buffer to display
	UINT offset = 0;
	UINT stride = sizeof(float) * 3;

	g_pID3D11Device_Context_sj->IASetVertexBuffers(0, 1, &g_pID3D11Buffer_Pyramid_PosVertexBuffer_sj, &stride, &offset);

	offset = 0;
	stride = sizeof(float) * 3;
	g_pID3D11Device_Context_sj->IASetVertexBuffers(1, 1, &g_pID3D11Buffer_VertexBuffer_Colors_Triangle_sj, &stride, &offset);
	g_pID3D11Device_Context_sj->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// translation which is concerened with	world matrix transformation
	translationMatrix = XMMatrixTranslation(-1.5f, 0.0f, 6.0f);
	rotationMatrix = XMMatrixRotationY(-fAngle);

	worldMatrix = XMMatrixIdentity();
	viewMatrix = XMMatrixIdentity();
	worldMatrix = rotationMatrix * translationMatrix;
	// final WorldView Projection Matrix
	wvpMatrix = worldMatrix * viewMatrix * g_PerspectiveProjectionMatrix_sj;

	// load the data into the constant buffer
	CBUFFER constantBuffer;
	constantBuffer.WorldViewProjectionMatrix_sj = wvpMatrix;
	g_pID3D11Device_Context_sj->UpdateSubresource(g_pID3D11Buffer_ConstantBuffer_sj, 0, NULL, &constantBuffer, 0, 0);
	g_pID3D11Device_Context_sj->Draw(12, 0);

	// ------------------------------- C U B E --------------------------------------
	offset = 0;
	stride = sizeof(float) * 3;
	g_pID3D11Device_Context_sj->IASetVertexBuffers(0, 1, &g_pID3D11Buffer_VertexBuffer_Position_Cube_sj, &stride, &offset);

	offset = 0;
	stride = sizeof(float) * 3;
	g_pID3D11Device_Context_sj->IASetVertexBuffers(1, 1, &g_pID3D11Buffer_VertexBuffer_Colors_Cube_sj, &stride, &offset);
	g_pID3D11Device_Context_sj->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// translation which is concerened with	world matrix transformation
	worldMatrix = XMMatrixIdentity();
	viewMatrix = XMMatrixIdentity();

	translationMatrix = XMMatrixIdentity();
	rotationMatrix = XMMatrixIdentity();

	translationMatrix = XMMatrixTranslation(1.5f, 0.0f, 6.0f);
	//rotationMatrix = XMMatrixRotationX(-fAngle);

	XMMATRIX r1 = XMMatrixRotationX(fAngle);
	XMMATRIX r2 = XMMatrixRotationY(fAngle);
	XMMATRIX r3 = XMMatrixRotationZ(fAngle);
	rotationMatrix = r1 * r2 * r3;

	XMMATRIX scaleM = XMMatrixScaling(0.75f, 0.75f, 0.75f);

	worldMatrix = scaleM * rotationMatrix * translationMatrix;
	wvpMatrix = worldMatrix * viewMatrix * g_PerspectiveProjectionMatrix_sj;

	constantBuffer.WorldViewProjectionMatrix_sj = wvpMatrix;
	g_pID3D11Device_Context_sj->UpdateSubresource(g_pID3D11Buffer_ConstantBuffer_sj, 0, NULL, &constantBuffer, 0, 0);

	g_pID3D11Device_Context_sj->Draw(6, 0);  
	g_pID3D11Device_Context_sj->Draw(6, 6);  
	g_pID3D11Device_Context_sj->Draw(6, 12); 
	g_pID3D11Device_Context_sj->Draw(6, 18); 
	g_pID3D11Device_Context_sj->Draw(6, 24); 
	g_pID3D11Device_Context_sj->Draw(6, 30); 

	g_pIDXGISwapChain->Present(0, 0);

	fAngle = fAngle + 0.0001f;
	if (fAngle >= 360.0f)
	{
		fAngle = 0.0f;
	}
}

HRESULT Resize(int iwidth, int iheight)
{
	HRESULT hr;

	// code
	if (g_ID3D11DepthStensilView)
	{
		g_ID3D11DepthStensilView->Release();
		g_ID3D11DepthStensilView = NULL;
	}

	if(g_pID3D11RenderTargetView_sj)
	{
		g_pID3D11RenderTargetView_sj->Release();
		g_pID3D11RenderTargetView_sj = NULL;
	}

	// resize swap chain buffers accordingly
	g_pIDXGISwapChain->ResizeBuffers(1, iwidth, iheight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	//
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer;
	g_pIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pID3D11Texture2D_BackBuffer);

	hr = g_pID3D11Device_sj->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &g_pID3D11RenderTargetView_sj);
	if(FAILED(hr))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "ID3D11Device::g_pID3D11Device_sj->CreateRenderTargetView()\n");
		fclose(g_fLogger_sj);
		return(hr);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "ID3D11Device::g_pID3D11Device_sj->CreateRenderTargetView()\n");
		fclose(g_fLogger_sj);
	}

	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	// create stensil buffer or z-buffer
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT)iwidth;
	textureDesc.Height = (UINT)iheight;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D *pID3D11Texture2D_DepthBuffer;
	g_pID3D11Device_sj->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2D_DepthBuffer);

	// creating depth stencil view from above depth stencil buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	hr = g_pID3D11Device_sj->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer, &depthStencilViewDesc, &g_ID3D11DepthStensilView);
	if (FAILED(hr))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "ID3D11Device::CreateDepthStencilView() Failed\n");
		fclose(g_fLogger_sj);
		return(hr);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "ID3D11Device::CreateDepthStencilView() Succed\n");
		fclose(g_fLogger_sj);
	}

	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	g_pID3D11Device_Context_sj->OMSetRenderTargets(1, &g_pID3D11RenderTargetView_sj, g_ID3D11DepthStensilView);

	// set render target view as render target
	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)iwidth;
	d3dViewPort.Height = (float)iheight;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;
	g_pID3D11Device_Context_sj->RSSetViewports(1, &d3dViewPort);

	// Set perspective projection matrix
	g_PerspectiveProjectionMatrix_sj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)iwidth / (float)iheight, 0.1f, 100.0f);

	return(hr);
}

void Uninitialize(void)
{
	// code
	if (g_pID3D11RenderTargetView_sj)
	{
		g_pID3D11RenderTargetView_sj->Release();
		g_pID3D11RenderTargetView_sj = NULL;
	}

	if (g_pIDXGISwapChain)
	{
		g_pIDXGISwapChain->Release();
		g_pIDXGISwapChain = NULL;
	}

	if (g_pID3D11Device_Context_sj)
	{
		g_pID3D11Device_Context_sj->Release();
		g_pID3D11Device_Context_sj = NULL;
	}

	if(g_pID3D11Device_sj)
	{
		g_pID3D11Device_sj->Release();
		g_pID3D11Device_sj = NULL;
	}

	if(g_fLogger_sj)
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "Uninitialize() Succedded\n");
		fprintf_s(g_fLogger_sj, "Successfully Created Log File\n");
		fclose(g_fLogger_sj);
	}
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

