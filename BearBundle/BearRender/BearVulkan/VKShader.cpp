#include "VKPCH.h"
size_t ShaderCounter = 0;
VKShader::VKShader(BearShaderType type):m_Type(type)
{
	ShaderCounter++;
	Shader.module = 0;
}

VKShader::~VKShader()
{
	ShaderCounter--;
	if (Shader.module)vkDestroyShaderModule(Factory->Device, Shader.module, 0);
}

#ifdef DEVELOPER_VERSION
static shaderc_include_result* CallbackIncluder(void* user_data, const char* requested_source, int type,const char* requesting_source, size_t include_depth)
{
	if (user_data == nullptr)return nullptr;
	shaderc_include_result* result = bear_alloc< shaderc_include_result>(1);
#ifdef UNICODE
	auto stream = reinterpret_cast<BearIncluder*>(user_data)->OpenAsBuffer(*BearEncoding::FastToUnicode(requested_source));
#else
	auto stream = reinterpret_cast<BearIncluder*>(user_data)->OpenAsBuffer(requested_source);
#endif
	if (!*stream)return nullptr;
	result->content_length = stream->Size();
	auto data = stream->Read();
	result->content = reinterpret_cast<char*>(*data);
	data.clear_no_free();
	result->source_name = requested_source;
	result->source_name_length = BearString::GetSize(requested_source);
	return result;
}
static void CallbackInclduerRelease(void* user_data, shaderc_include_result* include_result)
{
	bear_free(include_result->content);
	bear_free(include_result);
}
extern bool GDebugRender;
bool VKShader::LoadAsTextShaderc(const bchar* text, const bchar* entry_point, const BearMap<BearStringConteniar, BearStringConteniar>& defines, BearString& out_error, BearIncluder* includer, const bchar* file_name, const bchar* out_pdb)
{
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);

#ifdef VK_11
	shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
	shaderc_compile_options_set_target_spirv(options, shaderc_spirv_version_1_3);
#else
	shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
	shaderc_compile_options_set_target_spirv(options, shaderc_spirv_version_1_0);
#endif
#ifdef VK_11
	shaderc_compile_options_set_forced_version_profile(options, 460, shaderc_profile_core);
#else
	shaderc_compile_options_set_forced_version_profile(options, 450, shaderc_profile_core);
#endif
	if (GDebugRender)
	{
		shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_zero);
		shaderc_compile_options_set_generate_debug_info(options);
	}
	else
	{

		shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_performance);
	}
	shaderc_shader_kind shader_kind = shaderc_shader_kind::shaderc_vertex_shader;
	switch (m_Type)
	{
	case BearShaderType::Vertex:
		shader_kind = shaderc_shader_kind::shaderc_vertex_shader;
		break;
	case BearShaderType::Hull:
		shader_kind = shaderc_shader_kind::shaderc_tess_control_shader;
		break;
	case BearShaderType::Domain:
		shader_kind = shaderc_shader_kind::shaderc_tess_evaluation_shader;
		break;
	case BearShaderType::Geometry:
		shader_kind = shaderc_shader_kind::shaderc_geometry_shader;
		break;
	case BearShaderType::Pixel:
		shader_kind = shaderc_shader_kind::shaderc_fragment_shader;
		break;
	case BearShaderType::Compute:
		shader_kind = shaderc_shader_kind::shaderc_compute_shader;
		break;
	default:
		BEAR_CHECK(0);
		break;
	};
	shaderc_compile_options_set_include_callbacks(options, &CallbackIncluder, &CallbackInclduerRelease, reinterpret_cast<void*>(includer));
	for (auto b = defines.begin(), e = defines.end(); b != e; b++)
	{
#ifdef UNICODE
		shaderc_compile_options_add_macro_definition(options, *BearEncoding::FastToAnsi(*b->first), b->first.size(), *BearEncoding::FastToAnsi(*b->second), b->second.size());

#else
		shaderc_compile_options_add_macro_definition(options, *b->first, b->first.size(), *b->second, b->second.size());
#endif
	}
	BearStringAnsi FileName = !file_name ? "noname" :
#ifdef UNICODE
	(BearEncoding::ToAnsi(file_name));
#else
		(file_name);
#endif

#ifdef UNICODE
	m_EntryPointName = *BearEncoding::FastToAnsi(entry_point);
	shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, *BearEncoding::FastToAnsi(text), BearString::GetSize(text), shader_kind, *FileName, *BearEncoding::FastToAnsi(entry_point), options);
#else
	m_EntryPointName = entry_point;
	shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, text, strlen(text), shader_kind, *FileName, entry_point, options);
#endif

	if (shaderc_result_get_compilation_status(result) == shaderc_compilation_status_compilation_error)
	{
		const char* Text = shaderc_result_get_error_message(result);
#ifdef UNICODE
		out_error = BearEncoding::FastToUnicode(Text);
#else
		out_error = Text;
#endif
		shaderc_compile_options_release(options);
		shaderc_result_release(result);
		shaderc_compiler_release(compiler);

		return false;
	}
	if (shaderc_result_get_num_warnings(result))
	{
		const char* Text = shaderc_result_get_error_message(result);
#ifdef UNICODE
		out_error = BearEncoding::FastToUnicode(Text);
#else
		out_error = Text;
#endif
	}
	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
	{
		BEAR_PRINTF(TEXT("ShaderRC:Status %d"), shaderc_result_get_compilation_status(result));
	}
	BEAR_CHECK(shaderc_result_get_compilation_status(result) == shaderc_compilation_status_success);
	ShaderOnMemory.clear();

	ShaderOnMemory.resize(shaderc_result_get_length(result) / 4);
	memcpy(ShaderOnMemory.data(), shaderc_result_get_bytes(result), shaderc_result_get_length(result));
	BEAR_CHECK(shaderc_result_get_length(result) % 4 == 0);
	shaderc_compile_options_release(options);
	shaderc_result_release(result);
	shaderc_compiler_release(compiler);
	CreateShader();

	return true;
}
bool VKShader::LoadAsText(const bchar* text, const bchar* entry_point, const BearMap<BearStringConteniar, BearStringConteniar>& defines, BearString& out_error, BearIncluder* includer, const bchar* file_name, const bchar* out_pdb)
{
	switch (m_Type)
	{
	case BearShaderType::Mesh:
		break;
	case BearShaderType::Amplification:
		break;
#ifdef RTX
	case BearShaderType::RayTracing:
		return LoadAsTextDXC(text, entry_point, defines, out_error, includer, file_name, out_pdb);
#endif
	default:
		return LoadAsTextShaderc(text, entry_point, defines, out_error, includer, file_name, out_pdb);
		break;
	}
	BEAR_ASSERT(false);
	return false;
}



void* VKShader::GetPointer()
{
	return ShaderOnMemory.data();
}

size_t VKShader::GetSize()
{
	return ShaderOnMemory.size()*4;
}
#endif
void VKShader::LoadAsBinary(void* data, size_t size)
{
	if (size == 0)return;
	ShaderOnMemory.clear();
	BEAR_CHECK(size % 4 == 0);
	ShaderOnMemory.resize( size/4);
	memcpy(ShaderOnMemory.data(), data, size);
	CreateShader();
}

void VKShader::CreateShader()
{
	if (ShaderOnMemory.size() == 0)return;
	if (Shader.module)vkDestroyShaderModule(Factory->Device, Shader.module, 0);
	Shader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	Shader.pNext = NULL;
	Shader.pSpecializationInfo = NULL;
	Shader.flags = 0;
	switch (m_Type)
	{
	case BearShaderType::Vertex:
		Shader.stage = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case BearShaderType::Pixel:
		Shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case BearShaderType::Hull:
		Shader.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		break;
	case BearShaderType::Domain:
		Shader.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		break;
	case BearShaderType::Geometry:
		Shader.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case BearShaderType::Compute:
		Shader.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	case BearShaderType::RayTracing:
		break;
	default:
		BEAR_ASSERT(0);
		break;
	}

	Shader.pName = *m_EntryPointName;

	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = ShaderOnMemory.size()*sizeof(uint32);
	moduleCreateInfo.pCode = (uint32*)ShaderOnMemory.data();
	V_CHK(vkCreateShaderModule(Factory->Device, &moduleCreateInfo, NULL, &Shader.module));

}
