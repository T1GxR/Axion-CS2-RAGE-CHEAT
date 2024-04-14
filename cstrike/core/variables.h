#pragma once

#include "config.h"
#include "../features.h"

#pragma region variables_combo_entries
using VisualOverlayBox_t = int;

enum EVisualOverlayBox : VisualOverlayBox_t
{
	VISUAL_OVERLAY_BOX_NONE = 0,
	VISUAL_OVERLAY_BOX_FULL,
	VISUAL_OVERLAY_BOX_CORNERS,
	VISUAL_OVERLAY_BOX_MAX
};

using VisualChamMaterial_t = int;
enum EVisualsChamMaterials : VisualChamMaterial_t
{
	VISUAL_MATERIAL_PRIMARY_WHITE = 0,
	glowproperty = 1,
	glowproperty2 = 2,
	VISUAL_MATERIAL_MAX
};

using MiscDpiScale_t = int;

enum EMiscDpiScale : MiscDpiScale_t
{
	MISC_DPISCALE_DEFAULT = 0,
	MISC_DPISCALE_125,
	MISC_DPISCALE_150,
	MISC_DPISCALE_175,
	MISC_DPISCALE_200,
	MISC_DPISCALE_MAX
};

#pragma endregion

#pragma region variables_multicombo_entries
using MenuAddition_t = unsigned int;
enum EMenuAddition : MenuAddition_t
{
	MENU_ADDITION_NONE = 0U,
	MENU_ADDITION_DIM_BACKGROUND = 1 << 0,
	MENU_ADDITION_BACKGROUND_PARTICLE = 1 << 1,
	MENU_ADDITION_GLOW = 1 << 2,
	MENU_ADDITION_ALL = MENU_ADDITION_DIM_BACKGROUND | MENU_ADDITION_GLOW
};
using LegitCond_t = unsigned int;
enum LegitCond : LegitCond_t
{
	LEGIT_NONE = 0U,

	LEGIT_IN_AIR = 1 << 0,
	LEGIT_FLASHED = 1 << 1,
	LEGIT_IN_SMOKE = 1 << 2,
	LEGIT_DELAY_SHOT = 1 << 3

};

using ESPFlags_t = unsigned int;
enum EESPFlags : ESPFlags_t
{
	FLAGS_NONE = 0U,
	FLAGS_ARMOR = 1 << 0,
	FLAGS_DEFUSER = 1 << 1
};
#pragma endregion

struct rage_weapon_t {

};
struct Variables_t
{

#pragma region ragebot 
	C_ADD_VARIABLE_ARRAY(int, 7, rage_target_select, 0);
	C_ADD_VARIABLE_ARRAY(int, 7, rage_minimum_damage, 0);
	C_ADD_VARIABLE_ARRAY(int, 7, rage_minimum_hitchance, 0);

	C_ADD_VARIABLE(bool, rage_enable, false);
	C_ADD_VARIABLE_ARRAY(bool, 7, rage_hitchance, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, rapid_fire, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, rage_penetration, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, rage_safe_point, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, rage_auto_stop, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, rage_early_stop, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, rage_auto_scope, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, remove_weapon_accuracy_spread, 0);

	C_ADD_VARIABLE_ARRAY(bool, 7, hitbox_head, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, hitbox_neck, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, hitbox_uppeer_chest, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, hitbox_chest, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, hitbox_stomach, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, hitbox_legs, 0);
	C_ADD_VARIABLE_ARRAY(bool, 7, hitbox_feet, 0);



#pragma endregion
#pragma region variables_world 
	C_ADD_VARIABLE(bool, bNightmode, false);

#pragma endregion
#pragma region variables_visuals
	C_ADD_VARIABLE(bool, bVisualOverlay, false);

	C_ADD_VARIABLE(bool, bRemoveChamsT, false);

	C_ADD_VARIABLE(bool, bRemoveChamsOcclude, false);


	C_ADD_VARIABLE(FrameOverlayVar_t, overlayBox, FrameOverlayVar_t(false));
	C_ADD_VARIABLE(TextOverlayVar_t, overlayName, TextOverlayVar_t(false, false));
	C_ADD_VARIABLE(BarOverlayVar_t, overlayHealthBar, BarOverlayVar_t(false, false, false, 2.1f, Color_t(0, 255, 155), Color_t(255, 0, 155)));
	C_ADD_VARIABLE(BarOverlayVar_t, backgroundHealthbar, BarOverlayVar_t(false, false, false, 1.f, Color_t(0, 0, 0, 55), Color_t(0, 0, 0, 55)));

	C_ADD_VARIABLE(BarOverlayVar_t, AmmoBar, BarOverlayVar_t(false, false, false, 1.f, Color_t(0, 255, 155), Color_t(255, 0, 155)));
	C_ADD_VARIABLE(BarOverlayVar_t, AmmoBarBackground, BarOverlayVar_t(false, false, false, 1.f, Color_t(0, 0, 0, 55), Color_t(0, 0, 0, 55)));
	bool full_update = false;
	C_ADD_VARIABLE(bool, bVisualChams, false);
	C_ADD_VARIABLE(bool, bSkeleton, false);

	C_ADD_VARIABLE(int, nVisualChamMaterial, 0);
	C_ADD_VARIABLE(bool, bVisualChamsIgnoreZ, false); // invisible chams
	C_ADD_VARIABLE(bool, bNoShadow, false);

	C_ADD_VARIABLE(ColorPickerVar_t, colVisualChams, ColorPickerVar_t(0, 255, 0));
	C_ADD_VARIABLE(ColorPickerVar_t, colVisualChamsIgnoreZ, ColorPickerVar_t(255, 0, 0));
	C_ADD_VARIABLE(ColorPickerVar_t, colModulate, ColorPickerVar_t(255, 0, 0));

#pragma endregion
#pragma region legit
	C_ADD_VARIABLE(bool, legit_enable, false);
	C_ADD_VARIABLE(int, legit_weapon_selection, 0);
	C_ADD_VARIABLE(bool, legit_silent_aim, false);
	C_ADD_VARIABLE(bool, legit_delay_aim, false);
	C_ADD_VARIABLE(int, legit_delay_aim_ms, 0);

	C_ADD_VARIABLE(int, legit_target_selection, 0);
	C_ADD_VARIABLE(int, legit_target_selection_machinegun, 0);
	C_ADD_VARIABLE(int, legit_target_selection_assultrifles, 0);
	C_ADD_VARIABLE(int, legit_target_selection_snipers, 0);

	C_ADD_VARIABLE(bool, legit_fov_visualize, false);


	C_ADD_VARIABLE(ColorPickerVar_t, legit_fov_visualizeclr, ColorPickerVar_t(255, 88, 88));



	C_ADD_VARIABLE(int, legit_smooth_pistol, 1);
	C_ADD_VARIABLE(int, legit_smooth_machinegun, 1);
	C_ADD_VARIABLE(int, legit_smooth_assultrifles, 1);
	C_ADD_VARIABLE(int, legit_smooth_snipers, 1);

	C_ADD_VARIABLE(int, legit_rcs_shots_pistol, 0);
	C_ADD_VARIABLE(int, legit_rcs_shots_machinegun, 0);
	C_ADD_VARIABLE(int, legit_rcs_shots_assultrifles, 0);
	C_ADD_VARIABLE(int, legit_rcs_shots_snipers, 0);

	C_ADD_VARIABLE(bool, legit_rcs_pistol, false);
	C_ADD_VARIABLE(bool, legit_rcs_machinegun, false);
	C_ADD_VARIABLE(bool, legit_rcs_assultrifles, false);
	C_ADD_VARIABLE(bool, legit_rcs_snipers, false);

	C_ADD_VARIABLE(bool, PunchRandomization_pistol, false);
	C_ADD_VARIABLE(bool, PunchRandomization_machinegun, false);
	C_ADD_VARIABLE(bool, PunchRandomization_assultrifles, false);
	C_ADD_VARIABLE(bool, PunchRandomization_snipers, false);

	C_ADD_VARIABLE(float, punch_y_pistol, 0);
	C_ADD_VARIABLE(float, punch_x_pistol, 0);
	C_ADD_VARIABLE(float, punch_y_snipers, 0);
	C_ADD_VARIABLE(float, punch_x_snipers, 0);
	C_ADD_VARIABLE(float, punch_y_machinegun, 0);
	C_ADD_VARIABLE(float, punch_x_machinegun, 0);
	C_ADD_VARIABLE(float, punch_x_assultrifles, 0);
	C_ADD_VARIABLE(float, punch_y_assultrifles, 0);

	C_ADD_VARIABLE(bool, legit_rcs_shots_enable_pistol, false);
	C_ADD_VARIABLE(bool, legit_rcs_shots_enable_machinegun, false);
	C_ADD_VARIABLE(bool, legit_rcs_shots_enable_assultrifles, false);
	C_ADD_VARIABLE(bool, legit_rcs_shots_enable_snipers, false);

	C_ADD_VARIABLE(float, legit_rcs_smoothx_pistol, 0.f);
	C_ADD_VARIABLE(float, legit_rcs_smoothx_machinegun, 0.f);
	C_ADD_VARIABLE(float, legit_rcs_smoothx_assultrifles, 0.f);
	C_ADD_VARIABLE(float, legit_rcs_smoothx_snipers, 0.f);

	C_ADD_VARIABLE(float, legit_rcs_smoothy_pistol, 0.f);
	C_ADD_VARIABLE(float, legit_rcs_smoothy_machinegun, 0.f);
	C_ADD_VARIABLE(float, legit_rcs_smoothy_assultrifles, 0.f);
	C_ADD_VARIABLE(float, legit_rcs_smoothy_snipers, 0.f);

	C_ADD_VARIABLE(bool, legit_no_scope, false);

	C_ADD_VARIABLE(bool, legit_visibility_check_pistol, true);
	C_ADD_VARIABLE(bool, legit_visibility_check_machinegun, true);
	C_ADD_VARIABLE(bool, legit_visibility_check_assultrifles, true);
	C_ADD_VARIABLE(bool, legit_visibility_check_snipers, true);

	C_ADD_VARIABLE(bool, hitbox_head_pistol, false);
	C_ADD_VARIABLE(bool, hitbox_head_machinegun, false);
	C_ADD_VARIABLE(bool, hitbox_head_assultrifles, false);
	C_ADD_VARIABLE(bool, hitbox_head_snipers, false);

	C_ADD_VARIABLE(bool, hitbox_neck_pistol, false);
	C_ADD_VARIABLE(bool, hitbox_neck_machinegun, false);
	C_ADD_VARIABLE(bool, hitbox_neck_assultrifles, false);
	C_ADD_VARIABLE(bool, hitbox_neck_snipers, false);

	C_ADD_VARIABLE(bool, hitbox_uppeer_chest_pistol, false);
	C_ADD_VARIABLE(bool, hitbox_uppeer_chest_machinegun, false);
	C_ADD_VARIABLE(bool, hitbox_uppeer_chest_assultrifles, false);
	C_ADD_VARIABLE(bool, hitbox_uppeer_chest_snipers, false);
	C_ADD_VARIABLE(float, max_lagcompensation_time, 0.f);
	C_ADD_VARIABLE(bool, hitbox_chest_pistol, false);
	C_ADD_VARIABLE(bool, hitbox_chest_machinegun, false);
	C_ADD_VARIABLE(bool, hitbox_chest_assultrifles, false);
	C_ADD_VARIABLE(bool, hitbox_chest_snipers, false);

	C_ADD_VARIABLE(bool, hitbox_stomach_pistol, false);
	C_ADD_VARIABLE(bool, hitbox_stomach_machinegun, false);
	C_ADD_VARIABLE(bool, hitbox_stomach_assultrifles, false);
	C_ADD_VARIABLE(bool, hitbox_stomach_snipers, false);

	C_ADD_VARIABLE(bool, hitbox_leg_l_pistol, false);
	C_ADD_VARIABLE(bool, hitbox_leg_l_machinegun, false);
	C_ADD_VARIABLE(bool, hitbox_leg_l_assultrifles, false);
	C_ADD_VARIABLE(bool, hitbox_leg_l_snipers, false);

	C_ADD_VARIABLE(bool, hitbox_leg_r_pistol, false);
	C_ADD_VARIABLE(bool, hitbox_leg_r_machinegun, false);
	C_ADD_VARIABLE(bool, hitbox_leg_r_assultrifles, false);
	C_ADD_VARIABLE(bool, hitbox_leg_r_snipers, false);
	C_ADD_VARIABLE(bool, trigger_enable_p, false);
	C_ADD_VARIABLE(bool, trigger_on_key, false);
	C_ADD_VARIABLE(int, trigger_hitchance_p, 0);
	C_ADD_VARIABLE(bool, trigger_enable_a, false);
	C_ADD_VARIABLE(int, trigger_hitchance_a, 0);
	C_ADD_VARIABLE(bool, trigger_enable_m, false);
	C_ADD_VARIABLE(int, trigger_hitchance_m, 0);
	C_ADD_VARIABLE(bool, trigger_enable_s, false);
	C_ADD_VARIABLE(int, trigger_hitchance_s, 0);

	// PISTOL
	C_ADD_VARIABLE(int, legit_fov_pistol, 0);
	C_ADD_VARIABLE(KeyBind_t, legit_key_pistol, 0);
	C_ADD_VARIABLE(KeyBind_t, edge_bug_key, 0);

	// MACHINESGUN
	C_ADD_VARIABLE(int, legit_fov_machinegun, 0);
	C_ADD_VARIABLE(KeyBind_t, legit_key_machinegun, 0);

	// ASSULTRIFLES
	C_ADD_VARIABLE(int, legit_fov_assultrifles, 0);
	C_ADD_VARIABLE(KeyBind_t, legit_key_assultrifles, 0);

	// SNIPERS
	C_ADD_VARIABLE(int, legit_fov_snipers, 0);
	C_ADD_VARIABLE(KeyBind_t, legit_key_snipers, 0);
#pragma endregion

#pragma region antiaim
	C_ADD_VARIABLE(bool, bAntiAim, false);
	C_ADD_VARIABLE(int, iBaseYawType, 0);
	C_ADD_VARIABLE(int, iPitchType, 0);
#pragma endregion
#pragma region rage

	C_ADD_VARIABLE(int, rage_weapon_selection, 0);
	C_ADD_VARIABLE(bool, rage_silent_aim, false);
	C_ADD_VARIABLE(bool, rage_delay_aim, false);
	C_ADD_VARIABLE(int, rage_delay_aim_ms, 0);

	C_ADD_VARIABLE(int, rage_target_selection, 0);
	C_ADD_VARIABLE(int, rage_target_selection_machinegun, 0);
	C_ADD_VARIABLE(int, rage_target_selection_assultrifles, 0);
	C_ADD_VARIABLE(int, rage_target_selection_snipers, 0);

	C_ADD_VARIABLE(bool, rage_fov_visualize, false);

	C_ADD_VARIABLE(ColorPickerVar_t, rage_fov_visualizeclr, ColorPickerVar_t(255, 88, 88));

	C_ADD_VARIABLE(bool, remove_weapon_accuracy_spread_rage, false);

	C_ADD_VARIABLE(int, rage_hitchance_p, 1);
	C_ADD_VARIABLE(int, rage_hitchance_m, 1);
	C_ADD_VARIABLE(int, rage_hitchance_a, 1);
	C_ADD_VARIABLE(float, rage_hitchance_s, 1.f);

	C_ADD_VARIABLE(int, rage_rcs_shots_pistol, 0);
	C_ADD_VARIABLE(int, rage_rcs_shots_machinegun, 0);
	C_ADD_VARIABLE(int, rage_rcs_shots_assultrifles, 0);
	C_ADD_VARIABLE(int, rage_rcs_shots_snipers, 0);

	C_ADD_VARIABLE(bool, rage_rcs_pistol, false);
	C_ADD_VARIABLE(bool, rage_rcs_machinegun, false);
	C_ADD_VARIABLE(bool, rage_rcs_assultrifles, false);
	C_ADD_VARIABLE(bool, rage_rcs_snipers, false);

	C_ADD_VARIABLE(bool, PunchRandomization_rage_pistol, false);
	C_ADD_VARIABLE(bool, PunchRandomization_rage_machinegun, false);
	C_ADD_VARIABLE(bool, PunchRandomization_rage_assultrifles, false);
	C_ADD_VARIABLE(bool, PunchRandomization_rage_snipers, false);

	C_ADD_VARIABLE(float, punch_y_rage_pistol, 0);
	C_ADD_VARIABLE(float, punch_x_rage_pistol, 0);
	C_ADD_VARIABLE(float, punch_y_rage_snipers, 0);
	C_ADD_VARIABLE(float, punch_x_rage_snipers, 0);
	C_ADD_VARIABLE(float, punch_y_rage_machinegun, 0);
	C_ADD_VARIABLE(float, punch_x_rage_machinegun, 0);
	C_ADD_VARIABLE(float, punch_x_rage_assultrifles, 0);
	C_ADD_VARIABLE(float, punch_y_rage_assultrifles, 0);

	C_ADD_VARIABLE(bool, rage_rcs_shots_enable_pistol, false);
	C_ADD_VARIABLE(bool, rage_rcs_shots_enable_machinegun, false);
	C_ADD_VARIABLE(bool, rage_rcs_shots_enable_assultrifles, false);
	C_ADD_VARIABLE(bool, rage_rcs_shots_enable_snipers, false);

	C_ADD_VARIABLE(float, rage_rcs_smoothx_pistol, 0.f);
	C_ADD_VARIABLE(float, rage_rcs_smoothx_machinegun, 0.f);
	C_ADD_VARIABLE(float, rage_rcs_smoothx_assultrifles, 0.f);
	C_ADD_VARIABLE(float, rage_rcs_smoothx_snipers, 0.f);

	C_ADD_VARIABLE(float, rage_rcs_smoothy_pistol, 0.f);
	C_ADD_VARIABLE(float, rage_rcs_smoothy_machinegun, 0.f);
	C_ADD_VARIABLE(float, rage_rcs_smoothy_assultrifles, 0.f);
	C_ADD_VARIABLE(float, rage_rcs_smoothy_snipers, 0.f);

	C_ADD_VARIABLE(bool, rage_no_scope, false);

	C_ADD_VARIABLE(bool, rage_visibility_check_pistol, true);
	C_ADD_VARIABLE(bool, rage_visibility_check_machinegun, true);
	C_ADD_VARIABLE(bool, rage_visibility_check_assultrifles, true);
	C_ADD_VARIABLE(bool, rage_visibility_check_snipers, true);

	C_ADD_VARIABLE(bool, hitbox_head_rage_pistol, false);
	C_ADD_VARIABLE(bool, hitbox_head_rage_machinegun, false);
	C_ADD_VARIABLE(bool, hitbox_head_rage_assultrifles, false);
	C_ADD_VARIABLE(bool, hitbox_head_rage_snipers, false);

	C_ADD_VARIABLE(bool, hitbox_neck_rage_pistol, false);
	C_ADD_VARIABLE(bool, hitbox_neck_rage_machinegun, false);
	C_ADD_VARIABLE(bool, hitbox_neck_rage_assultrifles, false);
	C_ADD_VARIABLE(bool, hitbox_neck_rage_snipers, false);


	// ... Repeat the same pattern for other hitbox variables

	C_ADD_VARIABLE(bool, trigger_enable_r, false);
	C_ADD_VARIABLE(bool, trigger_on_key_r, false);
	C_ADD_VARIABLE(int, trigger_hitchance_r, 0);

	// PISTOL
	C_ADD_VARIABLE(int, rage_fov_pistol, 0);
	C_ADD_VARIABLE(KeyBind_t, rage_key_pistol, 0);

	// MACHINESGUN
	C_ADD_VARIABLE(int, rage_fov_machinegun, 0);
	C_ADD_VARIABLE(KeyBind_t, rage_key_machinegun, 0);

	// ASSULTRIFLES
	C_ADD_VARIABLE(int, rage_fov_assultrifles, 0);
	C_ADD_VARIABLE(KeyBind_t, rage_key_assultrifles, 0);

	// SNIPERS
	C_ADD_VARIABLE(int, rage_fov_snipers, 0);
	C_ADD_VARIABLE(KeyBind_t, rage_key_snipers, 0);

#pragma endregion

#pragma region variables_misc
	C_ADD_VARIABLE(bool, bAntiUntrusted, true);
	C_ADD_VARIABLE(bool, bWatermark, true);
	C_ADD_VARIABLE(bool, bFOV, false);
	C_ADD_VARIABLE(bool, bSetViewModelFOV, false);
	C_ADD_VARIABLE(bool, bAutostrafe, false);
	C_ADD_VARIABLE(bool, bAutostrafeAssistance, false);
	C_ADD_VARIABLE(bool, bAutoBHop, false);
	C_ADD_VARIABLE(bool, bThirdperson, false);
	C_ADD_VARIABLE(float, flThirdperson, 90.0f);
	C_ADD_VARIABLE(float, flSetViewModelFOV, 40.0f);
	C_ADD_VARIABLE(float, fFOVAmount, 30.0f);
	C_ADD_VARIABLE(bool, bThirdpersonNoInterp, true);
	C_ADD_VARIABLE(int, nAutoBHopChance, 100);
	C_ADD_VARIABLE(unsigned int, bAutostrafeMode, 0);
	C_ADD_VARIABLE(float, autostrafe_smooth, 99);
	C_ADD_VARIABLE(bool, edge_bug, false);

#pragma endregion

#pragma region variables_menu

	C_ADD_VARIABLE(unsigned int, nMenuKey, VK_INSERT);
	C_ADD_VARIABLE(unsigned int, nPanicKey, VK_END);
	C_ADD_VARIABLE(int, nDpiScale, 0);
	C_ADD_VARIABLE(KeyBind_t, OverlayKey, 0);
	C_ADD_VARIABLE(int, OverlayKeyCurrent, 0);
	C_ADD_VARIABLE(int, OverlayKeyStyle, 0);

	/*
	 * color navigation:
	 * [definition N][purpose]
	 * 1. primitive:
	 * - primtv 0 (text)
	 * - primtv 1 (background)
	 * - primtv 2 (disabled)
	 * - primtv 3 (control bg)
	 * - primtv 4 (border)
	 * - primtv 5 (hover)
	 *
	 * 2. accents:
	 * - accent 0 (main)
	 * - accent 1 (dark)
	 * - accent 2 (darker)
	 */
	C_ADD_VARIABLE(unsigned int, legit_conditions, LEGIT_NONE);

	C_ADD_VARIABLE(unsigned int, pEspFlags, FLAGS_NONE);
	C_ADD_VARIABLE(TextOverlayVar_t, HKFlag, TextOverlayVar_t(true, false));
	C_ADD_VARIABLE(TextOverlayVar_t, KitFlag, TextOverlayVar_t(true, false));

	C_ADD_VARIABLE(unsigned int, bMenuAdditional, MENU_ADDITION_ALL);
	C_ADD_VARIABLE(float, flAnimationSpeed, 1.f);

	C_ADD_VARIABLE(TextOverlayVar_t, Weaponesp, TextOverlayVar_t(false, true));
	C_ADD_VARIABLE(ColorPickerVar_t, colSkeleton, ColorPickerVar_t(88, 88, 88)); // (text)
	C_ADD_VARIABLE(ColorPickerVar_t, colSkeletonOutline, ColorPickerVar_t(0, 0, 0)); // (text)

	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv0, ColorPickerVar_t(255, 255, 255)); // (text)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv1, ColorPickerVar_t(50, 55, 70)); // (background)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv2, ColorPickerVar_t(190, 190, 190)); // (disabled)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv3, ColorPickerVar_t(20, 20, 30)); // (control bg)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv4, ColorPickerVar_t(0, 0, 0)); // (border)

	C_ADD_VARIABLE(ColorPickerVar_t, colAccent0, ColorPickerVar_t(85, 90, 160)); // (main)
	C_ADD_VARIABLE(ColorPickerVar_t, colAccent1, ColorPickerVar_t(100, 105, 175)); // (dark)
	C_ADD_VARIABLE(ColorPickerVar_t, colAccent2, ColorPickerVar_t(115, 120, 190)); // (darker)
#pragma endregion
};

inline Variables_t Vars = {};
