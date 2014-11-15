#include "basics.h"
#include "vector4.h"

// void Vector4::rotate(const Matrix44& mat)
// {
//   float newX = x * mat.row[0].x + y * mat.row[1].x + z * mat.row[2].x + t * mat.row[3].x;
//   float newY = x * mat.row[0].y + y * mat.row[1].y + z * mat.row[2].y + t * mat.row[3].y;
//   float newZ = x * mat.row[0].z + y * mat.row[1].z + z * mat.row[2].z + t * mat.row[3].z;
//   float newT = x * mat.row[0].t + y * mat.row[1].t + z * mat.row[2].t + t * mat.row[3].t;
//   x = newX;
//   y = newY;
//   z = newZ;
//   t = newT;
// }
// void Vector4::inverseRotate(const Matrix44& mat)
// {
//   float newX = x * mat.row[0].x + y * mat.row[0].y + z * mat.row[0].z + t * mat.row[0].t;
//   float newY = x * mat.row[1].x + y * mat.row[1].y + z * mat.row[1].z + t * mat.row[1].t;
//   float newZ = x * mat.row[2].x + y * mat.row[2].y + z * mat.row[2].z + t * mat.row[2].t;
//   float newT = x * mat.row[3].x + y * mat.row[3].y + z * mat.row[3].z + t * mat.row[3].t;
//   x = newX;
//   y = newY;
//   z = newZ;
//   t = newT;
// }

void Vector4::clamp(float minVal, float maxVal)
{
  x = clamped(x, minVal, maxVal);
  y = clamped(y, minVal, maxVal);
  z = clamped(z, minVal, maxVal);
  t = clamped(t, minVal, maxVal);
}

Vector4 operator *(float f, const Vector4& v)
{
  return Vector4(v.x * f, v.y * f, v.z * f, v.t * f);
}

// Vector4 Vector4::getRotationVector(const Direction& from, const Direction& to)
// {
//   Vector4 cross = Vector4::cross(from, to);
//   float dot = from.dot(to);
//   float mag = cross.normalise();
//   Angle angle = atan2f(mag, dot);
//   return cross * angle;
// }
