#include "math.h"

//used: getexportaddr
#include "memory.h"
#include "../core/sdk.h"
#include <imgui/imgui_internal.h>
bool MATH::Setup()
{
	bool bSuccess = true;

	const void* hTier0Lib = MEM::GetModuleBaseHandle(TIER0_DLL);
	if (hTier0Lib == nullptr)
		return false;

	fnRandomSeed = reinterpret_cast<decltype(fnRandomSeed)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomSeed")));
	bSuccess &= (fnRandomSeed != nullptr);

	fnRandomFloat = reinterpret_cast<decltype(fnRandomFloat)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomFloat")));
	bSuccess &= (fnRandomFloat != nullptr);

	fnRandomFloatExp = reinterpret_cast<decltype(fnRandomFloatExp)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomFloatExp")));
	bSuccess &= (fnRandomFloatExp != nullptr);

	fnRandomInt = reinterpret_cast<decltype(fnRandomInt)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomInt")));
	bSuccess &= (fnRandomInt != nullptr);

	fnRandomGaussianFloat = reinterpret_cast<decltype(fnRandomGaussianFloat)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomGaussianFloat")));
	bSuccess &= (fnRandomGaussianFloat != nullptr);

	return bSuccess;
}

// distance between to line segments
float MATH::segment_dist(Vector_t start1, Vector_t end1, Vector_t start2, Vector_t end2) noexcept {
    Vector_t u = end1 - start1;
    Vector_t v = end2 - start2;
    Vector_t w = start1 - start2;
    float a = u.DotProduct(u);
    float b = u.DotProduct(v);
    float c = v.DotProduct(v);
    float d = u.DotProduct(w);
    float e = v.DotProduct(w);
    float D = a * c - b * b;
    float sc, sN, sD = D;
    float tc, tN, tD = D;

    if (D < 0.001f) {
        sN = 0.0f;
        sD = 1.0f;
        tN = e;
        tD = c;
    }
    else {
        sN = (b * e - c * d);
        tN = (a * e - b * d);
        if (sN < 0.0f) {
            sN = 0.0f;
            tN = e;
            tD = c;
        }
        else if (sN > sD) {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0f) {
        tN = 0.0f;

        if (-d < 0.0f) {
            sN = 0.0f;
        }
        else if (-d > a) {
            sN = sD;
        }
        else {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD) {
        tN = tD;

        if ((-d + b) < 0.0f) {
            sN = 0;
        }
        else if ((-d + b) > a) {
            sN = sD;
        }
        else {
            sN = (-d + b);
            sD = a;
        }
    }

    sc = (std::abs(sN) < 0.001f ? 0.0f : sN / sD);
    tc = (std::abs(tN) < 0.001f ? 0.0f : tN / tD);

    Vector_t dP = w + (u * sc) - (v * tc);
    return dP.Length();
}
void MATH::TransformAABB(const Matrix3x4a_t& transform, const Vector_t& minsIn, const Vector_t& maxsIn, Vector_t& minsOut, Vector_t& maxsOut) {
	const Vector_t localCenter = (minsIn + maxsIn) * 0.5f;
	const Vector_t& localExtent = maxsIn - localCenter;

	const auto& mat = transform.arrData;
	const Vector_t worldAxisX{ mat[0][0], mat[0][1], mat[0][2] };
	const Vector_t worldAxisY{ mat[1][0], mat[1][1], mat[1][2] };
	const Vector_t worldAxisZ{ mat[2][0], mat[2][1], mat[2][2] };

	const Vector_t worldCenter = localCenter.Transform(transform);
	const Vector_t worldExtent{
		localExtent.DotProduct(worldAxisX),
		localExtent.DotProduct(worldAxisY),
		localExtent.DotProduct(worldAxisZ),
	};

	minsOut = worldCenter - worldExtent;
	maxsOut = worldCenter + worldExtent;
}
#include <numbers>

void MATH::angle_vector(const QAngle_t& angles, Vector_t& forward) noexcept {
	const float x = angles.x * std::numbers::pi_v<float> / 180.f;
	const float y = angles.y * std::numbers::pi_v<float> / 180.f;
	const float sp = std::sin(x);
	const float cp = std::cos(x);
	const float sy = std::sin(y);
	const float cy = std::cos(y);
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

Vector_t MATH::angle_vector(const QAngle_t& angles) noexcept {
	Vector_t forward;
	angle_vector(angles, forward);
	return forward;
}

void  MATH::anglevectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up )
{
	float cp = std::cos(M_DEG2RAD(angles.x)), sp = std::sin(M_DEG2RAD(angles.x));
	float cy = std::cos(M_DEG2RAD(angles.y)), sy = std::sin(M_DEG2RAD(angles.y));
	float cr = std::cos(M_DEG2RAD(angles.z)), sr = std::sin(M_DEG2RAD(angles.z));

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right) {
		right->x = -1.f * sr * sp * cy + -1.f * cr * -sy;
		right->y = -1.f * sr * sp * sy + -1.f * cr * cy;
		right->z = -1.f * sr * cp;
	}

	if (up) {
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}

void MATH::VectorAngless(const Vector_t& forward, QAngle_t& angles, Vector_t* up) {
	Vector_t  left;
	float   len, up_z, pitch, yaw, roll;

	// get 2d length.
	len = forward.Length2D();

	if (up && len > 0.001f) {
		pitch = M_RAD2DEG(std::atan2(-forward.z, len));
		yaw = M_RAD2DEG(std::atan2(forward.y, forward.x));

		// get left direction vector using cross product.
		left = (*up).CrossProduct(forward).Normalized();

		// calculate up_z.
		up_z = (left.y * forward.x) - (left.x * forward.y);

		// calculate roll.
		roll = M_RAD2DEG(std::atan2(left.z, up_z));
	}

	else {
		if (len > 0.f) {
			// calculate pitch and yaw.
			pitch = M_RAD2DEG(std::atan2(-forward.z, len));
			yaw = M_RAD2DEG(std::atan2(forward.y, forward.x));
			roll = 0.f;
		}

		else {
			pitch = (forward.z > 0.f) ? -90.f : 90.f;
			yaw = 0.f;
			roll = 0.f;
		}
	}

	// set out angles.
	angles = { pitch, yaw, roll };
}

bool MATH::WorldToScreen(const Vector_t& in, ImVec2& out) {
	if (!ImGui::GetCurrentContext()) return false;

	auto z = SDK::ViewMatrix[3][0] * in.x + SDK::ViewMatrix[3][1] * in.y +
		SDK::ViewMatrix[3][2] * in.z + SDK::ViewMatrix[3][3];

	if (z < 0.001f)
		return false;

	out = { (ImGui::GetIO().DisplaySize.x * 0.5f), (ImGui::GetIO().DisplaySize.y * 0.5f) };
	out.x *= 1.0f + (SDK::ViewMatrix[0][0] * in.x + SDK::ViewMatrix[0][1] * in.y +
		SDK::ViewMatrix[0][2] * in.z + SDK::ViewMatrix[0][3]) /
		z;
	out.y *= 1.0f - (SDK::ViewMatrix[1][0] * in.x + SDK::ViewMatrix[1][1] * in.y +
		SDK::ViewMatrix[1][2] * in.z + SDK::ViewMatrix[1][3]) /
		z;

	out = { out.x, out.y };
	return true;
}
