#include "DX12PCH.h"
bsize ShaderCounter = 0;
DX12Shader::DX12Shader(BearShaderType type):m_Type(type)
{
#ifndef DX11
	Shader = 0;
#endif
	ShaderCounter++;
}

DX12Shader::~DX12Shader()
{
#ifndef DX11
	if (Shader)Shader->Release();
#endif
	ShaderCounter--;
}
#ifdef DX11
class	D3DIncluder : public ID3DInclude
{
public:
	D3DIncluder(BearIncluder* includer) :m_Includer(includer)
	{

	}

	HRESULT __stdcall	Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		if (m_Includer == nullptr)return -1;
#ifdef UNICODE
		auto Steam = m_Includer->OpenAsStream(*BearEncoding::FastToUnicode(pFileName));
#else
		auto Steam = m_Includer->OpenAsStream(pFileName);
#endif
		if (*Steam == nullptr)return -1;
		*pBytes = static_cast<UINT>(Steam->Size());
		m_Data = Steam->Read();
		*ppData = *m_Data;

		return	S_OK;
	}
	HRESULT __stdcall	Close(LPCVOID	pData)
	{
		return	S_OK;
	}
private:
	BearRef<uint8> m_Data;
	BearIncluder* m_Includer;
};
#ifdef DEVELOPER_VERSION
extern bool GDebugRender;
#endif
bool DX12Shader::LoadAsText(const bchar* text, const bchar* entry_point, const BearMap<BearStringConteniar, BearStringConteniar>& defines, BearString& out_error, BearIncluder* includer, const bchar* file_name, const bchar* out_pdb)
{
	UINT CompileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#ifdef DEVELOPER_VERSION
	if (GDebugRender)
	{
		CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	}
#endif

	const char* Type2Text = "";
	switch (m_Type)
	{
	case BearShaderType::Vertex:
		Type2Text = "vs_5_0";
		break;
	case BearShaderType::Hull:
		Type2Text = "hs_5_0";
		break;
	case BearShaderType::Domain:
		Type2Text = "ds_5_0";
		break;
	case BearShaderType::Geometry:
		Type2Text = "gs_5_0";
		break;
	case BearShaderType::Pixel:
		Type2Text = "ps_5_0";
		break;
	case BearShaderType::Compute:
		Type2Text = "cs_5_0";
		break;
	default:
		BEAR_ASSERT(0);
		break;
	}
	ID3D10Blob* ErrorMessage;
	bool Result = true;
	D3D_SHADER_MACRO* Marcos = nullptr;
	if (defines.size())
	{
		Marcos = bear_alloc< D3D_SHADER_MACRO>(defines.size() + 1);
		bsize id = 0;
		for (auto b = defines.begin(), e = defines.end(); b != e; b++)
		{
#ifdef UNICODE
			Marcos[id].Name = BearString::Duplicate(*BearEncoding::FastToAnsi(*b->first));
			Marcos[id++].Definition = BearString::Duplicate(*BearEncoding::FastToAnsi(*b->second));
#else
			Marcos[id].Name = BearString::Duplicate(*b->first);
			Marcos[id++].Definition = BearString::Duplicate(*b->second);
#endif
		}
		Marcos[id].Name = 0;
		Marcos[id].Definition = 0;
	}
	BearStringAnsi FileName = !file_name ? "noname" :
#ifdef UNICODE
	(BearEncoding::ToAnsi(file_name));
#else
		(file_name);
#endif

	D3DIncluder Includer(includer);
#ifdef UNICODE
	if (FAILED(D3DCompile(*BearEncoding::FastToAnsi(text), BearString::GetSize(text), *FileName, Marcos, &Includer, *BearEncoding::FastToAnsi(entry_point), Type2Text, CompileFlags, 0, &Shader, &ErrorMessage)))
	{
		Result = false;
	}
#else
	if (FAILED(D3DCompile(text, BearString::GetSize(text), *FileName, Marcos, &Includer, entry_point, Type2Text, CompileFlags, 0, &Shader, &ErrorMessage)))
	{
		Result = false;
	}
#endif

	if (Marcos != nullptr)
	{
		for (bsize i = 0;  Marcos[i].Name; i++)
		{
			bear_free(Marcos[i].Name);
			bear_free(Marcos[i].Definition);
		}
		bear_free(Marcos);
	}
	if (ErrorMessage)
	{
#ifdef UNICODE
		out_error.append(*BearEncoding::FastToUnicode((char*)ErrorMessage->GetBufferPointer(), (char*)ErrorMessage->GetBufferPointer() + ErrorMessage->GetBufferSize()));
#else
		out_error.append((char*)ErrorMessage->GetBufferPointer(), ErrorMessage->GetBufferSize());
#endif
		ErrorMessage->Release();


		return false;
	}
	return Result;
}
#else

struct DXCInluder :public IDxcIncludeHandler
{
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject) override {
		return E_FAIL;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return 1;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		return 0;
	}
	wchar_t GPath[1024];
	wchar_t LPath[1024];
	BearVector<BearRef<BearBufferedReader>> Readers;
	BearIncluder* m_Includer;
	BearVector<IDxcBlobEncoding*> BlobEncodings;
	DXCInluder(BearIncluder* Includer) :m_Includer(Includer) {}
	~DXCInluder() 
	{
		
	}
	virtual HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
	{
		if (m_Includer == nullptr)return E_FAIL;

		BearStringPath Name;

		if (BearString::ExistPossition(pFilename, 0, L".---"))
		{
			pFilename += 4;
		}	
		if (BearString::ExistPossition(pFilename, 0, L".-"))
		{
			pFilename += 2;
		}

		BearString::Copy(Name,
#ifndef UNICODE
			*BearEncoding::FastToAnsi(pFilename)
#else
			pFilename
#endif
		);

		auto Steam = m_Includer->OpenAsBuffer(Name);
		
		if (!*Steam)return E_FAIL;
		IDxcBlobEncoding* PointerTextBlob;
		bool bIsUTF8 = false;

		if (Steam->Size() > 2)
		{
			char utf8_bom[3];
			Steam->ReadBuffer(utf8_bom, 3);
			bIsUTF8 = utf8_bom[0] == 0xEF;
			bIsUTF8 = bIsUTF8 && utf8_bom[1] == 0xBB;
			bIsUTF8 = bIsUTF8 && utf8_bom[2] == 0xBF;
			Steam->Seek(0);
		}
	
		R_CHK(Factory->DxcLibrary->CreateBlobWithEncodingFromPinned(Steam->Begin(), static_cast<UINT32>(Steam->Size()), bIsUTF8? DXC_CP_UTF8: DXC_CP_ACP, &PointerTextBlob));
		*ppIncludeSource =static_cast<IDxcBlob*>( PointerTextBlob);
		Readers.push_back(Steam);
		BlobEncodings.push_back(PointerTextBlob);
		return S_OK;
	}

};
#ifdef DEVELOPER_VERSION
extern bool GDebugRender;
#endif
bool DX12Shader::LoadAsText(const bchar* text, const bchar* entry_point, const BearMap<BearStringConteniar, BearStringConteniar>& defines, BearString& out_error, BearIncluder* includer, const bchar* file_name, const bchar* out_pdb)
{
	if (Shader)Shader->Release();

	bool bIsUTF8 = false;
	if (BearString::GetSize(text) > 2)
	{
		bIsUTF8 = text[0] == 0xEF;
		bIsUTF8 = bIsUTF8 && text[1] == 0xBB;
		bIsUTF8 = bIsUTF8 && text[2] == 0xBF;
	}

	CComPtr<IDxcResult> Result;
	DXCInluder LIncluder(includer);
	BearVector<const wchar_t*> Arguments;
	BearVector<wchar_t*> StringForDelete;
	Arguments.push_back(L"-Gec");
#ifdef DEVELOPER_VERSION
	if (GDebugRender)
	{
		Arguments.push_back(L"-Zi");
		Arguments.push_back(L"-Od");
	}
#endif
	switch (m_Type)
	{
	case BearShaderType::Pixel:
#ifdef DX12
		Arguments.push_back(L"-Tps_6_0");
#elif DX12_1
		Arguments.push_back(L"-Tps_6_3");
#endif
		Arguments.push_back(L"-E");
#ifdef UNICODE
		Arguments.push_back(entry_point);
#else
		StringForDelete.push_back(BearString::Duplicate(*BearEncoding::FastToUnicode(entry_point)));
		Arguments.push_back(StringForDelete.back());
#endif
		break;
	case BearShaderType::Hull:
#ifdef DX12
		Arguments.push_back(L"-Ths_6_0");
#elif DX12_1
		Arguments.push_back(L"-Ths_6_3");
#endif
#ifdef UNICODE
		Arguments.push_back(entry_point);
#else
		StringForDelete.push_back(BearString::Duplicate(*BearEncoding::FastToUnicode(entry_point)));
		Arguments.push_back(StringForDelete.back());
#endif
		break;

	case BearShaderType::Domain:
#ifdef DX12
		Arguments.push_back(L"-Tds_6_0");
#elif DX12_1
		Arguments.push_back(L"-Tds_6_3");
#endif
#ifdef UNICODE
		Arguments.push_back(entry_point);
#else
		StringForDelete.push_back(BearString::Duplicate(*BearEncoding::FastToUnicode(entry_point)));
		Arguments.push_back(StringForDelete.back());
#endif
		break;
	case BearShaderType::Geometry:
#ifdef DX12
		Arguments.push_back(L"-Tgs_6_0");
#elif DX12_1
		Arguments.push_back(L"-Tgs_6_3");
#endif
#ifdef UNICODE
		Arguments.push_back(entry_point);
#else
		StringForDelete.push_back(BearString::Duplicate(*BearEncoding::FastToUnicode(entry_point)));
		Arguments.push_back(StringForDelete.back());
#endif
		break;
	case BearShaderType::Vertex:
#ifdef DX12
		Arguments.push_back(L"-Tvs_6_0");
#elif DX12_1
		Arguments.push_back(L"-Tvs_6_3");
#endif
#ifdef UNICODE
		Arguments.push_back(entry_point);
#else
		StringForDelete.push_back(BearString::Duplicate(*BearEncoding::FastToUnicode(entry_point)));
		Arguments.push_back(StringForDelete.back());
#endif
		break;
	case BearShaderType::Compute:
#ifdef DX12
		Arguments.push_back(L"-Tcs_6_0");
#elif DX12_1
		Arguments.push_back(L"-Tcs_6_3");
#endif
#ifdef UNICODE
		Arguments.push_back(entry_point);
#else
		StringForDelete.push_back(BearString::Duplicate(*BearEncoding::FastToUnicode(entry_point)));
		Arguments.push_back(StringForDelete.back());
#endif
		break;
	case BearShaderType::Mesh:
#ifdef MESH_SHADING
		Arguments.push_back(L"-Tms_6_5");
#else
		BEAR_ASSERT(false);
#endif
#ifdef UNICODE
		Arguments.push_back(entry_point);
#else
		StringForDelete.push_back(BearString::Duplicate(*BearEncoding::FastToUnicode(entry_point)));
		Arguments.push_back(StringForDelete.back());
#endif
		break;
	case BearShaderType::Amplification:
#ifdef MESH_SHADING
		Arguments.push_back(L"-Tas_6_5");
#else
		BEAR_ASSERT(false);
#endif
#ifdef UNICODE
		Arguments.push_back(entry_point);
#else
		StringForDelete.push_back(BearString::Duplicate(*BearEncoding::FastToUnicode(entry_point)));
		Arguments.push_back(StringForDelete.back());
#endif
		break;
	case BearShaderType::RayTracing:
#ifdef RTX
		Arguments.push_back(L"-Tlib_6_3");
#else
		BEAR_ASSERT(false);
#endif
		break;
	default:
		break;
	}
	Arguments.push_back(L"-D");
	Arguments.push_back(L"DX12=1");

	{

		for (auto b = defines.begin(), e = defines.end(); b != e; b++)
		{


			BearString Temp;
			Temp.append(*b->first);
			Temp.append(TEXT("="));
			Temp.append(*b->second);
#ifdef UNICODE
			
			StringForDelete.push_back(BearString::Duplicate (*Temp));
#else
			StringForDelete.push_back( BearString::Duplicate(*BearEncoding::FastToUnicode(*Temp)));
#endif

			Arguments.push_back(L"-D");
			Arguments.push_back(StringForDelete.back());
		}
	}
	if (out_pdb)
	{
		Arguments.push_back(L"-Fd");
		BearStringUnicode NameFile;
#ifdef UNICODE
		NameFile = file_name;
#else
		NameFile = *BearEncoding::FastToUnicode(out_pdb);
#endif
		StringForDelete.push_back(BearString::Duplicate(*NameFile));
		Arguments.push_back(StringForDelete.back());
	}

	DxcBuffer Buffer;
	Buffer.Ptr = text;
	Buffer.Size = BearString::GetSize(text);
	Buffer.Encoding = bIsUTF8 ? DXC_CP_UTF8 : DXC_CP_ACP;

	R_CHK(Factory->DxcCompiler->Compile(&Buffer, (LPCWSTR*)Arguments.data(),static_cast<UINT32>( Arguments.size()), &LIncluder, IID_PPV_ARGS(&Result)));

	for (bsize i = 0; i < StringForDelete.size(); i++)
	{
		bear_free(StringForDelete[i]);
	}
	HRESULT ResultCode;
	R_CHK(Result->GetStatus(&ResultCode));
	if (FAILED(ResultCode))
	{
		IDxcBlobEncoding* PError;
		R_CHK(Result->GetErrorBuffer(&PError));

		BearVector<char> InfoLog(PError->GetBufferSize() + 1);
		memcpy(InfoLog.data(), PError->GetBufferPointer(), PError->GetBufferSize());
		InfoLog[PError->GetBufferSize()] = 0;

		BearStringAnsi ErrorMsg = "Shader Compiler Error:\n";
		ErrorMsg.append(InfoLog.data());
		PError->Release();
#ifdef UNICODE
		out_error.assign(*BearEncoding::FastToUnicode( *ErrorMsg));
#else
		out_error.assign(*ErrorMsg);
#endif
		return false;
	}


	CComPtr<IDxcBlobUtf16> pShaderName = nullptr;
	R_CHK(Result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&Shader), &pShaderName));
	return true;
}

#endif
void* DX12Shader::GetPointer()
{
#ifdef DX11
	if (Shader.Get()) return Shader.Get()->GetBufferPointer();
#else
	if (Shader) return Shader->GetBufferPointer();
#endif
	return ShaderOnMemory.Begin();
}

bsize DX12Shader::GetSize()
{
#ifdef DX11
	if (Shader.Get()) return Shader.Get()->GetBufferSize();
#else
	if (Shader) return Shader->GetBufferSize();
#endif
	return ShaderOnMemory.Size();
}

void DX12Shader::LoadAsBinary(void* data, bsize size)
{
#ifdef DX11
	Shader.Detach();
#else
	if (Shader)Shader->Release();
#endif
	ShaderOnMemory.Clear();
	ShaderOnMemory.WriteBuffer(data, size);

}