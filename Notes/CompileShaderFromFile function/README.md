## D3DXCompileShaderFromFile function  

```C++
HRESULT D3DXCompileShaderFromFile(  
  _In_        LPCTSTR             pSrcFile,  //指向指定文件名的字符串的指针。
  _In_  const D3DXMACRO           *pDefines,  //D3DXMACRO结构的可选零端数组。这个值可能为空。
  _In_        LPD3DXINCLUDE       pInclude,  //可选接口指针，ID3DXInclude，用于处理#include指令。如果该值为空，那么从文件编译时将遵守#include，从资源或内存编译时将导致错误。
  _In_        LPCSTR              pFunctionName,  //指向执行开始的着色器入口点函数的指针。
  _In_        LPCSTR              pProfile,  //指向一个确定着色器指令集的着色器配置文件的指针。
  _In_        DWORD               Flags,  //编译由各种标志标识的选项。Direct3D 10 HLSL编译器现在是默认的。
  _Out_       LPD3DXBUFFER        *ppShader,  //返回一个包含创建的着色器的缓冲区。这个缓冲区包含已编译的着色器代码，以及任何嵌入式调试和符号表信息。
  _Out_       LPD3DXBUFFER        *ppErrorMsgs,  //返回一个缓冲区，其中包含编译时遇到的错误和警告的列表。在调试模式下运行时，调试器会显示相同的消息。这个值可能为空。
  _Out_       LPD3DXCONSTANTTABLE *ppConstantTable  //返回一个ID3DXConstantTable接口，可以用来访问着色器常量。这个值可以为空。
);  
```

#### 参考资料  
[D3DXCompileShaderFromFile微软文档](https://docs.microsoft.com/zh-cn/windows/desktop/direct3d9/d3dxcompileshaderfromfile)  