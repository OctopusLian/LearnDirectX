## �Ըú��������  
- CPU�����ݴ��ڴ渴�Ƶ�����ӳ���ڴ��д���������Դ��  
- ���»�������  
- Ӧ�÷���֮һ�� ��̬������д�����ݡ�  
- test.cpp�ļ���UpdateSubresource����Ӧ�õ�һ�����ӡ�  


�������˾����������Ǳ�������Ⱦ��ʱ�������д�ڳ��������������GPU�Ϳ��Զ�ȡ�����ˡ�Ҫ���»����������ǿ���ʹ��`ID3D11DeviceContext::UpdateSubresource()` API������һ��ָ�봫�ݸ��洢������ɫ���ĳ���������˳����ͬ�ľ���  

```c++
    //
    // Update variables that change once per frame
    //
    CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose( g_World );
    cb.vMeshColor = g_vMeshColor;
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );  
```  
#### ע��  
1����Դ���ǿ������Ϊ���Դ档  
2��Row Pitchָ�ڴ��ȣ�Widthָ��Ч��ȡ�

#### �ο����ϣ�  
[΢��ٷ��ĵ�-UpdateSubresource](https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/nf-d3d11-id3d11devicecontext-updatesubresource)  
[΢��ٷ��ĵ�-D3D11CalcSubresource](https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/nf-d3d11-d3d11calcsubresource)   
![Row_Depth_Pitch](https://github.com/OctopusLian/LearnDirectX/blob/master/Notes/UpdateSubresource%20function/Row_Depth_Pitch.png)  
�������£�  
������ӻ��е�ÿ�������һ������Ԫ�أ�ÿ��Ԫ�صĴ�Сȡ������Դ�ĸ�ʽ�����磬�����Դ��ʽ��DXGI_FORMAT_R32G32B32A32_FLOAT����ÿ��Ԫ�صĴ�СΪ128λ����16�ֽڣ�1���ֽ�=8λ����  
���3D�������Ŀ��Ϊ2���߶�Ϊ3�����Ϊ4��    
Ҫ���������Դ��Դ�м���Դ��ȼ�࣬ʹ�����¹�ʽ:  
**��Դ�м��=[һ��Ԫ�ص��ֽڴ�С] * [һ���е�Ԫ������]**   
**��Դ��ȼ��=[Դ�м��]*[����(��)]**  
�ڱ���3D��������У�ÿ��Ԫ�صĴ�СΪ16�ֽڣ���ʽ����:  
��Դ�м��= 16���ֽڣ� * 2��Width�� = 32�ֽ�  
��Դ��ȼ��= 16 * 2 * 3��Height�� = 96�ֽ�  
