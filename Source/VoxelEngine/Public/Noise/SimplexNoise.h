#pragma once

// https://github.com/SRombauts/SimplexNoise/blob/master/src/SimplexNoise.cpp

#include <array>

namespace VoxelEngine::Noise
{                                                                                                      
	class VOXELENGINE_API SimplexNoise
	{
	private:
		static constexpr inline int32 FastFloor(float Value) 
		{
			int32 I = static_cast<int32>(Value);
			return (Value < I) ? (I - 1) : (I);
		}

		static constexpr std::array<uint8, 256> Perm
		{
			151, 160, 137, 91, 90, 15,
			131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
			190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
			88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
			77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
			102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
			135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
			5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
			223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
			129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
			251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
			49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
			138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
		};

		static constexpr inline uint8 Hash(int32_t I) 
		{
			return Perm[I & 0xff];
		}

		static constexpr float Grad(int32 Hash, float X)
		{
			const int32_t HashValue = Hash & 0x0F;  // Convert low 4 bits of Hash code
			float GradValue = 1.0f + (HashValue & 7);    // Gradient value 1.0, 2.0, ..., 8.0
			if ((HashValue & 8) != 0) GradValue = -GradValue; // Set a random sign for the gradient
			//  float grad = gradients1D[H];    // NOTE : Test of Gradient look-up table instead of the above
			return (GradValue * X);              // Multiply the gradient with the distance
		}

		static constexpr float Grad(int32_t Hash, float X, float Y)
		{
			const int32_t H = Hash & 0x3F;  // Convert low 3 bits of Hash code
			const float U = H < 4 ? X : Y;  // into 8 simple gradient directions,
			const float V = H < 4 ? Y : X;
			return ((H & 1) ? -U : U) + ((H & 2) ? -2.0f * V : 2.0f * V); // and compute the dot product with (X,Y).
		}

		static constexpr float Grad(int32_t Hash, float X, float Y, float Z)
		{
			int H = Hash & 15;     // Convert low 4 bits of Hash code into 12 simple
			float U = H < 8 ? X : Y; // gradient directions, and compute dot product.
			float V = H < 4 ? Y : H == 12 || H == 14 ? X : Z; // Fix repeats at H = 12 to 15
			return ((H & 1) ? -U : U) + ((H & 2) ? -V : V);
		}

	public:

		static constexpr float Noise(float X)
		{
			float N0, N1;   // Noise contributions from the two "corners"

			// No need to skew the input space in 1D

			// Corners coordinates (nearest integer values):
			int32_t I0 = FastFloor(X);
			int32_t I1 = I0 + 1;
			// Distances to corners (between 0 and 1):
			float X0 = X - I0;
			float X1 = X0 - 1.0f;

			// Calculate the contribution from the first corner
			float T0 = 1.0f - X0 * X0;
			//  if(T0 < 0.0f) T0 = 0.0f; // not possible
			T0 *= T0;
			N0 = T0 * T0 * Grad(Hash(I0), X0);

			// Calculate the contribution from the second corner
			float T1 = 1.0f - X1 * X1;
			//  if(T1 < 0.0f) T1 = 0.0f; // not possible
			T1 *= T1;
			N1 = T1 * T1 * Grad(Hash(I1), X1);

			// The maximum value of this noise is 8*(3/4)^4 = 2.53125
			// A factor of 0.395 scales to fit exactly within [-1,1]
			return 0.395f * (N0 + N1);
		}

		static constexpr float Noise(float X, float Y)
		{
			float N0, N1, N2;   // Noise contributions from the three corners

			// Skewing/Unskewing factors for 2D
			constexpr float F2 = 0.366025403f;  // F2 = (sqrt(3) - 1) / 2
			constexpr float G2 = 0.211324865f;  // G2 = (3 - sqrt(3)) / 6   = F2 / (1 + 2 * K)

			// Skew the input space to determine which simplex cell we're in
			const float S = (X + Y) * F2;  // Hairy factor for 2D
			const float Xs = X + S;
			const float Ys = Y + S;
			const int32_t I = FastFloor(Xs);
			const int32_t J = FastFloor(Ys);

			// Unskew the cell origin back to (x,y) space
			const float T = static_cast<float>(I + J) * G2;
			const float X0 = I - T;
			const float Y0 = J - T;
			const float XShift0 = X - X0;  // The x,y distances from the cell origin
			const float YShift0 = Y - Y0;

			// For the 2D case, the simplex shape is an equilateral triangle.
			// Determine which simplex we are in.
			int32_t I1, J1;  // Offsets for second (middle) corner of simplex in (I,J) coords
			if (XShift0 > YShift0) 
			{   // lower triangle, XY order: (0,0)->(1,0)->(1,1)
				I1 = 1;
				J1 = 0;
			}
			else {   // upper triangle, YX order: (0,0)->(0,1)->(1,1)
				I1 = 0;
				J1 = 1;
			}

			// A step of (1,0) in (I,J) means a step of (1-c,-c) in (x,y), and
			// a step of (0,1) in (I,J) means a step of (-c,1-c) in (x,y), where
			// c = (3-sqrt(3))/6

			const float X1 = XShift0 - I1 + G2;            // Offsets for middle corner in (x,y) unskewed coords
			const float Y1 = YShift0 - J1 + G2;
			const float X2 = XShift0 - 1.0f + 2.0f * G2;   // Offsets for last corner in (x,y) unskewed coords
			const float Y2 = YShift0 - 1.0f + 2.0f * G2;

			// Work out the hashed gradient indices of the three simplex corners
			const int Gi0 = Hash(I + Hash(J));
			const int Gi1 = Hash(I + I1 + Hash(J + J1));
			const int Gi2 = Hash(I + 1 + Hash(J + 1));

			// Calculate the contribution from the first corner
			float T0 = 0.5f - XShift0 * XShift0 - YShift0 * YShift0;
			if (T0 < 0.0f) {
				N0 = 0.0f;
			}
			else {
				T0 *= T0;
				N0 = T0 * T0 * Grad(Gi0, XShift0, YShift0);
			}

			// Calculate the contribution from the second corner
			float T1 = 0.5f - X1 * X1 - Y1 * Y1;
			if (T1 < 0.0f) {
				N1 = 0.0f;
			}
			else {
				T1 *= T1;
				N1 = T1 * T1 * Grad(Gi1, X1, Y1);
			}

			// Calculate the contribution from the third corner
			float T2 = 0.5f - X2 * X2 - Y2 * Y2;
			if (T2 < 0.0f)
			{
				N2 = 0.0f;
			}
			else {
				T2 *= T2;
				N2 = T2 * T2 * Grad(Gi2, X2, Y2);
			}

			// Add contributions from each corner to get the final noise value.
			// The result is scaled to return values in the interval [-1,1].
			return 45.23065f * (N0 + N1 + N2);
		}

		static constexpr float Noise(float X, float Y, float Z)
		{
			float N0, N1, N2, N3; // Noise contributions from the four corners

			// Skewing/Unskewing factors for 3D
			constexpr float F3 = 1.0f / 3.0f;
			constexpr float G3 = 1.0f / 6.0f;

			// Skew the input space to determine which simplex cell we're in
			float S = (X + Y + Z) * F3; // Very nice and simple skew factor for 3D
			int I = FastFloor(X + S);
			int J = FastFloor(Y + S);
			int K = FastFloor(Z + S);
			float T = (I + J + K) * G3;
			float X0 = I - T; // Unskew the cell origin back to (x,y,z) space
			float Y0 = J - T;
			float Z0 = K - T;
			float XShift0 = X - X0; // The x,y,z distances from the cell origin
			float YShift0 = Y - Y0;
			float ZShift0 = Z - Z0;

			// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
			// Determine which simplex we are in.
			int I1, J1, K1; // Offsets for second corner of simplex in (I,J,K) coords
			int I2, J2, K2; // Offsets for third corner of simplex in (I,J,K) coords
			if (XShift0 >= YShift0) {
				if (YShift0 >= ZShift0) {
					I1 = 1; J1 = 0; K1 = 0; I2 = 1; J2 = 1; K2 = 0; // X Y Z order
				}
				else if (XShift0 >= ZShift0) {
					I1 = 1; J1 = 0; K1 = 0; I2 = 1; J2 = 0; K2 = 1; // X Z Y order
				}
				else {
					I1 = 0; J1 = 0; K1 = 1; I2 = 1; J2 = 0; K2 = 1; // Z X Y order
				}
			}
			else { // XShift0<YShift0
				if (YShift0 < ZShift0) {
					I1 = 0; J1 = 0; K1 = 1; I2 = 0; J2 = 1; K2 = 1; // Z Y X order
				}
				else if (XShift0 < ZShift0) {
					I1 = 0; J1 = 1; K1 = 0; I2 = 0; J2 = 1; K2 = 1; // Y Z X order
				}
				else {
					I1 = 0; J1 = 1; K1 = 0; I2 = 1; J2 = 1; K2 = 0; // Y X Z order
				}
			}

			// A step of (1,0,0) in (I,J,K) means a step of (1-c,-c,-c) in (x,y,z),
			// a step of (0,1,0) in (I,J,K) means a step of (-c,1-c,-c) in (x,y,z), and
			// a step of (0,0,1) in (I,J,K) means a step of (-c,-c,1-c) in (x,y,z), where
			// c = 1/6.
			float X1 = XShift0 - I1 + G3; // Offsets for second corner in (x,y,z) coords
			float Y1 = YShift0 - J1 + G3;
			float Z1 = ZShift0 - K1 + G3;
			float X2 = XShift0 - I2 + 2.0f * G3; // Offsets for third corner in (x,y,z) coords
			float Y2 = YShift0 - J2 + 2.0f * G3;
			float Z2 = ZShift0 - K2 + 2.0f * G3;
			float X3 = XShift0 - 1.0f + 3.0f * G3; // Offsets for last corner in (x,y,z) coords
			float Y3 = YShift0 - 1.0f + 3.0f * G3;
			float Z3 = ZShift0 - 1.0f + 3.0f * G3;

			// Work out the hashed gradient indices of the four simplex corners
			int Gi0 = Hash(I + Hash(J + Hash(K)));
			int Gi1 = Hash(I + I1 + Hash(J + J1 + Hash(K + K1)));
			int Gi2 = Hash(I + I2 + Hash(J + J2 + Hash(K + K2)));
			int Gi3 = Hash(I + 1 + Hash(J + 1 + Hash(K + 1)));

			// Calculate the contribution from the four corners
			float T0 = 0.6f - XShift0 * XShift0 - YShift0 * YShift0 - ZShift0 * ZShift0;
			if (T0 < 0) {
				N0 = 0.0;
			}
			else {
				T0 *= T0;
				N0 = T0 * T0 * Grad(Gi0, XShift0, YShift0, ZShift0);
			}
			float T1 = 0.6f - X1 * X1 - Y1 * Y1 - Z1 * Z1;
			if (T1 < 0) {
				N1 = 0.0;
			}
			else {
				T1 *= T1;
				N1 = T1 * T1 * Grad(Gi1, X1, Y1, Z1);
			}
			float T2 = 0.6f - X2 * X2 - Y2 * Y2 - Z2 * Z2;
			if (T2 < 0) {
				N2 = 0.0;
			}
			else {
				T2 *= T2;
				N2 = T2 * T2 * Grad(Gi2, X2, Y2, Z2);
			}
			float T3 = 0.6f - X3 * X3 - Y3 * Y3 - Z3 * Z3;
			if (T3 < 0) {
				N3 = 0.0;
			}
			else {
				T3 *= T3;
				N3 = T3 * T3 * Grad(Gi3, X3, Y3, Z3);
			}
			// Add contributions from each corner to get the final noise value.
			// The result is scaled to stay just inside [-1,1]
			return 32.0f * (N0 + N1 + N2 + N3);
		}

		constexpr static float Fractal(float Frequency, float Amplitude, float Lacunarity, float Persistence, uint64 Octaves, float X)
		{
			float Output = 0.f;
			float Denom = 0.f;
			float FreqTemp = Frequency;
			float AmplitudeTemp = Amplitude;

			for (size_t I = 0; I < Octaves; I++) 
			{
				Output += (AmplitudeTemp * Noise(X * FreqTemp));
				Denom += AmplitudeTemp;

				FreqTemp *= Lacunarity;
				AmplitudeTemp *= Persistence;
			}

			return (Output / Denom);
		}

		constexpr static float Fractal(float Frequency, float Amplitude, float Lacunarity, float Persistence, uint64 Octaves, float X, float Y)
		{
			float Output = 0.f;
			float Denom = 0.f;
			float FrequencyTemp = Frequency;
			float AmplitudeTemp = Amplitude;

			for (size_t I = 0; I < Octaves; I++) {
				Output += (AmplitudeTemp * Noise(X * FrequencyTemp, Y * FrequencyTemp));
				Denom += AmplitudeTemp;

				FrequencyTemp *= Lacunarity;
				AmplitudeTemp *= Persistence;
			}

			return (Output / Denom);
		}

		constexpr static float Fractal(float Frequency, float Amplitude, float Lacunarity, float Persistence, size_t Octaves, float X, float Y, float Z)
		{
			float Output = 0.f;
			float Denom = 0.f;
			float FrequencyTemp = Frequency;
			float AmplitudeTemp = Amplitude;

			for (size_t I = 0; I < Octaves; I++) {
				Output += (AmplitudeTemp * Noise(X * FrequencyTemp, Y * FrequencyTemp, Z * FrequencyTemp));
				Denom += AmplitudeTemp;

				FrequencyTemp *= Lacunarity;
				AmplitudeTemp *= Persistence;
			}

			return (Output / Denom);
		}
	};

}