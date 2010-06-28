// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2009, Roboterclub Aachen e.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Roboterclub Aachen e.V. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBOTERCLUB AACHEN E.V. ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROBOTERCLUB AACHEN E.V. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 */
// ----------------------------------------------------------------------------

#ifndef	XPCC__VECTOR_2D_HPP
	#error	"Don't include this file directly, use 'point.hpp' instead!"
#endif

// ----------------------------------------------------------------------------
template<typename T>
xpcc::Vector2D<T>::Vector2D() :
	x(), y()
{
}

template<typename T>
xpcc::Vector2D<T>::Vector2D(const T& x, const T& y) : 
	x(x), y(y)
{
}

template<typename T>
xpcc::Vector2D<T>::Vector2D(const Point2D<T>& A, const Point2D<T>& B) :
	x(B.x - A.x), y(B.y - A.y)
{
}

// ----------------------------------------------------------------------------
template<typename T>
void
xpcc::Vector2D<T>::setX(const T& value)
{
	this->x = value;
}

// ----------------------------------------------------------------------------
template<typename T>
void
xpcc::Vector2D<T>::setY(const T& value)
{
	this->y = value;
}

// ----------------------------------------------------------------------------
template<typename T>
void
xpcc::Vector2D<T>::set(const T& x, const T& y)
{
	this->x = x;
	this->y = y;
}

// ----------------------------------------------------------------------------
template<typename T>
const T&
xpcc::Vector2D<T>::getX() const
{
	return this->x;
}

// ----------------------------------------------------------------------------
template<typename T>
const T&
xpcc::Vector2D<T>::getY() const
{
	return this->y;
}

// ----------------------------------------------------------------------------
template<typename T>
T
xpcc::Vector2D<T>::getLength() const
{
	float tx = this->x;
	float ty = this->y;
	
	return round(std::sqrt(tx*tx + ty*ty));
}

// ----------------------------------------------------------------------------
template<typename T>
typename xpcc::Vector2D<T>::WideType
xpcc::Vector2D<T>::getLengthSquared() const
{
	WideType tx = static_cast<WideType>(this->x);
	WideType ty = static_cast<WideType>(this->y);
	
	return tx * tx + ty * ty;
}

// ----------------------------------------------------------------------------
template<typename T>
float
xpcc::Vector2D<T>::getAngle() const
{
	return std::atan2(y, x);
}

// ----------------------------------------------------------------------------
template<typename T>
xpcc::Vector2D<T>&
xpcc::Vector2D<T>::normalize()
{
	T length = this->getLength();
	
	this->x = this->x / length;
	this->y = this->y / length;
	
	return *this;
}

// ----------------------------------------------------------------------------
template<typename T>
template<typename U>
xpcc::Vector2D<T>&
xpcc::Vector2D<T>::scale(const U& factor)
{
	this->x = this->x * factor;
	this->y = this->y * factor;

	return *this;
}

// ----------------------------------------------------------------------------
template<typename T>
xpcc::Vector2D<T>&
xpcc::Vector2D<T>::rotate(const float phi)
{
	float c = std::cos(phi);
	float s = std::sin(phi);
	
	// without rounding the result might be false for T = integer
	T tx =    round(c * this->x - s * this->y);
	this->y = round(s * this->x + c * this->y);
	this->x = tx;
	
	return *this;
}

// ----------------------------------------------------------------------------
template<typename T>
xpcc::Vector2D<T>&
xpcc::Vector2D<T>::operator += (const Vector2D &other)
{
	this->x += other.x;
	this->y += other.y;
	return *this;
}

// ----------------------------------------------------------------------------	
template<typename T>
xpcc::Vector2D<T>&
xpcc::Vector2D<T>::operator -= (const Vector2D &other)
{
	this->x -= other.x;
	this->y -= other.y;
	return *this;
}

// ----------------------------------------------------------------------------
template<typename T> template<typename U>
xpcc::Vector2D<U>
xpcc::Vector2D<T>::convert() const
{
	return Vector2D<U>(this->x, this->y);
}

template<typename T>
xpcc::Point2D<T>
xpcc::Vector2D<T>::toPoint() const
{
	return Point2D<T>(this->x, this->y);
}

// ----------------------------------------------------------------------------
template<typename T>
typename xpcc::Vector2D<T>::WideType
xpcc::Vector2D<T>::dotProduct(const xpcc::Vector2D<T>& a,
		const xpcc::Vector2D<T>& b)
{
	return (static_cast<WideType>(a.x) * static_cast<WideType>(b.x) +
			static_cast<WideType>(a.y) * static_cast<WideType>(b.y));
}

// ----------------------------------------------------------------------------
template <typename U>
xpcc::IOStream&
xpcc::operator << (xpcc::IOStream& s, const xpcc::Vector2D<U>& c)
{
	s << "x=" << c.x << ", y=" << c.y;
	return s;
}

// ----------------------------------------------------------------------------
template<typename U>
xpcc::Vector2D<U>
xpcc::operator - (const Vector2D<U> &a)
{
	return Vector2D<U>(-a.x, -a.y);
}

template<typename U>
xpcc::Vector2D<U>
xpcc::operator - (const Vector2D<U> &a, const Vector2D<U> &b)
{
	return Vector2D<U>(a.x - b.x, a.y - b.y);
}

template<typename U>
xpcc::Vector2D<U>
xpcc::operator + (const Vector2D<U> &a, const Vector2D<U> &b)
{
	return Vector2D<U>(a.x + b.x, a.y + b.y);
}


template<typename U>
bool
xpcc::operator == (const Vector2D<U> &a, const Vector2D<U> &b)
{
	return ((a.x == b.x) && (a.y == b.y));
}

template<typename U>
bool
xpcc::operator != (const Vector2D<U> &a, const Vector2D<U> &b)
{
	return ((a.x != b.x) || (a.y != b.y));
}