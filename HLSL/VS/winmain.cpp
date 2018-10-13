#include "winmain.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "d3dlib.h"

#define ReleaseCOM(x) {if(x!=NULL) x->Release(); x=NULL;}

HWND                                  g_hWnd;
HINSTANCE                            g_hInst;

//mesh parameters
//两个指向LPD3DXMESH的指针，分别用于存储源网络模型和目标网络模型
LPD3DXMESH                      g_SourceMesh;
LPD3DXMESH				  	    g_TargetMesh;

D3DMATERIAL9              *g_pMeshMaterials0;
LPDIRECT3DTEXTURE9         *g_pMeshTextures0;
DWORD                        g_dwNumSubsets0;
D3DMATERIAL9              *g_pMeshMaterials1;
LPDIRECT3DTEXTURE9         *g_pMeshTextures1;
DWORD                        g_dwNumSubsets1;

//vertex declaration
//顶点声明指针
IDirect3DVertexDeclaration9   *g_Decl = NULL;

//vertex shader parameters
//顶点着色器
IDirect3DVertexShader9        *g_VS   = NULL;
//常量表
ID3DXConstantTable* ConstTable = NULL;

//常量句柄
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
	HWND hWnd = CreateWindow("Direct3D", "vertex shader - 渐变动画", 
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

//声明变量
int Game_Init()
{
	D3D_Init();

	//加载源、目标网络模型
	Load_Meshes();

	//顶点声明
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

	//创建顶点着色器
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

	//创建顶点着色器
	g_pd3dDevice->CreateVertexShader((DWORD*)shader->GetBufferPointer(), &g_VS);
	//创建顶点声明
	g_pd3dDevice->CreateVertexDeclaration(MorphMeshDecl ,&g_Decl);

	//得到各常量句柄
	WVPMatrixHandle = ConstTable->GetConstantByName(0, "WVPMatrix");
	ScalarHandle = ConstTable->GetConstantByName(0, "Scalar");
	LightDirHandle = ConstTable->GetConstantByName(0, "LightDirection");

	//为着色器全局变量LightDirection赋值
	ConstTable->SetVector(g_pd3dDevice, LightDirHandle, &D3DXVECTOR4(0.0f, -1.0f, 0.0f, 0.0f));
	//设置各着色器变量为默认值
	ConstTable->SetDefaults(g_pd3dDevice);

	return 1;
}

//渲染
int Game_Main()
{

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(153,153,153), 1.0f, 0 );
	g_pd3dDevice->BeginScene();

	//为着色器全局变量WVPMatrix赋值
	D3DXMATRIX matWorld, matView, matProj;
	g_pd3dDevice->GetTransform(D3DTS_WORLD, &matWorld);
	g_pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
	g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);
	D3DXMATRIX matWVP;
	matWVP = matWorld * matView * matProj;

	ConstTable->SetMatrix(g_pd3dDevice, WVPMatrixHandle, &matWVP);

	//为着色器全局变量Scalar赋值，注意程序中获取时间标尺的方法
	float DolphinTimeFactor = (float)(timeGetTime() % 501) / 250.0f;
	float Scalar = (DolphinTimeFactor<=1.0f)?DolphinTimeFactor:(2.0f-DolphinTimeFactor);
	ConstTable->SetVector(g_pd3dDevice,ScalarHandle,&D3DXVECTOR4(1.0f-Scalar, Scalar, 0.0f, 0.0f));

	//设置顶点着色器和顶点声明
	g_pd3dDevice->SetVertexShader(g_VS);
	g_pd3dDevice->SetVertexDeclaration(g_Decl);

	//绑定目标网络模型的顶点缓存到第二个数据流中
	IDirect3DVertexBuffer9 *pVB = NULL;
	g_TargetMesh->GetVertexBuffer(&pVB);
	g_pd3dDevice->SetStreamSource(1, pVB, 0, D3DXGetFVFVertexSize(g_TargetMesh->GetFVF()));
	ReleaseCOM(pVB);

	//绑定目标网络模型的顶点缓存到第一个数据流中
	g_SourceMesh->GetVertexBuffer(&pVB);
	g_pd3dDevice->SetStreamSource(0, pVB, 0, D3DXGetFVFVertexSize(g_TargetMesh->GetFVF()));
	ReleaseCOM(pVB);

	//绘制Mesh网络模型
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


//注意程序是如何配合属性表调用DrawIndexedPrimitive方法进行绘制的
int DrawMesh(LPD3DXMESH pMesh,
			 LPDIRECT3DTEXTURE9 *pTextures,
             IDirect3DVertexShader9 *pShader, 
             IDirect3DVertexDeclaration9 *pDecl)
{
	//分别得到指向Mesh模型顶点缓冲区和索引缓冲区的指针
	IDirect3DVertexBuffer9 *pVB = NULL;
	IDirect3DIndexBuffer9 *pIB  = NULL;
	pMesh->GetVertexBuffer(&pVB);
	pMesh->GetIndexBuffer(&pIB);

	//Get attribute table
	//得到Mesh模型的属性列表
	DWORD NumAttributes;
	D3DXATTRIBUTERANGE *pAttributes = NULL;
	pMesh->GetAttributeTable(NULL, &NumAttributes);
	pAttributes = new D3DXATTRIBUTERANGE[NumAttributes];
	pMesh->GetAttributeTable(pAttributes, &NumAttributes);

	//Use the vertex shader interface passed
	//设置顶点着色器和顶点声明
	g_pd3dDevice->SetVertexShader(pShader);
	g_pd3dDevice->SetVertexDeclaration(pDecl);

	//Set stream sources
	//设置数据流
	g_pd3dDevice->SetStreamSource(0, pVB, 0, D3DXGetFVFVertexSize(pMesh->GetFVF()));
	g_pd3dDevice->SetIndices(pIB);

	//Go through each attribute group and render
	//遍历属性列表并配合其中的信息调用DrawIndexedPrimitive绘制各个子集
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