#pragma once

// @test: using interfaces in the header | not critical but could blow up someday with thousands of errors or affect to compilation time etc
// used: cgameentitysystem, ischemasystem
#include "../core/interfaces.h"
#include "interfaces/igameresourceservice.h"
#include "interfaces/ischemasystem.h"

// used: schema field
#include "../core/schema.h"
#include "../core/spoofcall/lazy_importer.hpp"
// used: l_print
#include "../utilities/log.h"
// used: vector_t
#include "datatypes/vector.h"
// used: qangle_t
#include "datatypes/qangle.h"
// used: ctransform
#include "datatypes/transform.h"
#include "datatypes/utlstring.h"
#include "datatypes/utlvector.h"
#include "datatypes/cstronghandle.hpp"
// used: cbasehandle
#include "entity_handle.h"
// used: game's definitions
#include "const.h"
#include "../core/pointer/pointer.hpp"
#include "../core/csig/sigscan.hpp"
#include "../utilities/draw.h"
#include "../core/schemav2.hpp"

#include <mutex>

using GameTime_t = std::int32_t;
using GameTick_t = std::int32_t;

enum TRACE_TYPE : int {
	DEFAULT = 0,
	AIMBOT = 1
};
class CFiringModeFloat
{
public:
	float flValue[2];
};
enum HitGroup_t : std::uint32_t {
	HITGROUP_INVALID = -1,
	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD = 1,
	HITGROUP_CHEST = 2,
	HITGROUP_STOMACH = 3,
	HITGROUP_LEFTARM = 4,
	HITGROUP_RIGHTARM = 5,
	HITGROUP_LEFTLEG = 6,
	HITGROUP_RIGHTLEG = 7,
	HITGROUP_NECK = 8,
	HITGROUP_UNUSED = 9,
	HITGROUP_GEAR = 10,
	HITGROUP_SPECIAL = 11,
	HITGROUP_COUNT = 12,
};

enum HITBOXES : uint32_t {
	HEAD = 6,
	NECK = 5,
	CHEST = 4,
	RIGHT_CHEST = 8,
	LEFT_CHEST = 13,
	STOMACH = 3,
	PELVIS = 2,
	CENTER = 1,
	L_LEG = 23,
	L_FEET = 24,
	R_LEG = 26,
	R_FEET = 27
};

enum CSWeaponType : std::uint32_t {
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL = 1,
	WEAPONTYPE_SUBMACHINEGUN = 2,
	WEAPONTYPE_RIFLE = 3,
	WEAPONTYPE_SHOTGUN = 4,
	WEAPONTYPE_SNIPER_RIFLE = 5,
	WEAPONTYPE_MACHINEGUN = 6,
	WEAPONTYPE_C4 = 7,
	WEAPONTYPE_TASER = 8,
	WEAPONTYPE_GRENADE = 9,
	WEAPONTYPE_EQUIPMENT = 10,
	WEAPONTYPE_STACKABLEITEM = 11,
	WEAPONTYPE_FISTS = 12,
	WEAPONTYPE_BREACHCHARGE = 13,
	WEAPONTYPE_BUMPMINE = 14,
	WEAPONTYPE_TABLET = 15,
	WEAPONTYPE_MELEE = 16,
	WEAPONTYPE_SHIELD = 17,
	WEAPONTYPE_ZONE_REPULSOR = 18,
	WEAPONTYPE_UNKNOWN = 19,
};
// MGetKV3ClassDefaults

class CEntityInstance;

class CEntityIdentity
{
public:
	CS_CLASS_NO_INITIALIZER(CEntityIdentity);

	// @note: handle index is not entity index
	SCHEMA_ADD_OFFSET(std::uint32_t, GetIndex, 0x10);
	SCHEMA_ADD_FIELD(const char*, GetDesignerName, "CEntityIdentity->m_designerName");
	SCHEMA_ADD_FIELD(std::uint32_t, GetFlags, "CEntityIdentity->m_flags");

	[[nodiscard]] bool IsValid()
	{
		return GetIndex() != INVALID_EHANDLE_INDEX;
	}

	[[nodiscard]] int GetEntryIndex()
	{
		if (!IsValid())
			return ENT_ENTRY_MASK;

		return GetIndex() & ENT_ENTRY_MASK;
	}

	[[nodiscard]] int GetSerialNumber()
	{
		return GetIndex() >> NUM_SERIAL_NUM_SHIFT_BITS;
	}

	CEntityInstance* pInstance; // 0x00
};

class CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER(CEntityInstance);

	void GetSchemaClassInfo(SchemaClassInfoData_t** pReturn)
	{
		return MEM::CallVFunc<void, 36U>(this, pReturn);
	}
	// cHoca
	[[nodiscard]] std::uint32_t GetEntityHandle()
	{
		using function_t = std::uint32_t(__fastcall*)(void*);
		static function_t fn = reinterpret_cast<function_t>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 85 C9 74 32 48 8B 41 10 48 85 C0 74 29 44")));
		CS_ASSERT(fn != nullptr);
		return fn(this);
	}

	[[nodiscard]] CBaseHandle GetRefEHandle()
	{
		CEntityIdentity* pIdentity = GetIdentity();
		if (pIdentity == nullptr)
			return CBaseHandle();

		return CBaseHandle(pIdentity->GetEntryIndex(), pIdentity->GetSerialNumber() - (pIdentity->GetFlags() & 1));
	}

	SCHEMA_ADD_FIELD(CEntityIdentity*, GetIdentity, "CEntityInstance->m_pEntity");
};


enum EEconItemQuality {
	IQ_UNDEFINED = -1,
	IQ_NORMAL,
	IQ_GENUINE,
	IQ_VINTAGE,
	IQ_UNUSUAL,
	IQ_UNIQUE,
	IQ_COMMUNITY,
	IQ_DEVELOPER,
	IQ_SELFMADE,
	IQ_CUSTOMIZED,
	IQ_STRANGE,
	IQ_COMPLETED,
	IQ_HAUNTED,
	IQ_TOURNAMENT,
	IQ_FAVORED
};

enum EEconItemRarity {
	IR_DEFAULT,
	IR_COMMON,
	IR_UNCOMMON,
	IR_RARE,
	IR_MYTHICAL,
	IR_LEGENDARY,
	IR_ANCIENT,
	IR_IMMORTAL
};

// https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/game/shared/econ/econ_item_constants.h#L39
enum EEconTypeID {
	k_EEconTypeItem = 1,
	k_EEconTypePersonaDataPublic = 2,
	k_EEconTypeGameAccountClient = 7,
	k_EEconTypeGameAccount = 8,
	k_EEconTypeEquipInstance = 31,
	k_EEconTypeDefaultEquippedDefinitionInstance = 42,
	k_EEconTypeDefaultEquippedDefinitionInstanceClient = 43,
	k_EEconTypeCoupon = 45,
	k_EEconTypeQuest = 46,
};

// MGetKV3ClassDefaults
class SkeletonBoneBounds_t
{
public:
	Vector_t m_vecCenter; // 0x0	
	Vector_t m_vecSize; // 0xc	
};

// Registered binary: animationsystem.dll (project 'modellib')
// Alignment: 8
// Size: 0x60
// 
// MGetKV3ClassDefaults
class RenderSkeletonBone_t
{
public:
	CS_CLASS_NO_INITIALIZER(RenderSkeletonBone_t);

	SCHEMA_ADD_OFFSET(const char*, m_boneName, 0x0);
	SCHEMA_ADD_OFFSET(const char*, m_parentName, 0x8);
	SCHEMA_ADD_OFFSET(SkeletonBoneBounds_t, m_bbox, 0x40);
	SCHEMA_ADD_OFFSET(float, m_flSphereRadius, 0x58);

};

// Registered binary: animationsystem.dll (project 'modellib')
// Alignment: 8
// Size: 0x50
// 
// MGetKV3ClassDefaults
class CRenderSkeleton
{
public:
	CS_CLASS_NO_INITIALIZER(CRenderSkeleton);

	SCHEMA_ADD_OFFSET(CUtlVector<RenderSkeletonBone_t>, m_bones, 0x0);
	SCHEMA_ADD_OFFSET(CUtlVector<int32_t>, m_boneParents, 0xB8);
	SCHEMA_ADD_OFFSET(int32_t, m_nBoneWeightCount, 0x48);

};




class c_attachments
{
public:
	char		pad_0x0000[0x10];		  // 0x0000
	const char* m_key;					  // 0x0010
	char		pad_0x0018[0x8];		  // 0x0018
	const char* m_name;					  // 0x0020
	const char* m_influenceNames[3];	  // 0x0028
	char		pad_0x0030[0x10];		  // 0x0030
	Vector4D_t	m_vInfluenceRotations[3]; // 0x0040
	Vector_t		m_vInfluenceOffsets[3];	  // 0x0070
	float		m_influenceWeights[3];	  // 0x0094
	char		pad_0x00A0[0x10];		  // 0x00A0
};

class c_drawcalls
{
public:
	__int32 m_nPrimitiveType; // 0x0000
	__int32 m_nBaseVertex;	  // 0x0004
	__int32 m_nVertexCount;	  // 0x0008
	__int32 m_nStartIndex;	  // 0x000C
	__int32 m_nIndexCount;	  // 0x0010
	float	m_flUvDensity;	  // 0x0014
	float	m_vTintColor[3];  // 0x0018
	float	m_flAlpha;		  // 0x0024
	char	pad_0x0028[0xC0]; // 0x0028

}; // Size=0x00E8

class c_bones
{
public:
	const char* m_boneName;		   // 0x0000
	const char* m_parentName;	   // 0x0008
	float		m_invBindPose[12]; // 0x0010
	Vector_t		m_vecCenter;	   // 0x0040
	Vector_t		m_vecSize;		   // 0x004C
	float		m_flSphereradius;  // 0x0058
	char		pad_0x005C[0x4];   // 0x005C

}; // Size=0x0060


class CRenderMesh
{

public:

	char		   pad_0x0000[40];	// 0x0000
	Vector_t		   m_vMinBounds;		// 0x0028
	Vector_t		   m_vMaxBounds;		// 0x0034
	char		   pad_0x0044[8];		// 0x0044
	c_drawcalls* m_drawCalls;			// 0x0048
	char		   pad_0x0050[104];	// 0x0050
	__int32		   m_skeletoncount;		// 0x00B8
	char		   pad_0x00BC[4];		// 0x00BC
	c_bones* m_skeleton;			// 0x00C0
	char		   pad_0x00C8[112];		// 0x00C8


};
class CRenderM {
public:
CRenderMesh*	mesh;
};
struct VPhysicsCollisionAttribute_t
{

public:
	CS_CLASS_NO_INITIALIZER(VPhysicsCollisionAttribute_t);

	SCHEMA_ADD_FIELD(uint8_t, m_nCollisionGroup, "VPhysicsCollisionAttribute_t->m_nCollisionGroup");
	SCHEMA_ADD_FIELD(uint8_t, m_nHierarchyId, "VPhysicsCollisionAttribute_t->m_nHierarchyId");
	SCHEMA_ADD_FIELD(uint8_t, m_nOwnerId, "VPhysicsCollisionAttribute_t->m_nOwnerId");
	SCHEMA_ADD_FIELD(uint8_t, m_nEntityId, "VPhysicsCollisionAttribute_t->m_nEntityId");
	SCHEMA_ADD_FIELD(uint8_t, m_nCollisionFunctionMask, "VPhysicsCollisionAttribute_t->m_nCollisionFunctionMask");

};
class CCollisionProperty
{
public:
	CS_CLASS_NO_INITIALIZER(CCollisionProperty);
	SCHEMA_ADD_FIELD(Vector_t, GetMins, "CCollisionProperty->m_vecMins");
	SCHEMA_ADD_FIELD(Vector_t, GetMaxs, "CCollisionProperty->m_vecMaxs");
	SCHEMA_ADD_FIELD(uint8_t, m_solidFlags, "CCollisionProperty->m_usSolidFlags");
	SCHEMA_ADD_FIELD(VPhysicsCollisionAttribute_t, m_collisionAttribute, "CCollisionProperty->m_collisionAttribute");
};

class CAnimBone
{
public:
	CS_CLASS_NO_INITIALIZER(CAnimBone);
	SCHEMA_EXTENDED(Vector_t, m_pos, "animationsystem.dll", "CAnimBone", "m_pos", 0);


};
class CPhysModel;

class CModelSkeleton
{
public:
	CModelSkeleton() = delete;
	CModelSkeleton(CModelSkeleton&&) = delete;
	CModelSkeleton(const CModelSkeleton&) = delete;

public:
	SCHEMA_ADD_OFFSET(CUtlVector<const char*>, vecBoneNames, 0x4);
	SCHEMA_ADD_OFFSET(CUtlVector<std::uint16_t>, vecBoneParent, 0x18);
	SCHEMA_ADD_OFFSET(CUtlVector<float>, m_boneSphere, 0x30);
	SCHEMA_ADD_OFFSET(CUtlVector<std::uint32_t>, m_nFlag, 0x48);
	SCHEMA_ADD_OFFSET(CUtlVector<Vector_t>, m_bonePosParent, 0x60);
	SCHEMA_ADD_OFFSET(CUtlVector<QuaternionAligned_t>, m_boneRotParent, 0x78);
	SCHEMA_ADD_OFFSET(CUtlVector<float>, m_boneScaleParent, 0x90);

};


// Alignment: 10
// Size: 0x58
struct PermModelInfo_t
{
public:
	int m_nFlags; // 0x0	
	Vector_t m_vHullMin; // 0x4	
	Vector_t m_vHullMax; // 0x10	
	Vector_t m_vViewMin; // 0x1c	
	Vector_t m_vViewMax; // 0x28	
	float m_flMass; // 0x34	
	Vector_t m_vEyePosition; // 0x38	
	float m_flMaxEyeDeflection; // 0x44	
	const char* m_sSurfaceProperty; // 0x48	
	const char* m_keyValueText; // 0x50	
};
struct PermModelExtPart_t
{
public:
	CTransform m_Transform; // 0x0	
	const char* m_Name; // 0x20	
	int32_t m_nParent; // 0x28		
};
// Alignment: 0
// Size: 0x1
class InfoForResourceTypeCRenderMesh
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x1]; // 0x0
public:
	// No members available
};
// Alignment: 0
// Size: 0x1
class InfoForResourceTypeCModel
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x1]; // 0x0
public:
	// No members available
};
// Alignment: 0
// Size: 0x1
class InfoForResourceTypeCAnimationGroup
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x1]; // 0x0
public:
	// No members available
};
// Alignment: 0
// Size: 0x1
class InfoForResourceTypeCSequenceGroupData
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x1]; // 0x0
public:
	// No members available
};

// Alignment: 0
// Size: 0x1
class InfoForResourceTypeCPhysAggregateData
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x1]; // 0x0
public:
	// No members available
};
// Alignment: 0
// Size: 0x1
class InfoForResourceTypeIMaterial2
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x1]; // 0x0
public:
	// No members available
};
// Alignment: 2
// Size: 0x20
struct MaterialGroup_t
{
public:

	CUtlString m_name; // 0x0	
	CUtlVector<CStrongHandle<InfoForResourceTypeIMaterial2>> m_materials; // 0x8	
};
// Alignment: 4
// Size: 0x4
enum class ModelBoneFlexComponent_t : uint32_t
{
	MODEL_BONE_FLEX_INVALID = 0xffffffffffffffff,
	MODEL_BONE_FLEX_TX = 0x0,
	MODEL_BONE_FLEX_TY = 0x1,
	MODEL_BONE_FLEX_TZ = 0x2,
};
// Alignment: 5
// Size: 0x20
struct ModelBoneFlexDriverControl_t
{
public:
	ModelBoneFlexComponent_t m_nBoneComponent; // 0x0	
	CUtlString m_flexController; // 0x8	
	uint32_t m_flexControllerToken; // 0x10	
	float m_flMin; // 0x14	
	float m_flMax; // 0x18	
};

// Alignment: 3
// Size: 0x28

struct ModelBoneFlexDriver_t
{
public:
	const char* m_boneName; // 0x0	
	uint32_t m_boneNameToken; // 0x8	
private:
	[[maybe_unused]] uint8_t __pad000c[0x4]; // 0xc
public:
	CUtlVector<ModelBoneFlexDriverControl_t> m_controls; // 0x10	
};
// Alignment: 2
// Size: 0x48
class CModelConfigElement
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x8]; // 0x0
public:
	CUtlString m_ElementName; // 0x8	
	CUtlVector<CModelConfigElement*> m_NestedElements; // 0x10	
};
// Alignment: 3
// Size: 0x28
class CModelConfig
{
public:
	CUtlString m_ConfigName; // 0x0	
	CUtlVector<CModelConfigElement*> m_Elements; // 0x8	
	bool m_bTopLevel; // 0x20	
};
// Alignment: 3
// Size: 0x20
class CModelConfigList
{
public:
	bool m_bHideMaterialGroupInTools; // 0x0	
	bool m_bHideRenderColorInTools; // 0x1	
private:
	[[maybe_unused]] uint8_t __pad0002[0x6]; // 0x2
public:
	CUtlVector<CModelConfig*> m_Configs; // 0x8	
};
// Alignment: 2
// Size: 0x10
struct PermModelDataAnimatedMaterialAttribute_t
{
public:
	CUtlString m_AttributeName; // 0x0	
	int32_t m_nNumChannels; // 0x8	
};
#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define PAD(SIZE) BYTE MACRO_CONCAT(_pad, __COUNTER__)[SIZE];
enum bone_flags : uint32_t {
	FLAG_NO_BONE_FLAGS = 0x0,
	FLAG_BONEFLEXDRIVER = 0x4,
	FLAG_CLOTH = 0x8,
	FLAG_PHYSICS = 0x10,
	FLAG_ATTACHMENT = 0x20,
	FLAG_ANIMATION = 0x40,
	FLAG_MESH = 0x80,
	FLAG_HITBOX = 0x100,
	FLAG_BONE_USED_BY_VERTEX_LOD0 = 0x400,
	FLAG_BONE_USED_BY_VERTEX_LOD1 = 0x800,
	FLAG_BONE_USED_BY_VERTEX_LOD2 = 0x1000,
	FLAG_BONE_USED_BY_VERTEX_LOD3 = 0x2000,
	FLAG_BONE_USED_BY_VERTEX_LOD4 = 0x4000,
	FLAG_BONE_USED_BY_VERTEX_LOD5 = 0x8000,
	FLAG_BONE_USED_BY_VERTEX_LOD6 = 0x10000,
	FLAG_BONE_USED_BY_VERTEX_LOD7 = 0x20000,
	FLAG_BONE_MERGE_READ = 0x40000,
	FLAG_BONE_MERGE_WRITE = 0x80000,
	FLAG_ALL_BONE_FLAGS = 0xfffff,
	BLEND_PREALIGNED = 0x100000,
	FLAG_RIGIDLENGTH = 0x200000,
	FLAG_PROCEDURAL = 0x400000,
};

struct alignas(16) bone_data {
	Vector_t pos;
	float scale;
	Vector4D_t rot;
};

#include "../sdk/datatypes/stronghandle.hpp"
class CModel {
public:
	CModel() = delete;
	CModel(CModel&&) = delete;
	CModel(const CModel&) = delete;
public:

	SCHEMA_ADD_OFFSET(const char*, szName, 0x8);
	SCHEMA_ADD_OFFSET(CModelSkeleton, m_modelSkeleton, 0x188);
	SCHEMA_ADD_OFFSET(CRenderM*, m_meshes, 0x78);
	//uint32_t GetHitboxesNum();
	uint32_t GetHitboxFlags(uint32_t index);
	const char* GetHitboxName(uint32_t index);
	uint32_t GetHitboxParent(uint32_t index);

	uint32_t GetHitboxesNum();

};

class CModelState
{
public:
	CModelState() = delete;
	CModelState(CModelState&&) = delete;
	CModelState(const CModelState&) = delete;

	SCHEMA_ADD_FIELD(CStrongHandle< CModel >, m_hModel, "CModelState->m_hModel");
	SCHEMA_ADD_OFFSET(CTransform*, BoneTransform, 0x80);

	bone_data* GetHitboxData() noexcept {
		bone_data* boneDataPtr = *reinterpret_cast<bone_data**>(this + 0x80);

		if (boneDataPtr == nullptr)
			boneDataPtr = *reinterpret_cast<bone_data**>(reinterpret_cast<uintptr_t>(this) + 0x80);

		CS_ASSERT(boneDataPtr != nullptr);

		return boneDataPtr;
	}


	Vector_t GetHitboxPos(uint32_t index) {
		auto hitbox = this->GetHitboxData();
		if (!hitbox)
			return nullptr;

		if (!(this->m_hModel()->GetHitboxFlags(index) & bone_flags::FLAG_HITBOX))
			return nullptr;

		auto parent_index = this->m_hModel()->GetHitboxParent(index);
		if (parent_index == -1)
			return nullptr;

		return hitbox[index].pos;
	}

	Vector4D_t GetHitboxRotation(uint32_t index) {
		auto hitbox = this->GetHitboxData();
		if (!hitbox)
			return 0;

		if (!(this->m_hModel()->GetHitboxFlags(index) & bone_flags::FLAG_HITBOX))
			return 0;

		auto parent_index = this->m_hModel()->GetHitboxParent(index);
		if (parent_index == -1)
			return 0;

		return hitbox[index].rot;
	}

	const char* GetHitboxName(uint32_t index) {
		auto hitbox = this->GetHitboxData();
		if (!hitbox)
			return nullptr;

		if (!(this->m_hModel()->GetHitboxFlags(index) & bone_flags::FLAG_HITBOX))
			return nullptr;

		auto parent_index = this->m_hModel()->GetHitboxParent(index);
		if (parent_index == -1)
			return nullptr;

		return this->m_hModel()->GetHitboxName(index);
	}
};



class CSkeletonInstance
{
public:
	CSkeletonInstance() = delete;
	CSkeletonInstance(CSkeletonInstance&&) = delete;
	CSkeletonInstance(const CSkeletonInstance&) = delete;

	SCHEMA_ADD_FIELD(CModelState, GetModel, "CSkeletonInstance->m_modelState");
	SCHEMA_ADD_FIELD(bool, m_bIsAnimationEnabled, "CSkeletonInstance->m_bIsAnimationEnabled");
	SCHEMA_ADD_FIELD(bool, m_bUseParentRenderBounds, "CSkeletonInstance->m_bUseParentRenderBounds");
	SCHEMA_ADD_FIELD(bool, m_bDisableSolidCollisionsForHierarchy, "CSkeletonInstance->m_bDisableSolidCollisionsForHierarchy");
	SCHEMA_ADD_FIELD(bool, m_bDirtyMotionType, "CSkeletonInstance->m_bDirtyMotionType");
	SCHEMA_ADD_FIELD(bool, m_bIsGeneratingLatchedParentSpaceState, "CSkeletonInstance->m_bIsGeneratingLatchedParentSpaceState");
	SCHEMA_ADD_FIELD(uint8_t, m_nHitboxSet, "CSkeletonInstance->m_nHitboxSet");
	void get_bone_data(bone_data& data, int index);
	void CS_FASTCALL calc_world_space_bones(uint32_t parent, uint32_t mask);
	void CS_FASTCALL spoofed_calc_world_space_bones(uint32_t mask);
};

class CGameSceneNode
{
public:
	CS_CLASS_NO_INITIALIZER(CGameSceneNode);

	void SetMeshGroupMask(uint64_t meshGroupMask);

	SCHEMA_ADD_FIELD(CTransform, GetNodeToWorld, "CGameSceneNode->m_nodeToWorld");
	SCHEMA_ADD_FIELD(CEntityInstance*, GetOwner, "CGameSceneNode->m_pOwner");
	SCHEMA_ADD_FIELD(Vector_t, m_vecOrigin, "CGameSceneNode->m_vecOrigin");
	SCHEMA_ADD_FIELD(Vector_t, GetAbsOrigin, "CGameSceneNode->m_vecAbsOrigin");
	SCHEMA_ADD_FIELD(Vector_t, GetRenderOrigin, "CGameSceneNode->m_vRenderOrigin");

	SCHEMA_ADD_FIELD(QAngle_t, GetAngleRotation, "CGameSceneNode->m_angRotation");
	SCHEMA_ADD_FIELD(QAngle_t, GetAbsAngleRotation, "CGameSceneNode->m_angAbsRotation");
	SCHEMA_ADD_FIELD(CTransform, GetTransform, "CGameSceneNode->m_nodeToWorld");

	SCHEMA_ADD_FIELD(bool, IsDormant, "CGameSceneNode->m_bDormant");

	CSkeletonInstance* GetSkeletonInstance()
	{
		return MEM::CallVFunc<CSkeletonInstance*, 8U>(this);
	}
};



class EntSubClassVDataBase {
public:
	template <typename T> requires std::derived_from<T, EntSubClassVDataBase>
		inline T* as() {
			return (T*)this;
		}
};

class BasePlayerWeaponVData : public EntSubClassVDataBase {
public:
	SCHEMA_ADD_OFFSET(int32_t, m_iMaxClip1, 0x1FC);

};
enum CSWeaponCategory : std::uint32_t {
	WEAPONCATEGORY_OTHER = 0,
	WEAPONCATEGORY_MELEE = 1,
	WEAPONCATEGORY_SECONDARY = 2,
	WEAPONCATEGORY_SMG = 3,
	WEAPONCATEGORY_RIFLE = 4,
	WEAPONCATEGORY_HEAVY = 5,
	WEAPONCATEGORY_COUNT = 6,
};

class CCSWeaponBaseVData : public BasePlayerWeaponVData {
public:
	SCHEMA_ADD_OFFSET(const char*, m_szName, 0xC20);
	SCHEMA_ADD_OFFSET(uint32_t, m_WeaponType, 0x248);
	SCHEMA_ADD_OFFSET(CSWeaponCategory, m_WeaponCategory, 0x24C);
	SCHEMA_ADD_OFFSET(CFiringModeFloat, m_flSpread, 0xC54);
	SCHEMA_ADD_OFFSET(float, m_flHeadshotMultiplier, 0xD44);
	SCHEMA_ADD_OFFSET(float, m_flArmorRatio, 0xD48);
	SCHEMA_ADD_OFFSET(float, m_flPenetration, 0xD4C);
	SCHEMA_ADD_OFFSET(float, m_flRange, 0xD50);
	SCHEMA_ADD_OFFSET(float, m_flRangeModifier, 0xD54);
	SCHEMA_ADD_OFFSET(bool, m_bIsFullAuto, 0xC3C);

	SCHEMA_ADD_OFFSET(int32_t, m_nDamage, 0xD40);
	SCHEMA_ADD_OFFSET(CFiringModeFloat, m_flInaccuracyCrouch, 0xC5C);
	SCHEMA_ADD_OFFSET(CFiringModeFloat, m_flInaccuracyStand, 0xC64);
	SCHEMA_ADD_OFFSET(CFiringModeFloat, m_flInaccuracyJump, 0xC6C);
	SCHEMA_ADD_OFFSET(CFiringModeFloat, m_flInaccuracyMove, 0xC8C);
	SCHEMA_ADD_OFFSET(CFiringModeFloat, m_flCycleTime, 0xC44);
	SCHEMA_ADD_OFFSET(CFiringModeFloat, m_flMaxSpeed, 0xC4C);


};

class C_BaseEntity : public CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseEntity);

	SCHEMA_ADD_FIELD(CGameSceneNode*, GetGameSceneNode, "C_BaseEntity->m_pGameSceneNode");
	SCHEMA_ADD_FIELD(CCollisionProperty*, GetCollision, "C_BaseEntity->m_pCollision");
	SCHEMA_ADD_FIELD(std::uint8_t, GetTeam, "C_BaseEntity->m_iTeamNum");
	SCHEMA_ADD_FIELD(CBaseHandle, GetOwnerHandle, "C_BaseEntity->m_hOwnerEntity");
	SCHEMA_ADD_FIELD(Vector_t, GetBaseVelocity, "C_BaseEntity->m_vecBaseVelocity");
	SCHEMA_ADD_FIELD(Vector_t, GetAbsVelocity, "C_BaseEntity->m_vecAbsVelocity");
	SCHEMA_ADD_FIELD(float, m_flSimulationTime, "C_BaseEntity->m_flSimulationTime");

	SCHEMA_ADD_FIELD(Vector_t, m_vecVelocity, "C_BaseEntity->m_vecVelocity");
	SCHEMA_ADD_FIELD(bool, IsTakingDamage, "C_BaseEntity->m_bTakesDamage");
	SCHEMA_ADD_FIELD(std::uint32_t, GetFlags, "C_BaseEntity->m_fFlags");
	SCHEMA_ADD_FIELD(std::int32_t, GetEflags, "C_BaseEntity->m_iEFlags");
	SCHEMA_ADD_FIELD(std::int32_t, GetMoveType, "C_BaseEntity->m_MoveType");
	SCHEMA_ADD_FIELD(std::uint8_t, GetLifeState, "C_BaseEntity->m_lifeState");
	SCHEMA_ADD_FIELD(std::int32_t, GetHealth, "C_BaseEntity->m_iHealth");
	SCHEMA_ADD_FIELD(std::int32_t, GetMaxHealth, "C_BaseEntity->m_iMaxHealth");
	SCHEMA_ADD_FIELD(float, GetWaterLevel, "C_BaseEntity->m_flWaterLevel");
	SCHEMA_ADD_FIELD_OFFSET(EntSubClassVDataBase*, m_pVDataBase, "C_BaseEntity->m_nSubclassID", 0x8);
	SCHEMA_ADD_FIELD(Vector_t, m_vecViewOffset, "C_BaseModelEntity->m_vecViewOffset");

	void HitboxToWorldTransforms(uint32_t mask);
	bool ComputeHitboxSurroundingBox(Vector_t& min, Vector_t& max);
	static C_BaseEntity* GetLocalPlayer();
	// get entity origin on scene
	[[nodiscard]] const Vector_t& GetSceneOrigin();

	// @note: we shouldn't use this, just use GetSchemaClassInfo(...) and compare the class name instead
	[[nodiscard]] bool IsBasePlayerController()
	{
		return MEM::CallVFunc<bool, 145U>(this);
	}

	// same with this
	[[nodiscard]] bool IsWeapon()
	{
		return MEM::CallVFunc<bool, 150U>(this);
	}

	// same with this
	[[nodiscard]] bool IsViewModel()
	{
		return MEM::CallVFunc<bool, 242U>(this);
	}

	[[nodiscard]] bool IsValidMoveType() noexcept {
		if (!this)
			return false;

		const auto move_type = this->GetMoveType();
		return move_type != MOVETYPE_NOCLIP
			&& move_type != MOVETYPE_LADDER;
	}
};


class CGlowProperty;
class C_BaseModelEntity : public C_BaseEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseModelEntity);

	SCHEMA_ADD_FIELD(CCollisionProperty, GetCollisionInstance, "C_BaseModelEntity->m_Collision");
	SCHEMA_ADD_FIELD(CGlowProperty, GetGlowProperty, "C_BaseModelEntity->m_Glow");
	SCHEMA_ADD_FIELD(Vector_t, GetViewOffset, "C_BaseModelEntity->m_vecViewOffset");
	SCHEMA_ADD_FIELD(GameTime_t, GetCreationTime, "C_BaseModelEntity->m_flCreateTime");
	SCHEMA_ADD_FIELD(GameTick_t, GetCreationTick, "C_BaseModelEntity->m_nCreationTick");
	SCHEMA_ADD_FIELD(CBaseHandle, GetMoveParent, "C_BaseModelEntity->m_hOldMoveParent");
	SCHEMA_ADD_FIELD(std::float_t, GetAnimTime, "C_BaseModelEntity->m_flAnimTime");
	SCHEMA_ADD_FIELD(std::float_t, GetSimulationTime, "C_BaseModelEntity->m_flSimulationTime");
	void SetModel(const char* name);


};
enum ESOCacheEvent {
	/// Dummy sentinel value
	eSOCacheEvent_None = 0,

	/// We received a our first update from the GC and are subscribed
	eSOCacheEvent_Subscribed = 1,

	/// We lost connection to GC or GC notified us that we are no longer
	/// subscribed. Objects stay in the cache, but we no longer receive updates
	eSOCacheEvent_Unsubscribed = 2,

	/// We received a full update from the GC on a cache for which we were
	/// already subscribed. This can happen if connectivity is lost, and then
	/// restored before we realized it was lost.
	eSOCacheEvent_Resubscribed = 3,

	/// We received an incremental update from the GC about specific object(s)
	/// being added, updated, or removed from the cache
	eSOCacheEvent_Incremental = 4,

	/// A lister was added to the cache
	/// @see CGCClientSharedObjectCache::AddListener
	eSOCacheEvent_ListenerAdded = 5,

	/// A lister was removed from the cache
	/// @see CGCClientSharedObjectCache::RemoveListener
	eSOCacheEvent_ListenerRemoved = 6,
};

class CPlayer_CameraServices;

class CPlayer_WeaponServices : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(CPlayer_WeaponServices);

	SCHEMA_ADD_OFFSET(GameTime_t, m_flNextAttack, 0xB8);

	SCHEMA_ADD_FIELD(CBaseHandle, m_hActiveWeapon, "CPlayer_WeaponServices->m_hActiveWeapon");
	SCHEMA_ADD_FIELD(uint16_t[32], m_iAmmo, "CPlayer_WeaponServices->m_iAmmo");

};

class CCSPlayer_ViewModelServices {
public:

	SCHEMA_ADD_FIELD(CBaseHandle, m_hViewModel, "CCSPlayer_ViewModelServices->m_hViewModel");

};
class CSharedObject;

class CAnimGraphNetworkedVariables;

class CAnimationGraphInstance {
public:
	char pad_0x0000[0x2E0]; //0x0000
	CAnimGraphNetworkedVariables* pAnimGraphNetworkedVariables; //0x02E0 
};

class C_BaseViewModel : public C_BaseModelEntity {
public:
	SCHEMA_ADD_FIELD(CBaseHandle, m_hWeapon, "C_BaseViewModel->m_hWeapon");

};

class C_CSGOViewModel : public C_BaseViewModel {
public:
	char pad_0x0000[0xD08]; //0x0000
	CAnimationGraphInstance* pAnimationGraphInstance; //0x0D08 
};
class CGCClientSharedObjectTypeCache {
public:
	auto AddObject(CSharedObject* pObject) {
		return MEM::CallVFunc<bool, 1u>(this, pObject);
	}

	auto RemoveObject(CSharedObject* soIndex) {
		return MEM::CallVFunc<CSharedObject*, 3u>(this, soIndex);

	}

	template <typename T>
	auto& GetVecObjects() {
		return *reinterpret_cast<CUtlVector<T>*>((uintptr_t)(this) + 0x8);
	}
};

class CCSInputMessage
{
public:
	int32_t m_frame_tick_count; //0x0000
	float m_frame_tick_fraction; //0x0004
	int32_t m_player_tick_count; //0x0008
	float m_player_tick_fraction; //0x000C
	Vector_t m_view_angles; //0x0010
	Vector_t m_shoot_position; //0x001C
	int32_t m_target_index; //0x0028
	Vector_t m_target_head_position; //0x002C
	Vector_t m_target_abs_origin; //0x0038
	Vector_t m_target_angle; //0x0044
	int32_t m_sv_show_hit_registration; //0x0050
	int32_t m_entry_index_max; //0x0054
	int32_t m_index_idk; //0x0058
}; //Size: 0x005C


class CGCClientSharedObjectCache {
public:
	CGCClientSharedObjectTypeCache* CreateBaseTypeCache(int nClassID);
};
struct SOID_t {
	uint64_t m_id;
	uint32_t m_type;
	uint32_t m_padding;
};

class CGCClient {
public:
	CGCClientSharedObjectCache* FindSOCache(SOID_t ID,
		bool bCreateIfMissing = true);
};

class CEconItem {
	void SetDynamicAttributeValue(int index, void* value);
	void SetDynamicAttributeValueString(int index, const char* value);

public:
	static CEconItem* CreateInstance();

	void Destruct() {
		return MEM::CallVFunc<void, 1U>(this);
	}

	void SetPaintKit(float kit) { SetDynamicAttributeValue(6, &kit); }
	void SetPaintSeed(float seed) { SetDynamicAttributeValue(7, &seed); }
	void SetPaintWear(float wear) { SetDynamicAttributeValue(8, &wear); }
	void SetStatTrak(int count) { SetDynamicAttributeValue(80, &count); }
	void SetStatTrakType(int type) { SetDynamicAttributeValue(81, &type); }
	void SetCustomName(const char* pName) {
		SetDynamicAttributeValueString(111, pName);
	}

	char pad0[0x10];  // 2 vtables
	uint64_t m_ulID;
	uint64_t m_ulOriginalID;
	void* m_pCustomDataOptimizedObject;
	uint32_t m_unAccountID;
	uint32_t m_unInventory;
	uint16_t m_unDefIndex;
	uint16_t m_unOrigin : 5;
	uint16_t m_nQuality : 4;
	uint16_t m_unLevel : 2;
	uint16_t m_nRarity : 4;
	uint16_t m_dirtybitInUse : 1;
	int16_t m_iItemSet;
	int m_bSOUpdateFrame;
	uint8_t m_unFlags;
};

class CEconItemDefinition {
public:
	bool IsWeapon();
	bool IsKnife(bool excludeDefault, const char* name);
	bool IsGlove(bool excludeDefault, const char* name);

	auto GetModelName() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0xD8);
	}

	auto GetStickersSupportedCount() {
		return *reinterpret_cast<int*>((uintptr_t)(this) + 0x100); // 0x118
	}

	auto GetSimpleWeaponName() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x1F0);
	}

	auto GetLoadoutSlot() {
		return *reinterpret_cast<int*>((uintptr_t)(this) + 0x2E8);
	}

	char pad0[0x8];  // vtable
	void* m_pKVItem;
	uint16_t m_nDefIndex;
	CUtlVector<uint16_t> m_nAssociatedItemsDefIndexes;
	bool m_bEnabled;
	const char* m_szPrefab;
	uint8_t m_unMinItemLevel;
	uint8_t m_unMaxItemLevel;
	uint8_t m_nItemRarity;
	uint8_t m_nItemQuality;
	uint8_t m_nForcedItemQuality;
	uint8_t m_nDefaultDropItemQuality;
	uint8_t m_nDefaultDropQuantity;
	CUtlVector<void*> m_vecStaticAttributes;
	uint8_t m_nPopularitySeed;
	void* m_pPortraitsKV;
	const char* m_pszItemBaseName;
	bool m_bProperName;
	const char* m_pszItemTypeName;
	uint32_t m_unItemTypeID;
	const char* m_pszItemDesc;
};

class CGCClientSystem {
public:
	CGCClientSystem* GetInstance();
	CGCClient* GetCGCClient() {
		return reinterpret_cast<CGCClient*>((uintptr_t)(this) + 0xB8);
	}
};
class CCSPlayerInventory;
class C_EconItemView {
public:
	CS_CLASS_NO_INITIALIZER(C_EconItemView);
	CEconItem* GetSOCData(CCSPlayerInventory* inventory);
	auto GetCustomPaintKitIndex() { return MEM::CallVFunc<int, 2u>(this); }
	auto GetStaticData() { return MEM::CallVFunc<CEconItemDefinition*, 13u>(this); }
	SCHEMA_ADD_FIELD(uint32_t, m_iAccountID, "C_EconItemView->m_iAccountID");
	SCHEMA_ADD_FIELD(uint32_t, m_iItemIDHigh, "C_EconItemView->m_iItemIDHigh");
	SCHEMA_ADD_FIELD(bool, m_bDisallowSOC, "C_EconItemView->m_bDisallowSOC");
	SCHEMA_ADD_FIELD(uint32_t, m_iItemIDLow, "C_EconItemView->m_iItemIDLow");
	SCHEMA_ADD_FIELD(bool, m_bInitialized, "C_EconItemView->m_bInitialized");
	SCHEMA_ADD_OFFSET(bool, m_bIsStoreItem, 0x1EA);
	SCHEMA_ADD_OFFSET(bool, m_bIsTradeItem, 0x1EA);


	SCHEMA_ADD_FIELD(uint64_t, m_iItemID, "C_EconItemView->m_iItemID");
	SCHEMA_ADD_FIELD(uint64_t, m_iItemDefinitionIndex, "C_EconItemView->m_iItemDefinitionIndex");
	SCHEMA_ADD_FIELD(const char*, m_szCustomName, "C_EconItemView->m_szCustomName");

};

class CAttributeManager {
public:
	CS_CLASS_NO_INITIALIZER(CAttributeManager);


	SCHEMA_ADD_OFFSET(C_EconItemView, m_Item, 0x50);
};

class C_EconEntity : public C_BaseModelEntity {
public:
	CS_CLASS_NO_INITIALIZER(C_EconEntity);
	/* offset */
	SCHEMA_ADD_FIELD(CAttributeManager, m_AttributeManager, "C_EconEntity->m_AttributeManager");
	SCHEMA_ADD_FIELD(uint32_t, m_OriginalOwnerXuidLow, "C_EconEntity->m_OriginalOwnerXuidLow");
	SCHEMA_ADD_FIELD(uint32_t, m_OriginalOwnerXuidHigh, "C_EconEntity->m_OriginalOwnerXuidHigh");
	SCHEMA_ADD_OFFSET(bool, m_bShouldGlow, 0x15B8);

	uint64_t GetOriginalOwnerXuid() {
		return ((uint64_t)(m_OriginalOwnerXuidHigh()) << 32) |
			m_OriginalOwnerXuidLow();
	}

};


class CBasePlayerWeapon : public C_EconEntity {
public:
	CS_CLASS_NO_INITIALIZER(CBasePlayerWeapon);

	SCHEMA_ADD_FIELD(GameTick_t, m_nNextPrimaryAttackTick, "C_BasePlayerWeapon->m_nNextPrimaryAttackTick");
	SCHEMA_ADD_FIELD(float, m_flNextPrimaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextPrimaryAttackTickRatio");
	SCHEMA_ADD_FIELD(GameTick_t, m_nNextSecondaryAttackTick, "C_BasePlayerWeapon->m_nNextSecondaryAttackTick");
	SCHEMA_ADD_FIELD(float, m_flNextSecondaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextSecondaryAttackTickRatio");
	SCHEMA_ADD_OFFSET(float, m_flRecoilIndex, 0x1734);

	SCHEMA_ADD_FIELD(int32_t, clip1, "C_BasePlayerWeapon->m_iClip1");
	BasePlayerWeaponVData* GetVData() noexcept {
		return m_pVDataBase()->as<BasePlayerWeaponVData>();
	}
};

enum CSWeaponMode : int /* it works atleast */
{
	Primary_Mode = 0,
	Secondary_Mode = 1,
	WeaponMode_MAX = 2,
};
//xored
// Created with ReClass.NET by KN4CK3R

class unknknownpointer
{
public:
	char pad_0000[328]; //0x0000
}; //Size: 0x0148

class N000001D5
{
public:
	char pad_0000[72]; //0x0000
}; //Size: 0x0048

class N0000008F
{
public:
	char pad_0000[72]; //0x0000
}; //Size: 0x0048

class N000001E0
{
public:
	char pad_0000[1352]; //0x0000
}; //Size: 0x0548

class N0000028B
{
public:
	char pad_0000[1096]; //0x0000
}; //Size: 0x0448

class IPrediction
{
public:
	void* N00000001; //0x0000
	char pad_0008[32]; //0x0008
	void* N00000006; //0x0028
	char pad_0030[152]; //0x0030
	float LocalPlayerUnkPos; //0x00C8
	float timesinceinair; //0x00CC
	char pad_00D0[24]; //0x00D0
	float N0000001E; //0x00E8
	char pad_00EC[220]; //0x00EC
	void* buttonstates; //0x01C8
	void* N0000003B; //0x01D0
	void* N0000003C; //0x01D8
	char pad_01E0[240]; //0x01E0
	Vector_t parentidxorigin; //0x02D0
	char pad_02DC[96]; //0x02DC
	uint64_t parentindex; //0x033C
	char pad_0344[96]; //0x0344
	float m_flRadius; //0x03A4
	char pad_03A8[4252]; //0x03A8
}; //Size: 0x1444


class CNetInputMessage;

class C_CSWeaponBase : public CBasePlayerWeapon {
public:
	void AddStattrakEntity();
	void AddNametagEntity();
	SCHEMA_ADD_OFFSET(CCSWeaponBaseVData*, datawep, 0x368);
	SCHEMA_ADD_OFFSET(int32_t, m_zoomLevel, 0x1A70);
	SCHEMA_ADD_OFFSET(bool, m_bInReload, 0x1744);
	SCHEMA_ADD_FIELD(float, m_fAccuracyPenalty, "C_CSWeaponBase->m_fAccuracyPenalty");
	SCHEMA_ADD_FIELD(CSWeaponMode, m_weaponMode, CS_XOR("C_CSWeaponBase->m_weaponMode"));
	SCHEMA_ADD_FIELD(int, m_iOriginalTeamNumber, CS_XOR("C_CSWeaponBase->m_iOriginalTeamNumber"));

	SCHEMA_ADD_FIELD(bool, m_bUIWeapon, CS_XOR("C_CSWeaponBase->m_bUIWeapon"));
	//SCHEMA_ADD_FIELD(void, m_hStattrakAttachment, CS_XOR("C_CSWeaponBase->m_iNumEmptyAttacks"));
	//SCHEMA_ADD_FIELD(void, m_hNametagAttachment, CS_XOR("C_CSWeaponBase->m_iNumEmptyAttacks"));
	__forceinline float get_spread() {
		using original_fn = float(__fastcall*)(void*);
		return (*(original_fn**)this)[349](this);
	}

	__forceinline float get_inaccuracy() {
		using original_fn = float(__fastcall*)(void*);
		return (*(original_fn**)this)[399](this);

	}

	// for reference.    
// sv_accelerate_use_weapon_speed inside "CGameMovement::Accelerate"
// 48 8B C4 F3 0F 11 50 ? 41 55 41 56 41 57 48 81 EC ? ? ? ? F2 41 0F 10 21 4D 8B E9 F2 0F 10 5A ? 0F 28 C4 44 0F 29 40 ? 0F 28 CB F3 44 0F 10 42 ? 4C 8B FA F3 45 0F 59 41 ? 
// mov     rcx, rbx
// call    qword ptr [ rax + 0AF8h ] <- index
// mov     rax, [ rbx ]
	__forceinline float get_max_speed() {
		using original_fn = float(__fastcall*)(void*);
		return (*(original_fn**)this)[340](this);
	}

};

class C_BaseCSGrenade : public C_CSWeaponBase
{
public:
	SCHEMA_ADD_FIELD(bool, IsHeldByPlayer, "C_BaseCSGrenade->m_bIsHeldByPlayer");
	SCHEMA_ADD_FIELD(bool, IsPinPulled, "C_BaseCSGrenade->m_bPinPulled");
	SCHEMA_ADD_FIELD(GameTime_t, GetThrowTime, "C_BaseCSGrenade->m_fThrowTime");
	SCHEMA_ADD_FIELD(float, GetThrowStrength, "C_BaseCSGrenade->m_flThrowStrength");
};

class C_CSPlayerPawnBase : public C_BaseEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSPlayerPawnBase);

	SCHEMA_ADD_FIELD(CCSPlayer_ViewModelServices*, GetViewModelServices, CS_XOR("C_CSPlayerPawnBase->m_pViewModelServices"));
	SCHEMA_ADD_FIELD(bool, IsScoped, CS_XOR("C_CSPlayerPawnBase->m_bIsScoped"));
	SCHEMA_ADD_FIELD(bool, IsDefusing, CS_XOR("C_CSPlayerPawnBase->m_bIsDefusing"));
	SCHEMA_ADD_FIELD(bool, IsGrabbingHostage, CS_XOR("C_CSPlayerPawnBase->m_bIsGrabbingHostage"));
	SCHEMA_ADD_FIELD(float, GetLowerBodyYawTarget, CS_XOR("C_CSPlayerPawnBase->m_flLowerBodyYawTarget"));
	SCHEMA_ADD_FIELD(int, GetShotsFired, CS_XOR("C_CSPlayerPawnBase->m_iShotsFired"));
	SCHEMA_ADD_FIELD(float, GetFlashMaxAlpha, CS_XOR("C_CSPlayerPawnBase->m_flFlashMaxAlpha"));
	SCHEMA_ADD_FIELD(float, GetFlashDuration, CS_XOR("C_CSPlayerPawnBase->m_flFlashDuration"));
	SCHEMA_ADD_FIELD(Vector_t, GetLastSmokeOverlayColor, CS_XOR("C_CSPlayerPawnBase->m_vLastSmokeOverlayColor"));
	SCHEMA_ADD_FIELD(int, GetSurvivalTeam, CS_XOR("C_CSPlayerPawnBase->m_nSurvivalTeam")); // danger zone
	SCHEMA_ADD_FIELD(std::int32_t, GetArmorValue, CS_XOR("C_CSPlayerPawnBase->m_ArmorValue"));
	SCHEMA_ADD_OFFSET(bool, m_bInLanding, 0x17C0);
	SCHEMA_ADD_FIELD(float, m_flLandingTime, CS_XOR("C_CSPlayerPawnBase->m_flLandingTime"));


};

class CPlayer_MovementServices
{
public:
	CS_CLASS_NO_INITIALIZER(CPlayer_MovementServices);

	SCHEMA_ADD_FIELD(float, m_flMaxspeed, CS_XOR("CPlayer_MovementServices->m_flMaxspeed"));
	SCHEMA_ADD_FIELD(float, m_flForwardMove, CS_XOR("CPlayer_MovementServices->m_flForwardMove"));
	SCHEMA_ADD_FIELD(float, m_flLeftMove, CS_XOR("CPlayer_MovementServices->m_flLeftMove"));
	SCHEMA_ADD_OFFSET(float, m_flSurfaceFriction, 0x1FC);
};

class CPlayer_ItemServices {
public:
	SCHEMA_ADD_FIELD(bool, m_bHasDefuser, CS_XOR("CCSPlayer_ItemServices->m_bHasDefuser"));
	SCHEMA_ADD_FIELD(bool, m_bHasHelmet, CS_XOR("CCSPlayer_ItemServices->m_bHasHelmet"));
	SCHEMA_ADD_FIELD(bool, m_bHasHeavyArmor, CS_XOR("CCSPlayer_ItemServices->m_bHasHeavyArmor"));
};

class C_CSPlayerPawn : public C_CSPlayerPawnBase
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSPlayerPawn);


	SCHEMA_ADD_OFFSET(bool, m_bNeedToReApplyGloves, 0x1918);
	SCHEMA_ADD_OFFSET(C_EconItemView, m_EconGloves, 0x1920);
	SCHEMA_ADD_FIELD(uint8_t, m_nEconGlovesChanged, CS_XOR("C_CSPlayerPawnBase->m_nEconGlovesChanged"));
	SCHEMA_ADD_FIELD(bool, m_bGunGameImmunity, CS_XOR("C_CSPlayerPawnBase->m_bGunGameImmunity"));
	SCHEMA_ADD_FIELD(bool, m_bIsWalking, CS_XOR("C_CSPlayerPawnBase->m_bIsWalking"));
	SCHEMA_ADD_FIELD(CBaseHandle, GetController, CS_XOR("C_BasePlayerPawn->m_hController"));
	SCHEMA_ADD_FIELD(CPlayer_WeaponServices*, GetWeaponServices, CS_XOR("C_BasePlayerPawn->m_pWeaponServices"));
	SCHEMA_ADD_FIELD(CPlayer_ItemServices*, GetItemServices, CS_XOR("C_BasePlayerPawn->m_pItemServices"));
	SCHEMA_ADD_FIELD(CPlayer_CameraServices*, GetCameraServices, CS_XOR("C_BasePlayerPawn->m_pCameraServices"));
	SCHEMA_ADD_FIELD(CPlayer_MovementServices*, m_pMovementServices, CS_XOR("C_BasePlayerPawn->m_pMovementServices"));

	SCHEMA_ADD_OFFSET(CUtlVectorCS2< QAngle_t >, m_aimPunchCache, 0x17A0);
	SCHEMA_ADD_OFFSET(QAngle_t, m_aimPunchAngle, 0x177C);

	[[nodiscard]] Vector_t GetBonePosition(int out);
	[[nodiscard]] bool hasArmour(const int hitgroup);
	[[nodiscard]] bool CalculateBoundingBox(ImVec4& bbox, bool compute_surrounding_box);
	[[nodiscard]] bool IsOtherEnemy(C_CSPlayerPawn* pOther);
	[[nodiscard]] int GetAssociatedTeam();
	[[nodiscard]] void CalculateHitboxData(uint32_t idx, Vector_t& pos, Vector4D_t& rot, float& scale, bool predict = false);
	float GetProperAccuracy();
	float GetProperSpread();
	uint32_t GetHitGroup(int idx);
	bool InsideCrosshair(C_CSPlayerPawn* local, QAngle_t ang, float range);
	[[nodiscard]] bool TracePoint(C_CSPlayerPawn* target, Vector_t point);
	[[nodiscard]] bool Visible(C_CSPlayerPawn* start_ent, int type = 0, bool v = false);
	[[nodiscard]] Vector_t GetEyePosition();


	std::uint32_t GetEntHandle() {
		std::uint32_t Result = -1;
		if (this && this->GetCollision() && !(this->GetCollision()->m_solidFlags() & 4)) {
			Result = this->GetOwnerHandle().GetEntryIndex();
		}
		return Result;
	}
	[[nodiscard]] bool CanShoot(float time);
	[[nodiscard]] bool CanNextAttack(float time);

	[[nodiscard]] C_CSWeaponBase* ActiveWeapon();
};

struct CachedEntity_t {
	enum Type { UNKNOWN = 0, PLAYER_CONTROLLER, BASE_WEAPON, CHICKEN };

	CBaseHandle m_handle;
	Type m_type;
	int index;
	bool reset;
	// std::unordered_map<int, std::unique_ptr<backtrack_entity>> record{ };
	ImVec4 m_bbox;
	bool m_draw;  // If the player is not visible, it will be false.
	Vector_t hitboxpos[30];
};

// typedef for entity callback
using entity_callback = void(*)(int index, const CBaseHandle handle) noexcept;
struct entity_callbacks {
	entity_callback add;
	entity_callback remove;
};

extern std::vector<CachedEntity_t> g_cachedEntities;
extern std::mutex g_cachedEntitiesMutex;
extern std::unordered_multimap<CachedEntity_t::Type, entity_callbacks> g_Callbacks;

namespace EntCache {
	void CacheCurrentEntities();
	void OnAddEntity(CEntityInstance* pEntity, CBaseHandle handle);
	void OnRemoveEntity(CEntityInstance* pEntity, CBaseHandle handle);
	void RegisterCallback(CachedEntity_t::Type type, entity_callback add, entity_callback remove) noexcept;
}  // namespace EntCache

class CBasePlayerController : public C_BaseModelEntity {
public:
	CS_CLASS_NO_INITIALIZER(CBasePlayerController);

	SCHEMA_ADD_FIELD(std::uint64_t, GetSteamId, CS_XOR("CBasePlayerController->m_steamID"));
	SCHEMA_ADD_FIELD(CBaseHandle, GetPawnHandle, CS_XOR("CBasePlayerController->m_hPawn"));
	SCHEMA_ADD_FIELD(bool, IsLocalPlayerController, CS_XOR("CBasePlayerController->m_bIsLocalPlayerController"));
};


class CCSPlayerController : public CBasePlayerController {
public:
	CS_CLASS_NO_INITIALIZER(CCSPlayerController);

	[[nodiscard]] static CCSPlayerController* GetLocalPlayerController();

	bool IsThrowingGrenade(C_CSWeaponBase* pBaseWeapon);

	// @note: always get origin from pawn not controller
	[[nodiscard]] const Vector_t& GetPawnOrigin();

	[[nodiscard]] C_CSWeaponBase* GetPlayerWeapon(C_CSPlayerPawn* pPlayer);

	SCHEMA_ADD_FIELD(std::uint32_t, GetPing, CS_XOR("CCSPlayerController->m_iPing"));
	SCHEMA_ADD_FIELD(const char*, GetPlayerName, CS_XOR("CCSPlayerController->m_sSanitizedPlayerName"));
	SCHEMA_ADD_FIELD(std::int32_t, GetPawnHealth, CS_XOR("CCSPlayerController->m_iPawnHealth"));
	SCHEMA_ADD_FIELD(std::int32_t, GetPawnArmor, CS_XOR("CCSPlayerController->m_iPawnArmor"));
	SCHEMA_ADD_FIELD(bool, m_bPawnHasDefuser, CS_XOR("CCSPlayerController->m_bPawnHasDefuser"));
	SCHEMA_ADD_FIELD(bool, m_bPawnHasHelmet, CS_XOR("CCSPlayerController->m_bPawnHasHelmet"));
	SCHEMA_ADD_FIELD(bool, IsPawnAlive, CS_XOR("CCSPlayerController->m_bPawnIsAlive"));
	SCHEMA_ADD_FIELD(CBaseHandle, GetPlayerPawnHandle, CS_XOR("CCSPlayerController->m_hPlayerPawn"));
	SCHEMA_ADD_FIELD(CBaseHandle, m_hPawn, CS_XOR("CBasePlayerController->m_hPawn"));
	SCHEMA_ADD_FIELD(uint32_t, m_nTickBase, CS_XOR("CBasePlayerController->m_nTickBase"));
	SCHEMA_ADD_FIELD(CBaseHandle, m_hPredictedPawn, CS_XOR("CBasePlayerController->m_hPredictedPawn"));
};