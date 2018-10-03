#pragma once

#include "vMathLibrary.hpp"

class vPoint
{
	private:

		float x, y, z;

	public:

		vPoint():x(0.0f), y(0.0f), z(0.0f){}
		vPoint(float x, float y, float z):x(x), y(y), z(z){}
		vPoint(float *point):x(point[0]), y(point[1]), z(point[2]){}

		bool isEqual(vPoint p)
		{
			if (x == p.x && y == p.y && z == p.z)
				return true;

			return false;
		}

		void setPosition(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }
		void setPosition(float *point) { x = point[0]; y = point[1]; z = point[2]; }

		float getX() { return x; }
		float getY() { return y; }
		float getZ() { return z; }

		void operator+=(vPoint p)
		{
			x += p.x;
			y += p.y;
			z += p.z;
		}

		void operator-=(vPoint p)
		{
			x -= p.x;
			y -= p.y;
			z -= p.z;
		}

		void operator*=(vPoint p)
		{
			x *= p.x;
			y *= p.y;
			z *= p.z;
		}

		void operator/=(vPoint p)
		{
			x /= p.x;
			y /= p.y;
			z /= p.z;
		}
};
