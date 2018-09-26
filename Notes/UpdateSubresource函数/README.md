## 对该函数的理解  
- CPU将数据从内存复制到不可映射内存中创建的子资源。  
- 更新缓冲区。  
- 应用方向之一是 动态向纹理写入内容。  
- test.cpp文件是UpdateSubresource函数应用的一个例子。  


我们有了矩阵，现在我们必须在渲染的时候把它们写在常量缓冲区里，这样GPU就可以读取它们了。要更新缓冲区，我们可以使用`ID3D11DeviceContext::UpdateSubresource()` API，并将一个指针传递给存储在与着色器的常量缓冲区顺序相同的矩阵。  

```c++
    //
    // Update variables that change once per frame
    //
    CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose( g_World );
    cb.vMeshColor = g_vMeshColor;
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );  
```  
#### 注意  
1，资源我们可以理解为：显存。  

2，要计算给定资源的源行间距和源深度间距，使用以下公式:  
**源行间距=[一个元素的字节大小] * [一行中的元素数量]**   
**源深度间距=[源行间距]*[行数(高)]**  

#### 参考资料：  
[微软官方文档-UpdateSubresource](https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/nf-d3d11-id3d11devicecontext-updatesubresource)  
[微软官方文档-D3D11CalcSubresource](https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/nf-d3d11-d3d11calcsubresource)   
![Row_Depth_Pitch](https://github.com/OctopusLian/LearnDirectX/blob/master/Notes/UpdateSubresource%E5%87%BD%E6%95%B0/Row_Depth_Pitch.png)