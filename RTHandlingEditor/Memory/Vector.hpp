#pragma once
#include <cmath>

namespace rage
{
	class Vec2V
	{
	public:
		Vec2V() = default;

		Vec2V(float x, float y)
		:
			x(x), y(y)
		{}

		float x{};
		float y{};

		Vec2V operator + (Vec2V const& right)
		{
			return { this->x + right.x, this->y + right.y };
		}

		Vec2V operator - (Vec2V const& right)
		{
			return { this->x - right.x, this->y - right.y };
		}

		Vec2V operator * (float const& scale)
		{
			return { this->x * scale, this->y * scale };
		}

		Vec2V operator * (Vec2V& scale)
		{
			return { this->x * scale.x, this->y * scale.y };
		}

		operator bool()
		{
			return !IsZero();
		}

		bool IsZero() const
		{
			return (this->x == 0 && this->y == 0);
		}
	};

	class Vec3
	{
	public:
		Vec3() = default;

		Vec3(float x, float y, float z)
		:
			x(x), y(y), z(z)
		{}

		Vec3(Vec2V const& vec2)
		:
			x(vec2.x),
			y(vec2.y)
		{}

		float x{};
		float y{};
		float z{};

		Vec3 operator + (Vec3 const& right)
		{
			return { this->x + right.x, this->y + right.y, this->z + right.z };
		}

		Vec3 operator - (Vec3 const& right)
		{
			return { this->x - right.x, this->y - right.y, this->z - right.z };
		}

		Vec3 operator * (float const& scale)
		{
			return { this->x * scale, this->y * scale, this->z * scale };
		}

		Vec3 operator * (Vec3& scale)
		{
			return { this->x * scale.x, this->y * scale.y, this->z * scale.z };
		}

		operator bool()
		{
			return !IsZero();
		}

		float Magnitude()
		{
			return sqrtf(x * x + y * y + z * z);
		}

		Vec3 Normalize()
		{
			float mag = Magnitude();
			return  { x / mag, y / mag, z / mag };
		}

		float Distance(Vec3 const& right)
		{
			auto x = static_cast<float>(std::pow((this->x - right.x), 2));
			auto y = static_cast<float>(std::pow((this->y - right.y), 2));
			auto z = static_cast<float>(std::pow((this->z - right.z), 2));
			return std::sqrt(x + y + z);
		}

		Vec3 Cross(Vec3 const& right)
		{
			Vec3 result;
			result.x = this->y * right.z - this->z * right.y;
			result.y = this->z * right.x - this->x * right.z;
			result.z = this->x * right.y - this->y * right.x;
			return result;
		}

		bool IsZero() const
		{
			return (this->x == 0 && this->y == 0 && this->z == 0);
		}

		Vec2V Vec2()
		{
			return { x, y };
		}
	};

	// Vec3V has a padding at the end so that it is 128 bits and is so much faster thanks to SIMD/xmm intrinsics
	class Vec3V : public Vec3
	{
	public:
		Vec3V() = default;

		Vec3V(float x, float y, float z)
		:
			Vec3(x, y, z)
		{}

		Vec3V(Vec2V const& vec2)
		:
			Vec3(vec2)
		{}
	private:
		float m_Extra;
	};

	class Vec4V
	{
	public:
		Vec4V() = default;

		Vec4V(float x, float y, float z, float w)
		:
			x(x), y(y), z(z), w(w)
		{}
		Vec4V(Vec3V vec3)
		:
			x(vec3.x), y(vec3.y), z(vec3.z)
		{}
		Vec4V(Vec2V vec2)
		:
			x(vec2.x), y(vec2.y)
		{}

		float x{};
		float y{};
		float z{};
		float w{};

		Vec4V operator + (Vec4V const& right)
		{
			return { this->x + right.x, this->y + right.y, this->z + right.z, this->w + right.w };
		}

		Vec4V operator - (Vec4V const& right)
		{
			return { this->x - right.x, this->y - right.y, this->z - right.z, this->w - right.w };
		}

		Vec4V operator * (float const& scale)
		{
			return { this->x * scale, this->y * scale, this->z * scale, this->w * scale };
		}

		Vec4V operator * (Vec4V& scale)
		{
			return { this->x * scale.x, this->y * scale.y, this->z * scale.z, this->w * scale.w };
		}

		operator bool()
		{
			return !IsZero();
		}

		bool IsZero() const
		{
			return (this->x == 0 && this->y == 0);
		}

		Vec3V Vec3()
		{
			return { x, y ,z };
		}

		Vec2V FirstVec2()
		{
			return { x, y };
		}

		Vec2V SecondVec2()
		{
			return { z, w };
		}
	};
}