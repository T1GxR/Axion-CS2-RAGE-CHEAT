#pragma once
#include <stack>

#include "../../Utils/InputSys.hpp"

/* Prototypes */
struct MultiItem_t {
	std::string name;
	bool* value;
};

/* Context structs */
struct DropdownInfo_t {
	size_t HashedID;

	std::map<size_t, float> uScroll;

	std::vector<std::string> Elements;
	int* Option;

	int MaxItems;

	bool out_anim;
	bool DraggingScroll;

	float Size = 0.f;
	Vector2D Pos = { 0, 0 };
};

struct MultiDropdownInfo_t {
	size_t HashedID;

	std::map<size_t, float> uScroll;

	std::vector<MultiItem_t> Elements;

	int MaxItems;
	bool DraggingScroll;

	float Size = 0.f;
	Vector2D Pos = { 0, 0 };
};