#include "winmain.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "d3dlib.h"

#define ReleaseCOM(x) {if(x!=NULL) x->Release(); x=NULL;}

HWND                                  g_hWnd;
HINSTANCE                            g_hInst;

//mesh parameters
LPD3DXMESH                      g_SourceMesh;
LPD3DXMESH				  	    g_TargetMesh;
D3DMATERIAL9              *g_pMeshMaterials0;
LPDIRECT3DTEXTURE9         *g_pMeshTextures0;
DWORD                        g_dwNumSubsets0;
D3DMATERIAL9              *g_pMeshMaterials1;
LPDIRECT3DTEXTURE9         *g_pMeshTextures1;
DWORD                        g_dwNumSubsets1;

//vertex declaration
IDirect3DVertexDeclaration9   *g_Decl = NULL;

//vertex shader parameters
IDirect3DVertexShader9        *g_VS   = NULL;
ID3DXConstantTable* ConstTable = NULL;

D3DXHANDLE WVPMatrixHandle          = 0;
D3DXHANDLE ScalarHandle                  = 0;
D3DXHANDLE LightDirHandle                = 0;

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT);
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int Game_Init();
int Game_Main();
int Game_Uninit();
int Load_Meshes();
int DrawMesh(LPD3DXMESH pMesh, LPDIRECT3DTEXTURE9 *pTextures, IDirect3DVertexShader9 *pShader, IDirect3DVertexDeclaration9 *pDecl);

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// Register the window class.
	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
					 GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					 "Direct3D", NULL};

	RegisterClassEx(&wc);

	// Create the application's window.
	HWND hWnd = CreateWindow("Direct3D", "vertex shader - ½¥±ä¶¯»­", 
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

	Load_Meshes();

	D3DVERTEXELEMENT9 MorphMeshDecl[] =
	{
	  //1st stream is for source mesh - position, normal, texcoord
	  { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	  { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
	  { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

	  //2nd stream is for target mesh - position, normal
	  { 1,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
	  { 1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   1 },
	  D3DDECL_END()
	};

	ID3DXBuffer* shader      = NULL;
	ID3DXBuffer* errorBuffer = NULL;
	D3DXCompileShaderFromFile("vs.txt",
							  0,
							  0,
						      "Main", // entry point function name
							  "vs_1_1",
						  	  D3DXSHADER_DEBUG,
						      &shader,
							  &errorBuffer,
						   	  &ConstTable);
	
	if(errorBuffer)
	{
		::MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		ReleaseCOM(errorBuffer);
	}

	g_pd3dDevice->CreateVertexShader((DWORD*)shader->GetBufferPointer(), &g_VS);
	g_pd3dDevice->CreateVertexDeclaration(MorphMeshDecl ,&g_Decl);

	WVPMatrixHandle = ConstTable->GetConstantByName(0, "WVPMatrix");
	ScalarHandle = ConstTable->GetConstantByName(0, "Scalar");
	LightDirHandle = ConstTable->GetConstantByName(0, "LightDirection");

	ConstTable->SetVector(g_pd3dDevice, LightDirHandle, &D3DXVECTOR4(0.0f, -1.0f, 0.0f, 0.0f));
	ConstTable->SetDefaults(g_pd3dDevice);

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
	D3DXMATRIX matWVP;
	matWVP = matWorld * matView * matProj;

	ConstTable->SetMatrix(g_pd3dDevice, WVPMatrixHandle, &matWVP);

	float DolphinTimeFactor = (float)(timeGetTime() % 501) / 250.0f;
	float Scalar = (DolphinTimeFactor<=1.0f)?DolphinTimeFactor:(2.0f-DolphinTimeFactor);
	ConstTable->SetVector(g_pd3dDevice,ScalarHandle,&D3DXVECTOR4(1.0f-Scalar, Scalar, 0.0f, 0.0f));

	g_pd3dDevice->SetVertexShader(g_VS);
	g_pd3dDevice->SetVertexDeclaration(g_Decl);

	IDirect3DVertexBuffer9 *pVB = NULL;
	g_TargetMesh->GetVertexBuffer(&pVB);
	g_pd3dDevice->SetStreamSource(1, pVB, 0, D3DXGetFVFVertexSize(g_TargetMesh->GetFVF()));
	ReleaseCOM(pVB);

	g_SourceMesh->GetVertexBuffer(&pVB);
	g_pd3dDevice->SetStreamSource(0, pVB, 0, D3DXGetFVFVertexSize(g_TargetMesh->GetFVF()));
	ReleaseCOM(pVB);

	DrawMesh(g_SourceMesh, g_pMeshTextures0, g_VS, g_Decl);

	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	return 1;
}

int Game_Uninit()
{
	for (DWORD i = 0; i < g_dwNumSubsets0; i++)
		ReleaseCOM(g_pMeshTextures0[i]);
	delete [] g_pMeshMaterials0;
	delete [] g_pMeshTextures0;
	ReleaseCOM(g_SourceMesh);

	for (int i = 0; i < g_dwNumSubsets1; i++)
		ReleaseCOM(g_pMeshTextures1[i]);
	delete [] g_pMeshMaterials1;
	delete [] g_pMeshTextures1;
	ReleaseCOM(g_TargetMesh);

	ReleaseCOM(g_VS);
	ReleaseCOM(ConstTable);

	D3D_Uninit();

	return 1;
}

int Load_Meshes()
{
	//load source mesh
	LPD3DXBUFFER pD3DXMtrlBuffer;
	::D3DXLoadMeshFromX("Dolphin1.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice, 
						NULL, &pD3DXMtrlBuffer, NULL,
						&g_dwNumSubsets0,
						&g_SourceMesh);

	g_pMeshMaterials0 = new D3DMATERIAL9[g_dwNumSubsets0];
	g_pMeshTextures0 = new LPDIRECT3DTEXTURE9[g_dwNumSubsets0];

	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	for(DWORD i=0; i < g_dwNumSubsets0; i++)
	{
		//copy materials
		g_pMeshMaterials0[i] = d3dxMaterials[i].MatD3D;
		g_pMeshMaterials0[i].Ambient = g_pMeshMaterials0[i].Diffuse;

		//load texture
		::D3DXCreateTextureFromFile(g_pd3dDevice,
									d3dxMaterials[i].pTextureFilename,
									&g_pMeshTextures0[i] );
	}

	//Optimize the mesh for better attribute access
	g_SourceMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT, NULL, NULL, NULL, NULL);

	ReleaseCOM(pD3DXMtrlBuffer);

	//load source mesh
	pD3DXMtrlBuffer;
	::D3DXLoadMeshFromX("Dolphin3.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice, 
						NULL, &pD3DXMtrlBuffer, NULL,
						&g_dwNumSubsets1,
						&g_TargetMesh);

	g_pMeshMaterials1 = new D3DMATERIAL9[g_dwNumSubsets1];
	g_pMeshTextures1 = new LPDIRECT3DTEXTURE9[g_dwNumSubsets1];

	d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	for(int i=0; i < g_dwNumSubsets1; i++)
	{
		//copy materials
		g_pMeshMaterials1[i] = d3dxMaterials[i].MatD3D;
		g_pMeshMaterials1[i].Ambient = g_pMeshMaterials1[i].Diffuse;

		//load texture
		::D3DXCreateTextureFromFile(g_pd3dDevice,
									d3dxMaterials[i].pTextureFilename,
									&g_pMeshTextures1[i] );
	}

	//Optimize the mesh for better attribute access
	g_TargetMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT, NULL, NULL, NULL, NULL);

	ReleaseCOM(pD3DXMtrlBuffer);

	return 1;
}

int DrawMesh(LPD3DXMESH pMesh,
			 LPDIRECT3DTEXTURE9 *pTextures,
             IDirect3DVertexShader9 *pShader, 
             IDirect3DVertexDeclaration9 *pDecl)
{
	IDirect3DVertexBuffer9 *pVB = NULL;
	IDirect3DIndexBuffer9 *pIB  = NULL;
	pMesh->GetVertexBuffer(&pVB);
	pMesh->GetIndexBuffer(&pIB);

	//Get attribute table
	DWORD NumAttributes;
	D3DXATTRIBUTERANGE *pAttributes = NULL;
	pMesh->GetAttributeTable(NULL, &NumAttributes);
	pAttributes = new D3DXATTRIBUTERANGE[NumAttributes];
	pMesh->GetAttributeTable(pAttributes, &NumAttributes);

	//Use the vertex shader interface passed
	g_pd3dDevice->SetVertexShader(pShader);
	g_pd3dDevice->SetVertexDeclaration(pDecl);

	//Set stream sources
	g_pd3dDevice->SetStreamSource(0, pVB, 0, D3DXGetFVFVertexSize(pMesh->GetFVF()));
	g_pd3dDevice->SetIndices(pIB);

	//Go through each attribute group and render
	for(DWORD i=0;i<NumAttributes;i++)
	{
		if(pAttributes[i].FaceCount)
		{
			//Get material number
			DWORD MatNum = pAttributes[i].AttribId;

			//Set texture
			g_pd3dDevice->SetTexture(0, pTextures[MatNum]);

			//Draw the mesh subset
			g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
											 pAttributes[i].VertexStart,
											 pAttributes[i].VertexCount,
											 pAttributes[i].FaceStart * 3,
											 pAttributes[i].FaceCount);
		}
	}
	
	//Free resources
	ReleaseCOM(pVB);
	ReleaseCOM(pIB);
	delete [] pAttributes;

	return 1;
}