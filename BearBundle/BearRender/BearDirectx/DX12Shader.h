#pragma once
class DX12Shader :public BearRHI::BearRHIShader
{
public:
	DX12Shader(BearShaderType type);
	virtual ~DX12Shader();
	virtual bool LoadAsText(const bchar* text, const bchar* entry_point, const BearMap<BearStringConteniar, BearStringConteniar>& defines, BearString& out_error, BearIncluder* includer, const bchar* file_name, const bchar* out_pdb);

	virtual void* GetPointer();
	virtual	bsize GetSize();
	virtual	void LoadAsBinary(void* data, bsize size);
#ifdef DX11
	ComPtr<ID3DBlob> Shader;
#else
	IDxcBlob* Shader;
#endif
	BearMemoryStream ShaderOnMemory;
	inline bool IsType(BearShaderType type)const { return m_Type == type; }
private:
	BearShaderType m_Type;
};