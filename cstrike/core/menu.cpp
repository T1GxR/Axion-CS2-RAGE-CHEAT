#include "menu.h"

// used: config variables
#include "variables.h"
// used: entity stuff for skinchanger etc
#include "../cstrike/sdk/entity.h"
// used: iinputsystem
#include "interfaces.h"
#include "../sdk/interfaces/iengineclient.h"
#include "../sdk/interfaces/inetworkclientservice.h"
#include "../sdk/interfaces/iglobalvars.h"
#include "../sdk/interfaces/ienginecvar.h"
// used: overlay's context
#include "../features/visuals/overlay.h"
// used: notifications
#include "../utilities/notify.h"
#include "gui.hpp"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "../cstrike/features/skins/ccsplayerinventory.hpp"
#include "../cstrike/features/skins/ccsinventorymanager.hpp"
#include "../cstrike/features/skins/skin_changer.hpp"
#include "imgui/imgui_edited.hpp"
#pragma region menu_array_entries
static void RenderInventoryWindow();
int page = 0;
float tab_alpha = 0.f;
float tab_add = 0.f;
int active_tab = 0;
static constexpr const char* arrMiscDpiScale[] = {
	"100%",
	"125%",
	"150%",
	"175%",
	"200%"
};
int subtab;
static const std::pair<const char*, const std::size_t> arrColors[] = {
	{ "[accent] - main", Vars.colAccent0 },
	{ "[accent] - dark (hover)", Vars.colAccent1 },
	{ "[accent] - darker (active)", Vars.colAccent2 },
	{ "[primitive] - text", Vars.colPrimtv0 },
	{ "[primitive] - background", Vars.colPrimtv1 },
	{ "[primitive] - disabled", Vars.colPrimtv2 },
	{ "[primitive] - frame background", Vars.colPrimtv3 },
	{ "[primitive] - border", Vars.colPrimtv4 },
};

static constexpr const char* arrMenuAddition[] = {
	"dim",
	"particle",
	"glow"
};
static constexpr const char* arrEspFlags[] = {
	"Armor",
	"KIT"
};
static constexpr const char* arrLegitCond[] = {
	"In air",
	"Flashed",
	"Thru smoke",
	"Delay on kill"
};
static constexpr const char* arrMovementStrafer[] = {
	"Adjust mouse",
	"Directional"
};

enum TAB : int {
	rage = 0,
	legit = 1,
	visuals = 2,
	misc = 3,
	skinchanger = 4,
	cloud = 5,
	scripting = 6,
};

enum SUBTAB : int {
	first = 0,
	seccond = 1,
	third = 4,
	fifth = 5,
};

#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*_ARR)))       // Size of a static C-style array. Don't use on pointers!

// Function to extract the unique identifier from the itemBaseName
std::string ExtractIdentifier(const std::string& itemBaseName, const std::string& modelName) {
	// Find the position of the modelName
	size_t modelPos = itemBaseName.find(modelName);

	// If modelName is found, extract the substring after it
	if (modelPos != std::string::npos) {
		// Find the next "/"
		size_t nextSlashPos = itemBaseName.find("/", modelPos + modelName.length());

		// Extract the substring after modelName until the next "/"
		return itemBaseName.substr(modelPos + modelName.length(), nextSlashPos - (modelPos + modelName.length()));
	}

	// If modelName is not found, return an empty string
	return "";
}
ImTextureID CreateTextureFromMemory(void* imageData, int width, int height) {
	ID3D11Texture2D* pTexture = nullptr;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = imageData;
	initData.SysMemPitch = width * 4; // Assuming 4 channels (R8G8B8A8)

	if (FAILED(I::Device->CreateTexture2D(&desc, &initData, &pTexture))) {
		// Handle creation failure
		return 0;
	}

	return (ImTextureID)pTexture;
}
enum wep_type : int {
	PISTOL = 1,
	 HEAVY_PISTOL = 2,
	 ASSULT = 3,
	 SNIPERS = 4,
	 SCOUT = 5, 
	 AWP =6,
};
void MENU::RenderMainWindow()
{
	static constexpr float windowWidth = 540.f;

	struct DumpedSkin_t {
		std::string m_name = "";
		int m_ID = 0;
		int m_rarity = 0;
	};
	struct DumpedItem_t {
		std::string m_name = "";
		uint16_t m_defIdx = 0;
		void* m_image = nullptr;
		ImTextureID m_textureID = nullptr;
		int m_rarity = 0;
		bool m_unusualItem = false;
		std::vector<DumpedSkin_t> m_dumpedSkins{};
		DumpedSkin_t* pSelectedSkin = nullptr;
	};
	static std::vector<DumpedItem_t> vecDumpedItems;
	static DumpedItem_t* pSelectedItem = nullptr;

	CEconItemSchema* pItemSchema =
		I::Client->GetEconItemSystem()->GetEconItemSchema();


	// Render the ImGui draw data using the DirectX 11 blur shader
	//blurShader.Render(drawData);
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	// @test: we should always update the animation?
	animMenuDimBackground.Update(io.DeltaTime, style.AnimationSpeed);
	if (!bMainWindowOpened)
		return;

	const ImVec2 vecScreenSize = io.DisplaySize;
	const float flBackgroundAlpha = animMenuDimBackground.GetValue(1.f);
	flDpiScale = 1.50f;

	// @note: we call this every frame because we utilizing rainbow color as well! however it's not really performance friendly?
	UpdateStyle(&style);

	if (flBackgroundAlpha > 0.f)
	{
		if (C_GET(unsigned int, Vars.bMenuAdditional) & MENU_ADDITION_DIM_BACKGROUND)
			D::AddDrawListRect(ImGui::GetBackgroundDrawList(), ImVec2(0, 0), vecScreenSize, C_GET(ColorPickerVar_t, Vars.colPrimtv1).colValue.Set<COLOR_A>(125 * flBackgroundAlpha), DRAW_RECT_FILLED);

		if (C_GET(unsigned int, Vars.bMenuAdditional) & MENU_ADDITION_BACKGROUND_PARTICLE)
			menuParticle.Render(ImGui::GetBackgroundDrawList(), vecScreenSize, flBackgroundAlpha);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, flBackgroundAlpha);


	style.WindowPadding = ImVec2(0, 0);
	style.ItemSpacing = ImVec2(10 * dpi, 10 * dpi);
	style.WindowBorderSize = 0;
	style.ScrollbarSize = 3.f * dpi;
	Color_t color = Color_t(235, 94, 52, 255 );
	c::accent = color.GetVec4(1.f) ;

	ImGui::SetNextWindowSize(c::background::size* dpi);


	// render main window
	if (ImGui::Begin(CS_XOR("handle0000"), &bMainWindowOpened, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
	


		const ImVec2& pos = ImGui::GetWindowPos();
		const ImVec2& region = ImGui::GetContentRegionMax();
		const ImVec2& spacing = style.ItemSpacing;

		ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + c::background::size * dpi, ImGui::GetColorU32(c::background::filling), c::background::rounding);
		ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + ImVec2(200.f * dpi, c::background::size.y * dpi), ImGui::GetColorU32(c::tab::border), c::background::rounding, ImDrawFlags_RoundCornersLeft);
		ImGui::GetBackgroundDrawList()->AddLine(pos + ImVec2(200.f * dpi, 0.f), pos + ImVec2(200.f, c::background::size.y * dpi), ImGui::GetColorU32(c::background::stroke), 1.f);

		ImGui::GetBackgroundDrawList()->AddRect(pos, pos + c::background::size * dpi, ImGui::GetColorU32(c::background::stroke), c::background::rounding);

		ImGui::SetCursorPos({ 5, 10 });
		ImGui::BeginGroup();
		{
			std::vector<std::vector<std::string>> tab_columns = {
				{ "c", "b", "f", "o", "e" },
				{ "Ragebot", "Antiaim", "Visuals", "Skins", "Misc" },
				{ "Aims agressively at targets...",  "Accuracy assistance...", "Visualisation", "Items customization...", "Save/Load configs, engine..." },
				{ "Have you switched to the Aimbot tab? You're just crazy!", "Have you switched to the Visuals tab, do you want to get banned?", "You switched to the skins tab, why do you need self-deception??", "You switched over.. And yes, to hell with it, come up with a script yourself.", "You switched over.. And yes, to hell with it, come up with a script yourself." }
			};

			const int num_tabs = tab_columns[0].size();

			for (int i = 0; i < num_tabs; ++i)
				if (edited::Tab(page == i, tab_columns[0][i].c_str(), tab_columns[1][i].c_str(), tab_columns[2][i].c_str(), ImVec2(180, 50))) {
					page = i;

					//notificationSystem.AddNotification(tab_columns[3][i], 1000);
				}
		}
		ImGui::EndGroup();

		tab_alpha = ImLerp(tab_alpha, (page == active_tab) ? 1.f : 0.f, 15.f * ImGui::GetIO().DeltaTime);
		if (tab_alpha < 0.01f && tab_add < 0.01f) active_tab = page;

		ImGui::SetCursorPos(ImVec2(200, 100 - (tab_alpha * 100)));
		auto current_weapon = C_GET(int, Vars.rage_weapon_selection);

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * style.Alpha);
		{
			if (active_tab == 0)
			{
				edited::BeginChild(CS_XOR("##Container0"), ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), CS_XOR("Weapons"));
					const char* weapons[7]{ CS_XOR("Default"), CS_XOR("Pistols"), CS_XOR("Heavy Pistols"),CS_XOR("Assult Rifles"), CS_XOR("Auto"),CS_XOR("Scout"), CS_XOR("Awp") };
					edited::Combo(CS_XOR("Weapon"), CS_XOR("Select weapon for current configuration"), &C_GET(int, Vars.rage_weapon_selection), weapons, IM_ARRAYSIZE(weapons), 6);
					// run rage cfg depending on weapon
					{
						ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "General");
						edited::Checkbox("Enabled", "Activate ragebot", &C_GET(bool, Vars.rage_enable));

						const char* targets_select[3]{ CS_XOR("Distance"), CS_XOR("Damage"),CS_XOR("Crosshair") };
						edited::Combo(CS_XOR("Target selection"), CS_XOR("Select target based on conditions"), &C_GET_ARRAY(int, 7, Vars.rage_target_select, current_weapon), targets_select, IM_ARRAYSIZE(targets_select), 6);

						if (current_weapon > 3)
							edited::Checkbox("Auto Scope", "Automatically scope if a we found a target hitta...", &C_GET_ARRAY(bool, 7, Vars.rage_auto_scope, current_weapon));

						edited::Checkbox("Rapid fire", "Allows you to fire multiple bullets ignoring fire rate", &C_GET_ARRAY(bool, 7, Vars.rapid_fire, current_weapon));

						edited::Checkbox("Auto stop", "Stops local player in order to maintain best accuracy", &C_GET_ARRAY(bool, 7, Vars.rage_auto_stop, current_weapon));

						ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Accuracy");

						edited::Checkbox("Hitchance", "Allows you to hit players with more accuracy", &C_GET_ARRAY(bool, 7, Vars.rage_hitchance, current_weapon));
						if (C_GET_ARRAY(bool, 7, Vars.rage_hitchance, current_weapon))
							ImGui::SliderInt(CS_XOR("chance"), &C_GET_ARRAY(int, 7, Vars.rage_minimum_hitchance, current_weapon), 0, 100);

						edited::Checkbox("Penetration", "Allows you to hit players thru objects", &C_GET_ARRAY(bool, 7, Vars.rage_penetration, current_weapon));
						if (C_GET_ARRAY(bool, 7, Vars.rage_penetration, current_weapon))
							ImGui::SliderInt(CS_XOR("Damage"), &C_GET_ARRAY(int, 7, Vars.rage_minimum_damage, current_weapon), 0, 100);
					}
				}
				edited::EndChild();
				ImGui::SameLine(0, 0);

				edited::BeginChild("##Container1", ImVec2((c::background::size.x - 200 * dpi) / 2, c::background::size.y), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Hitbox System");

					/* render model preview*/
					ImGui::SetCursorPos({ 55, 75 });
					ImGui::Image((void*)I::Maintexture, ImVec2(278, 380));
					switch (current_weapon) {
					case PISTOL:
						edited::pointbox(CS_XOR("##head"), &C_GET_ARRAY(bool, 7, Vars.hitbox_head, 1), 0, 115.f, 105.f);
						edited::pointbox(CS_XOR("##chest"), &C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 1), 0, 130.f, 160.f);
						edited::pointbox(CS_XOR("##stomach"), &C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 1), 0, 130.f, 230.f);
						edited::pointbox(CS_XOR("##leg_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 1), 0, 110.f, 320);
						edited::pointbox(CS_XOR("##leg_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 1), 0, 170.f, 320);
						edited::pointbox(CS_XOR("##feet_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 1), 0, 120.f, 400);
						edited::pointbox(CS_XOR("##feet_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 1), 0, 175.f, 400);
						break;
					case HEAVY_PISTOL:
						edited::pointbox(CS_XOR("##head"), &C_GET_ARRAY(bool, 7, Vars.hitbox_head, 2), 0, 115.f, 105.f);
						edited::pointbox(CS_XOR("##chest"), &C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 2), 0, 130.f, 160.f);
						edited::pointbox(CS_XOR("##stomach"), &C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 2), 0, 130.f, 230.f);
						edited::pointbox(CS_XOR("##leg_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 2), 0, 110.f, 320);
						edited::pointbox(CS_XOR("##leg_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 2), 0, 170.f, 320);
						edited::pointbox(CS_XOR("##feet_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 2), 0, 120.f, 400);
						edited::pointbox(CS_XOR("##feet_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 2), 0, 175.f, 400);
						break;
					case ASSULT:
						edited::pointbox(CS_XOR("##head"), &C_GET_ARRAY(bool, 7, Vars.hitbox_head, 3), 0, 115.f, 105.f);
						edited::pointbox(CS_XOR("##chest"), &C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 3), 0, 130.f, 160.f);
						edited::pointbox(CS_XOR("##stomach"), &C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 3), 0, 130.f, 230.f);
						edited::pointbox(CS_XOR("##leg_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 3), 0, 110.f, 320);
						edited::pointbox(CS_XOR("##leg_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 3), 0, 170.f, 320);
						edited::pointbox(CS_XOR("##feet_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 3), 0, 120.f, 400);
						edited::pointbox(CS_XOR("##feet_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 3), 0, 175.f, 400);
						break;
					case SNIPERS:
						edited::pointbox(CS_XOR("##head"), &C_GET_ARRAY(bool, 7, Vars.hitbox_head, 4), 0, 115.f, 105.f);
						edited::pointbox(CS_XOR("##chest"), &C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 4), 0, 130.f, 160.f);
						edited::pointbox(CS_XOR("##stomach"), &C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 4), 0, 130.f, 230.f);
						edited::pointbox(CS_XOR("##leg_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 4), 0, 110.f, 320);
						edited::pointbox(CS_XOR("##leg_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 4), 0, 170.f, 320);
						edited::pointbox(CS_XOR("##feet_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 4), 0, 120.f, 400);
						edited::pointbox(CS_XOR("##feet_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 4), 0, 175.f, 400);
						break;
					case SCOUT:
						edited::pointbox(CS_XOR("##head"), &C_GET_ARRAY(bool, 7, Vars.hitbox_head, 5), 0, 115.f, 105.f);
						edited::pointbox(CS_XOR("##chest"), &C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 5), 0, 130.f, 160.f);
						edited::pointbox(CS_XOR("##stomach"), &C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 5), 0, 130.f, 230.f);
						edited::pointbox(CS_XOR("##leg_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 5), 0, 110.f, 320);
						edited::pointbox(CS_XOR("##leg_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 5), 0, 170.f, 320);
						edited::pointbox(CS_XOR("##feet_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 5), 0, 120.f, 400);
						edited::pointbox(CS_XOR("##feet_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 5), 0, 175.f, 400);
						break;
					case AWP:
						edited::pointbox(CS_XOR("##head"), &C_GET_ARRAY(bool, 7, Vars.hitbox_head, 6), 0, 115.f, 105.f);
						edited::pointbox(CS_XOR("##chest"), &C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 6), 0, 130.f, 160.f);
						edited::pointbox(CS_XOR("##stomach"), &C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 6), 0, 130.f, 230.f);
						edited::pointbox(CS_XOR("##leg_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 6), 0, 110.f, 320);
						edited::pointbox(CS_XOR("##leg_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 6), 0, 170.f, 320);
						edited::pointbox(CS_XOR("##feet_l"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 6), 0, 120.f, 400);
						edited::pointbox(CS_XOR("##feet_r"), &C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 6), 0, 175.f, 400);
						break;
					}

				}
				edited::EndChild();

			}
			else if (active_tab == 1)
			{
				edited::BeginChild("##Container0", ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Antiaim");
					edited::Checkbox(CS_XOR("Enable"), CS_XOR("Enables Antiaim"), &C_GET(bool, Vars.bAntiAim));
					const char* PitchTypes[4]{ CS_XOR("Off"), CS_XOR("Down"),CS_XOR("Up"), CS_XOR("Zero")};
					edited::Combo(CS_XOR("Pitch"), CS_XOR("Pitch Type"), &C_GET(int, Vars.iPitchType), PitchTypes, IM_ARRAYSIZE(PitchTypes), 4);

					const char* BaseYawTypes[3]{ CS_XOR("Off"), CS_XOR("Backwards"),CS_XOR("Forwards") };
					edited::Combo(CS_XOR("Base Yaw"), CS_XOR("Backwards / Forwards"), &C_GET(int, Vars.iBaseYawType), BaseYawTypes, IM_ARRAYSIZE(BaseYawTypes), 3);
				}
				edited::EndChild();
			}
			else if (active_tab == 2) {
				edited::BeginChild("##Container0", ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
				{

					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Players");

					edited::Checkbox(CS_XOR("Enable"), CS_XOR(""), &C_GET(bool, Vars.bVisualOverlay));
					edited::Checkbox(CS_XOR("Bounding box"), CS_XOR("Shows player bounding box"), &C_GET(FrameOverlayVar_t, Vars.overlayBox).bEnable);
					edited::Checkbox(CS_XOR("Name"), CS_XOR("Shows player name"), &C_GET(TextOverlayVar_t, Vars.overlayName).bEnable);
					edited::Checkbox(CS_XOR("Health bar"), CS_XOR("Shows player health"), &C_GET(BarOverlayVar_t, Vars.overlayHealthBar).bEnable);
					edited::Checkbox(CS_XOR("Ammo bar"), CS_XOR("Shows player weapon ammo"), &C_GET(BarOverlayVar_t, Vars.AmmoBar).bEnable);
					edited::Checkbox(CS_XOR("Weapon"), CS_XOR("Shows player weapon name"), &C_GET(TextOverlayVar_t, Vars.Weaponesp).bEnable);
					edited::Checkbox(CS_XOR("Skeleton"), CS_XOR("Shows player bones as skeleton"), &C_GET(bool, Vars.bSkeleton));

					if (C_GET(bool, Vars.bSkeleton))
						edited::Color(CS_XOR("##skeletoncolor"), CS_XOR("Change menu accent color"), &C_GET(ColorPickerVar_t, Vars.colSkeleton).colValue, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

					edited::MultiCombo(CS_XOR("Flags"), &C_GET(unsigned int, Vars.pEspFlags), arrEspFlags, CS_ARRAYSIZE(arrEspFlags));
				
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Chams");

					edited::Checkbox(CS_XOR("Chams"), CS_XOR("Shows player chams"), &C_GET(bool, Vars.bVisualChams));
					if (C_GET(bool, Vars.bVisualChams))
						edited::Color(CS_XOR("##chamscolor"), CS_XOR("Change chams color"), &C_GET(ColorPickerVar_t, Vars.colVisualChams).colValue, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
			
					edited::Checkbox(CS_XOR("Invisible"), CS_XOR("Shows player xqz chams"), &C_GET(bool, Vars.bVisualChamsIgnoreZ));
					if (C_GET(bool, Vars.bVisualChamsIgnoreZ))
						edited::Color(CS_XOR("##chamscolorxqz"), CS_XOR("Change xqz chams color"), &C_GET(ColorPickerVar_t, Vars.colVisualChamsIgnoreZ).colValue, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

					const char* chams[3]{ CS_XOR("Flat"), CS_XOR("Default"),CS_XOR("Illumin") };
					edited::Combo(CS_XOR("Models"), CS_XOR(""), &C_GET(int, Vars.nVisualChamMaterial), chams, IM_ARRAYSIZE(chams), 3);

				}
				edited::EndChild();

				ImGui::SameLine(0, 0);

				edited::BeginChild("##Container1", ImVec2((c::background::size.x - 200 * dpi) / 2, c::background::size.y), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Preview");

					/* render model preview*/
					ImGui::SetCursorPos({ 65, 75 });
					ImGui::Image((void*)I::Maintexture, ImVec2(278, 380));

					using namespace F::VISUALS::OVERLAY;

					ImGuiStyle& style = ImGui::GetStyle();
					// @note: call this function inside rendermainwindow, else expect a crash...
					const ImVec2 vecMenuPos = ImGui::GetWindowPos();
					const ImVec2 vecMenuSize = ImGui::GetWindowSize();

					const ImVec2 vecOverlayPadding = ImVec2(65 * dpi, 58 * dpi);  // Adjusted the Y position

					const ImVec2 vecWindowPos = ImGui::GetWindowPos();
					const ImVec2 vecWindowSize = ImGui::GetWindowSize();

					ImDrawList* pDrawList = ImGui::GetWindowDrawList();
					Context_t context;

					ImVec4 vecBox = {
						vecWindowPos.x + vecOverlayPadding.x,
						vecWindowPos.y + vecOverlayPadding.y,
						vecWindowPos.x + vecWindowSize.x - vecOverlayPadding.x,
						vecWindowPos.y + vecWindowSize.y - vecOverlayPadding.y - 10.f
					};

					if (const auto& boxOverlayConfig = C_GET(FrameOverlayVar_t, Vars.overlayBox); boxOverlayConfig.bEnable)
					{
						const bool bHovered = context.AddBoxComponent(pDrawList, vecBox, 1, boxOverlayConfig.flThickness, boxOverlayConfig.flRounding, boxOverlayConfig.colPrimary, boxOverlayConfig.colOutline);

						if (bHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
							ImGui::OpenPopup(CS_XOR("context##box.component"));

						if (ImGui::BeginPopup(CS_XOR("context##box.component"), ImGuiWindowFlags_NoResize))
						{
							ImVec2 size = ImVec2(135, 275);
							ImGui::SetWindowSize(size); 

							edited::Color(CS_XOR("Primary##box.component"), "" , & C_GET(FrameOverlayVar_t, Vars.overlayBox).colPrimary, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar);
							edited::Color(CS_XOR("Outline##box.component"), "", & C_GET(FrameOverlayVar_t, Vars.overlayBox).colOutline, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar);
							ImGui::SliderFloat(CS_XOR("Thickness##box.component"), &C_GET(FrameOverlayVar_t, Vars.overlayBox).flThickness, 1.f, 5.f, CS_XOR("%.1f"), ImGuiSliderFlags_AlwaysClamp);
							ImGui::SliderFloat(CS_XOR("Rounding##box.component"), &C_GET(FrameOverlayVar_t, Vars.overlayBox).flRounding, 1.f, 5.f, CS_XOR("%.1f"), ImGuiSliderFlags_AlwaysClamp);
							ImGui::EndPopup();
						}
					}

					//name
					if (const auto& nameOverlayConfig = C_GET(TextOverlayVar_t, Vars.overlayName); nameOverlayConfig.bEnable)
						context.AddComponent(new CTextComponent(true, false, SIDE_TOP, DIR_TOP, FONT::pVisual, CS_XOR("Name"), Vars.overlayName));

					// health
					if (const auto& healthOverlayConfig = C_GET(BarOverlayVar_t, Vars.overlayHealthBar); healthOverlayConfig.bEnable)
					{
						const float flFactor = M_SIN(ImGui::GetTime() * 5.f) * 0.55f + 0.45f;
						context.AddComponent(new CBarComponent(true, SIDE_LEFT, vecBox, 100.f, flFactor, Vars.overlayHealthBar));
					}

					// weapon
					if (const auto& weaponOverlayConfig = C_GET(TextOverlayVar_t, Vars.Weaponesp); weaponOverlayConfig.bEnable)
						context.AddComponent(new CTextComponent(true, true, SIDE_BOTTOM, DIR_BOTTOM, FONT::pVisual, CS_XOR("Weapon"), Vars.Weaponesp));

					// armour
					if (const auto& armorOverlayConfig = C_GET(BarOverlayVar_t, Vars.AmmoBar); armorOverlayConfig.bEnable)
					{
						const float flArmorFactor = M_SIN(ImGui::GetTime() * 5.f) * 0.55f + 0.45f;
						context.AddComponent(new CBarComponent(true, SIDE_BOTTOM, vecBox, 32.f, flArmorFactor, Vars.AmmoBar));
					}

					// flags 
					{
						if (C_GET(unsigned int, Vars.pEspFlags) & FLAGS_ARMOR) {

							if (const auto& hkcfg = C_GET(TextOverlayVar_t, Vars.HKFlag); hkcfg.bEnable)
								context.AddComponent(new CTextComponent(true, false, SIDE_RIGHT, DIR_RIGHT, FONT::pEspWepName, CS_XOR("HK"), Vars.HKFlag));
						}

						if (C_GET(unsigned int, Vars.pEspFlags) & FLAGS_DEFUSER) {

							if (const auto& kitcfg = C_GET(TextOverlayVar_t, Vars.KitFlag); kitcfg.bEnable)
								context.AddComponent(new CTextComponent(true, false, SIDE_RIGHT, DIR_BOTTOM, FONT::pEspWepName, CS_XOR("KIT"), Vars.KitFlag));
						}
					}
					// only render context preview if overlay is enabled
					context.Render(pDrawList, vecBox);

				}
				edited::EndChild();
			}
			else if (active_tab == 4) {
			edited::BeginChild("##Container0", ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
			{

				ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Players");

				edited::Checkbox(CS_XOR("Anti untrusted"), CS_XOR(""), &C_GET(bool, Vars.bAntiUntrusted));
				edited::Checkbox(CS_XOR("Thirdperson"), CS_XOR("Puts you in thirdperson"), &C_GET(bool, Vars.bThirdperson));
				if (C_GET(bool, Vars.bThirdperson))
				{
					edited::SliderFloat(CS_XOR("Thirdperson distance"), CS_XOR("Thirdperson cam distance"), &C_GET(float, Vars.flThirdperson), 0.f, 150.f);
				}

				edited::Checkbox(CS_XOR("FOV Changer"), CS_XOR("Makes your FOV bigger"), &C_GET(bool, Vars.bFOV));
				if (C_GET(bool, Vars.bFOV))
				{
					edited::SliderFloat(CS_XOR("FOV Amount"), CS_XOR("How much you change your FOV"), &C_GET(float, Vars.fFOVAmount), 30.f, 150.f);
				}

				edited::Checkbox(CS_XOR("View FOV Changer"), CS_XOR("Makes Arms Far"), &C_GET(bool, Vars.bSetViewModelFOV));
				if (C_GET(bool, Vars.bSetViewModelFOV))
				{
					edited::SliderFloat(CS_XOR("View FOV Amount"), CS_XOR("Amount"), &C_GET(float, Vars.flSetViewModelFOV), 40.f, 150.f);
				}

				edited::Checkbox(CS_XOR("Bunny hop"), CS_XOR("Automatic jumps for you"), &C_GET(bool, Vars.bAutoBHop));
				edited::Checkbox(CS_XOR("Auto strafer"), CS_XOR("Make movement easier"), &C_GET(bool, Vars.bAutostrafe));
				edited::Checkbox(CS_XOR("Edge bug"), CS_XOR("Edge bug"), &C_GET(bool, Vars.edge_bug));
				edited::MultiCombo(CS_XOR("Strafe modes"), &C_GET(unsigned int, Vars.bAutostrafeMode), arrMovementStrafer, CS_ARRAYSIZE(arrMovementStrafer));
				edited::Color(CS_XOR("##menuaccent"), CS_XOR("Change menu accent color"), &color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

			}
			edited::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::SetCursorPos(ImVec2(527, 60));
			edited::BeginChild("##Container1", ImVec2((c::background::size.x - 200 * dpi) / 2, c::background::size.y), NULL);
			{
				ImGui::Columns(2, CS_XOR("#CONFIG"), false);
				{
					ImGui::PushItemWidth(-1);

					// check selected configuration for magic value
					if (nSelectedConfig == ~1U)
					{
						// set default configuration as selected on first use
						for (std::size_t i = 0U; i < C::vecFileNames.size(); i++)
						{
							if (CRT::StringCompare(C::vecFileNames[i], CS_XOR(CS_CONFIGURATION_DEFAULT_FILE_NAME CS_CONFIGURATION_FILE_EXTENSION)) == 0)
								nSelectedConfig = i;
						}
					}

					if (ImGui::BeginListBox(CS_XOR("##config.list"), C::vecFileNames.size(), 5))
					{
						for (std::size_t i = 0U; i < C::vecFileNames.size(); i++)
						{
							// Convert wide string to narrow string
							const std::wstring& wideName = C::vecFileNames[i];
							const int bufferSize = 512; // Adjust the buffer size as needed
							char narrowName[bufferSize];
							std::wcstombs(narrowName, wideName.c_str(), bufferSize);

							if (ImGui::Selectable(narrowName, (nSelectedConfig == i)))
								nSelectedConfig = i;
						}

						ImGui::EndListBox();
					}

					ImGui::PopItemWidth();
				}
				ImGui::NextColumn();
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));
					ImGui::PushItemWidth(-1);
					if (ImGui::InputTextWithHint(CS_XOR("##config.file"), "create new...", szConfigFile, sizeof(szConfigFile), ImGuiInputTextFlags_EnterReturnsTrue))
					{
						// check if the filename isn't empty
						if (const std::size_t nConfigFileLength = CRT::StringLength(szConfigFile); nConfigFileLength > 0U)
						{
							CRT::WString_t wszConfigFile(szConfigFile);

							if (C::CreateFile(wszConfigFile.Data()))
								// set created config as selected @todo: dependent on current 'C::CreateFile' behaviour, generally it must be replaced by search
								nSelectedConfig = C::vecFileNames.size() - 1U;

							// clear string
							CRT::MemorySet(szConfigFile, 0U, sizeof(szConfigFile));
						}
					}
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip(CS_XOR("press enter to create new configuration"));

					if (ImGui::Button(CS_XOR("save"), ImVec2(-1, 15 * MENU::flDpiScale)))
					{
						C::SaveFile(nSelectedConfig);
						NOTIFY::Push({ N_TYPE_SUCCESS, CS_XOR("config saved") });
					}
					if (ImGui::Button(CS_XOR("load"), ImVec2(-1, 15 * MENU::flDpiScale)))
					{
						C::LoadFile(nSelectedConfig);
						NOTIFY::Push({ N_TYPE_SUCCESS, CS_XOR("config loaded") });
					}
					if (ImGui::Button(CS_XOR("remove"), ImVec2(-1, 15 * MENU::flDpiScale)))
					{
						ImGui::OpenPopup(CS_XOR("confirmation##config.remove"));
					}
					if (ImGui::Button(CS_XOR("refresh"), ImVec2(-1, 15 * MENU::flDpiScale)))
					{
						C::Refresh();
						NOTIFY::Push({ N_TYPE_INFO, CS_XOR("configs refreshed") });
					}
					ImGui::PopItemWidth();
					ImGui::PopStyleVar();
				}
				ImGui::Columns(1);

				if (ImGui::BeginPopupModal(CS_XOR("confirmation##config.remove"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
				{
					CRT::String_t<MAX_PATH> szCurrentConfig(C::vecFileNames[nSelectedConfig]);

					ImGui::Text(CS_XOR("are you sure you want to remove \"%s\" configuration?"), szCurrentConfig);
					ImGui::Spacing();

					if (ImGui::Button(CS_XOR("no"), ImVec2(ImGui::GetContentRegionAvail().x / 2.f, 0)))
					{
						ImGui::CloseCurrentPopup();
						NOTIFY::Push({ N_TYPE_ERROR, CS_XOR("canceled") });
					}
					ImGui::SameLine();

					if (ImGui::Button(CS_XOR("yes"), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
					{
						C::RemoveFile(nSelectedConfig);

						// reset selected configuration index
						nSelectedConfig = ~0U;

						NOTIFY::Push({ N_TYPE_WARNING, CS_XOR("config removed") });

						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}
			edited::EndChild();
			}
			else if (active_tab == 3) {

				edited::BeginChild("##Container0", ImVec2((600), c::background::size.y), NULL);
				{					
					if (edited::Button(CS_XOR("Full update"), ImVec2(120, 50), 0)) {
						Vars.full_update = true;
						
					}
						
					if (vecDumpedItems.empty() && edited::Button(CS_XOR("Dump items"),  ImVec2(120, 50), 0)) {


						const CUtlMap<int, CEconItemDefinition*>& vecItems =
							pItemSchema->GetSortedItemDefinitionMap();
						CUtlMap<int, CPaintKit*>& vecPaintKits =
							pItemSchema->GetPaintKits();
						const CUtlMap<uint64_t, AlternateIconData_t>& vecAlternateIcons =
							pItemSchema->GetAlternateIconsMap();

						for (const auto& it : vecItems) {
							CEconItemDefinition* pItem = it.m_value;
							if (!pItem) continue;

							const bool isWeapon = pItem->IsWeapon();
							const std::string gloveType = CS_XOR("#Type_Hands");
							const std::string knifeType = CS_XOR("#CSGO_Type_Knife");
					
							bool isGlove = (pItem->m_pszItemBaseName == gloveType);
							bool isKnife = (pItem->m_pszItemBaseName == knifeType);

							const char* itemBaseName = pItem->GetSimpleWeaponName();


							if (!itemBaseName || itemBaseName[0] == '\0') continue;

							const uint16_t defIdx = pItem->m_nDefIndex;

							DumpedItem_t dumpedItem;
							dumpedItem.m_name = I::Localize->FindSafe(itemBaseName);
							dumpedItem.m_image = pItem->m_pKVItem;
							dumpedItem.m_defIdx = defIdx;
							dumpedItem.m_rarity = pItem->m_nItemRarity;
							if (isKnife | isGlove) {
								dumpedItem.m_unusualItem = true;
							}


							// Load the image and set the texture ID.
							if (dumpedItem.m_image) {
								dumpedItem.m_textureID = CreateTextureFromMemory(dumpedItem.m_image, 120, 280);
							}

							// We filter skins by guns.
							for (const auto& it : vecPaintKits) {
								CPaintKit* pPaintKit = it.m_value;
								if (!pPaintKit || pPaintKit->PaintKitId() == 0 || pPaintKit->PaintKitId() == 9001)
									continue;

								const uint64_t skinKey =
									Helper_GetAlternateIconKeyForWeaponPaintWearItem(
										defIdx, pPaintKit->PaintKitId(), 0);
								if (vecAlternateIcons.FindByKey(skinKey)) {
									DumpedSkin_t dumpedSkin;
									dumpedSkin.m_name = I::Localize->FindSafe(
										pPaintKit->PaintKitDescriptionTag());
									dumpedSkin.m_ID = pPaintKit->PaintKitId();
									dumpedSkin.m_rarity = pPaintKit->PaintKitRarity();
									dumpedItem.m_dumpedSkins.emplace_back(dumpedSkin);
								}
							}

							// Sort skins by rarity.
							if (!dumpedItem.m_dumpedSkins.empty() && isWeapon) {
								std::sort(dumpedItem.m_dumpedSkins.begin(),
									dumpedItem.m_dumpedSkins.end(),
									[](const DumpedSkin_t& a, const DumpedSkin_t& b) {
										return a.m_rarity > b.m_rarity;
									});
							}

							vecDumpedItems.emplace_back(dumpedItem);
						}
					}
					static char IconFilterText[128] = "";


					if (!vecDumpedItems.empty()) {
						if (edited::Button("Add all items", ImVec2(120, 50), 0)) {
							for (const auto& item : vecDumpedItems) {
								for (const auto& skin : item.m_dumpedSkins) {
									CEconItem* pItem = CEconItem::CreateInstance();
									L_PRINT(LOG_INFO) << "item addr:" << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << reinterpret_cast<uintptr_t>(pItem);
									if (pItem) {
										CCSPlayerInventory* pInventory =
											CCSPlayerInventory::GetInstance();
										auto highestIDs = pInventory->GetHighestIDs();
										L_PRINT(LOG_INFO) << "uid:" << pItem->m_ulID << " id:" << pItem->m_unAccountID << "idx:" << pItem->m_unDefIndex;
										pItem->m_ulID = highestIDs.first + 1;
										pItem->m_unInventory = highestIDs.second + 1;
										pItem->m_unAccountID =
											uint32_t(pInventory->GetOwner().m_id);
										pItem->m_unDefIndex = item.m_defIdx;
										if (item.m_unusualItem) pItem->m_nQuality = IQ_UNUSUAL;
										pItem->m_nRarity =
											std::clamp(item.m_rarity + skin.m_rarity - 1, 0,
												(skin.m_rarity == 7) ? 7 : 6);

										pItem->SetPaintKit((float)skin.m_ID);
										pItem->SetPaintSeed(1.f);
										if (pInventory->AddEconItem(pItem))
											skin_changer::AddEconItemToList(pItem);
									}
								}
							}
						}

					}

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Will cause lag on weaker computers.");
					if (!vecDumpedItems.empty()) {

						static ImGuiTextFilter itemFilter;
						itemFilter.Draw("Type here to filter Items...", windowWidth);
					

						// ...

						// Modify the loop for items to check against the item filter.
						if (ImGui::BeginListBox("##items", { windowWidth, 110.f })) {
							for (auto& item : vecDumpedItems) {
								if (!itemFilter.PassFilter(item.m_name.c_str()))
									continue;

								ImGui::PushID(&item);
								if (ImGui::Selectable(item.m_name.c_str(), pSelectedItem == &item)) {
									if (pSelectedItem == &item)
										pSelectedItem = nullptr;
									else
										pSelectedItem = &item;
								}
								ImGui::PopID();
							}
							ImGui::EndListBox();
						}
						static char skinFilterText[128] = "";

						if (pSelectedItem) {
							if (!pSelectedItem->m_dumpedSkins.empty()) {


								static ImGuiTextFilter skinFilter;
								skinFilter.Draw("Type here to filter Skins...", windowWidth);

								if (ImGui::BeginListBox("##skins", { windowWidth, 110.f })) {
									for (auto& skin : pSelectedItem->m_dumpedSkins) {
										if (!skinFilter.PassFilter(skin.m_name.c_str()))
											continue;

										ImGui::PushID(&skin);
										if (ImGui::Selectable(
											skin.m_name.c_str(),
											pSelectedItem->pSelectedSkin == &skin)) {
											if (pSelectedItem->pSelectedSkin == &skin)
												pSelectedItem->pSelectedSkin = nullptr;
											else
												pSelectedItem->pSelectedSkin = &skin;
										}
										ImGui::PopID();
									}
									ImGui::EndListBox();
								}
							}

							char buttonLabel[128];
							snprintf(buttonLabel, 128, "Add every %s skin",
								pSelectedItem->m_name.c_str());

							if (edited::Button(buttonLabel, ImVec2(120, 55), 0)) {
								for (const auto& skin : pSelectedItem->m_dumpedSkins) {
									CEconItem* pItem = CEconItem::CreateInstance();
									if (pItem) {
										CCSPlayerInventory* pInventory =
											CCSPlayerInventory::GetInstance();

										auto highestIDs = pInventory->GetHighestIDs();

										pItem->m_ulID = highestIDs.first + 1;
										pItem->m_unInventory = highestIDs.second + 1;
										pItem->m_unAccountID =
											uint32_t(pInventory->GetOwner().m_id);
										pItem->m_unDefIndex = pSelectedItem->m_defIdx;
										if (pSelectedItem->m_unusualItem)
											pItem->m_nQuality = IQ_UNUSUAL;
										pItem->m_nRarity = std::clamp(
											pSelectedItem->m_rarity + skin.m_rarity - 1, 0,
											(skin.m_rarity == 7) ? 7 : 6);

										pItem->SetPaintKit((float)skin.m_ID);
										pItem->SetPaintSeed(1.f);
										if (pInventory->AddEconItem(pItem))
											skin_changer::AddEconItemToList(pItem);
									}
								}
							}
							ImGui::SameLine();
							if (pSelectedItem->pSelectedSkin) {
								static float kitWear = 0.f;
								static int kitSeed = 1;
								static int gunKills = -1;
								static char gunName[32];

								bool vanillaSkin = pSelectedItem->pSelectedSkin->m_ID == 0;
								snprintf(
									buttonLabel, 128, "Add %s%s%s",
									pSelectedItem->m_name.c_str(), vanillaSkin ? "" : " | ",
									vanillaSkin ? ""
									: pSelectedItem->pSelectedSkin->m_name.c_str());

								if (edited::Button(buttonLabel, ImVec2(120, 55), 0)) {
									CEconItem* pItem = CEconItem::CreateInstance();
									if (pItem) {
										CCSPlayerInventory* pInventory =
											CCSPlayerInventory::GetInstance();

										auto highestIDs = pInventory->GetHighestIDs();
										L_PRINT(LOG_INFO) << "item addr:" << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << reinterpret_cast<uintptr_t>(pItem);
										L_PRINT(LOG_INFO) << "uid:" << pItem->m_ulID << " id:" << pItem->m_unAccountID << "idx:" << pItem->m_unDefIndex;

										pItem->m_ulID = highestIDs.first + 1;
										pItem->m_unInventory = highestIDs.second + 1;
										pItem->m_unAccountID =
											uint32_t(pInventory->GetOwner().m_id);
										pItem->m_unDefIndex = pSelectedItem->m_defIdx;

										if (pSelectedItem->m_unusualItem)
											pItem->m_nQuality = IQ_UNUSUAL;

										// I don't know nor do care why the rarity is calculated
										// like this. [Formula]
										pItem->m_nRarity = std::clamp(
											pSelectedItem->m_rarity +
											pSelectedItem->pSelectedSkin->m_rarity - 1,
											0,
											(pSelectedItem->pSelectedSkin->m_rarity == 7) ? 7
											: 6);

										pItem->SetPaintKit(
											(float)pSelectedItem->pSelectedSkin->m_ID);
										pItem->SetPaintSeed((float)kitSeed);
										pItem->SetPaintWear(kitWear);

										if (gunKills >= 0) {
											pItem->SetStatTrak(gunKills);
											pItem->SetStatTrakType(0);

											// Applied automatically on knives.
											if (pItem->m_nQuality != IQ_UNUSUAL)
												pItem->m_nQuality = IQ_STRANGE;
										}

										if (pInventory->AddEconItem(pItem))
											skin_changer::AddEconItemToList(pItem);

										kitWear = 0.f;
										kitSeed = 1;
										gunKills = -1;
										memset(gunName, '\0', IM_ARRAYSIZE(gunName));
									}
								}

								ImGui::Dummy({ 0, 8 });
								ImGui::SeparatorText("Extra settings");

								ImGui::TextUnformatted("Wear Rating");
								ImGui::SetNextItemWidth(windowWidth);
								ImGui::SliderFloat("##slider1", &kitWear, 0.f, 1.f, "%.9f",
									ImGuiSliderFlags_Logarithmic);

								ImGui::TextUnformatted("Pattern Template");
								ImGui::SetNextItemWidth(windowWidth);
								ImGui::SliderInt("##slider2", &kitSeed, 1, 1000);

								ImGui::TextUnformatted("StatTrak Count");
								ImGui::SetNextItemWidth(windowWidth);
								ImGui::SliderInt("##slider3", &gunKills, -1, INT_MAX / 2,
									gunKills == -1 ? "Not StatTrak" : "%d",
									ImGuiSliderFlags_Logarithmic);

								ImGui::TextUnformatted("Custom Name");
								ImGui::SetNextItemWidth(windowWidth);
								ImGui::InputTextWithHint("##input1", "Default", gunName,
									IM_ARRAYSIZE(gunName));
							}
						}
					}
				}
				edited::EndChild();

			}
		}
		

		ImGui::PopStyleVar();
	}
	ImGui::End();

	ImGui::PopStyleVar();

}


void MENU::RenderWatermark()
{
	if (!C_GET(bool, Vars.bWatermark) || !bMainWindowOpened)
		return;

	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.f, 0.f, 0.f, 0.03f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.03f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.03f));
	ImGui::PushFont(FONT::pExtra);
	ImGui::BeginMainMenuBar();
	{
		ImGui::Dummy(ImVec2(1, 1));

#ifdef _DEBUG
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), CS_XOR("debug"));
#endif
		if (CRT::StringString(GetCommandLineW(), CS_XOR(L"-insecure")) != nullptr)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), CS_XOR("insecure"));

		if (I::Engine->IsInGame())
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), CS_XOR("in-game"));

		static ImVec2 vecNameSize = ImGui::CalcTextSize(CS_XOR("cs2project | " __DATE__ " " __TIME__));
		ImGui::SameLine(ImGui::GetContentRegionMax().x - vecNameSize.x - style.FramePadding.x);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), CS_XOR("cs2project | " __DATE__ " " __TIME__));
	}
	ImGui::EndMainMenuBar();
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
}

void MENU::UpdateStyle(ImGuiStyle* pStyle)
{
	ImGuiStyle& style = pStyle != nullptr ? *pStyle : ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.87f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.26f, 0.26f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.26f, 0.26f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] =  c::tab::tab_active;
	style.Colors[ImGuiCol_ScrollbarGrab] = c::tab::tab_active;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = c::accent;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = c::accent;
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(c::accent.x, c::accent.y, c::accent.z, 0.70f);
	style.Colors[ImGuiCol_SliderGrabActive] = c::accent;
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 0.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0);
	style.Colors[ImGuiCol_Tab] = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.80f);
	style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_HeaderHovered];
	style.Colors[ImGuiCol_TabActive] = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.60f);
	style.Colors[ImGuiCol_TabUnfocused] = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.80f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.40f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


	C_GET(ColorPickerVar_t, Vars.colPrimtv0).UpdateRainbow(); // (text)
	C_GET(ColorPickerVar_t, Vars.colPrimtv1).UpdateRainbow(); // (background)
	C_GET(ColorPickerVar_t, Vars.colPrimtv2).UpdateRainbow(); // (disabled)
	C_GET(ColorPickerVar_t, Vars.colPrimtv3).UpdateRainbow(); // (control bg)
	C_GET(ColorPickerVar_t, Vars.colPrimtv4).UpdateRainbow(); // (border)

	C_GET(ColorPickerVar_t, Vars.colAccent0).UpdateRainbow(); // (main)
	C_GET(ColorPickerVar_t, Vars.colAccent1).UpdateRainbow(); // (dark)
	C_GET(ColorPickerVar_t, Vars.colAccent2).UpdateRainbow(); // (darker)

	// update animation speed
	style.AnimationSpeed = C_GET(float, Vars.flAnimationSpeed) / 10.f;
}
static void RenderInventoryWindow() {
	static constexpr float windowWidth = 540.f;

	struct DumpedSkin_t {
		std::string m_name = "";
		int m_ID = 0;
		int m_rarity = 0;
	};
	struct DumpedItem_t {
		std::string m_name = "";
		uint16_t m_defIdx = 0;
		int m_rarity = 0;
		bool m_unusualItem = false;
		std::vector<DumpedSkin_t> m_dumpedSkins{};
		DumpedSkin_t* pSelectedSkin = nullptr;
	};
	static std::vector<DumpedItem_t> vecDumpedItems;
	static DumpedItem_t* pSelectedItem = nullptr;

	CEconItemSchema* pItemSchema =
		I::Client->GetEconItemSystem()->GetEconItemSchema();


	if (ImGui::Begin("cs2sdk item dumper", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (vecDumpedItems.empty() &&
			ImGui::Button("Dump items", { windowWidth, 0 })) {
		

			const CUtlMap<int, CEconItemDefinition*>& vecItems =
				pItemSchema->GetSortedItemDefinitionMap();
			const CUtlMap<int, CPaintKit*>& vecPaintKits =
				pItemSchema->GetPaintKits();
			const CUtlMap<uint64_t, AlternateIconData_t>& vecAlternateIcons =
				pItemSchema->GetAlternateIconsMap();

			for (const auto& it : vecItems) {
				CEconItemDefinition* pItem = it.m_value;
				if (!pItem) continue;

				const bool isWeapon = pItem->IsWeapon();
				
				auto isKnife = (pItem->m_pszItemTypeName != "#CSGO_Type_Knife");
			//	auto isGloves = pItem->IsGlove(true, pItem->m_pszItemTypeName);
				 
				if (!isWeapon && !isKnife) continue;

				// Some items don't have names.
				const char* itemBaseName = pItem->m_pszItemBaseName;
				if (!itemBaseName || itemBaseName[0] == '\0') continue;

				const uint16_t defIdx = pItem->m_nDefIndex;

				DumpedItem_t dumpedItem;
				dumpedItem.m_name = I::Localize->FindSafe(itemBaseName);
				dumpedItem.m_defIdx = defIdx;
				dumpedItem.m_rarity = pItem->m_nItemRarity;
				if (isKnife) {
					dumpedItem.m_unusualItem = true;
				}

			

				// We filter skins by guns.
				for (const auto& it : vecPaintKits) {
					CPaintKit* pPaintKit = it.m_value;
					if (!pPaintKit || pPaintKit->PaintKitId() == 0 || pPaintKit->PaintKitId() == 9001)
						continue;

					const uint64_t skinKey =
						Helper_GetAlternateIconKeyForWeaponPaintWearItem(
							defIdx, pPaintKit->PaintKitId(), 0);
					if (vecAlternateIcons.FindByKey(skinKey)) {
						DumpedSkin_t dumpedSkin;
						dumpedSkin.m_name = I::Localize->FindSafe(
							pPaintKit->PaintKitDescriptionTag());
						dumpedSkin.m_ID = pPaintKit->PaintKitId();
						dumpedSkin.m_rarity = pPaintKit->PaintKitRarity();
						dumpedItem.m_dumpedSkins.emplace_back(dumpedSkin);
					}
				}

				// Sort skins by rarity.
				if (!dumpedItem.m_dumpedSkins.empty() && isWeapon) {
					std::sort(dumpedItem.m_dumpedSkins.begin(),
						dumpedItem.m_dumpedSkins.end(),
						[](const DumpedSkin_t& a, const DumpedSkin_t& b) {
							return a.m_rarity > b.m_rarity;
						});
				}

				vecDumpedItems.emplace_back(dumpedItem);
			}
		}


		if (!vecDumpedItems.empty()) {
			if (ImGui::Button("Add all items", { windowWidth, 0.f })) {
				for (const auto& item : vecDumpedItems) {
					for (const auto& skin : item.m_dumpedSkins) {
						CEconItem* pItem = CEconItem::CreateInstance();
						L_PRINT(LOG_INFO) << "item addr:" << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << reinterpret_cast<uintptr_t>(pItem);
						if (pItem) {
							CCSPlayerInventory* pInventory =
								CCSPlayerInventory::GetInstance();

							auto highestIDs = pInventory->GetHighestIDs();
							L_PRINT(LOG_INFO) << "uid:" << pItem->m_ulID << " id:" << pItem->m_unAccountID << "idx:" << pItem->m_unDefIndex;
							pItem->m_ulID = highestIDs.first + 1;
							pItem->m_unInventory = highestIDs.second + 1;
							pItem->m_unAccountID =
								uint32_t(pInventory->GetOwner().m_id);
							pItem->m_unDefIndex = item.m_defIdx;
							if (item.m_unusualItem) pItem->m_nQuality = IQ_UNUSUAL;
							pItem->m_nRarity =
								std::clamp(item.m_rarity + skin.m_rarity - 1, 0,
									(skin.m_rarity == 7) ? 7 : 6);

							pItem->SetPaintKit((float)skin.m_ID);
							pItem->SetPaintSeed(1.f);
							if (pInventory->AddEconItem(pItem))
								skin_changer::AddEconItemToList(pItem);
						}
					}
				}
			}

		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Will cause lag on weaker computers.");

		static ImGuiTextFilter itemFilter;
		itemFilter.Draw("##filter", windowWidth);

		if (ImGui::BeginListBox("##items", { windowWidth, 140.f })) {
			for (auto& item : vecDumpedItems) {
				if (!itemFilter.PassFilter(item.m_name.c_str())) continue;

				ImGui::PushID(&item);
				if (ImGui::Selectable(item.m_name.c_str(),
					pSelectedItem == &item)) {
					if (pSelectedItem == &item)
						pSelectedItem = nullptr;
					else
						pSelectedItem = &item;
				}
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}

		if (pSelectedItem) {
			if (!pSelectedItem->m_dumpedSkins.empty()) {
				static ImGuiTextFilter skinFilter;
				skinFilter.Draw("##filter2", windowWidth);

				if (ImGui::BeginListBox("##skins", { windowWidth, 140.f })) {
					for (auto& skin : pSelectedItem->m_dumpedSkins) {
						if (!skinFilter.PassFilter(skin.m_name.c_str()))
							continue;

						ImGui::PushID(&skin);
						if (ImGui::Selectable(
							skin.m_name.c_str(),
							pSelectedItem->pSelectedSkin == &skin)) {
							if (pSelectedItem->pSelectedSkin == &skin)
								pSelectedItem->pSelectedSkin = nullptr;
							else
								pSelectedItem->pSelectedSkin = &skin;
						}
						ImGui::PopID();
					}
					ImGui::EndListBox();
				}
			}

			char buttonLabel[128];
			snprintf(buttonLabel, 128, "Add every %s skin",
				pSelectedItem->m_name.c_str());

			if (ImGui::Button(buttonLabel, { windowWidth, 0.f })) {
				for (const auto& skin : pSelectedItem->m_dumpedSkins) {
					CEconItem* pItem = CEconItem::CreateInstance();
					if (pItem) {
						CCSPlayerInventory* pInventory =
							CCSPlayerInventory::GetInstance();

						auto highestIDs = pInventory->GetHighestIDs();

						pItem->m_ulID = highestIDs.first + 1;
						pItem->m_unInventory = highestIDs.second + 1;
						pItem->m_unAccountID =
							uint32_t(pInventory->GetOwner().m_id);
						pItem->m_unDefIndex = pSelectedItem->m_defIdx;
						if (pSelectedItem->m_unusualItem)
							pItem->m_nQuality = IQ_UNUSUAL;
						pItem->m_nRarity = std::clamp(
							pSelectedItem->m_rarity + skin.m_rarity - 1, 0,
							(skin.m_rarity == 7) ? 7 : 6);

						pItem->SetPaintKit((float)skin.m_ID);
						pItem->SetPaintSeed(1.f);
						if (pInventory->AddEconItem(pItem))
							skin_changer::AddEconItemToList(pItem);
					}
				}
			}

			if (pSelectedItem->pSelectedSkin) {
				static float kitWear = 0.f;
				static int kitSeed = 1;
				static int gunKills = -1;
				static char gunName[32];

				bool vanillaSkin = pSelectedItem->pSelectedSkin->m_ID == 0;
				snprintf(
					buttonLabel, 128, "Add %s%s%s",
					pSelectedItem->m_name.c_str(), vanillaSkin ? "" : " | ",
					vanillaSkin ? ""
					: pSelectedItem->pSelectedSkin->m_name.c_str());

				if (ImGui::Button(buttonLabel, { windowWidth, 0.f })) {
					CEconItem* pItem = CEconItem::CreateInstance();
					if (pItem) {
						CCSPlayerInventory* pInventory =
							CCSPlayerInventory::GetInstance();

						auto highestIDs = pInventory->GetHighestIDs();
						L_PRINT(LOG_INFO) << "item addr:" << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << reinterpret_cast<uintptr_t>(pItem);
						L_PRINT(LOG_INFO) << "uid:" << pItem->m_ulID << " id:" << pItem->m_unAccountID << "idx:" << pItem->m_unDefIndex;

						pItem->m_ulID = highestIDs.first + 1;
						pItem->m_unInventory = highestIDs.second + 1;
						pItem->m_unAccountID =
							uint32_t(pInventory->GetOwner().m_id);
						pItem->m_unDefIndex = pSelectedItem->m_defIdx;

						if (pSelectedItem->m_unusualItem)
							pItem->m_nQuality = IQ_UNUSUAL;

						// I don't know nor do care why the rarity is calculated
						// like this. [Formula]
						pItem->m_nRarity = std::clamp(
							pSelectedItem->m_rarity +
							pSelectedItem->pSelectedSkin->m_rarity - 1,
							0,
							(pSelectedItem->pSelectedSkin->m_rarity == 7) ? 7
							: 6);

						pItem->SetPaintKit(
							(float)pSelectedItem->pSelectedSkin->m_ID);
						pItem->SetPaintSeed((float)kitSeed);
						pItem->SetPaintWear(kitWear);

						if (gunKills >= 0) {
							pItem->SetStatTrak(gunKills);
							pItem->SetStatTrakType(0);

							// Applied automatically on knives.
							if (pItem->m_nQuality != IQ_UNUSUAL)
								pItem->m_nQuality = IQ_STRANGE;
						}

						if (pInventory->AddEconItem(pItem))
							skin_changer::AddEconItemToList(pItem);

						kitWear = 0.f;
						kitSeed = 1;
						gunKills = -1;
						memset(gunName, '\0', IM_ARRAYSIZE(gunName));
					}
				}

				ImGui::Dummy({ 0, 8 });
				ImGui::SeparatorText("Extra settings");

				ImGui::TextUnformatted("Wear Rating");
				ImGui::SetNextItemWidth(windowWidth);
				ImGui::SliderFloat("##slider1", &kitWear, 0.f, 1.f, "%.9f",
					ImGuiSliderFlags_Logarithmic);

				ImGui::TextUnformatted("Pattern Template");
				ImGui::SetNextItemWidth(windowWidth);
				ImGui::SliderInt("##slider2", &kitSeed, 1, 1000);

				ImGui::TextUnformatted("StatTrak Count");
				ImGui::SetNextItemWidth(windowWidth);
				ImGui::SliderInt("##slider3", &gunKills, -1, INT_MAX / 2,
					gunKills == -1 ? "Not StatTrak" : "%d",
					ImGuiSliderFlags_Logarithmic);

				ImGui::TextUnformatted("Custom Name");
				ImGui::SetNextItemWidth(windowWidth);
				ImGui::InputTextWithHint("##input1", "Default", gunName,
					IM_ARRAYSIZE(gunName));
			}
		}
	}

	ImGui::End();
}


#pragma region menu_tabs

void T::Render(const char* szTabBar, const CTab* arrTabs, const unsigned long long nTabsCount, int* nCurrentTab, ImGuiTabBarFlags flags)
{
	if (ImGui::BeginTabBar(szTabBar, flags))
	{
		for (std::size_t i = 0U; i < nTabsCount; i++)
		{
			// add tab
			if (ImGui::BeginTabItem(arrTabs[i].szName))
			{
				// set current tab index
				*nCurrentTab = (int)i;
				ImGui::EndTabItem();
			}
		}

		// render inner tab
		if (arrTabs[*nCurrentTab].pRenderFunction != nullptr)
			arrTabs[*nCurrentTab].pRenderFunction();

		ImGui::EndTabBar();
	}
}


#pragma endregion

#pragma region menu_particle

void MENU::ParticleContext_t::Render(ImDrawList* pDrawList, const ImVec2& vecScreenSize, const float flAlpha)
{
	if (this->vecParticles.empty())
	{
		for (int i = 0; i < 100; i++)
			this->AddParticle(ImGui::GetIO().DisplaySize);
	}

	for (auto& particle : this->vecParticles)
	{
		this->DrawParticle(pDrawList, particle, C_GET(ColorPickerVar_t, Vars.colAccent0).colValue.Set<COLOR_A>(flAlpha * 255));
		this->UpdatePosition(particle, vecScreenSize);
		this->FindConnections(pDrawList, particle, C_GET(ColorPickerVar_t, Vars.colAccent2).colValue.Set<COLOR_A>(flAlpha * 255), 200.f);
	}
}

void MENU::ParticleContext_t::AddParticle(const ImVec2& vecScreenSize)
{
	// exceeded limit
	if (this->vecParticles.size() >= 200UL)
		return;

	// @note: random speed value
	static constexpr float flSpeed = 100.f;
	this->vecParticles.emplace_back(
	ImVec2(MATH::fnRandomFloat(0.f, vecScreenSize.x), MATH::fnRandomFloat(0.f, vecScreenSize.y)),
	ImVec2(MATH::fnRandomFloat(-flSpeed, flSpeed), MATH::fnRandomFloat(-flSpeed, flSpeed)));
}

void MENU::ParticleContext_t::DrawParticle(ImDrawList* pDrawList, ParticleData_t& particle, const Color_t& colPrimary)
{
	D::AddDrawListCircle(pDrawList, particle.vecPosition, 2.f, colPrimary, 12, DRAW_CIRCLE_OUTLINE | DRAW_CIRCLE_FILLED);
}

void MENU::ParticleContext_t::FindConnections(ImDrawList* pDrawList, ParticleData_t& particle, const Color_t& colPrimary, float flMaxDistance)
{
	for (auto& currentParticle : this->vecParticles)
	{
		// skip current particle
		if (&currentParticle == &particle)
			continue;

		/// @note: calcuate length distance 2d, return FLT_MAX if failed
		const float flDistance = ImLength(particle.vecPosition - currentParticle.vecPosition, FLT_MAX);
		if (flDistance <= flMaxDistance)
			this->DrawConnection(pDrawList, particle, currentParticle, (flMaxDistance - flDistance) / flMaxDistance, colPrimary);
	}
}

void MENU::ParticleContext_t::DrawConnection(ImDrawList* pDrawList, ParticleData_t& particle, ParticleData_t& otherParticle, float flAlpha, const Color_t& colPrimary) const
{
	D::AddDrawListLine(pDrawList, particle.vecPosition, otherParticle.vecPosition, colPrimary.Set<COLOR_A>(flAlpha * 255), 1.f);
}

void MENU::ParticleContext_t::UpdatePosition(ParticleData_t& particle, const ImVec2& vecScreenSize) const
{
	this->ResolveScreenCollision(particle, vecScreenSize);

	ImGuiStyle& style = ImGui::GetStyle();

	// move particle
	particle.vecPosition.x += (particle.vecVelocity.x * style.AnimationSpeed * 10.f) * ImGui::GetIO().DeltaTime;
	particle.vecPosition.y += (particle.vecVelocity.y * style.AnimationSpeed * 10.f) * ImGui::GetIO().DeltaTime;
}

void MENU::ParticleContext_t::ResolveScreenCollision(ParticleData_t& particle, const ImVec2& vecScreenSize) const
{
	if (particle.vecPosition.x + particle.vecVelocity.x > vecScreenSize.x || particle.vecPosition.x + particle.vecVelocity.x < 0)
		particle.vecVelocity.x = -particle.vecVelocity.x;

	if (particle.vecPosition.y + particle.vecVelocity.y > vecScreenSize.y || particle.vecPosition.y + particle.vecVelocity.y < 0)
		particle.vecVelocity.y = -particle.vecVelocity.y;
}

#pragma endregion
