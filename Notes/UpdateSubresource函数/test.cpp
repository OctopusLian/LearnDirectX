void UpdateSubresource(  
  [in]  ID3D11Resource *pDstResource,   //要贴纹理的目标
  [in]  UINT DstSubresource,  
  [in]  const D3D11_BOX *pDstBox, // 这个用来描述要覆盖目标的那个区域  
  [in]  const void *pSrcData, // 数据源，可以理解为内存  
  [in]  UINT SrcRowPitch, // 数据源行间距  
  [in]  UINT SrcDepthPitch // 数据源深度间距  
);  

//test.cpp
HRESULT CTex_DX11::LoadTex(const TP_BOX *pDstBox, const void *pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch)  //加载纹理
{
	if (pDstBox==NULL || pSrcData==NULL)  //如果覆盖目标区域或数据源为空
		return E_POINTER;
	ID3D11Device* pDevice = GetDevice11FromControlCenter();
	if(!pDevice)  //如果设备不存在
		return E_POINTER;

	ID3D11DeviceContext *pDeviceContext = NULL;
	pDevice->GetImmediateContext(&pDeviceContext);
	if (m_pTex==NULL || pDeviceContext==NULL)
		return E_POINTER;

	HRESULT hResult = S_OK;
	TP_D3D11TEXTURE_DESC desc;
	GetDesc(&desc);
	if (desc.eUsage==D3D11_USAGE_IMMUTABLE || (desc.uBindFlags&D3D11_BIND_DEPTH_STENCIL) || desc.uSampleCount>1)
		hResult = E_FAIL;
	else
	{
		D3D11_BOX box = {pDstBox->left, pDstBox->top, pDstBox->front, pDstBox->right, pDstBox->bottom, pDstBox->back};  //立方体的六个面
		pDeviceContext->UpdateSubresource(m_pTex, D3D11CalcSubresource(0,0,desc.uMipLevels), &box, pSrcData, SrcRowPitch, SrcDepthPitch);  //&box为覆盖目标区域，uMipLevels给纹理加入层级
		pDeviceContext->Flush();  //更新
	}

	pDeviceContext->Release();  //释放
	return S_OK;
}
