#pragma once

#include "vPoint.hpp"

class vVector
{
	private:

		float x, y, z;

	public:

		vVector() :x(0.0f), y(0.0f), z(0.0f) {}
		vVector(float x, float y, float z) :x(x), y(y), z(z) {}
		vVector(float *point) :x(point[0]), y(point[1]), z(point[2]) {}
		vVector(vPoint v) :x(v.getX()), y(v.getY()), z(v.getZ()) {}
		vVector(vPoint p1, vPoint p2) { x = p2.getX() - p1.getX(); y = p2.getY() - p1.getY(); z = p2.getZ() - p1.getZ(); }

		void setPosition(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }
		void setPosition(float *point) { x = point[0]; y = point[1]; z = point[2]; }
		void setPosition(vPoint v) { x = v.getX(); y = v.getY(); z = v.getZ(); }

		float getX() { return x; }
		float getY() { return y; }
		float getZ() { return z; }

		//STATICS

		static bool isEqual(vVector v1, vVector v2)
		{
			if (v1.getX() == v2.getX() && v1.getY() == v2.getY() && v1.getZ() == v2.getZ())
				return true;

			return false;
		}

		static vVector normalize(vVector v1)
		{
			float vl = sqrt((vQuadrate(v1.getX()) + vQuadrate(v1.getY()) + vQuadrate(v1.getZ())));

			v1 /= vl;

			return v1;
		}

		static vVector vCross(vVector v1, vVector v2)
		{
			vVector v3;

			v3.setPosition((v1.getY() * v2.getZ() - v1.getZ() * v2.getY()), (v1.getX() * v2.getZ() - v1.getZ() * v2.getX()), (v1.getX() * v2.getY() - v1.getY() * v2.getX()));

			return v3;
		}

		static vVector vCross(vVector * v1, vVector * v2)
		{
			vVector v3;

			v3.setPosition((v1->getY() * v2->getZ() - v1->getZ() * v2->getY()), (v1->getX() * v2->getZ() - v1->getZ() * v2->getX()), (v1->getX() * v2->getY() - v1->getY() * v2->getX()));

			return v3;
		}

		static float vDot(vVector v1, vVector v2)
		{
			return  (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
		}

		static float vDot(vVector v1, vPoint v2)
		{
			return  (v1.x * v2.getX() + v1.y * v2.getY() + v1.z * v2.getZ());
		}

		static bool vValidate(vVector v1)
		{
			if (v1.x == 0.0f && v1.y == 0.0f && v1.z == 0.0f)
			{
				return false;
			}

			return true;
		}

		void operator+=(vVector p)
		{
			x += p.x;
			y += p.y;
			z += p.z;
		}

		void operator-=(vVector p)
		{
			x -= p.x;
			y -= p.y;
			z -= p.z;
		}

		void operator*=(vVector p)
		{
			x *= p.x;
			y *= p.y;
			z *= p.z;
		}

		void operator/=(vVector p)
		{
			x /= p.x;
			y /= p.y;
			z /= p.z;
		}

		void operator/=(float f)
		{
			x /= f;
			y /= f;
			z /= f;
		}
};
