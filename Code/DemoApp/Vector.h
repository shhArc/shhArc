/////////////////////////////////////////////////////////////////////////////////
// Copright 2024 David K Bhowmik
// This file is part of shhArc.
//
// shhArc is free software: you can redistribute it and/or modify
// it under the terms of the Lesser GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// shhArc is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the Lesser GNU General Public License
// along with shhArc.  If not, see <http://www.gnu.org/licenses/>.
//
// You may alternatively use this source under the terms of a specific 
// version of the shhArc Unrestricted License provided you have obtained 
// such a license from the copyright holder.
/////////////////////////////////////////////////////////////////////////////////

#ifndef VECTOR_H
#define VECTOR_H

#include "../Config/MemoryDefines.h"
#include "../Common/Debug.h"


namespace shh {
	class  Vector : public GCObject
	{
		DECLARE_MEMORY_MANAGED(Vector);


	public:
		float myX, myY, myZ;

		inline Vector();
		inline Vector(float x, float y, float z, bool normalise = false);
		inline Vector(const Vector& other);
		~Vector() {}

		inline void Set(float x, float y, float z);

		inline const Vector& operator =(const Vector& other);
		inline friend bool operator ==(const Vector& a, const Vector& b);
		inline friend bool operator !=(const Vector& a, const Vector& b);
		inline bool Equals(Vector other, float tolerance = 0.01f) const;
		inline Vector operator - () const;
		inline friend Vector operator +(Vector a, const Vector& b);
		inline const Vector& operator +=(const Vector& other);
		inline const Vector& Add(const Vector& a, const Vector& b);
		inline const Vector& operator -=(const Vector& other);
		inline const Vector& Subtract(const Vector& a, const Vector& b);

		inline const Vector& operator *=(const float s);
		inline const Vector& operator /=(const float s);
		inline const Vector& Scale(const float s);

		inline float GetLength() const;
		inline float GetLengthSquared() const;
		inline float Normalise();
		inline float Dot(const Vector& other) const;
		inline const Vector& Cross(const Vector& a, const Vector& b);
		inline const Vector& Inverse();
		inline const Vector& Interpolate(const Vector& a, const Vector& b, const float t);

		inline bool operator<(const Vector& other) const;
		inline float const& operator[](int i) const;
		inline float& operator[](int i);
	};


	// --------------------------------------------------------------------------						
	// Function:	Vector
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Vector::Vector() :
		myX(0.0),
		myY(0.0),
		myZ(0.0)
	{}


	// --------------------------------------------------------------------------						
	// Function:	Vector
	// Description:	constructor
	// Arguments:	x, y, z, whether to normalize
	// Returns:		none
	// --------------------------------------------------------------------------
	Vector::Vector(const float x, const float y, const float z, bool normalise) :
		myX(x),
		myY(y),
		myZ(z)
	{
		if (normalise)
			Normalise();
	}


	// --------------------------------------------------------------------------						
	// Function:	Vector
	// Description:	constructor
	// Arguments:	vecotr to copy
	// Returns:		none
	// --------------------------------------------------------------------------
	Vector::Vector(const Vector& other) :
		myX(other.myX), myY(other.myY), myZ(other.myZ)
	{}


	// --------------------------------------------------------------------------						
	// Function:	Set
	// Description:	sets x, y, z
	// Arguments:	x, y, z
	// Returns:		none
	// --------------------------------------------------------------------------
	void Vector::Set(const float x, const float y, const float z)
	{
		myX = x;
		myY = y;
		myZ = z;
	}


	// --------------------------------------------------------------------------						
	// Function:	=
	// Description:	assignment
	// Arguments:	other
	// Returns:		this
	// --------------------------------------------------------------------------
	const Vector& Vector::operator =(const Vector& other)
	{
		myX = other.myX;
		myY = other.myY;
		myZ = other.myZ;

		return *this;
	}



	// --------------------------------------------------------------------------						
	// Function:	==
	// Description:	equality test
	// Arguments:	vector a, vector b
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool operator ==(const Vector& a, const Vector& b)
	{
		return (
			a.myX == b.myX &&
			a.myY == b.myY &&
			a.myZ == b.myZ);
	}


	// --------------------------------------------------------------------------						
	// Function:	!=
	// Description:	inequality test
	// Arguments:	vector a, vector b
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool operator !=(const Vector& a, const Vector& b)
	{
		return !(a == b);
	}


	// --------------------------------------------------------------------------						
	// Function:	Equals
	// Description:	equality test
	// Arguments:	other, tolerance (the amount of variation allowed for them
	//				still to be considered equal)
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Vector::Equals(Vector other, float tolerance) const
	{
		other -= *this;
		return (
			other.myX >= -tolerance && other.myX <= tolerance &&
			other.myY >= -tolerance && other.myY <= tolerance &&
			other.myZ >= -tolerance && other.myZ <= tolerance);
	}


	// --------------------------------------------------------------------------						
	// Function:	+=
	// Description:	add
	// Arguments:	other vector
	// Returns:		this
	// --------------------------------------------------------------------------
	const Vector& Vector::operator +=(const Vector& other)
	{
		myX += other.myX;
		myY += other.myY;
		myZ += other.myZ;

		return *this;
	}



	// --------------------------------------------------------------------------						
	// Function:	+=
	// Description:	add
	// Arguments:	vector, vector b
	// Returns:		this
	// --------------------------------------------------------------------------
	const Vector& Vector::Add(const Vector& a, const Vector& b)
	{
		myX = a.myX + b.myX;
		myY = a.myY + b.myY;
		myZ = a.myZ + b.myZ;

		return *this;
	}




	// --------------------------------------------------------------------------						
	// Function:	-=
	// Description:	subtract
	// Arguments:	other vector
	// Returns:		this
	// --------------------------------------------------------------------------
	const Vector& Vector::operator-=(const Vector& other)
	{
		myX -= other.myX;
		myY -= other.myY;
		myZ -= other.myZ;

		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	-=
	// Description:	subtract
	// Arguments:	vector, vector b
	// Returns:		this
	// --------------------------------------------------------------------------
	const Vector& Vector::Subtract(const Vector& a, const Vector& b)
	{
		myX = a.myX - b.myX;
		myY = a.myY - b.myY;
		myZ = a.myZ - b.myZ;

		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	*=
	// Description:	multiply
	// Arguments:	scalar
	// Returns:		this
	// --------------------------------------------------------------------------
	const Vector& Vector::operator*=(const float s)
	{
		myX *= s;
		myY *= s;
		myZ *= s;

		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	/=
	// Description:	divide
	// Arguments:	scalar
	// Returns:		this
	// --------------------------------------------------------------------------
	const Vector& Vector::operator /=(const float s)
	{
		DEBUG_ASSERT(s != 0.0f);
		myX = myX / s;
		myY = myY / s;
		myZ = myZ / s;

		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	Scale
	// Description:	multiply
	// Arguments:	scalar
	// Returns:		this
	// --------------------------------------------------------------------------
	const Vector& Vector::Scale(const float s)
	{
		DEBUG_ASSERT(s != 0.0f);
		myX *= s;
		myY *= s;
		myZ *= s;

		return *this;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLength
	// Description:	Returns the magnitude of the vector |v|.
	// Arguments:	none
	// Returns:		length
	// --------------------------------------------------------------------------
	float Vector::GetLength() const
	{
		return sqrtf((myX * myX) + (myY * myY) + (myZ * myZ));
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLengthSquared
	// Description:	Returns the squared magnitude of the vector.
	// Arguments:	none
	// Returns:		length
	// --------------------------------------------------------------------------
	float Vector::GetLengthSquared() const
	{
		return (myX * myX) + (myY * myY) + (myZ * myZ);
	}


	// --------------------------------------------------------------------------						
	// Function:	Normalise
	// Description:	normalises vector
	// Arguments:	none
	// Returns:		length
	// --------------------------------------------------------------------------
	float Vector::Normalise()
	{
		float l = GetLength();
		if (l > 0.0f)
		{
			myX /= l;
			myY /= l;
			myZ /= l;
		}

		return l;
	}


	// --------------------------------------------------------------------------						
	// Function:	Dot
	// Description:	Computes the dot product of this and other.
	//				Dot == 0, implies, at least one of the vectors is
	//				zero, or the vectors are perpendicular.
	//				a . b = |a| |b| cosA
	// Arguments:	none
	// Returns:		dot product
	// --------------------------------------------------------------------------
	float Vector::Dot(const Vector& other) const
	{
		return (myX * other.myX) + (myY * other.myY) + (myZ * other.myZ);
	}


	// --------------------------------------------------------------------------						
	// Function:	Dot
	// Description:	this = a myX b.
	//				(i.e. a vector normal to the plane of a & b).
	//				Cross == 0, implies, at least one of the vectors is
	//				zero, or the vectors are parallel.
	//				a x b = |a| |b| sinA n
	// Arguments:	vectora, vector b
	// Returns:		this
	//----------------------------------------------------------------
	const Vector& Vector::Cross(const Vector& a, const Vector& b)
	{
		myX = (a.myY * b.myZ) - (a.myZ * b.myY);
		myY = (a.myZ * b.myX) - (a.myX * b.myZ);
		myZ = (a.myX * b.myY) - (a.myY * b.myX);

		return *this;
	}


	// ---------------------------------------------------------------						
	// Function:	Inverse
	// Description:	invets the vector
	// Arguments:	none
	// Returns:		this
	//----------------------------------------------------------------
	const Vector& Vector::Inverse()
	{
		myX = -myX;
		myY = -myY;
		myZ = -myZ;

		return *this;
	}


	// ---------------------------------------------------------------						
	// Function:	Interpolate
	// Description:	Set this vector between vectors a & b.
	//				t: range: 0.0..1.0.
	//				0.0 -> this = a, 1.0 -> this = b.
	// Arguments:	vector a, vector b, range
	// Returns:		this
	//----------------------------------------------------------------
	const Vector& Vector::Interpolate(const Vector& a, const Vector& b, const float t)
	{
		DEBUG_ASSERT(t >= 0.0f && t <= 1.0f);
*this = b;
		*this -= a;
		*this *= t;
		*this += a;

		return *this;
	}


	// ---------------------------------------------------------------						
	// Function:	<
	// Description:	less than length
	// Arguments:	other vector 
	// Returns:		bool
	//----------------------------------------------------------------
	inline bool Vector::operator<(const Vector& other) const 
	{ 
		return GetLength() < other.GetLength(); 
	}


	// ---------------------------------------------------------------						
	// Function:	[]
	// Description:	indexing
	// Arguments:	index
	// Returns:		value
	//----------------------------------------------------------------
	inline float const& Vector::operator[](int i) const
	{ 
		return (&myX)[i]; 
	
	}

	// ---------------------------------------------------------------						
	// Function:	[]
	// Description:	indexing
	// Arguments:	index
	// Returns:		value
	//----------------------------------------------------------------
	inline float& Vector::operator[](int i)
	{ 
		return (&myX)[i]; 
	}

}
#endif
