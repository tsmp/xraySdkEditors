#include "VKPCH.h"

#ifdef DEVELOPER_VERSION
#ifdef RTX

struct DXCInluder :public IDxcIncludeHandler
{
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID , void** ) override {
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
	DXCInluder(BearIncluder* includer) :m_Includer(includer) {}
	~DXCInluder()
	{

	}
	virtual HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR filename, IDxcBlob** include_source)
	{
		if (m_Includer == nullptr)return E_FAIL;

		BearStringPath Name;

		if (BearString::ExistPossition(filename, 0, L".///"))
		{
			filename += 4;
		}
		if (BearString::ExistPossition(filename, 0, L"./"))
		{
			filename += 2;
		}

		BearString::Copy(Name,
#ifndef UNICODE
			* BearEncoding::FastToAnsi(filename)
#else
			filename
#endif
		);

		auto steam = m_Includer->OpenAsBuffer(Name);

		if (!*steam)return E_FAIL;
		IDxcBlobEncoding* PointerTextBlob;
		bool bIsUTF8 = false;

		if (steam->Size() > 2)
		{
			char utf8_bom[3];
			steam->ReadBuffer(utf8_bom, 3);
			bIsUTF8 = utf8_bom[0] == 0xEF;
			bIsUTF8 = bIsUTF8 && utf8_bom[1] == 0xBB;
			bIsUTF8 = bIsUTF8 && utf8_bom[2] == 0xBF;
			steam->Seek(0);
		}

		BEAR_ASSERT(SUCCEEDED(Factory->DxcLibrary->CreateBlobWithEncodingFromPinned(steam->Begin(), static_cast<UINT32>(steam->Size()), bIsUTF8 ? DXC_CP_UTF8 : DXC_CP_ACP, &PointerTextBlob)));
		*include_source = static_cast<IDxcBlob*>(PointerTextBlob);
		Readers.push_back(steam);
		BlobEncodings.push_back(PointerTextBlob);
		return S_OK;
	}

};

extern bool GDebugRender;
bool VKShader::LoadAsTextDXC(const bchar* text, const bchar* entry_point, const BearMap<BearStringConteniar, BearStringConteniar>& defines, BearString& out_error, BearIncluder* includer, const bchar* file_name,const bchar*out_pdb)
{
	bool bIsUTF8 = false;
	if (BearString::GetSize(text) > 2)
	{
		bIsUTF8 = text[0] == 0xEF;
		bIsUTF8 = bIsUTF8 && text[1] == 0xBB;
		bIsUTF8 = bIsUTF8 && text[2] == 0xBF;

	}

	IDxcResult* Result;
	DXCInluder LIncluder(includer);
	BearVector<const wchar_t*> Arguments;
	BearVector<wchar_t*> StringForDelete;
	Arguments.push_back(L"-spirv");
	Arguments.push_back(L"-fvk-use-scalar-layout"); 	
	if (GDebugRender)
	{
		Arguments.push_back(L"-Od");
	}
	else
	{
	}
	switch (m_Type)
	{

	case BearShaderType::RayTracing:
		Arguments.push_back(L"-fspv-extension=SPV_NV_ray_tracing");
		Arguments.push_back(L"-Tlib_6_3");
		break;
	default:
		BEAR_CHECK(false);
		break;
	}

	Arguments.push_back(L"-DENABLE_SPIRV_CODEGEN=ON");
	Arguments.push_back(L"-D");
	Arguments.push_back(L"VULKAN=1");

	{

		for (auto b = defines.begin(), e = defines.end(); b != e; b++)
		{


			BearString Temp;
			Temp.append(*b->first);
			Temp.append(TEXT("="));
			Temp.append(*b->second);
#ifdef UNICODE

			StringForDelete.push_back(BearString::Duplicate(*Temp));
#else
			StringForDelete.push_back(BearString::Duplicate(*BearEncoding::FastToUnicode(*Temp)));
#endif

			Arguments.push_back(L"-D");
			Arguments.push_back(StringForDelete.back());
		}
	}
	//DXCInluder includer;

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

	BEAR_ASSERT(SUCCEEDED(Factory->DxcCompiler->Compile(&Buffer, (LPCWSTR*)Arguments.data(), static_cast<UINT32>(Arguments.size()), &LIncluder, IID_PPV_ARGS(&Result))));

	for (bsize i = 0; i < StringForDelete.size(); i++)
	{
		bear_free(StringForDelete[i]);
	}
	HRESULT ResultCode;
	BEAR_ASSERT(SUCCEEDED(Result->GetStatus(&ResultCode)));
	if (FAILED(ResultCode))
	{
		IDxcBlobEncoding* PError;
		BEAR_ASSERT(SUCCEEDED(Result->GetErrorBuffer(&PError)));

		BearVector<char> infoLog(PError->GetBufferSize() + 1);
		memcpy(infoLog.data(), PError->GetBufferPointer(), PError->GetBufferSize());
		infoLog[PError->GetBufferSize()] = 0;

		BearStringAnsi errorMsg = "Shader Compiler Error:\n";
		errorMsg.append(infoLog.data());
		PError->Release();
#ifdef UNICODE
		out_error.assign(*BearEncoding::FastToUnicode(*errorMsg));
#else
		out_error.assign(*errorMsg);
#endif
		return false;
	}

	IDxcBlob* ShaderBled;
	IDxcBlobUtf16* pShaderName = nullptr;
	BEAR_ASSERT(SUCCEEDED(Result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&ShaderBled), &pShaderName)));
	BEAR_CHECK(ShaderBled->GetBufferSize() % 4 == 0);
	ShaderOnMemory.resize(ShaderBled->GetBufferSize() / 4);
	bear_copy(ShaderOnMemory.data(), ShaderBled->GetBufferPointer(), ShaderBled->GetBufferSize());
	ShaderBled->Release();
	if(pShaderName)
		pShaderName->Release();
	Result->Release();
	CreateShader();
	Shader.pName = BearStringConteniarAnsi::Reserve(
#ifdef UNICODE
		* BearEncoding::FastToAnsi(entry_point)
#else
		entry_point
#endif
	);
	return true;
}
#endif
#endif