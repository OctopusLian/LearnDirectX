#ifndef _MYD3DLIB_H
#define _MYD3DLIB_H

extern LPDIRECT3D9             g_pD3D; // Used to create the D3DDevice
extern LPDIRECT3DDEVICE9       g_pd3dDevice; // Our rendering device

int D3D_Init();
int D3D_Uninit();
int Render();

#endif //_MYD3DLIB_H