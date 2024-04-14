#include "vector.h"

#include "matrix.h"
#include "qangle.h"

// used: m_rad2deg
#include "../../utilities/math.h"

[[nodiscard]] Vector_t Vector_t::Transform(const Matrix3x4_t& matTransform) const
{
	return {
		this->DotProduct(matTransform[0]) + matTransform[0][3],
		this->DotProduct(matTransform[1]) + matTransform[1][3],
		this->DotProduct(matTransform[2]) + matTransform[2][3]
	};
}
[[nodiscard]] QAngle_t Vector_t::ToEulerAngles(Vector_t* pseudoup /*= nullptr*/) {
	auto pitch = 0.0f;
	auto yaw = 0.0f;
	auto roll = 0.0f;

	auto length = this->ToVector2D().Length();

	if (pseudoup) {
		auto left = pseudoup->CrossProduct(*this);

		left.Normalizes();

		pitch = ToDegrees(std::atan2(-this->z, length));

		if (pitch < 0.0f)
			pitch += 360.0f;

		if (length > 0.001f) {
			yaw = ToDegrees(std::atan2(this->y, this->x));

			if (yaw < 0.0f)
				yaw += 360.0f;

			auto up_z = (this->x * left.y) - (this->y * left.x);

			roll = ToDegrees(std::atan2(left.z, up_z));

			if (roll < 0.0f)
				roll += 360.0f;
		}
		else {
			yaw = ToDegrees(std::atan2(-left.x, left.y));

			if (yaw < 0.0f)
				yaw += 360.0f;
		}
	}
	else {
		if (this->x == 0.0f && this->y == 0.0f) {
			if (this->z > 0.0f)
				pitch = 270.0f;
			else
				pitch = 90.0f;
		}
		else {
			pitch = ToDegrees(std::atan2(-this->z, length));

			if (pitch < 0.0f)
				pitch += 360.0f;

			yaw = ToDegrees(std::atan2(this->y, this->x));

			if (yaw < 0.0f)
				yaw += 360.0f;
		}
	}

	return { pitch, yaw, roll };
}

[[nodiscard]] QAngle_t Vector_t::ToAngles() const
{
	float flPitch, flYaw;
	if (this->x == 0.0f && this->y == 0.0f)
	{
		flPitch = (this->z > 0.0f) ? 270.f : 90.f;
		flYaw = 0.0f;
	}
	else
	{
		flPitch = M_RAD2DEG(std::atan2f(-this->z, this->Length2D()));

		if (flPitch < 0.f)
			flPitch += 360.f;

		flYaw = M_RAD2DEG(std::atan2f(this->y, this->x));

		if (flYaw < 0.f)
			flYaw += 360.f;
	}

	return { flPitch, flYaw, 0.0f };
}

[[nodiscard]] Matrix3x4_t Vector_t::ToMatrix() const
{
	Vector_t vecRight = {}, vecUp = {};
	this->ToDirections(&vecRight, &vecUp);

	Matrix3x4a_t matOutput = {};
	matOutput.SetForward(*this);
	matOutput.SetLeft(-vecRight);
	matOutput.SetUp(vecUp);
	return matOutput;
}