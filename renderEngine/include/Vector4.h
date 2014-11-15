#pragma once
#include "basics.h"

class Vector4 // actually a Minkowski vector
{
public:
  Vector4(){}  
  inline Vector4(const Vector3 &vec, float T)
  {
    x = vec.x; y = vec.y; z = vec.z; t = T;
  }
  inline Vector4(float X, float Y, float Z, float T)
  {
    x = X; y = Y; z = Z; t = T;
  }
  float x, y, z, t;
  inline Vector4 operator *(float f) const
  {
    return Vector4(x * f, y * f, z * f, t * f);
  }
  inline Vector4 operator *(const Vector4& vec) const
  {
    return Vector4(x * vec.x, y * vec.y, z * vec.z, t * vec.t);
  }
  inline void operator *=(float f)
  {
    x *= f;
    y *= f;
    z *= f;
    t *= f;
  }
  inline Vector4 operator /(float f) const
  {
    float div = 1.0f/f;
    return Vector4(x * div, y * div, z * div, t * div);
  }
  inline Vector4 operator /(const Vector4& div) const
  {
    return Vector4(x / div.x, y / div.y, z / div.z, t / div.t);
  }
  inline void operator /=(float f) 
  {
    float div = 1.0f/f;
    x *= div;
    y *= div;
    z *= div;
    t *= div;
  }
  inline Vector4 operator +(const Vector4& v) const
  {
    return Vector4(x + v.x, y + v.y, z + v.z, t + v.t);
  }
  inline void operator +=(const Vector4& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    t += v.t;
  }
  inline Vector4 operator -() const
  {
    return Vector4(-x, -y, -z, -t);
  }
  inline Vector4 operator -(const Vector4& v) const
  {
    return Vector4(x - v.x, y - v.y, z - v.z, t - v.t);
  }
  inline void operator -=(const Vector4& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    t -= v.t;
  }
  inline void set(const Vector4& v)
  {
    x = v.x;
    y = v.y;
    z = v.z;
    t = v.t;
  }
  inline void set(float _x, float _y, float _z, float _t)
  {
    x = _x;
    y = _y;
    z = _z;
    t = _t;
  }
  inline void setToZero()
  {
    x = y = z = t = 0.0f;
  }
  inline float magnitudeSquared() const
  {
    return x*x + y*y + z*z - t*t;
  }
  inline float magnitude() const
  {
    return sqrtf(absf(magnitudeSquared()));
  }
  inline float normalise()
  {
    float f = magnitude();
    if (f < 1e-10f)
    {
      set(0,0,1,0);
      return 0.0f;
    }
    float d = 1.0f / f;
    x *= d; y *= d; z *= d; t *= d;
    return f;
  }
  static inline Vector4 normalise(const Vector4& v)
  {
    float f = absf(v.magnitudeSquared());
    return f < 1e-10f ? Vector4(0,0,1,0) : v / sqrtf(f);
  }
  inline float dot(const Vector4& v) const
  {
    return x * v.x + y * v.y + z * v.z - t * v.t;
  }
  void boost(const Vector4 &normal)
  {
    Vector4 normalSpatial = normal;
    normalSpatial.t = 0;
    Vector4 diagonal1 = normalSpatial + Vector4(0,0,0,1);
    Vector4 diagonal2 = -normalSpatial + Vector4(0,0,0,1);
    float length1 = dot(diagonal1); 
    float length2 = dot(diagonal2);
    *this = (diagonal1*length1*normal.t + diagonal2*length2/normal.t)/1.4142f;
  }
  void unboost(const Vector4 &normal)
  {
    Vector4 norm = normal;
    norm.t = 1/norm.t;
    boost(norm);
  }
//   void rotate(const class Matrix44& mat);
//   void inverseRotate(const class Matrix44& mat);
  void clamp(float minVal, float maxVal);
};
Vector4 operator *(float f, const Vector4& v);
