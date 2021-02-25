#pragma once

#include <array>

// angle indexes
enum
{
	PITCH,		// up / down
	YAW,		// left / right
	ROLL		// fall over
};

template <size_t size, typename T>
constexpr size_t lengthof(T (&)[size]) { return size; }

template<typename T, size_t C>
using array = std::array<T, C>;

template<typename T>
using array3_t = array<T, 3>;

template<typename T>
using array4_t = array<T, 4>;

template<typename T, size_t C>
struct vector_t : std::array<T, C>
{
	using self = vector_t<T, C>;
	using applyFunctor = T(*)(T);

	constexpr self Apply(applyFunctor func) const
	{
		self product;
		
		for (size_t i = 0; i < C; i++)
			product.at(i) = func(this->at(i));

		return product;
	}

	using applyFunctorRef = T(*)(const T &);

	constexpr self Apply(applyFunctorRef func) const
	{
		self product;
		
		for (size_t i = 0; i < C; i++)
			product.at(i) = func(this->at(i));

		return product;
	}

	[[nodiscard]] constexpr T Dot(const self &b) const
	{
		T product = 0;

		for (size_t i = 0; i < C; i++)
			product += this->at(i) * b.at(i);

		return product;
	}

	[[nodiscard]] constexpr self operator-(const self &b) const
	{
		self product;
		
		for (size_t i = 0; i < C; i++)
			product.at(i) = this->at(i) - b.at(i);

		return product;
	}
	
	self operator-=(const self &b)
	{
		return (*this = *this - b);
	}

	[[nodiscard]] constexpr self operator+(const self &b) const
	{
		self product;
		
		for (size_t i = 0; i < C; i++)
			product.at(i) = this->at(i) + b.at(i);

		return product;
	}
	
	self operator+=(const self &b)
	{
		return (*this = *this + b);
	}

	[[nodiscard]] constexpr self operator-() const
	{
		self product;
		
		for (size_t i = 0; i < C; i++)
			product.at(i) = -this->at(i);

		return product;
	}

	template<typename Ts>
	[[nodiscard]] constexpr self operator*(const Ts &scale) const
	{
		self product;
		
		for (size_t i = 0; i < C; i++)
			product.at(i) = this->at(i) * scale;

		return product;
	}
	
	template<typename Ts>
	self operator*=(const Ts &scale)
	{
		return (*this = *this * scale);
	}

	template<typename Ts>
	[[nodiscard]] constexpr self operator/(const Ts &divisor) const
	{
		self product;
		
		for (size_t i = 0; i < C; i++)
			product.at(i) = this->at(i) / divisor;

		return product;
	}
	
	template<typename Ts>
	self operator/=(const Ts &divisor)
	{
		return (*this = *this / divisor);
	}
	
	template<typename Ts>
	[[nodiscard]] constexpr bool Compare(const self &b, const Ts &epsilon) const
	{
		for (size_t i = 0; i < C; i++)
			if (abs(this->at(0) - b.at(0)) >= epsilon)
				return false;

		return true;
	}

	[[nodiscard]] constexpr T LengthSquared() const
	{
		return Dot(*this);
	}

	[[nodiscard]] constexpr T Length() const
	{
		return sqrt(LengthSquared());
	}

	[[nodiscard]] constexpr T DistanceSquared(const self &b) const
	{
		return (*this - b).LengthSquared();
	}

	[[nodiscard]] constexpr T Distance(const self &b) const
	{
		return (*this - b).Length();
	}

	constexpr T Normalize(self &o) const
	{
		auto length = Length();

		if (length)
			o = *this * (static_cast<T>(1.0) / length);
		else
			o.Clear();
		
		return length;
	}

	inline T Normalize()
	{
		return Normalize(*this);
	}

	[[nodiscard]] constexpr self Normalized() const
	{
		self product;
		Normalize(product);
		return product;
	}

	constexpr void Clear()
	{
		this->fill(0);
	}

	[[nodiscard]] constexpr self Cross(const self &b) const
	{
		return {
			this->at(1) * b.at(2) - this->at(2) * b.at(1),
			this->at(2) * b.at(0) - this->at(0) * b.at(2),
			this->at(0) * b.at(1) - this->at(1) * b.at(0)
		};
	}

	template<typename Tf, typename Tr, typename Tu>
	inline void AngleVectors(Tf forward, Tr right, Tu up) const
	{
		static_assert(std::is_null_pointer_v<Tf> || std::is_pointer_v<Tf>);
		static_assert(std::is_null_pointer_v<Tr> || std::is_pointer_v<Tr>);
		static_assert(std::is_null_pointer_v<Tu> || std::is_pointer_v<Tu>);

		T angle = this->at(YAW) * (M_PI*2 / 360);
		T sy = sin(angle);
		T cy = cos(angle);

		angle = this->at(PITCH) * (M_PI*2 / 360);
		T sp = sin(angle);
		T cp = cos(angle);
	
		if constexpr (!std::is_null_pointer_v<Tf>)
			*forward = { cp * cy, cp * sy, -sp };
	
		if constexpr (!std::is_null_pointer_v<Tr> || !std::is_null_pointer_v<Tu>)
		{
			angle = this->at(ROLL) * (M_PI*2 / 360);
			T sr = sin(angle);
			T cr = cos(angle);
		
			if constexpr (!std::is_null_pointer_v<Tr>)
				*right = { (-1 * sr * sp * cy + -1 * cr * -sy), (-1 * sr * sp * sy + -1 * cr * cy), -1 * sr * cp };
		
			if constexpr (!std::is_null_pointer_v<Tu>)
				*up = { (cr * sp * cy + -sr * -sy), (cr * sp * sy + -sr * cy), cr * cp };
		}
	}

	inline std::array<self, 3> AngleVectors() const
	{
		std::array<self, 3> angles;
		AngleVectors(&angles[0], &angles[1], &angles[2]);
		return angles;
	}

	[[nodiscard]] inline self Forward() const
	{
		self forward;
		AngleVectors(&forward, nullptr, nullptr);
		return forward;
	}

	[[nodiscard]] inline self Right() const
	{
		self right;
		AngleVectors(nullptr, &right, nullptr);
		return right;
	}

	[[nodiscard]] inline self Up() const
	{
		self up;
		AngleVectors(nullptr, nullptr, &up);
		return up;
	}

	[[nodiscard]] inline self ToAngles() const
	{
		T yaw, pitch;
	
		if (this->at(YAW) == 0 && this->at(PITCH) == 0)
		{
			yaw = 0;
			if (this->at(ROLL) > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
		// PMM - fixed to correct for pitch of 0
			if (this->at(PITCH))
				yaw = atan2(this->at(YAW), this->at(PITCH)) * 180 / M_PI;
			else if (this->at(YAW) > 0)
				yaw = 90;
			else
				yaw = 270;

			if (yaw < 0)
				yaw += 360;

			T forward = sqrt(this->at(PITCH) * this->at(PITCH) + this->at(YAW) * this->at(YAW));
			pitch = atan2(this->at(ROLL), forward) * 180 / M_PI;
			if (pitch < 0)
				pitch += 360;
		}

		return { -pitch, yaw, 0 };
	}

	[[nodiscard]] inline T ToYaw() const
	{
		return ToAngles().at(YAW);
	}
};

template<typename T>
using vector3_t = vector_t<T, 3>;

template<typename T>
using vector4_t = vector_t<T, 4>;

using vec_t = float;
using vec3_t = vector3_t<vec_t>;
using vec4_t = vector4_t<vec_t>;

template<typename T>
constexpr T pi = static_cast<T>(3.141592653589793238462643383279502884e+00);

#ifndef M_PI
	constexpr vec_t M_PI = pi<float>;
#endif

constexpr vec3_t vec3_origin { 0, 0, 0 };

template<typename T>
inline void ClearBounds (array3_t<T> &mins, array3_t<T> &maxs)
{
	mins[0] = mins[1] = mins[2] = std::numeric_limits<T>::max();
	maxs[0] = maxs[1] = maxs[2] = std::numeric_limits<T>::min();
}

template<typename Tv, typename Tm>
inline void AddPointToBounds (const array3_t<Tv> &v, array3_t<Tm> &mins, array3_t<Tm> &maxs)
{
	for (size_t i = 0; i < std::tuple_size_v<array3_t<Tv>>; i++)
	{
		const Tv &val = v[i];

		if (val < mins[i])
			mins[i] = val;
		
		if (val > maxs[i])
			maxs[i] = val;
	}
}

template<typename T>
constexpr T anglemod(const T &x) { return (360.0f / 65536) * (static_cast<int32_t>(x * (65536 / 360.0f)) & 65535); }

template<typename T>
constexpr T min(const T &a, const T &b)
{
	return (a < b) ? a : b;
}

template<typename T>
constexpr T max(const T &a, const T &b)
{
	return (a > b) ? a : b;
}

template<typename T>
constexpr T clamp(const T &value, const T &min, const T &max)
{
	return (value < min) ? min : (value > max) ? max : value;
}