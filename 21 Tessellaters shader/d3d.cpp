/*
Module Name:
	Direct3D Window Geometry Shader

Abstract:
	Direct3D Application to show triangle in Perspective View

Revision History:

	Date: April 06, 2019.
	Desc: Started

	Date: April 06, 2019.
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

#define WIN_WIDTH  800
#define WIN_HEIGHT 600

/////////////////////////////////////////////////////
// Global Variables declarations and initializations
/////////////////////////////////////////////////////

FILE* g_fLogger_sj;
char g_szLogFileName[] = "LOG.txt";

DWORD g_dwStyle;
HWND g_hwnd = NULL;
WINDOWPLACEMENT g_wpPrev = { sizeof(WINDOWPLACEMENT) };

bool g_bFullScreen			= false;
bool g_boActiveWindow		= false;
bool g_boEscapeKeyIsPressed = false;

HDC g_hdc = NULL;

float g_ClearColor_sj[4]; // RGBA
ID3D11Device		*g_pID3D11Device_sj = NULL;
IDXGISwapChain		*g_pIDXGISwapChain = NULL;
ID3D11DeviceContext *g_pID3D11Device_Context_sj = NULL;
ID3D11RenderTargetView *g_pID3D11RenderTargetView_sj = NULL;

ID3D11VertexShader	*g_pID3D11VertexShader_sj = NULL;
ID3D11HullShader	*g_pID3D11HullShader_sj = NULL;
ID3D11DomainShader	*g_pID3D11DomainShader_sj = NULL;
ID3D11PixelShader	*g_pID3D11PixelShader_sj = NULL;

ID3D11Buffer		*g_pID3D11Buffer_VertexBuffer_sj = NULL;
ID3D11InputLayout	*g_pID3D11InputLayout_sj		= NULL;

ID3D11Buffer       *g_pID3D11Buffer_ConstantBuffer_HullShader   = NULL;
ID3D11Buffer       *g_pID3D11Buffer_ConstantBuffer_DomainShader = NULL;
ID3D11Buffer       *g_pID3D11Buffer_ConstantBuffer_PixelShader  = NULL;

struct CBUFFER_HULL_SHADER
{
	XMVECTOR Hull_Constant_Function_Params;
};

struct CBUFFER_DOMAIN_SHADER
{
	XMMATRIX WorldViewProjectionMatrix_sj;
};

struct CBUFFER_PIXEL_SHADER
{
	XMVECTOR LineColor;
};

unsigned int g_uiNumberOfLineSegments = 1;

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
	if (0 != fopen_s(&g_fLogger_sj, g_szLogFileName, "w"))
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
			if (false == g_boEscapeKeyIsPressed)
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

		case VK_UP:
			g_uiNumberOfLineSegments++;
			if (g_uiNumberOfLineSegments >= 50)
			{
				g_uiNumberOfLineSegments = 50;
			}

			break;

		case VK_DOWN:
			g_uiNumberOfLineSegments--;
			if (g_uiNumberOfLineSegments <= 0)
			{
				g_uiNumberOfLineSegments = 1;
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
		if (d3dDriverType_sj == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(g_fLogger_sj, "Hardware Type\n");
		}
		else if (d3dDriverType_sj == D3D_DRIVER_TYPE_WARP)
		{
			fprintf_s(g_fLogger_sj, "Warp Type\n");
		}
		else if (d3dDriverType_sj == D3D_DRIVER_TYPE_REFERENCE)
		{
			fprintf_s(g_fLogger_sj, "Reference Type. \n");
		}
		else
		{
			fprintf_s(g_fLogger_sj, "Unknown Type. \n");
		}

		fprintf_s(g_fLogger_sj, "The Supported Highest Feature Level Is ");
		if (D3D_FEATURE_LEVEL_11_0 == d3dFeatureLevel_aquired_sj)
		{
			fprintf_s(g_fLogger_sj, "11.0\n");
		}
		else if (D3D_FEATURE_LEVEL_10_1 == d3dFeatureLevel_aquired_sj)
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
		"struct vertex_output" \
		"{" \
		"float4 position : POSITION;" \
		"};" \

		"vertex_output main(float2 pos: POSITION)" \
		"{" \
			"vertex_output output;" \
			"output.position = float4(pos, 0.0f, 1.0f);" \
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
	if (FAILED(hr_sj))
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

	const char * hullShaderSourceCode = 
		"cbuffer ConstantBuffer" \
		"{" \
			"float4 hull_constant_function_params;" \
		"}" \

		"struct vertex_output" \
		"{" \
			"float4 position : POSITION;" \
		"};" \

		"struct hull_constant_output" \
		"{" \
			"float edges[2] : SV_TESSFACTOR;" \
		"};" \

		"hull_constant_output hull_constant_function(void)" \
		"{" \
			"hull_constant_output output;" \
			"float numberOfStrips = hull_constant_function_params[0];" \
			"float numberOfSegments = hull_constant_function_params[1];" \

			"output.edges[0] = numberOfStrips; " \
			"output.edges[1] = numberOfSegments; " \
			"return (output);" \

		"}" \

		"struct hull_output" \
		"{" \
			"float4 position : POSITION;" \
		"};" \
		"[domain(\"isoline\")]" \
		"[partitioning(\"integer\")]" \
		"[outputtopology(\"line\")]" \
		"[outputcontrolpoints(4)]" \
		"[patchconstantfunc(\"hull_constant_function\")]" \

		"hull_output main(InputPatch<vertex_output, 4> input_patch, uint i: SV_OUTPUTCONTROLPOINTID)" \
		"{" \
			"hull_output output;" \
			"output.position = input_patch[i].position;" \
			"return(output);" \
		"}";

	ID3DBlob *p_ID3DBlob_HullShaderCode = NULL;
	p_ID3DBlob_Error = NULL;
	hr_sj = D3DCompile(hullShaderSourceCode,
			lstrlenA(hullShaderSourceCode) + 1,
			"HS",
			NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"hs_5_0",
		0,
		0,
		&p_ID3DBlob_HullShaderCode,
		&p_ID3DBlob_Error
	);

	if (FAILED(hr_sj))
	{
		if (NULL != p_ID3DBlob_Error)
		{
			fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
			fprintf_s(
						g_fLogger_sj,
						"D3DCompile() failed for Hull Shader %s\n",
						(char *)p_ID3DBlob_Error->GetBufferPointer()
					);
			fclose(g_fLogger_sj);
			return(hr_sj);
		}
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "D3DCompile() failed for Hull Shader\n");
		fclose(g_fLogger_sj);
	}

	hr_sj = g_pID3D11Device_sj->CreateHullShader(
		p_ID3DBlob_HullShaderCode->GetBufferPointer(), p_ID3DBlob_HullShaderCode->GetBufferSize(),
		NULL, &g_pID3D11HullShader_sj);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "ID3D11Device::CreateHullShader() Failed \n");
		fclose(g_fLogger_sj);
		return(hr_sj);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateHullShader() Succedded\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->HSSetShader(g_pID3D11HullShader_sj, 0, 0);
	p_ID3DBlob_HullShaderCode->Release();
	p_ID3DBlob_HullShaderCode = NULL;

	// Domain Shader
	const char *domainShaderSourceCode = 
		"cbuffer ConstantBuffer" \
		"{" \
			"float4x4 worldViewProjectionMatrix;" \
		"}" \
		"struct hull_constant_output" \
		"{" \
		"float edges[2] : SV_TESSFACTOR;" \
		"};" \
		"struct hull_output" \
		"{" \
		"float4 position : POSITION;" \
		"};" \
		"struct domain_output" \
		"{" \
		"float4 position : SV_POSITION;" \
		"};" \

		"[domain(\"isoline\")]" \
		"domain_output main(hull_constant_output input, " \
			"OutputPatch<hull_output, 4> output_patch, float2 tessCoord: SV_DOMAINLOCATION)" \
		"{" \
			"domain_output output;" \
			"float u = tessCoord.x;" \
			"float3 p0 = output_patch[0].position.xyz;" \
			"float3 p1 = output_patch[1].position.xyz;" \
			"float3 p2 = output_patch[2].position.xyz;" \
			"float3 p3 = output_patch[3].position.xyz;" \
			"float3 u1 = (1.0f - u);" \
			"float3 u2 = u * u;" \
			"float3 b3 = u2 * u;" \
			"float3 b2 = 3.0f * u2 * u1;" \
			"float3 b1 = 3.0f * u * u1 *u1;" \
			"float3 b0 = u1 * u1 * u1;" \
			"float3 p = p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;" \
			"output.position = mul(worldViewProjectionMatrix, float4(p, 1.0f));" \
			"return output;" \
		"}";
	
	ID3DBlob *pID3DBlob_DomainShaderCode = NULL;
	p_ID3DBlob_Error = NULL;

	hr_sj = D3DCompile(domainShaderSourceCode,
				lstrlenA(domainShaderSourceCode) + 1,
				"DS",
				NULL,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main",
				"ds_5_0",
				0,
				0,
				&pID3DBlob_DomainShaderCode,
				&p_ID3DBlob_Error
			);
	if (FAILED(hr_sj))
	{
		if (NULL != p_ID3DBlob_Error)
		{
			fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
			fprintf_s(
				g_fLogger_sj,
				"D3DCompile() failed for Domain Shader %s\n",
				(char *)p_ID3DBlob_Error->GetBufferPointer()
			);
			fclose(g_fLogger_sj);
			return(hr_sj);
		}
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "D3DCompile() Succedded for Domain Shader\n");
		fclose(g_fLogger_sj);
	}

	hr_sj = g_pID3D11Device_sj->CreateDomainShader(pID3DBlob_DomainShaderCode->GetBufferPointer(),
		pID3DBlob_DomainShaderCode->GetBufferSize(), NULL, &g_pID3D11DomainShader_sj);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf_s(g_fLogger_sj, "ID3D11Device::CreateDomainShader() Failed\n");
		fclose(g_fLogger_sj);
		return(hr_sj);
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateDomainShader() Succedded\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->DSSetShader(g_pID3D11DomainShader_sj, 0, 0);
	pID3DBlob_DomainShaderCode->Release();
	pID3DBlob_DomainShaderCode = NULL;


	const char * pixelShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
			"float4 lineColor;" \
		"}" \
		"float4 main(void) : SV_TARGET" \
		"{" \
			"float4 color;" \
			"color = lineColor;" \
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
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
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

	// input layouts
	D3D11_INPUT_ELEMENT_DESC inputElementDesc;
	inputElementDesc.SemanticName = "POSITION";
	inputElementDesc.SemanticIndex = 0;
	inputElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc.InputSlot = 0;
	inputElementDesc.AlignedByteOffset = 0;
	inputElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc.InstanceDataStepRate = 0;

	hr_sj = g_pID3D11Device_sj->CreateInputLayout(&inputElementDesc,
		1,
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
		fprintf(g_fLogger_sj, "ID3D11Device::CreateInputLayout() Succedded\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->IASetInputLayout(g_pID3D11InputLayout_sj);
	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_VertexShaderCode = NULL;
	pID3DBlob_PixelShaderCode->Release();
	pID3DBlob_PixelShaderCode = NULL;

	float vertices[] =
	{
		-1.0f, -1.0f, -0.5f, 1.0f, 0.5f, -1.0f, 1.0f, 1.0f
	};

	// create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr_sj = g_pID3D11Device_sj->CreateBuffer(&bufferDesc, NULL,
		&g_pID3D11Buffer_VertexBuffer_sj);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() Failed\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() Succeded For Vertex Buffer()\n");
		fclose(g_fLogger_sj);
	}

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	g_pID3D11Device_Context_sj->Map(g_pID3D11Buffer_VertexBuffer_sj, NULL,
		D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, vertices, sizeof(vertices));
	g_pID3D11Device_Context_sj->Unmap(g_pID3D11Buffer_VertexBuffer_sj, NULL);

	// define and set the constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER_HULL_SHADER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr_sj = g_pID3D11Device_sj->CreateBuffer(&bufferDesc_ConstantBuffer, 0,
		&g_pID3D11Buffer_ConstantBuffer_HullShader);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() Failed for constant Buffer For Hull Shader\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj, "ID3D11Device::CreateBuffer() Succedded For Constant Buffer For Hull Shader\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->HSSetConstantBuffers(
		0, 1, &g_pID3D11Buffer_ConstantBuffer_HullShader);

	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER_DOMAIN_SHADER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr_sj = g_pID3D11Device_sj->CreateBuffer(&bufferDesc_ConstantBuffer, 0,
		&g_pID3D11Buffer_ConstantBuffer_DomainShader);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj,
			"ID3D11Device::CreateBuffer() Failed for constant Buffer For Domain Shader\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj,
			"ID3D11Device::CreateBuffer() Succedded For Constant Buffer For Domain Shader\n");
		fclose(g_fLogger_sj);
	}
	g_pID3D11Device_Context_sj->DSSetConstantBuffers(
		0, 1, &g_pID3D11Buffer_ConstantBuffer_DomainShader);

	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER_PIXEL_SHADER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr_sj = g_pID3D11Device_sj->CreateBuffer(&bufferDesc_ConstantBuffer, 0,
		&g_pID3D11Buffer_ConstantBuffer_PixelShader);
	if (FAILED(hr_sj))
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj,
			"ID3D11Device::CreateBuffer() Failed for constant Buffer For Pixel Shader\n");
		fclose(g_fLogger_sj);
		return hr_sj;
	}
	else
	{
		fopen_s(&g_fLogger_sj, g_szLogFileName, "a+");
		fprintf(g_fLogger_sj,
			"ID3D11Device::CreateBuffer() Succedded For Constant Buffer For Pixel Shader\n");
		fclose(g_fLogger_sj);
	}

	g_pID3D11Device_Context_sj->PSSetConstantBuffers(0, 1,
		&g_pID3D11Buffer_ConstantBuffer_PixelShader);

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

	return 0;
}

void Display(void)
{
	// initialization & declarations
	XMMATRIX wvpMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX worldMatrix;
	XMMATRIX translationMatrix;

	//XMMATRIX rotationMatrix;

	// code
	// clear render target view to a chosen color
	g_pID3D11Device_Context_sj->ClearRenderTargetView(g_pID3D11RenderTargetView_sj, g_ClearColor_sj);

	// select which vertex buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	g_pID3D11Device_Context_sj->IASetVertexBuffers(0, 1, &g_pID3D11Buffer_VertexBuffer_sj, &stride, &offset);

	// select geometry primitive
	g_pID3D11Device_Context_sj->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	// translation which is concerened with	world matri transformation
	translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 4.0f);

	worldMatrix = XMMatrixIdentity();
	viewMatrix = XMMatrixIdentity();

	worldMatrix = translationMatrix;

	// final WorldView Projection Matrix

	wvpMatrix = worldMatrix * viewMatrix * g_PerspectiveProjectionMatrix_sj;

	// load the data into the constant buffer
	CBUFFER_DOMAIN_SHADER constantBuffer_domainShader;
	constantBuffer_domainShader.WorldViewProjectionMatrix_sj = wvpMatrix;
	g_pID3D11Device_Context_sj->UpdateSubresource(
		g_pID3D11Buffer_ConstantBuffer_DomainShader, 0, NULL, &constantBuffer_domainShader, 0, 0);

	// load the data into constant buffer for hull shader
	CBUFFER_HULL_SHADER constantBuffer_hullShader;
	constantBuffer_hullShader.Hull_Constant_Function_Params =
		XMVectorSet(1.0f, (FLOAT)g_uiNumberOfLineSegments, 0.0f, 0.0f);

	g_pID3D11Device_Context_sj->UpdateSubresource(g_pID3D11Buffer_ConstantBuffer_HullShader,
		0, NULL, &constantBuffer_hullShader, 0, 0);

	TCHAR str[255];
	wsprintf(str, TEXT("Direct3D11 Window [ Segments = %2d]"), g_uiNumberOfLineSegments);
	SetWindowText(g_hwnd, str);

	CBUFFER_PIXEL_SHADER constantBuffer_pixelShader;
	constantBuffer_pixelShader.LineColor = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f); // Yellow

	g_pID3D11Device_Context_sj->UpdateSubresource(
		g_pID3D11Buffer_ConstantBuffer_PixelShader, 0, NULL,
		&constantBuffer_pixelShader, 0, 0);
	g_pID3D11Device_Context_sj->Draw(4, 0);
	g_pIDXGISwapChain->Present(0, 0);
}

HRESULT Resize(int iwidth, int iheight)
{
	HRESULT hr;

	// code
	if (g_pID3D11RenderTargetView_sj)
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

	if (FAILED(hr))
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

	g_pID3D11Device_Context_sj->OMSetRenderTargets(1, &g_pID3D11RenderTargetView_sj, NULL);

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

	if (g_pID3D11Device_sj)
	{
		g_pID3D11Device_sj->Release();
		g_pID3D11Device_sj = NULL;
	}

	if (g_fLogger_sj)
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
