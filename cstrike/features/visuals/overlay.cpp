// used: [stl] vector
#include <vector>
// used: [stl] sort
#include <algorithm>

#include "overlay.h"

// used: cheat variables
#include "../../core/variables.h"

// used: entity
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../cstrike/sdk/interfaces/iglobalvars.h"
#include "../../sdk/interfaces/cgameentitysystem.h"



// used: sdk variables
#include "../../core/sdk.h"

// used: l_print
#include "../../utilities/log.h"
// used: inputsystem
#include "../../utilities/inputsystem.h"
// used: draw system
#include "../../utilities/draw.h"

// used: mainwindowopened
#include "../../core/menu.h"
#include "../../sdk/datatypes/utlvector.h"
#include <cstddef> 
#include <xcharconv.h>
#include <xiosbase>
#include <iostream>
#include "imgui/imgui_edited.hpp"
using namespace F::VISUALS;
float mAlpha[65];
#pragma region visual_overlay_components

ImVec2 OVERLAY::CBaseComponent::GetBasePosition(const ImVec4& box) const
{
	return { box[this->nSide == SIDE_RIGHT ? SIDE_RIGHT : SIDE_LEFT], box[this->nSide == SIDE_BOTTOM ? SIDE_BOTTOM : SIDE_TOP] };
}

ImVec2 OVERLAY::CBaseDirectionalComponent::GetBasePosition(const ImVec4& box) const
{
	ImVec2 vecBasePosition = {};

	if (this->nSide == SIDE_TOP || this->nSide == SIDE_BOTTOM)
	{
		vecBasePosition = { (box[SIDE_LEFT] + box[SIDE_RIGHT]) * 0.5f, box[this->nSide] };
	}
	else if (this->nSide == SIDE_LEFT || this->nSide == SIDE_RIGHT)
	{
		vecBasePosition = { box[this->nSide], box[this->nDirection == DIR_TOP ? SIDE_BOTTOM : SIDE_TOP] };
	}
	else
	{
		L_PRINT(LOG_ERROR) << CS_XOR("CBaseDirectionalComponent::GetBasePosition: invalid side: ") << this->nSide;
		return vecBasePosition;
	}

	if (this->nSide != SIDE_RIGHT && this->nDirection != DIR_RIGHT)
		vecBasePosition.x -= this->vecSize.x * ((static_cast<std::uint8_t>(this->nDirection) == static_cast<std::uint8_t>(this->nSide) && (this->nSide & 1U) == 1U) ? 0.5f : 1.0f);

	if (this->nSide == SIDE_TOP || this->nDirection == DIR_TOP)
		vecBasePosition.y -= this->vecSize.y;

	return vecBasePosition;
}

OVERLAY::CBarComponent::CBarComponent(const bool bIsMenuItem, const EAlignSide nAlignSide, const ImVec4& vecBox,const float max_limit, const float flProgressFactor, const std::size_t uOverlayVarIndex, const float alphaM) :
	bIsMenuItem(bIsMenuItem), uOverlayVarIndex(uOverlayVarIndex), max_limit(MATH::Clamp(flProgressFactor, 0.f, 1.f)), flProgressFactor(MATH::Clamp(flProgressFactor, 0.f, 1.f)), alphaMultiplier(alphaM)
{
	this->nSide = nAlignSide;
	const bool bIsHorizontal = ((nAlignSide & 1U) == 1U);
	this->value = static_cast<int>(flProgressFactor * 100);
	this->alphaMultiplier = alphaM;
	if (this->nSide == SIDE_BOTTOM || this->nSide == SIDE_TOP) 
		this->value_sz = std::to_string(static_cast<int>(flProgressFactor * max_limit)) + CS_XOR(" ");
	else
		this->value_sz = std::to_string(static_cast<int>(flProgressFactor * 100)) + CS_XOR(" ");

	const BarOverlayVar_t& overlayConfig = C_GET(BarOverlayVar_t, uOverlayVarIndex);
	this->vecSize = { (bIsHorizontal ? vecBox[SIDE_RIGHT] - vecBox[SIDE_LEFT] : overlayConfig.flThickness), (bIsHorizontal ? overlayConfig.flThickness : vecBox[SIDE_BOTTOM] - vecBox[SIDE_TOP]) };
}

void OVERLAY::CBarComponent::Render(ImDrawList* pDrawList, const ImVec2& vecPosition)
{
	BarOverlayVar_t& overlayConfig = C_GET(BarOverlayVar_t, uOverlayVarIndex);
	const ImVec2 vecThicknessOffset = { overlayConfig.flThickness, overlayConfig.flThickness };
	ImVec2 vecMin = vecPosition, vecMax = vecPosition + this->vecSize;
	overlayConfig.colShadow.SetAlphaM(this->alphaMultiplier);

	
	if (this->nSide == SIDE_BOTTOM || this->nSide == SIDE_TOP) {
		// bar glow shadow
		if (overlayConfig.bGlowShadow) {
			Color_t shadow_effect_color = overlayConfig.bUseFactorColor ? Color_t::FromHSB((flProgressFactor * 120.f) / 360.f, 1.0f, 0.6f * this->alphaMultiplier) : overlayConfig.colShadow;
			D::AddDrawListShadowRect(
				pDrawList,
				ImVec2(vecMin.x + 5.f, vecMin.y),
				ImVec2(vecMax.x - 5.f, vecMax.y),
				shadow_effect_color,
				16.f,
				0.f
			);
		}
		// background glow
		if (overlayConfig.bBackground)
			pDrawList->AddShadowRect(ImVec2(vecMin.x + 2.f, vecMin.y), ImVec2(vecMax.x - 2.f, vecMax.y), overlayConfig.colBackground.GetU32(this->alphaMultiplier), 1.f, ImVec2(0, 0));
		// outline
		if (overlayConfig.bOutline)
			pDrawList->AddRect(ImVec2(vecMin.x + 2.f, vecMin.y ), ImVec2(vecMax.x - 2.f, vecMax.y), overlayConfig.colOutline.GetU32( this->alphaMultiplier), 0.f, ImDrawFlags_None, overlayConfig.flThickness);
	}
	else {
		// bar glow shadow
		if (overlayConfig.bGlowShadow) {
			Color_t shadow_effect_color = overlayConfig.bUseFactorColor ? Color_t::FromHSB((flProgressFactor * 120.f) / 360.f, 1.0f, 0.6f * this->alphaMultiplier) : overlayConfig.colShadow;
			D::AddDrawListShadowRect(
				pDrawList,
				ImVec2(vecMin.x, vecMin.y + 5.f),
				ImVec2(vecMax.x, vecMax.y - 5.f),
				shadow_effect_color,
				16.f,
				0.f
			);
		}

		// background glow
		if (overlayConfig.bBackground)
			pDrawList->AddShadowRect(ImVec2(vecMin.x, vecMin.y + 2.f), ImVec2(vecMax.x, vecMax.y - 2.f), overlayConfig.colBackground.GetU32(this->alphaMultiplier), 1.f, ImVec2(0, 0));
		
		// outline
		if (overlayConfig.bOutline)
			pDrawList->AddRect(ImVec2(vecMin.x, vecMin.y + 2.f), ImVec2(vecMax.x, vecMax.y - 2.f), overlayConfig.colOutline.GetU32(this->alphaMultiplier), 0.f, ImDrawFlags_None, overlayConfig.flThickness);

	}
	
	// account outline offset
	vecMin += vecThicknessOffset;
	vecMax -= vecThicknessOffset;

	const ImVec2 vecLineSize = vecMax - vecMin;
	float flPrevProgressFactor = flProgressFactor; // Store the previous progress factor
	flProgressFactor = std::clamp(flProgressFactor, 0.0f, 1.0f); // Ensure progress factor is within the valid range (0 to 1)

	// Apply animation smoothing
	const float flAnimationSpeed = 8.f; // Adjust as needed
	flPrevProgressFactor = std::lerp(flPrevProgressFactor, flProgressFactor, flAnimationSpeed * I::GlobalVars->flFrameTime);

	// Modify active side axis by factor
	if ((this->nSide & 1U) == 0U)
		vecMin.y += vecLineSize.y * (1.0f - flPrevProgressFactor);
	else
		vecMax.x -= vecLineSize.x * (1.0f - flPrevProgressFactor);

	if (overlayConfig.bGradient && !overlayConfig.bUseFactorColor)
	{
		if (this->nSide == SIDE_LEFT || this->nSide == SIDE_RIGHT)
			pDrawList->AddRectFilledMultiColor(vecMin, vecMax, overlayConfig.colPrimary.GetU32(this->alphaMultiplier), overlayConfig.colPrimary.GetU32(this->alphaMultiplier), overlayConfig.colSecondary.GetU32(this->alphaMultiplier), overlayConfig.colSecondary.GetU32(this->alphaMultiplier));
		else
			pDrawList->AddRectFilledMultiColor(vecMin, vecMax, overlayConfig.colSecondary.GetU32(this->alphaMultiplier), overlayConfig.colPrimary.GetU32(this->alphaMultiplier), overlayConfig.colPrimary.GetU32(this->alphaMultiplier), overlayConfig.colSecondary.GetU32(this->alphaMultiplier));
	}
	else
	{
		const ImU32 u32Color = overlayConfig.bUseFactorColor ? Color_t::FromHSB((flProgressFactor * 120.f) / 360.f, 1.0f, 1.0f).GetU32(this->alphaMultiplier) : overlayConfig.colPrimary.GetU32(this->alphaMultiplier);
		pDrawList->AddRectFilled(vecMin, vecMax, u32Color, 0.f, ImDrawFlags_None);
	}

	
	std::string sz = this->value_sz;

	// Render text on the bar with black outline
	if (!sz.empty() && overlayConfig.bShowValue && this->value <= 92.f) {
		ImVec2 textSize = ImGui::CalcTextSize(sz.c_str());
		ImVec2 textPos = ImVec2((vecMin.x + vecMax.x - textSize.x) * 0.5f, (vecMin.y + vecMax.y - textSize.y) * 0.5f);

		// modify active side axis by factor
		int fill = this->nSide == SIDE_BOTTOM ? (int)std::round(flPrevProgressFactor * vecLineSize.y / this->max_limit) : (int)std::round(flPrevProgressFactor * vecLineSize.y / 100.f);

		if (this->nSide == SIDE_BOTTOM || this->nSide == SIDE_TOP) {
			textPos.y = vecMin.y - 7;
			textPos.x = vecMin.x + (fill)-5;
		}
		else {
			textPos.y = vecMin.y + (fill)-7;
			textPos.x = vecMin.x - 5;
		}
		// Draw the black outline first
		const int outlineThickness = 1; // Adjust as needed
		pDrawList->AddText(FONT::pEspHealth, FONT::pEspHealth->FontSize, ImVec2(textPos.x - outlineThickness, textPos.y), IM_COL32(0, 0, 0, 255 * this->alphaMultiplier), sz.c_str());
		pDrawList->AddText(FONT::pEspHealth, FONT::pEspHealth->FontSize, ImVec2(textPos.x + outlineThickness, textPos.y), IM_COL32(0, 0, 0, 255 * this->alphaMultiplier), sz.c_str());
		pDrawList->AddText(FONT::pEspHealth, FONT::pEspHealth->FontSize, ImVec2(textPos.x, textPos.y - outlineThickness), IM_COL32(0, 0, 0, 255 * this->alphaMultiplier), sz.c_str());
		pDrawList->AddText(FONT::pEspHealth, FONT::pEspHealth->FontSize, ImVec2(textPos.x, textPos.y + outlineThickness), IM_COL32(0, 0, 0, 255 * this->alphaMultiplier), sz.c_str());

		// Draw the original text on top
		pDrawList->AddText(FONT::pEspHealth, FONT::pEspHealth->FontSize, textPos, IM_COL32(255, 255, 255, 255 * this->alphaMultiplier), sz.c_str());
	}

	// only open menu item if menu is opened and overlay is enabled
	bIsMenuItem &= (MENU::bMainWindowOpened && overlayConfig.bEnable);
	if (bIsMenuItem)
	{
		// @note: padding 2.f incase the thickness is too small
		this->bIsHovered = ImRect(vecPosition - ImVec2(2.f, 2.f), vecPosition + this->vecSize + ImVec2(2.f, 2.f)).Contains(ImGui::GetIO().MousePos);
		// if component is hovered + right clicked
		if (this->bIsHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			ImGui::OpenPopup(CS_XOR(std::to_string(this->uOverlayVarIndex).c_str()));
	
		if (ImGui::BeginPopup(CS_XOR(std::to_string(this->uOverlayVarIndex).c_str()), ImGuiWindowFlags_NoResize  | ImGuiWindowFlags_NoMove))
		{
			ImVec2 size = ImVec2(150, 320);
			ImGui::SetWindowSize(size);

			
			if (!overlayConfig.bUseFactorColor) {
				edited::SmallCheckbox(CS_XOR("Gradient"), & overlayConfig.bGradient, 1);
				edited::Color(CS_XOR("##gradientcolor"), "", &overlayConfig.colSecondary, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			}

			edited::SmallCheckbox(CS_XOR("Factor"), &overlayConfig.bUseFactorColor, 1);
			edited::Color(CS_XOR("##factorcolor"), "", & overlayConfig.colPrimary, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

			edited::SmallCheckbox(CS_XOR("Glow"), &overlayConfig.bGlowShadow, 1);
			edited::Color(CS_XOR("##glowcolor"), "", &overlayConfig.colShadow, ImGuiColorEditFlags_AlphaBar |ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel );

			edited::SmallCheckbox(CS_XOR("Outline"), &overlayConfig.bOutline, 1);
			edited::Color(CS_XOR("##outlinecolor"), "", &overlayConfig.colOutline, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

			edited::SmallCheckbox(CS_XOR("Back"), &overlayConfig.bBackground, 1);
			edited::Color(CS_XOR("##backgroundcolor"), "", &overlayConfig.colBackground, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

			edited::SmallCheckbox(CS_XOR("Text"), &overlayConfig.bShowValue, 1);

			ImGui::SliderFloat(CS_XOR("Width"),  &overlayConfig.flThickness, 1.0f, 10.0f, CS_XOR("%.1f"), ImGuiSliderFlags_NoInput);
		
			ImGui::EndPopup();
		}
	}
	else
		// dont process hovered on menu close...
		this->bIsHovered = false;
}

OVERLAY::CTextComponent::CTextComponent(const bool bIsMenuItem, const bool bIcons, const EAlignSide nAlignSide, const EAlignDirection nAlignDirection, const ImFont* pFont, const char* szText, const std::size_t uOverlayVarIndex, const float alphaM) :
	bIsMenuItem(bIsMenuItem), bIcon(bIcons), pFont(pFont), uOverlayVarIndex(uOverlayVarIndex), alphaMultiplier(alphaM)
{
	const TextOverlayVar_t& overlayConfig = C_GET(TextOverlayVar_t, uOverlayVarIndex);
	// allocate own buffer to safely store a copy of the string
	this->bIcon = bIcons;
	this->alphaMultiplier = alphaM;
	this->szText = new char[CRT::StringLength(szText) + 1U];
	CRT::StringCopy(this->szText, szText);
	this->nSide = nAlignSide;
	this->nDirection = nAlignDirection;
	this->vecSize = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0.0f, szText) + overlayConfig.flThickness;
}

OVERLAY::CTextComponent::~CTextComponent()
{
	// deallocate buffer of the copied string
	delete[] this->szText;
}

void OVERLAY::CTextComponent::Render(ImDrawList* pDrawList, const ImVec2& vecPosition)
{
	TextOverlayVar_t& overlayConfig = C_GET(TextOverlayVar_t, this->uOverlayVarIndex);

	const ImVec2 vecOutlineOffset = { overlayConfig.flThickness, overlayConfig.flThickness };
	

	// Adjust the text position based on the side
	ImVec2 textPos = vecPosition;

	// Adjust the text position based on the side
	if (this->nSide == SIDE_BOTTOM) {
		// Move the text 2 pixels below for SIDE_BOTTOM
		textPos.y += 2.0f;
	}
	else if (this->nSide == SIDE_TOP) {
		// Move the text 2 pixels above for SIDE_TOP
		textPos.y -= 1.0f;
	}

	// @test: used for spacing debugging
	//pDrawList->AddRect(textPos, textPos + this->vecSize, IM_COL32(255, 255, 255, 255));
	// @todo: fix this cringe shit after gui merge
	if (overlayConfig.flThickness >= 1.0f)
	{
		// Adjust the text position by 2 pixels above
		pDrawList->AddText(this->pFont, this->pFont->FontSize, textPos, overlayConfig.colOutline.GetU32(this->alphaMultiplier), this->szText);
		pDrawList->AddText(this->pFont, this->pFont->FontSize, textPos + vecOutlineOffset * 2.0f, overlayConfig.colOutline.GetU32(this->alphaMultiplier), this->szText);
	}

	// Adjust the text position by 2 pixels above
	pDrawList->AddText(this->pFont, this->pFont->FontSize, textPos + vecOutlineOffset, overlayConfig.colPrimary.GetU32(this->alphaMultiplier), this->szText);

	// only open menu item if menu is opened and overlay is enabled
	bIsMenuItem &= MENU::bMainWindowOpened && overlayConfig.bEnable;
	if (bIsMenuItem)
	{
		this->bIsHovered = ImRect(vecPosition, vecPosition + this->vecSize).Contains(ImGui::GetIO().MousePos);
		// @test: used for spacing debugging
		//pDrawList->AddRect(vecPosition, vecPosition + this->vecSize, IM_COL32(this->bIsHovered ? 0 : 255, this->bIsHovered ? 255 : 0, 0, 255));

		// if component is hovered + right clicked
		if (this->bIsHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			ImGui::OpenPopup(CS_XOR(this->szText));

		if (ImGui::BeginPopup(CS_XOR(this->szText), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{

			ImVec2 size = ImVec2(90, 150);
			ImGui::SetWindowSize(size); // Adjust the size as needed

			edited::Color(CS_XOR("Primary"), "", & overlayConfig.colPrimary, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar);
			edited::Color(CS_XOR("Outline"), "", &overlayConfig.colOutline, ImGuiColorEditFlags_NoInputs  | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar);
			if (this->bIcon) {
				edited::SmallCheckbox(CS_XOR("Icon"), &overlayConfig.bIcon, 1);
			}


			ImGui::EndPopup();
		}
	}
}

#pragma endregion

#pragma region visual_overlay_context

bool OVERLAY::Context_t::AddBoxComponent(ImDrawList* pDrawList, const ImVec4& vecBox, const int nType, float flThickness, float flRounding, const Color_t& colPrimary, const Color_t& colOutline , const float AlphaMultiplier )
{
	flThickness = std::floorf(flThickness);
	const ImVec2 vecThicknessOffset = { flThickness, flThickness };

	switch (nType)
	{
	case VISUAL_OVERLAY_BOX_FULL:
	{
		const ImVec2 vecBoxMin = { vecBox[SIDE_LEFT], vecBox[SIDE_TOP] };
		const ImVec2 vecBoxMax = { vecBox[SIDE_RIGHT], vecBox[SIDE_BOTTOM] };

		// inner outline
		pDrawList->AddRect(vecBoxMin + vecThicknessOffset * 2.0f, vecBoxMax - vecThicknessOffset * 2.0f, colOutline.GetU32(AlphaMultiplier), flRounding, ImDrawFlags_RoundCornersAll, flThickness);
		// primary box
		pDrawList->AddRect(vecBoxMin + vecThicknessOffset, vecBoxMax - vecThicknessOffset, colPrimary.GetU32(AlphaMultiplier), flRounding, ImDrawFlags_RoundCornersAll, flThickness);
		// outer outline
		pDrawList->AddRect(vecBoxMin, vecBoxMax, colOutline.GetU32(AlphaMultiplier), flRounding, ImDrawFlags_RoundCornersAll, flThickness);

		break;
	}
	case VISUAL_OVERLAY_BOX_CORNERS:
	{
		// corner part of the whole line
		constexpr float flPartRatio = 0.25f;

		const float flCornerWidth = ((vecBox[SIDE_RIGHT] - vecBox[SIDE_LEFT]) * flPartRatio);
		const float flCornerHeight = ((vecBox[SIDE_BOTTOM] - vecBox[SIDE_TOP]) * flPartRatio);

		const ImVec2 arrCornerPoints[4][3] = {
			// top-left
			{ ImVec2(vecBox[SIDE_LEFT], vecBox[SIDE_TOP] + flCornerHeight) + vecThicknessOffset, ImVec2(vecBox[SIDE_LEFT], vecBox[SIDE_TOP]) + vecThicknessOffset, ImVec2(vecBox[SIDE_LEFT] + flCornerWidth, vecBox[SIDE_TOP]) + vecThicknessOffset },

			// top-right
			{ ImVec2(vecBox[SIDE_RIGHT] - flCornerWidth - vecThicknessOffset.x, vecBox[SIDE_TOP] + vecThicknessOffset.y * 2.0f), ImVec2(vecBox[SIDE_RIGHT] - vecThicknessOffset.x, vecBox[SIDE_TOP] + vecThicknessOffset.y * 2.0f), ImVec2(vecBox[SIDE_RIGHT] - vecThicknessOffset.x, vecBox[SIDE_TOP] + flCornerHeight + vecThicknessOffset.y * 2.0f) },

			// bottom-left
			{ ImVec2(vecBox[SIDE_LEFT] + flCornerWidth + vecThicknessOffset.x, vecBox[SIDE_BOTTOM] - vecThicknessOffset.y * 2.0f), ImVec2(vecBox[SIDE_LEFT] + vecThicknessOffset.x, vecBox[SIDE_BOTTOM] - vecThicknessOffset.y * 2.0f), ImVec2(vecBox[SIDE_LEFT] + vecThicknessOffset.x, vecBox[SIDE_BOTTOM] - flCornerHeight - vecThicknessOffset.y * 2.0f) },

			// bottom-right
			{ ImVec2(vecBox[SIDE_RIGHT], vecBox[SIDE_BOTTOM] - flCornerHeight) - vecThicknessOffset, ImVec2(vecBox[SIDE_RIGHT], vecBox[SIDE_BOTTOM]) - vecThicknessOffset, ImVec2(vecBox[SIDE_RIGHT] - flCornerWidth, vecBox[SIDE_BOTTOM]) - vecThicknessOffset }
		};

		for (std::size_t i = 0U; i < CS_ARRAYSIZE(arrCornerPoints); i++)
		{
			const auto& arrLinePoints = arrCornerPoints[i];
			const ImVec2 vecHalfPixelOffset = ((i & 1U) == 1U ? ImVec2(-0.5f, -0.5f) : ImVec2(0.5f, 0.5f));

			// @todo: we can even do not clear path and reuse it
			pDrawList->PathLineTo(arrLinePoints[0] + vecHalfPixelOffset);
			pDrawList->PathLineTo(arrLinePoints[1] + vecHalfPixelOffset);
			pDrawList->PathLineTo(arrLinePoints[2] + vecHalfPixelOffset);
			pDrawList->PathStroke(colOutline.GetU32(AlphaMultiplier), false, flThickness + 1.0f);

			pDrawList->PathLineTo(arrLinePoints[0] + vecHalfPixelOffset);
			pDrawList->PathLineTo(arrLinePoints[1] + vecHalfPixelOffset);
			pDrawList->PathLineTo(arrLinePoints[2] + vecHalfPixelOffset);
			pDrawList->PathStroke(colPrimary.GetU32(AlphaMultiplier), false, flThickness);
		}

		break;
	}
	default:
		break;
	}

	// accumulate spacing for next side/directional components
	for (float& flSidePadding : this->arrSidePaddings)
		flSidePadding += this->flComponentSpacing;

	return ImRect(vecBox).Contains(ImGui::GetIO().MousePos);
}

ImVec4 OVERLAY::Context_t::AddFrameComponent(ImDrawList* pDrawList, const ImVec2& vecScreen, const EAlignSide nSide, const Color_t& colBackground, const float flRounding, const ImDrawFlags nRoundingCorners)
{
	// calculate frame size by previously added components on active side
	const ImVec2 vecFrameSize = this->GetTotalDirectionalSize(nSide);

	ImVec2 vecFrameMin = { vecScreen.x - vecFrameSize.x * 0.5f, vecScreen.y - vecFrameSize.y };
	ImVec2 vecFrameMax = { vecScreen.x + vecFrameSize.x * 0.5f, vecScreen.y };

	pDrawList->AddRectFilled(vecFrameMin - this->flComponentSpacing, vecFrameMax + this->flComponentSpacing, colBackground.GetU32(), flRounding, nRoundingCorners);

	// accumulate spacing for next side/directional components
	for (float& flSidePadding : this->arrSidePaddings)
		flSidePadding += this->flComponentSpacing;

	return { vecFrameMin.x, vecFrameMin.y, vecFrameMax.x, vecFrameMax.y };
}

/*
 * @todo: currently not well designed, make it more flexible for use cases where we need e.g. previous frame bar factor etc
 * also to optimize this, allocate components at stack instead of heap + make all context units static and do not realloc components storage every frame, but reset (like memset idk) it at the end of frame
 */
void OVERLAY::Context_t::AddComponent(CBaseComponent* pComponent)
{
	// guarantee that first directional component on each side is in the primary direction
	if (pComponent->IsDirectional())
	{
		CBaseDirectionalComponent* pDirectionalComponent = static_cast<CBaseDirectionalComponent*>(pComponent);

		// check if it's not an exception direction and there are no components in the primary direction
		if (((pDirectionalComponent->nSide & 1U) == 1U || pDirectionalComponent->nDirection != DIR_TOP) && this->arrSideDirectionPaddings[pDirectionalComponent->nSide][pDirectionalComponent->nSide] == 0.0f)
			pDirectionalComponent->nDirection = static_cast<EAlignDirection>(pDirectionalComponent->nSide);
	}

	float& flSidePadding = this->arrSidePaddings[pComponent->nSide];

	if (pComponent->IsDirectional())
	{
		CBaseDirectionalComponent* pDirectionalComponent = static_cast<CBaseDirectionalComponent*>(pComponent);
		float(&arrDirectionPaddings)[DIR_MAX] = this->arrSideDirectionPaddings[pDirectionalComponent->nSide];

		// directional components don't change side paddings, but take them into account
		pComponent->vecOffset[pDirectionalComponent->nSide & 1U] += ((pDirectionalComponent->nSide < 2U) ? -flSidePadding : flSidePadding);

		// check if the component is in the same direction as the side and it's the first component in this direction
		if (static_cast<std::uint8_t>(pDirectionalComponent->nDirection) == static_cast<std::uint8_t>(pDirectionalComponent->nSide) && arrDirectionPaddings[pDirectionalComponent->nDirection] == 0.0f)
		{
			// accumulate paddings for sub-directions
			for (std::uint8_t nSubDirection = DIR_LEFT; nSubDirection < DIR_MAX; nSubDirection++)
			{
				/*
				 * exclude conflicting sub-directions
				 *
				 * SIDE_LEFT[0]: DIR_LEFT[0], DIR_BOTTOM[3] | ~2 & ~1
				 * SIDE_TOP[1]: DIR_LEFT[0], DIR_TOP[1], DIR_RIGHT[2] | ~3
				 * SIDE_RIGHT[2]: DIR_RIGHT[2], DIR_BOTTOM[3] | ~0 & ~1
				 * SIDE_BOTTOM[3]: DIR_LEFT[0], DIR_RIGHT[2], DIR_BOTTOM[3] | ~1
				 */
				if (nSubDirection == pDirectionalComponent->nSide || nSubDirection == ((pDirectionalComponent->nSide + 2U) & 3U) || (nSubDirection == DIR_TOP && (pDirectionalComponent->nSide & 1U) == 0U))
					continue;

				arrDirectionPaddings[nSubDirection] += (pDirectionalComponent->vecSize[nSubDirection == DIR_BOTTOM ? SIDE_TOP : SIDE_LEFT] * (((pDirectionalComponent->nSide & 1U) == 1U) ? 0.5f : 1.0f) + this->flComponentSpacing);
			}
		}

		float& flSideDirectionPadding = arrDirectionPaddings[pDirectionalComponent->nDirection];

		// append direction padding to offset
		pComponent->vecOffset[pDirectionalComponent->nDirection & 1U] += ((pDirectionalComponent->nDirection < 2U) ? -flSideDirectionPadding : flSideDirectionPadding);

		// accumulate direction padding for next component
		flSideDirectionPadding += pDirectionalComponent->vecSize[pDirectionalComponent->nDirection & 1U];

		// accumulate spacing for next directional components
		flSideDirectionPadding += this->flComponentSpacing;
	}
	else
	{
		// append side padding to offset
		pComponent->vecOffset[pComponent->nSide & 1U] += ((pComponent->nSide < 2U) ? -(flSidePadding + pComponent->vecSize[pComponent->nSide]) : flSidePadding);

		// accumulate side padding for next component
		flSidePadding += pComponent->vecSize[pComponent->nSide & 1U];

		// accumulate spacing for next components
		flSidePadding += this->flComponentSpacing;
	}

	this->vecComponents.push_back(pComponent);
}

ImVec2 OVERLAY::Context_t::GetTotalDirectionalSize(const EAlignSide nSide) const
{
	ImVec2 vecSideSize = {};

	// @todo: we should peek max of bottom + side or top directions at horizontal sides
	const float(&arrDirectionPaddings)[DIR_MAX] = this->arrSideDirectionPaddings[nSide];
	for (std::uint8_t nSubDirection = DIR_LEFT; nSubDirection < DIR_MAX; nSubDirection++)
		vecSideSize[nSubDirection & 1U] += arrDirectionPaddings[nSubDirection];

	return vecSideSize;
}

void OVERLAY::Context_t::Render(ImDrawList* pDrawList, const ImVec4& vecBox) const
{
	bool bCenteredFirstSideDirectional[SIDE_MAX] = {};

	for (CBaseComponent* const pComponent : this->vecComponents)
	{
		ImVec2 vecPosition = pComponent->GetBasePosition(vecBox);

		// check if the component is in the side that supports multi-component centering
		if (pComponent->nSide == SIDE_TOP || pComponent->nSide == SIDE_BOTTOM)
		{
			// check if the component is directional
			if (CBaseDirectionalComponent* const pDirectionalComponent = static_cast<CBaseDirectionalComponent*>(pComponent); pDirectionalComponent->IsDirectional())
			{
				const float(&arrDirectionPaddings)[DIR_MAX] = this->arrSideDirectionPaddings[pComponent->nSide];

				// check if the component has horizontal direction
				if (static_cast<std::uint8_t>(pDirectionalComponent->nDirection) != static_cast<std::uint8_t>(pDirectionalComponent->nSide))
					// add centering offset to the component's offset
					pDirectionalComponent->vecOffset.x += (arrDirectionPaddings[DIR_LEFT] - arrDirectionPaddings[DIR_RIGHT]) * 0.5f;
				// otherwise check if it's the first component in direction as side
				else if (!bCenteredFirstSideDirectional[pDirectionalComponent->nSide])
				{
					// add centering offset to the component's offset
					pDirectionalComponent->vecOffset.x += (arrDirectionPaddings[DIR_LEFT] - arrDirectionPaddings[DIR_RIGHT]) * 0.5f;

					bCenteredFirstSideDirectional[pDirectionalComponent->nSide] = true;
				}
			}
		}

		// add final component offset to the base position
		vecPosition += pComponent->vecOffset;

		pComponent->Render(pDrawList, vecPosition);
	}
}

#pragma endregion
/*
void OVERLAY::OnFrameStageNotify(CCSPlayerController* pLocalController)
{
	// only render when in-game
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return;

	if (!C_GET(bool, Vars.bVisualOverlay))
		return;


	enum ESortEntityType : int
	{
		SORT_ENTITY_NONE = -1,
		SORT_ENTITY_PLAYER = 0,
	};

	struct SortEntityObject_t
	{
		SortEntityObject_t(C_BaseEntity* pEntity, CBaseHandle hEntity, float flDistance, ESortEntityType nSortType) :
			pEntity(pEntity), hEntity(hEntity), flDistance(flDistance), nSortType(nSortType) { }

		C_BaseEntity* pEntity;
		CBaseHandle hEntity;
		float flDistance;
		ESortEntityType nSortType;
	};

	const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();

	std::vector<SortEntityObject_t> vecSortedEntities = {};
	vecSortedEntities.reserve(nHighestIndex);

	// @note: 0 is resved for world entity 'CWorld'
	for (int nIndex = 1; nIndex <= nHighestIndex; nIndex++)
	{
		C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(nIndex);
		if (pEntity == nullptr)
			continue;

		SchemaClassInfoData_t* pClassInfo = nullptr;
		pEntity->GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			continue;

		const FNV1A_t uHashedName = FNV1A::Hash(pClassInfo->szNname);

		ESortEntityType nEntityType = SORT_ENTITY_NONE;
		Vector_t vecOrigin = Vector_t();

		if (uHashedName == FNV1A::HashConst("CCSPlayerController"))
		{
			nEntityType = SORT_ENTITY_PLAYER;
			CCSPlayerController* pPlayer = reinterpret_cast<CCSPlayerController*>(pEntity);
			if (pPlayer == nullptr)
				continue;

			vecOrigin = pPlayer->GetPawnOrigin();
		}

		// only add sortable entities
		if (nEntityType != SORT_ENTITY_NONE)
			vecSortedEntities.emplace_back(pEntity, pEntity->GetRefEHandle(), SDK::CameraPosition.DistTo(vecOrigin), nEntityType);
	}

	// sort entities by distance to draw them from the farthest to the nearest
	std::ranges::sort(vecSortedEntities.begin(), vecSortedEntities.end(), std::ranges::greater{}, &SortEntityObject_t::flDistance);

	for (auto& [pEntity, hEntity, flDistance, nSortType] : vecSortedEntities)
	{
		// if the handle is invalid, skip this entity
		if (!hEntity.IsValid())
			continue;

		switch (nSortType)
		{
		case SORT_ENTITY_PLAYER:
		{
			CCSPlayerController* pPlayer = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(hEntity);
			if (pPlayer == nullptr)
				break;

			if (!pPlayer->IsPawnAlive())
				break;

			Player(pEntity, pLocalController, pPlayer, flDistance);

			break;
		}
		default:
			break;
		}
	}
}*/
#include <mutex>

static ImDrawList* g_pBackgroundDrawList = nullptr;
static CCSPlayerController* pLocalController = nullptr;
static C_CSPlayerPawn* pLocal = nullptr;

#include  "../legit/legit.h"


void F::VISUALS::OVERLAY::Render()
{
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame()) return;

	if (!ImGui::GetBackgroundDrawList())
		return;

	CCSPlayerController* pLocalController = CCSPlayerController::GetLocalPlayerController();
	if (!pLocalController) 
		return;

	pLocal = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalController->GetPawnHandle());
	if (!pLocal) 
		return;


	const std::lock_guard<std::mutex> guard{ g_cachedEntitiesMutex };
	for (const auto& it : g_cachedEntities) {

		if (!it.m_draw) continue;

		C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(it.m_handle);
		if (pEntity == nullptr)
			continue;

		// Additional sanity check.
		CBaseHandle hEntity = pEntity->GetRefEHandle();
		if (hEntity != it.m_handle) continue;

		switch (it.m_type) {
		case CachedEntity_t::PLAYER_CONTROLLER:
			CCSPlayerController* pPlayer = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(hEntity);
			if (pPlayer == nullptr)
				break;

			OnPlayer(pPlayer, it.m_bbox);

			break;
		}
	}

}

void F::VISUALS::OVERLAY::CalculateBoundingBoxes()
{
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame()) return;

	if (!pLocal)
		return;

	if (!I::GameResourceService->pGameEntitySystem) return;

	const std::lock_guard<std::mutex> guard{ g_cachedEntitiesMutex };

	for (auto& it : g_cachedEntities) {
		C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(it.m_handle);
		if (!pEntity) continue;

		// Additional sanity check.
		CBaseHandle hEntity = pEntity->GetRefEHandle();
		if (hEntity != it.m_handle) continue;

		switch (it.m_type) {

		case CachedEntity_t::PLAYER_CONTROLLER:

			CCSPlayerController* pPlayer = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(hEntity);
			if (pPlayer == nullptr)
				break;

			C_CSPlayerPawn* pPlayerPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
			if (pPlayerPawn == nullptr)
				break;

			it.m_draw = pPlayerPawn->CalculateBoundingBox(it.m_bbox, false);
		//	it.hitboxpos[6] = pPlayerPawn->UpdateHitboxData(6);
			break;
		}
	}
}
#include <map>
#include "../../core/spoofcall/virtualization/VirtualizerSDK64.h"
namespace WeaponsIcons {
	std::map<std::string, const wchar_t*> gunIcons = {
		{"weapon_p90", L"P"},
		{"weapon_mp9", L"O"},
		{"weapon_mp5sd", L"O"},
		{"weapon_m4a4", L"M"},
		{"weapon_knife", L"]"},
		{"weapon_knife_ct", L"]"},
		{"weapon_knife_t", L"]"},
		{"weapon_deagle", L"A"},
		{"weapon_elite", L"B"},
		{"weapon_fiveseven", L"C"},
		{"weapon_glock", L"D"},
		{"weapon_revolver", L"J"},
		{"weapon_hkp2000", L"E"},
		{"weapon_p250", L"F"},
		{"weapon_usp_silencer", L"G"},
		{"weapon_tec9", L"H"},
		{"weapon_cz75a", L"I"},
		{"weapon_mac10", L"K"},
		{"weapon_ump45", L"L"},
		{"weapon_bizon", L"M"},
		{"weapon_mp7", L"N"},
		{"weapon_galilar", L"Q"},
		{"weapon_famas", L"R"},
		{"weapon_m4a1_silencer", L"T"},
		{"weapon_m4a1", L"S"},
		{"weapon_aug", L"U"},
		{"weapon_sg556", L"V"},
		{"weapon_ak47", L"W"},
		{"weapon_g3sg1", L"X"},
		{"weapon_scar20", L"Y"},
		{"weapon_awp", L"Z"},
		{"weapon_ssg08", L"a"},
		{"weapon_xm1014", L"b"},
		{"weapon_sawedoff", L"c"},
		{"weapon_mag7", L"d"},
		{"weapon_nova", L"e"},
		{"weapon_negev", L"f"},
		{"weapon_m249", L"g"},
		{"weapon_taser", L"h"},
		{"weapon_flashbang", L"i"},
		{"weapon_hegrenade", L"j"},
		{"weapon_smokegrenade", L"k"},
		{"weapon_molotov", L"l"},
		{"weapon_decoy", L"m"},
		{"weapon_incgrenade", L"n"},
		{"weapon_c4", L"o"},
		{"weapon_bayonet", L"]"},
		{"weapon_knife_survival_bowie", L"]"},
		{"weapon_knife_butterfly", L"]"},
		{"weapon_knife_canis", L"]"},
		{"weapon_knife_cord", L"]"},
		{"weapon_knife_css", L"]"},
		{"weapon_knife_falchion", L"]"},
		{"weapon_knife_flip", L"]"},
		{"weapon_knife_gut", L"]"},
		{"weapon_knife_karambit", L"]"},
		{"weapon_knife_twinblade", L"]"},
		{"weapon_knife_kukri", L"]"},
		{"weapon_knife_m9_bayonet", L"]"},
		{"weapon_knife_outdoor", L"]"},
		{"weapon_knife_push", L"]"},
		{"weapon_knife_skeleton", L"]"},
		{"weapon_knife_stiletto", L"]"},
		{"weapon_knife_tactical", L"]"},
		{"weapon_knife_widowmaker", L"]"},
		{"weapon_knife_ursus", L"]"}
	};

	wchar_t get(std::string designerName) {
		try {
			return *gunIcons.at(designerName);
		}
		catch (std::out_of_range& const e) {
			return NULL;
		}
	}
};
static Vector_t get_target_angle(C_CSPlayerPawn* localplayer, Vector_t position)
{
	Vector_t eye_position = localplayer->GetEyePosition();
	Vector_t angle = position;

	angle.x = position.x - eye_position.x;
	angle.y = position.y - eye_position.y;
	angle.z = position.z - eye_position.z;

	angle.Normalizes();
	MATH::vec_angles(angle, &angle);

	angle.clamp();
	return angle;
}
void OVERLAY::CalculateSkeleton(Context_t ctx, CCSPlayerController* pPlayerController, C_CSPlayerPawn* player, const ImVec4& out) {
	if (!pLocal)
		return;

	auto game_scene_node = player->GetGameSceneNode();
	if (!game_scene_node)
		return;

	auto skeleton = game_scene_node->GetSkeletonInstance();
	if (!skeleton)
		return ;
	auto model_state = &skeleton->GetModel();
	CStrongHandle<CModel> model = model_state->m_hModel();
	auto model_skelet = &model->m_modelSkeleton();

	if (!model_skelet)
		return ;


	skeleton->calc_world_space_bones(0, bone_flags::FLAG_HITBOX);

	const auto num_bones = model->GetHitboxesNum();
	auto bones = model_state->GetHitboxData();

	/* this method is not proper fuck it*/
	/*CTransform* boneToWorldTransform = model_state->BoneTransform();
	const Matrix3x4_t boneMatTransform = boneToWorldTransform->quatOrientation.ToMatrix(boneToWorldTransform->vecPosition);
	/////////////////////////////////////////////////////////////////////////////////*/

	for (uint32_t i = 0; i < num_bones; i++) {
		if (!(model->GetHitboxFlags(i) & bone_flags::FLAG_HITBOX)) {
			continue;
		}

		auto parent_index = model->GetHitboxParent(i);
		if (parent_index == -1)
			continue;

		ImVec2 start_scr, end_scr;
		if (!D::WorldToScreen(bones[i].pos, start_scr) || !D::WorldToScreen(bones[parent_index].pos, end_scr))
			continue;

		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(start_scr.x, start_scr.y), ImVec2(end_scr.x, end_scr.y), C_GET(ColorPickerVar_t, Vars.colSkeletonOutline).colValue.GetU32(), 2.f);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(start_scr.x, start_scr.y), ImVec2(end_scr.x, end_scr.y), C_GET(ColorPickerVar_t, Vars.colSkeleton).colValue.GetU32(), 1.f);
/*
		char buf[16];
		sprintf_s(buf, "%d", i); // Convert bone ID to a string
		ImVec2 textPos = ImVec2(start_scr.x + (end_scr.x - start_scr.x) * 0.5f, start_scr.y + (end_scr.y - start_scr.y) * 0.5f);
		ImGui::GetBackgroundDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), buf);
*/
	}
}
bool OVERLAY::IsValid(CCSPlayerController* player) {
	if (!player)
		return false;

	C_CSPlayerPawn* pPlayerPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(player->GetPawnHandle());
	if (!pPlayerPawn)
		return false;


	auto idx = player->GetIdentity()->GetEntryIndex();
	if (pPlayerPawn->GetHealth() <= 0) {
		static constexpr auto ALPHA_FREQUENCY = 255 / 70.f;
		mAlpha[idx] -= ALPHA_FREQUENCY * I::GlobalVars->flFrameTime;
	}
	else {
		if (player->GetGameSceneNode()->IsDormant()) {
			if (mAlpha[idx] < 0.6f) {
				mAlpha[idx] -= (1.0f / 1.0f) * I::GlobalVars->flFrameTime;
				mAlpha[idx] = std::clamp(mAlpha[idx], 0.f, 0.6f);
			}
			else {
				mAlpha[idx] -= (1.0f / 20.f) * I::GlobalVars->flFrameTime;
			}
		}
		else {
			mAlpha[idx] += (1.0f / 0.2f) * I::GlobalVars->flFrameTime;
			mAlpha[idx] = std::clamp(mAlpha[idx], 0.f, 1.f);
		}
	}
	return (mAlpha[idx] > 0.f);
}
#include "../cstrike/features/lagcomp/lagcomp.h"
#include "../cstrike/core/spoofcall/Invoker.h"
void OVERLAY::OnPlayer(CCSPlayerController* player, const ImVec4& out) {

	if (!pLocal)
		return;

	if (!player)
		return;

	C_CSPlayerPawn* pPlayerPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(player->GetPawnHandle());
	if (pPlayerPawn == nullptr)
		return;

	if (pPlayerPawn->GetHealth() <= 0)
		return;

	if (!player->GetIdentity())
		return;

	Context_t context;

	bool bIsEnemy = (pLocal->IsOtherEnemy(pPlayerPawn));
	if (!bIsEnemy)
		return;

	if (C_GET(bool, Vars.bSkeleton))
		F::VISUALS::OVERLAY::CalculateSkeleton(context, player, pPlayerPawn, out);

	if (!C_GET(bool, Vars.bVisualOverlay))
		return;

	/* bounding box esp */
	if (const auto& frameOverlayConfig = C_GET(FrameOverlayVar_t, Vars.overlayBox); frameOverlayConfig.bEnable) {

		context.AddBoxComponent(ImGui::GetBackgroundDrawList(), out, 1, frameOverlayConfig.flThickness, frameOverlayConfig.flRounding, frameOverlayConfig.colPrimary, frameOverlayConfig.colOutline, 1.f);
	}
	/* health bar esp */
	if (const auto& health_config = C_GET(BarOverlayVar_t, Vars.overlayHealthBar); health_config.bEnable) {

		auto hp = pPlayerPawn->GetHealth() / 100.f;

		// Adjust alpha value for health bar

		context.AddComponent(new CBarComponent(false, SIDE_LEFT, out, 100, hp, Vars.overlayHealthBar, 1.f));
	}

	/* name esp */
	if (const auto& name_config = C_GET(TextOverlayVar_t, Vars.overlayName); name_config.bEnable) {
		const char* szName = CS_XOR("");

		if (player->GetPlayerName() != nullptr)
			szName = player->GetPlayerName();

		context.AddComponent(new CTextComponent(true, false, SIDE_TOP, DIR_TOP, FONT::pEspName, szName, Vars.overlayName,1.f));
	}

	CPlayer_WeaponServices* WeaponServices = pPlayerPawn->GetWeaponServices();
	if (WeaponServices) {
		auto ActiveWeapon = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(WeaponServices->m_hActiveWeapon());
		if (ActiveWeapon) {

			auto pAttributeContainer = &ActiveWeapon->m_AttributeManager();
			if (!pAttributeContainer) return;

			auto data = ActiveWeapon->datawep();
			if (!data)
				return;

			auto szWeaponName = data->m_szName();

			if (szWeaponName == CS_XOR("NULL"))
				return;


			const char* weaponPrefix = CS_XOR("weapon_");
			const char* weaponNameStart = strstr(szWeaponName, weaponPrefix);
			const char* extractedWeaponName;

			if (!weaponNameStart)
				extractedWeaponName = szWeaponName;

			weaponNameStart += strlen(weaponPrefix);
			extractedWeaponName = weaponNameStart;

			if (const auto& weaponOverlayConfig = C_GET(TextOverlayVar_t, Vars.Weaponesp); weaponOverlayConfig.bEnable) {

				if (weaponOverlayConfig.bIcon) {
#ifdef _DEBUG
					context.AddComponent(new CTextComponent(true, false, SIDE_BOTTOM, DIR_BOTTOM, FONT::pEspWepName, CS_XOR(extractedWeaponName), Vars.Weaponesp, 1.f));
#else
					const char weaponIconsName = WeaponsIcons::get(szWeaponName);
					context.AddComponent(new CTextComponent(true, true, SIDE_BOTTOM, DIR_BOTTOM, FONT::pEspIcons, &weaponIconsName, Vars.Weaponesp, mAlpha[idx]));
#endif
				}
				else {
					context.AddComponent(new CTextComponent(true, false, SIDE_BOTTOM, DIR_BOTTOM, FONT::pEspWepName, CS_XOR(extractedWeaponName), Vars.Weaponesp, 1.f));
				}
			}

			if (const auto& ammo_config = C_GET(BarOverlayVar_t, Vars.AmmoBar); ammo_config.bEnable) {
				auto VData = ActiveWeapon->GetVData();
				if (VData) {
					const auto ammo = ActiveWeapon->clip1();
					const auto max_ammo = VData->m_iMaxClip1();

					if (max_ammo > 0) {
						const float factor = static_cast<float>(ammo) / max_ammo;
						// here we use the SIDE_BOTTOM which overrided max_ammo as the var used for factor limitation
						context.AddComponent(new CBarComponent(true, SIDE_BOTTOM, out, max_ammo, factor, Vars.AmmoBar, 1.f));
					}
				}
			}

		}
	}
	/* Start Player Flags*/
	{
		if (C_GET(unsigned int, Vars.pEspFlags) & FLAGS_ARMOR) {
			if (pPlayerPawn->GetArmorValue() > 0) {
				if (const auto& hk_cfg = C_GET(TextOverlayVar_t, Vars.HKFlag); hk_cfg.bEnable) {
					const char* szName = CS_XOR("K");

					if (player->m_bPawnHasHelmet())
						szName = CS_XOR("HK");

					context.AddComponent(new CTextComponent(true, false, SIDE_RIGHT, DIR_RIGHT, FONT::pEspFlagsName, szName, Vars.HKFlag, 1.f));
				}
			}
		}
		if (C_GET(unsigned int, Vars.pEspFlags) & FLAGS_DEFUSER) {
			if (player->m_bPawnHasDefuser() > 0) {
				if (const auto& kit_cfg = C_GET(TextOverlayVar_t, Vars.KitFlag); kit_cfg.bEnable) {
					const char* szName = CS_XOR("KIT");
					context.AddComponent(new CTextComponent(true, false, SIDE_RIGHT, DIR_BOTTOM, FONT::pEspFlagsName, szName, Vars.KitFlag, 1.f));
				}
			}
	
		}
	}


	context.Render(ImGui::GetBackgroundDrawList(), out);

}

