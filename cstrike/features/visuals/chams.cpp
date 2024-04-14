#include "chams.h"

// used: game's interfaces
#include "../../core/interfaces.h"
#include "../../sdk/interfaces/imaterialsystem.h"
#include "../../sdk/interfaces/igameresourceservice.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../core/sdk.h"
#include "../../sdk/entity.h"
#include "../cstrike/sdk/datatypes/resourceutils.h"
#include "../cstrike/sdk/datatypes/buf/utlbuffer.h"

#include <iostream>
// used: original call in hooked function
#include "../../core/hooks.h"

// used: cheat variables
#include"../cstrike/sdk/interfaces/imaterialsystem.h"
#include "../../core/variables.h"
#include <stdexcept>
inline bool(CS_FASTCALL* fnLoadBuffer)(CUtlBuffer*) = nullptr;

// custom material
struct custom_material_data_t {
	material2_t* m_material;
	material2_t* m_material_z;
};

// material list.
enum material_list_t {
	material_white,
	material_default,
	material_illum,
	material_size
};

// array
static custom_material_data_t array_materials[material_size]; // material max

material2_t* create_material_glow2_visible(const char* m_name) {
	const char szVmatBuffer[] = R"#(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d}
			format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
			{
                shader = "solidcolor.vfx"
                F_SELF_ILLUM = 1
                F_PAINT_VERTEX_COLORS = 1
                F_TRANSLUCENT = 1 
                F_IGNOREZ = 0
                F_DISABLE_Z_WRITE = 0
                F_DISABLE_Z_BUFFERING = 0
               
                g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
				g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
                g_tSelfIllumMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                TextureAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
                g_tAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
                
                g_vColorTint = [ 20.000000, 20.000000, 20.000000, 20.000000 ]
                g_flSelfIllumScale = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
                g_flSelfIllumBrightness = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
                g_vSelfIllumTint = [ 10.000000, 10.000000, 10.000000, 10.000000 ]
			} )#";

	auto buffer = new unsigned char[0x100 + sizeof(CKeyValues3)];
	CRT::MemorySet(buffer, 0, sizeof(buffer));
	CKeyValues3* kv3 = (CKeyValues3*)(buffer + 0x100);

	KV3IVD_t kv3ID;
	kv3ID.szName = m_name;
	kv3ID.unk0 = 0x469806E97412167C;
	kv3ID.unk1 = 0xE73790B53EE6F2AF;

	if (!MEM::load_key_value(kv3, nullptr, szVmatBuffer, &kv3ID, nullptr))
		throw;

	material2_t** custom_material;
	MEM::fnCreateMaterial(nullptr, &custom_material, m_name, kv3, 0, 1);

	return *custom_material;
}

material2_t* create_material_glow2_invisible(const char* m_name) {
	const char szVmatBuffer[] = R"#(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d}
			format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
			{
                shader = "solidcolor.vfx"
                F_SELF_ILLUM = 1
                F_PAINT_VERTEX_COLORS = 1
                F_TRANSLUCENT = 1 
                F_IGNOREZ = 1
                F_DISABLE_Z_WRITE = 1
                F_DISABLE_Z_BUFFERING = 1
               
                g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
				g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
                g_tSelfIllumMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                TextureAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
                g_tAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
                
                g_vColorTint = [ 20.000000, 20.000000, 20.000000, 20.000000 ]
                g_flSelfIllumScale = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
                g_flSelfIllumBrightness = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
                g_vSelfIllumTint = [ 10.000000, 10.000000, 10.000000, 10.000000 ]
			} )#";

	auto buffer = new unsigned char[0x100 + sizeof(CKeyValues3)];
	CRT::MemorySet(buffer, 0, sizeof(buffer));
	CKeyValues3* kv3 = (CKeyValues3*)(buffer + 0x100);

	KV3IVD_t kv3ID;
	kv3ID.szName = m_name;
	kv3ID.unk0 = 0x469806E97412167C;
	kv3ID.unk1 = 0xE73790B53EE6F2AF;

	if (!MEM::load_key_value(kv3, nullptr, szVmatBuffer, &kv3ID, nullptr))
		throw;

	material2_t** custom_material;
	MEM::fnCreateMaterial(nullptr, &custom_material, m_name, kv3, 0, 1);

	return *custom_material;
}

material2_t* create_material_default_visible(const char* m_name) {
	const char szVmatBuffer[] = R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_complex.vfx"
    F_PAINT_VERTEX_COLORS = 1
    F_TRANSLUCENT = 1
    
    g_vColorTint = [1, 1, 1, 1]
    TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"

})";

	auto buffer = new unsigned char[0x100 + sizeof(CKeyValues3)];
	CRT::MemorySet(buffer, 0, sizeof(buffer));
	CKeyValues3* kv3 = (CKeyValues3*)(buffer + 0x100);

	KV3IVD_t kv3ID;
	kv3ID.szName = m_name;
	kv3ID.unk0 = 0x469806E97412167C;
	kv3ID.unk1 = 0xE73790B53EE6F2AF;

	if (!MEM::load_key_value(kv3, nullptr, szVmatBuffer, &kv3ID, nullptr))
		throw;

	material2_t** custom_material;
	MEM::fnCreateMaterial(nullptr, &custom_material, m_name, kv3, 0, 1);

	return *custom_material;
}
material2_t* create_material_default_invisible(const char* m_name) {
	const char szVmatBuffer[] = R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_complex.vfx"
    F_PAINT_VERTEX_COLORS = 1
    F_TRANSLUCENT = 1
    F_DISABLE_Z_BUFFERING = 1
    
    g_vColorTint = [1, 1, 1, 1]
    TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"

})";

	auto buffer = new unsigned char[0x100 + sizeof(CKeyValues3)];
	CRT::MemorySet(buffer, 0, sizeof(buffer));
	CKeyValues3* kv3 = (CKeyValues3*)(buffer + 0x100);

	KV3IVD_t kv3ID;
	kv3ID.szName = m_name;
	kv3ID.unk0 = 0x469806E97412167C;
	kv3ID.unk1 = 0xE73790B53EE6F2AF;

	if (!MEM::load_key_value(kv3, nullptr, szVmatBuffer, &kv3ID, nullptr))
		throw;

	material2_t** custom_material;
	MEM::fnCreateMaterial(nullptr, &custom_material, m_name, kv3, 0, 1);

	return *custom_material;
}
material2_t* create_material_flat_visible(const char* m_name) {
	const char szVmatBuffer[] = R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_unlitgeneric.vfx"
	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_BLEND_MODE = 1

	g_vColorTint = [1, 1, 1, 1]
	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"

})";

	auto buffer = new unsigned char[0x100 + sizeof(CKeyValues3)];
	CRT::MemorySet(buffer, 0, sizeof(buffer));
	CKeyValues3* kv3 = (CKeyValues3*)(buffer + 0x100);

	KV3IVD_t kv3ID;
	kv3ID.szName = m_name;
	kv3ID.unk0 = 0x469806E97412167C;
	kv3ID.unk1 = 0xE73790B53EE6F2AF;

	if (!MEM::load_key_value(kv3, nullptr, szVmatBuffer, &kv3ID, nullptr))
		throw;

	material2_t** custom_material;
	MEM::fnCreateMaterial(nullptr, &custom_material, m_name, kv3, 0, 1);

	return *custom_material;
}
material2_t* create_material_flat_invisible(const char* m_name) {
	const char szVmatBuffer[] = R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_unlitgeneric.vfx"
	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_BLEND_MODE = 1
    F_DISABLE_Z_BUFFERING = 1

	g_vColorTint = [1, 1, 1, 1]
	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"

})";

	auto buffer = new unsigned char[0x100 + sizeof(CKeyValues3)];
	CRT::MemorySet(buffer, 0, sizeof(buffer));
	CKeyValues3* kv3 = (CKeyValues3*)(buffer + 0x100);

	KV3IVD_t kv3ID;
	kv3ID.szName = m_name;
	kv3ID.unk0 = 0x469806E97412167C;
	kv3ID.unk1 = 0xE73790B53EE6F2AF;

	if (!MEM::load_key_value(kv3, nullptr, szVmatBuffer, &kv3ID, nullptr))
		throw;

	material2_t** custom_material;
	MEM::fnCreateMaterial(nullptr, &custom_material, m_name, kv3, 0, 1);

	return *custom_material;
}
material2_t* create_material_illum_visible(const char* m_name) {
	const char szVmatBuffer[] = R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
     shader = "csgo_complex.vfx"
 
    g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tSelfIllumMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    TextureAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
    g_tAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
    
    g_vColorTint = [ 20.000000, 20.000000, 20.000000, 20.000000 ]
    g_flSelfIllumScale = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
    g_flSelfIllumBrightness = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
    g_vSelfIllumTint = [ 10.000000, 10.000000, 10.000000, 10.000000 ]

    F_SELF_ILLUM = 1
    
    F_PAINT_VERTEX_COLORS = 1

    F_TRANSLUCENT = 1

})";

	auto buffer = new unsigned char[0x100 + sizeof(CKeyValues3)];
	CRT::MemorySet(buffer, 0, sizeof(buffer));
	CKeyValues3* kv3 = (CKeyValues3*)(buffer + 0x100);

	KV3IVD_t kv3ID;
	kv3ID.szName = m_name;
	kv3ID.unk0 = 0x469806E97412167C;
	kv3ID.unk1 = 0xE73790B53EE6F2AF;

	if (!MEM::load_key_value(kv3, nullptr, szVmatBuffer, &kv3ID, nullptr))
		throw;

	material2_t** custom_material;
	MEM::fnCreateMaterial(nullptr, &custom_material, m_name, kv3, 0, 1);

	return *custom_material;
}
material2_t* create_material_illum_invisible(const char* m_name) {
	const char szVmatBuffer[] = R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
     shader = "csgo_complex.vfx"
 
    g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    g_tSelfIllumMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
    TextureAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
    g_tAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
    
    g_vColorTint = [ 20.000000, 20.000000, 20.000000, 20.000000 ]
    g_flSelfIllumScale = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
    g_flSelfIllumBrightness = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
    g_vSelfIllumTint = [ 10.000000, 10.000000, 10.000000, 10.000000 ]

    F_SELF_ILLUM = 1
    
    F_PAINT_VERTEX_COLORS = 1

    F_TRANSLUCENT = 1

    F_DISABLE_Z_BUFFERING = 1

})";

	auto buffer = new unsigned char[0x100 + sizeof(CKeyValues3)];
	CRT::MemorySet(buffer, 0, sizeof(buffer));
	CKeyValues3* kv3 = (CKeyValues3*)(buffer + 0x100);

	KV3IVD_t kv3ID;
	kv3ID.szName = m_name;
	kv3ID.unk0 = 0x469806E97412167C;
	kv3ID.unk1 = 0xE73790B53EE6F2AF;

	if (!MEM::load_key_value(kv3, nullptr, szVmatBuffer, &kv3ID, nullptr))
		throw;

	material2_t** custom_material;
	MEM::fnCreateMaterial(nullptr, &custom_material, m_name, kv3, 0, 1);

	return *custom_material;
}
// other functions.
bool ams::chams_t::initialize() {
	// first we have to check if this shit is alr initialized
	if (this->m_initialized) {
		return this->m_initialized;
	}

	// only initialize this for moment, later we're gonna init more
	array_materials[material_list_t::material_white] = custom_material_data_t{
	.m_material = CreateMaterial(CS_XOR("primary_white"), CS_XOR("materials/dev/primary_white.vmat"), CS_XOR("csgo_unlitgeneric.vfx"), true, true, false),
		.m_material_z = CreateMaterial(CS_XOR("primary_white_invisible"), CS_XOR("materials/dev/primary_white.vmat"), CS_XOR("csgo_unlitgeneric.vfx"), true, true, true)
	};

	array_materials[material_list_t::material_default] = custom_material_data_t{
	.m_material = CreateMaterial(CS_XOR("primary_white"), CS_XOR("materials/dev/primary_white.vmat"), CS_XOR("csgo_unlitgeneric.vfx"), true, true, false),
		.m_material_z = CreateMaterial(CS_XOR("primary_white_invisible"), CS_XOR("materials/dev/primary_white.vmat"), CS_XOR("csgo_unlitgeneric.vfx"), true, true, true)
	};

	array_materials[material_list_t::material_illum] = custom_material_data_t{
.m_material = CreateMaterial(CS_XOR("primary_white"), CS_XOR("materials/dev/primary_white.vmat"), CS_XOR("csgo_unlitgeneric.vfx"), true, true, false),
		.m_material_z = CreateMaterial(CS_XOR("primary_white_invisible"), CS_XOR("materials/dev/primary_white.vmat"), CS_XOR("csgo_unlitgeneric.vfx"), true, true, true)
	};

	// we initialized by now
	this->m_initialized = true;

	// iterate.
	for (auto& [m_visible, m_invisible] : array_materials) {
		if (m_visible == nullptr || m_invisible == nullptr)
			this->m_initialized = false;
	}

	// finish and return value
	return this->m_initialized;
}

void ams::chams_t::destroy() {
	// nothing to destroy boss
}

bool ams::chams_t::draw_object(void* animatable_object, void* dx11, material_data_t* arr_material_data, int data_count,
	void* scene_view, void* scene_layer, void* unk1, void* unk2) {
	// check if we initialized.
	if (!this->m_initialized) {
		return false;
	}
	if (!C_GET(bool, Vars.bVisualChams))
	{
		return false;
		}
	if (!SDK::LocalPawn)
		return false;
	// no chams enable.
	// return false

	// no data on material
	if (arr_material_data == nullptr) {
		return false;
	}

	// no data on scene object.
	if (arr_material_data->m_scene_animable == nullptr) {
		return false;
	}

	// owner.
	CBaseHandle m_owner = arr_material_data->m_scene_animable->m_owner;

	// get entity.
	auto m_entity = I::GameResourceService->pGameEntitySystem ->Get< C_BaseEntity >(m_owner);
	if (m_entity == nullptr) {
		return false;
	}

	// schema info
	SchemaClassInfoData_t* m_class_info;
	m_entity->GetSchemaClassInfo(&m_class_info);
	if (m_class_info == nullptr) {
		return false;
	}

	// compare string.
	if (CRT::StringCompare(m_class_info->szName, CS_XOR("C_CSPlayerPawn")) != 0)
		return false;

	// get pawn.
	auto m_pawn = I::GameResourceService->pGameEntitySystem->Get<  C_CSPlayerPawn>(m_owner);
	if (m_pawn == nullptr) {
		return false;
	}

	// other entity
	if (!m_pawn->IsOtherEnemy(SDK::LocalPawn)) {
		return false;
	}

	// it is alive
	if (m_pawn->GetHealth() <= 0) {
		return false;
	}

	// we passed to everythign.
	// please render material.
	return this->override_material(animatable_object, dx11, arr_material_data, data_count,
		scene_view, scene_layer, unk1, unk2);
}

material2_t* ams::chams_t::CreateMaterial(const char* szName, const char* szMaterialVMAT, const char* szShaderType, bool bBlendMode, bool bTranslucent, bool bDisableZBuffering)
{
	material_data_t* pData = reinterpret_cast<material_data_t*>(static_cast<std::byte*>(MEM_STACKALLOC(0x200)) + 0x50);
	material2_t** pMatPrototype;

	I::MaterialSystem2->find_or_create_from_resource(&pMatPrototype, szMaterialVMAT);
	if (pMatPrototype == nullptr)
		return nullptr;
	// @note: SetCreateDataByMaterial failed on release build idk why
#ifdef _DEBUG
	I::MaterialSystem2->set_create_data_by_material(pData, &pMatPrototype);
	pData->set_shader_type(szShaderType);

	pData->set_material_function(CS_XOR("F_DISABLE_Z_BUFFERING"), bDisableZBuffering ? 1 : 0);
	pData->set_material_function(CS_XOR("F_BLEND_MODE"), bBlendMode ? 1 : 0);
	pData->set_material_function(CS_XOR("F_TRANSLUCENT"), bTranslucent ? 1 : 0);

	material2_t** pMaterial;
	I::MaterialSystem2->create_material(&pMaterial, szName, pData);
	return *pMaterial;
#endif
	return *pMatPrototype;
}

bool ams::chams_t::override_material(void* animatable_object, void* dx11, material_data_t* arr_material_data, int data_count,
	void* scene_view, void* scene_layer, void* unk1, void* unk2) {

	const auto original = H::hkDrawObject.GetOriginal();
	const custom_material_data_t customMaterial = array_materials[C_GET(int, Vars.nVisualChamMaterial)];

	if (C_GET(bool, Vars.bVisualChamsIgnoreZ))
	{
		arr_material_data->m_material = customMaterial.m_material_z;
		arr_material_data->m_color = C_GET(ColorPickerVar_t, Vars.colVisualChamsIgnoreZ).colValue;
		I::MaterialSystem2->set_color(arr_material_data, C_GET(ColorPickerVar_t, Vars.colVisualChamsIgnoreZ).colValue);

		original(animatable_object, dx11, arr_material_data, data_count, scene_view, scene_layer, unk1, unk2);
	}

	arr_material_data->m_material = customMaterial.m_material;
	arr_material_data->m_color = C_GET(ColorPickerVar_t, Vars.colVisualChams).colValue;
	I::MaterialSystem2->set_color(arr_material_data, C_GET(ColorPickerVar_t, Vars.colVisualChams).colValue);
	original(animatable_object, dx11, arr_material_data, data_count, scene_view, scene_layer, unk1, unk2);

	return true;
}
