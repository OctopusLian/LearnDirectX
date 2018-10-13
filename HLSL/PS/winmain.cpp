#include "winmain.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "d3dlib.h"

#define ReleaseCOM(x) {if(x!=NULL) x->Release(); x=NULL;}

struct CUSTOMVERTEX
{
	//定点位置坐标
	float x,y,z;
	//两套纹理坐标
	float tu0, tv0;
	float tu1, tv1;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_TEX2)

HWND                             g_hWnd;
HINSTANCE                       g_hInst;

//*******声明变量********
//pixel shader parameters
//顶点着色器
LPDIRECT3DPIXELSHADER9 pixelShader   = 0;
//常量表
ID3DXConstantTable* pixelConstTable  = 0;

//常量句柄
D3DXHANDLE ScalarHandle              = 0;
D3DXHANDLE Samp0Handle                = 0;
D3DXHANDLE Samp1Handle                = 0;

//常量描述结构
D3DXCONSTANT_DESC Samp0Desc;
D3DXCONSTANT_DESC Samp1Desc;

//四边形顶点缓存
LPDIRECT3DVERTEXBUFFER9 quadVB  = NULL;
//两个纹理
LPDIRECT3DTEXTURE9 quadTexture0 = NULL; 
LPDIRECT3DTEXTURE9 quadTexture1 = NULL;

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT);
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int Game_Init();
int Game_Main();
int Game_Uninit();

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// Register the window class.
	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
					 GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					 "Direct3D", NULL};

	RegisterClassEx(&wc);

	// Create the application's window.
	HWND hWnd = CreateWindow("Direct3D", "pixel shader - 混和纹理", 
							 WS_OVERLAPPEDWINDOW, 200, 150, 640, 480,
	                         GetDesktopWindow(), NULL, wc.hInstance, NULL);
	g_hWnd =hWnd;
	g_hInst = hInst;

	Game_Init();

	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);

	// The message loop
	MSG msg; 
	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Game_Main();
	}

	Game_Uninit();

	//Unregister the window class
	UnregisterClass("Direct3D", hInst);
	return 0;
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

//*******初始化应用程序*********
int Game_Init()
{
	D3D_Init();

	//set up vertex buffer
	//创建四边形顶点模拟
	CUSTOMVERTEX quad[] = 
	//  x      y      z    tu0   tv0   tu1   tv1 
	{{-3.0f, -3.0f, 10.0f, 0.0f, 1.0f, 0.0f, 1.0f},
	{ -3.0f,  3.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f},
	{  3.0f, -3.0f, 10.0f, 1.0f, 1.0f, 1.0f, 1.0f},
	{  3.0f,  3.0f, 10.0f, 1.0f, 0.0f, 1.0f, 0.0f}};

	//创建顶点缓存
	void *ptr = NULL;
	g_pd3dDevice->CreateVertexBuffer(sizeof(quad),
									 D3DUSAGE_WRITEONLY,
									 0,
									 D3DPOOL_MANAGED,
									 &quadVB,
									 NULL);
	quadVB->Lock(0, 0, (void**)&ptr, 0);
	memcpy((void*)ptr, (void*)quad, sizeof(quad));
	quadVB->Unlock();

	//set up texture 
	//创建纹理
	//读这个纹理上传到显存里去
	D3DXCreateTextureFromFile(g_pd3dDevice, "porpcart.jpg", &quadTexture0);
	D3DXCreateTextureFromFile(g_pd3dDevice, "luoqi.jpg", &quadTexture1);

	//检测系统是否支持像素着色器
	//detect if pixel shader is supported by the device
	D3DCAPS9 caps;
	g_pd3dDevice->GetDeviceCaps(&caps);
	if(caps.PixelShaderVersion < D3DPS_VERSION(1, 0))
	{
		MessageBox(0, "NotSupport Pixel Shader - FAILED", 0, 0);
		exit(0);
	}

	//create pixel shader
	//创建像素着色器
	ID3DXBuffer* codeBuffer        = 0;
	ID3DXBuffer* errorBuffer       = 0;

	HRESULT hr = D3DXCompileShaderFromFile("ps.txt",
										   0,
										   0,
										   "PS_Main", // entry point function name
										   "ps_2_0",
										   D3DXSHADER_DEBUG,
										   &codeBuffer,
										   &errorBuffer,
										   &pixelConstTable);

	// output any error messages
	if(errorBuffer)
	{
		MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		ReleaseCOM(errorBuffer);
	}

	if(FAILED(hr))
	{
		MessageBox(0, "D3DXCompileShaderFromFile() - FAILED", 0, 0);
		return false;
	}


	hr = g_pd3dDevice->CreatePixelShader((DWORD*)codeBuffer->GetBufferPointer(), &pixelShader);

	if(FAILED(hr))
	{
		MessageBox(0, "CreatePixelShader - FAILED", 0, 0);
		return false;
	}

	ReleaseCOM(codeBuffer);
	ReleaseCOM(errorBuffer);

	//得到各常量句柄
	ScalarHandle = pixelConstTable->GetConstantByName(0, "Scalar");
	Samp0Handle = pixelConstTable->GetConstantByName(0, "Samp0");
	Samp1Handle = pixelConstTable->GetConstantByName(0, "Samp1");

	//得到对着色器变量Samp0，Samp1的描述
	UINT count;
	pixelConstTable->GetConstantDesc(Samp0Handle, &Samp0Desc, &count);
	pixelConstTable->GetConstantDesc(Samp1Handle, &Samp1Desc, &count);

	//设定各着色器变量为初始值
	pixelConstTable->SetDefaults(g_pd3dDevice);

	return 1;
}

//***********渲染***********
int Game_Main()
{
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(153,153,153), 1.0f, 0 );
	g_pd3dDevice->BeginScene();

	//为着色器全局变量Scalar赋值
	D3DXVECTOR4 scalar(0.5f, 0.5f, 0.0f, 1.0f);
	pixelConstTable->SetVector(g_pd3dDevice, ScalarHandle, &scalar);

	//设置像素着色器
	g_pd3dDevice->SetPixelShader(pixelShader);

	//设置定点格式，绑定数据流
	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	g_pd3dDevice->SetStreamSource(0, quadVB, 0, sizeof(CUSTOMVERTEX));

	//set texture
	//设置第一、二层纹理
	g_pd3dDevice->SetTexture(Samp0Desc.RegisterIndex, quadTexture0);
	g_pd3dDevice->SetTexture(Samp1Desc.RegisterIndex, quadTexture1);

	//绘制图形
	g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

	return 1;
}

int Game_Uninit()
{
	ReleaseCOM(pixelShader);
	ReleaseCOM(pixelConstTable);
	ReleaseCOM(quadVB);
	ReleaseCOM(quadTexture0);
	ReleaseCOM(quadTexture1);

	D3D_Uninit();

	return 1;
}