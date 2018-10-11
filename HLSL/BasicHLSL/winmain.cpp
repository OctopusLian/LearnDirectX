#include "winmain.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "d3dlib.h"

#define ReleaseCOM(x) {if(x!=NULL) x->Release(); x=NULL;}

HWND                             g_hWnd;
HINSTANCE                       g_hInst;

IDirect3DVertexShader9* BasicShader = 0;  //定点着色器指针
ID3DXConstantTable* BasicConstTable = 0;

D3DXHANDLE WVPMatrixHandle          = 0;  //设置句柄
D3DXHANDLE ColorHandle              = 0;  //设置句柄

ID3DXMesh* Teapot                   = 0;  //指向程序中D3D茶壶模型的指针

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
	HWND hWnd = CreateWindow("Direct3D", "BasicHLSL", 
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

int Game_Init()
{
	D3D_Init();

	D3DXCreateTeapot(g_pd3dDevice, &Teapot, 0);

	ID3DXBuffer* shaderBuffer      = 0;
	ID3DXBuffer* errorBuffer       = 0;

	HRESULT hr = D3DXCompileShaderFromFile("BasicHLSL.txt",
										   0,
										   0,
										   "SetColor", // 入口函数名称
										   "vs_1_1",  //顶点着色器版本号
										   D3DXSHADER_DEBUG,  //DEBUG模式编译
										   &shaderBuffer,  //指向编译后的着色器代码的指针
										   &errorBuffer,
										   &BasicConstTable);  //常量表指针

	// output any error messages
	if(errorBuffer)
	{
		MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		ReleaseCOM(errorBuffer);
		exit(0);
	}

	if(FAILED(hr))
	{
		MessageBox(0, "D3DXCompileShaderFromFile() - FAILED", 0, 0);
		return false;
	}

	hr = g_pd3dDevice->CreateVertexShader((DWORD*)shaderBuffer->GetBufferPointer(), &BasicShader);

	if(FAILED(hr))
	{
		MessageBox(0, "CreateVertexShader - FAILED", 0, 0);
		return false;
	}

	ReleaseCOM(shaderBuffer);

	//通过变量名分别得到对应的两个句柄
	WVPMatrixHandle = BasicConstTable->GetConstantByName(0, "WVPMatrix");  
	ColorHandle = BasicConstTable->GetConstantByName(0, "color");

	//通过句柄对着色器变量进行赋值
	BasicConstTable->SetDefaults(g_pd3dDevice);  //先设置各变量为默认值

	return 1;
}

//渲染代码如下
int Game_Main()
{
	//设定顶点的变换坐标和颜色值
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(153,153,153), 1.0f, 0 );
	g_pd3dDevice->BeginScene();  //开始渲染

	//得到世界矩阵、观察矩阵和投影矩阵
	D3DXMATRIX matWorld, matView, matProj;
	g_pd3dDevice->GetTransform(D3DTS_WORLD, &matWorld);
	g_pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
	g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);

	D3DXMATRIX matWVP = matWorld * matView * matProj;
	//通过句柄对着色器中的WVPMatrix 变量进行赋值
	BasicConstTable->SetMatrix(g_pd3dDevice, WVPMatrixHandle, &matWVP);
	
	D3DXVECTOR4 color(1.0f, 1.0f, 0.0f, 1.0f);
	//通过句柄对着色器中的color变量进行赋值，这里我们赋值为黄色
	BasicConstTable->SetVector(g_pd3dDevice, ColorHandle, &color);

	//把顶点着色器设定到渲染管道中
	g_pd3dDevice->SetVertexShader(BasicShader);

	//绘制模型子集
	Teapot->DrawSubset(0);

	//渲染完毕
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

	return 1;
}

int Game_Uninit()
{
    ReleaseCOM(BasicShader);
    ReleaseCOM(BasicConstTable);

	ReleaseCOM(Teapot);

	D3D_Uninit();

	return 1;
}

