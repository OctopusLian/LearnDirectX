#include "winmain.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "d3dlib.h"
#include "stdio.h"

#define ReleaseCOM(x) {if(x!=NULL) x->Release(); x=NULL;}

struct CUSTOMVERTEX
{
	float x,y,z;
	float tu0, tv0;
	float tu1, tv1;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_TEX2)

HWND                             g_hWnd;
HINSTANCE                       g_hInst;

ID3DXEffect *g_pEffect              = 0;

D3DXHANDLE WVPMatrixHandle          = 0;
D3DXHANDLE ScalarHandle             = 0;
D3DXHANDLE Tex0Handle               = 0;
D3DXHANDLE Tex1Handle               = 0;
D3DXHANDLE TechHandle               = 0;

LPDIRECT3DVERTEXBUFFER9 quadVB  = NULL;
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
	HWND hWnd = CreateWindow("Direct3D", "Effect", 
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

	//set up vertex buffer
	CUSTOMVERTEX quad[] = 
	//  x      y      z    tu0   tv0   tu1   tv1 
	{{-3.0f, -3.0f, 10.0f, 0.0f, 1.0f, 0.0f, 1.0f},
	{ -3.0f,  3.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f},
	{  3.0f, -3.0f, 10.0f, 1.0f, 1.0f, 1.0f, 1.0f},
	{  3.0f,  3.0f, 10.0f, 1.0f, 0.0f, 1.0f, 0.0f}};

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
	D3DXCreateTextureFromFile(g_pd3dDevice, "chopper.bmp", &quadTexture0);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Bleach.jpg", &quadTexture1);

	//detect if pixel shader is supported by the device
	D3DCAPS9 caps;
	g_pd3dDevice->GetDeviceCaps(&caps);
	if(caps.PixelShaderVersion < D3DPS_VERSION(1, 1))
	{
		MessageBox(0, "NotSupport Pixel Shader - FAILED", 0, 0);
		exit(0);
	}

	//create effect
	ID3DXBuffer* errorBuffer       = 0;

	HRESULT hr = D3DXCreateEffectFromFile(g_pd3dDevice,
										  "Effect.txt",
										  0,
										  0,
										  D3DXSHADER_DEBUG,
										  0,
										  &g_pEffect,
										  &errorBuffer);

	// output any error messages
	if(errorBuffer)
	{
		MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		ReleaseCOM(errorBuffer);
		exit(0);
	}

	if(FAILED(hr))
	{
		MessageBox(0, "D3DXCreateEffectFromFile() - FAILED", 0, 0);
		return false;
	}

	WVPMatrixHandle = g_pEffect->GetParameterByName(0, "WVPMatrix");
	ScalarHandle = g_pEffect->GetParameterByName(0, "Scalar");
	Tex0Handle = g_pEffect->GetParameterByName(0, "Tex0");
	Tex1Handle = g_pEffect->GetParameterByName(0, "Tex1");

	TechHandle = g_pEffect->GetTechniqueByName("T0");

	g_pEffect->SetTexture(Tex0Handle, quadTexture0);
	g_pEffect->SetTexture(Tex1Handle, quadTexture1);

	return 1;
}

int Game_Main()
{
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(153,153,153), 1.0f, 0 );
	g_pd3dDevice->BeginScene();

	D3DXMATRIX matWorld, matView, matProj;
	g_pd3dDevice->GetTransform(D3DTS_WORLD, &matWorld);
	g_pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
	g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);

	D3DXMATRIX matWVP = matWorld * matView * matProj;
	g_pEffect->SetMatrix(WVPMatrixHandle, &matWVP);
	
	D3DXVECTOR4 scalar(0.5f, 0.5f, 0.0f, 1.0f);
	g_pEffect->SetVector(ScalarHandle, &scalar);
	
	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	g_pd3dDevice->SetStreamSource(0, quadVB, 0, sizeof(CUSTOMVERTEX));

	g_pEffect->SetTechnique(TechHandle);

	UINT numPasses = 0;
	//begin technique
	g_pEffect->Begin(&numPasses, 0);
	for(UINT i = 0; i<numPasses; ++i)
	{
		//begin pass
		g_pEffect->BeginPass(i);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		//end pass
		g_pEffect->EndPass();
	}
	//end technique
	g_pEffect->End();

	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

	return 1;
}

int Game_Uninit()
{
	ReleaseCOM(quadVB);
	ReleaseCOM(quadTexture0); 
	ReleaseCOM(quadTexture1);
	ReleaseCOM(g_pEffect);

	D3D_Uninit();

	return 1;
}

