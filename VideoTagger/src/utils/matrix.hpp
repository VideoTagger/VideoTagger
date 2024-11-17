#pragma once

namespace vt::utils
{
	struct matrix
	{
		constexpr matrix(float value = 0.0f) : data{ value, value, value, value, value, value, value, value, value, value, value, value, value, value, value, value } {}
		float data[16];

		static inline constexpr float front[3] = { 0.0f, 0.0f, -1.0f };
		static inline constexpr float up[3] = { 0.0f, 1.0f, 0.0f };

		static constexpr matrix ortho(float left, float right, float bottom, float top, float near_z, float far_z)
		{
			matrix mat;

			mat[0] = 2.0f / (right - left);
			mat[5] = 2.0f / (top - bottom);
			mat[10] = -2.0f / (far_z - near_z);
			mat[15] = 1.0f;

			mat[12] = -(right + left) / (right - left);
			mat[13] = -(top + bottom) / (top - bottom);
			mat[14] = -(far_z + near_z) / (far_z - near_z);
			return mat;
		}

		static matrix look_at(float* eye, float* target, const float* up = up)
		{
			float z[3] = { eye[0] - target[0], eye[1] - target[1], eye[2] - target[2] };
			normalize(z);

			float x[3]{};
			cross_product(up, z, x);
			normalize(x);

			float y[3]{};
			cross_product(z, x, y);

			float look_at[16]
			{
				x[0], y[0], z[0], 0.0f,
				x[1], y[1], z[1], 0.0f,
				x[2], y[2], z[2], 0.0f,
				-dot(x, eye),
				-dot(y, eye),
				-dot(z, eye),
				1.0f
			};

			matrix result;
			std::memcpy(result.data, look_at, sizeof(float) * 16);
			return result;
		}

		static constexpr matrix inv(const matrix& m)
		{
			matrix inv;
			float det{};

			inv[0] = m[5] * m[10] * m[15] -
				m[5] * m[11] * m[14] -
				m[9] * m[6] * m[15] +
				m[9] * m[7] * m[14] +
				m[13] * m[6] * m[11] -
				m[13] * m[7] * m[10];

			inv[4] = -m[4] * m[10] * m[15] +
				m[4] * m[11] * m[14] +
				m[8] * m[6] * m[15] -
				m[8] * m[7] * m[14] -
				m[12] * m[6] * m[11] +
				m[12] * m[7] * m[10];

			inv[8] = m[4] * m[9] * m[15] -
				m[4] * m[11] * m[13] -
				m[8] * m[5] * m[15] +
				m[8] * m[7] * m[13] +
				m[12] * m[5] * m[11] -
				m[12] * m[7] * m[9];

			inv[12] = -m[4] * m[9] * m[14] +
				m[4] * m[10] * m[13] +
				m[8] * m[5] * m[14] -
				m[8] * m[6] * m[13] -
				m[12] * m[5] * m[10] +
				m[12] * m[6] * m[9];

			inv[1] = -m[1] * m[10] * m[15] +
				m[1] * m[11] * m[14] +
				m[9] * m[2] * m[15] -
				m[9] * m[3] * m[14] -
				m[13] * m[2] * m[11] +
				m[13] * m[3] * m[10];

			inv[5] = m[0] * m[10] * m[15] -
				m[0] * m[11] * m[14] -
				m[8] * m[2] * m[15] +
				m[8] * m[3] * m[14] +
				m[12] * m[2] * m[11] -
				m[12] * m[3] * m[10];

			inv[9] = -m[0] * m[9] * m[15] +
				m[0] * m[11] * m[13] +
				m[8] * m[1] * m[15] -
				m[8] * m[3] * m[13] -
				m[12] * m[1] * m[11] +
				m[12] * m[3] * m[9];

			inv[13] = m[0] * m[9] * m[14] -
				m[0] * m[10] * m[13] -
				m[8] * m[1] * m[14] +
				m[8] * m[2] * m[13] +
				m[12] * m[1] * m[10] -
				m[12] * m[2] * m[9];

			inv[2] = m[1] * m[6] * m[15] -
				m[1] * m[7] * m[14] -
				m[5] * m[2] * m[15] +
				m[5] * m[3] * m[14] +
				m[13] * m[2] * m[7] -
				m[13] * m[3] * m[6];

			inv[6] = -m[0] * m[6] * m[15] +
				m[0] * m[7] * m[14] +
				m[4] * m[2] * m[15] -
				m[4] * m[3] * m[14] -
				m[12] * m[2] * m[7] +
				m[12] * m[3] * m[6];

			inv[10] = m[0] * m[5] * m[15] -
				m[0] * m[7] * m[13] -
				m[4] * m[1] * m[15] +
				m[4] * m[3] * m[13] +
				m[12] * m[1] * m[7] -
				m[12] * m[3] * m[5];

			inv[14] = -m[0] * m[5] * m[14] +
				m[0] * m[6] * m[13] +
				m[4] * m[1] * m[14] -
				m[4] * m[2] * m[13] -
				m[12] * m[1] * m[6] +
				m[12] * m[2] * m[5];

			inv[3] = -m[1] * m[6] * m[11] +
				m[1] * m[7] * m[10] +
				m[5] * m[2] * m[11] -
				m[5] * m[3] * m[10] -
				m[9] * m[2] * m[7] +
				m[9] * m[3] * m[6];

			inv[7] = m[0] * m[6] * m[11] -
				m[0] * m[7] * m[10] -
				m[4] * m[2] * m[11] +
				m[4] * m[3] * m[10] +
				m[8] * m[2] * m[7] -
				m[8] * m[3] * m[6];

			inv[11] = -m[0] * m[5] * m[11] +
				m[0] * m[7] * m[9] +
				m[4] * m[1] * m[11] -
				m[4] * m[3] * m[9] -
				m[8] * m[1] * m[7] +
				m[8] * m[3] * m[5];

			inv[15] = m[0] * m[5] * m[10] -
				m[0] * m[6] * m[9] -
				m[4] * m[1] * m[10] +
				m[4] * m[2] * m[9] +
				m[8] * m[1] * m[6] -
				m[8] * m[2] * m[5];

			// Calculate the determinant
			det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

			// If the determinant is zero, the matrix is not invertible
			if (det == 0)
			{
				throw std::runtime_error("Matrix det == 0");
			}

			// Calculate the inverse by dividing by the determinant
			det = 1.0f / det;

			for (size_t i = 0; i < 16; ++i)
			{
				inv[i] = inv[i] * det;
			}
			return inv;
		}

		constexpr const float& operator[](size_t index) const
		{
			return data[index];
		}

		constexpr float& operator[](size_t index)
		{
			return data[index];
		}

	private:
		static constexpr float dot(float* x1, float* x2)
		{
			return x1[0] * x2[0] + x1[1] * x2[1] + x1[2] * x2[2];
		}

		static void normalize(float* v)
		{
			float il = 1.f / (std::sqrtf(dot(v, v)) + FLT_EPSILON);
			v[0] = v[0] * il;
			v[1] = v[1] * il;
			v[2] = v[2] * il;
		}

		static constexpr void cross_product(const float* a, const float* b, float* result)
		{
			result[0] = a[1] * b[2] - a[2] * b[1];
			result[1] = a[2] * b[0] - a[0] * b[2];
			result[2] = a[0] * b[1] - a[1] * b[0];
		}
	};
}
