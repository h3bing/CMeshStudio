#ifndef MATH_TYPES_H
#define MATH_TYPES_H

#include <cmath>

// 浮点数精度比较
const float EPSILON = 1e-6;

inline bool floatEqual(float a, float b) {
  return std::fabs(a - b) < EPSILON;
}

inline bool floatLess(float a, float b) {
  return a < b - EPSILON;
}

inline bool floatGreater(float a, float b) {
  return a > b + EPSILON;
}

// 角度和弧度互换
inline float degreesToRadians(float degrees) {
  return degrees * M_PI / 180.0f;
}

inline float radiansToDegrees(float radians) {
  return radians * 180.0f / M_PI;
}

// 最小最大值函数
inline float min2(float a, float b) {
  return a < b ? a : b;
}

inline float max2(float a, float b) {
  return a > b ? a : b;
}

inline float min3(float a, float b, float c) {
  return min2(min2(a, b), c);
}

inline float max3(float a, float b, float c) {
  return max2(max2(a, b), c);
}

class PVector3
{
private:
  float m_x, m_y, m_z;

public:
  PVector3() : m_x(0), m_y(0), m_z(0) {}
  PVector3(float x, float y, float z) : m_x(x), m_y(y), m_z(z) {}
  
  float x() const { return m_x; }
  float y() const { return m_y; }
  float z() const { return m_z; }
  
  void setX(float x) { m_x = x; }
  void setY(float y) { m_y = y; }
  void setZ(float z) { m_z = z; }
  
  PVector3 operator+(const PVector3& other) const { return PVector3(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z); }
  PVector3 operator-(const PVector3& other) const { return PVector3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z); }
  PVector3 operator*(float scalar) const { return PVector3(m_x * scalar, m_y * scalar, m_z * scalar); }
  PVector3 operator/(float scalar) const { return PVector3(m_x / scalar, m_y / scalar, m_z / scalar); }
  
  float dot(const PVector3& other) const { return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z; }
  PVector3 cross(const PVector3& other) const { return PVector3(m_y * other.m_z - m_z * other.m_y, m_z * other.m_x - m_x * other.m_z, m_x * other.m_y - m_y * other.m_x); }
  float length() const { return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z); }
  PVector3 normalize() const { float len = length(); return len > 0 ? *this / len : *this; }
};

class PPoint3
{
private:
  float m_x, m_y, m_z;

public:
  PPoint3() : m_x(0), m_y(0), m_z(0) {}
  PPoint3(float x, float y, float z) : m_x(x), m_y(y), m_z(z) {}
  
  float x() const { return m_x; }
  float y() const { return m_y; }
  float z() const { return m_z; }
  
  void setX(float x) { m_x = x; }
  void setY(float y) { m_y = y; }
  void setZ(float z) { m_z = z; }
  
  PPoint3 operator+(const PVector3& vector) const { return PPoint3(m_x + vector.x(), m_y + vector.y(), m_z + vector.z()); }
  PPoint3 operator-(const PVector3& vector) const { return PPoint3(m_x - vector.x(), m_y - vector.y(), m_z - vector.z()); }
  PVector3 operator-(const PPoint3& other) const { return PVector3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z); }
};

class PMatrix4
{
private:
  float m_data[16]; // Column-major order (OpenGL standard)

public:
  PMatrix4() { for (int i = 0; i < 16; i++) m_data[i] = 0; m_data[0] = m_data[5] = m_data[10] = m_data[15] = 1; }
  
  static PMatrix4 identity() { 
    PMatrix4 mat; 
    for (int i = 0; i < 16; i++) mat.m_data[i] = 0; 
    mat.m_data[0] = mat.m_data[5] = mat.m_data[10] = mat.m_data[15] = 1; 
    return mat; 
  }
  
  static PMatrix4 translate(float x, float y, float z) {
    PMatrix4 mat = identity();
    mat.m_data[12] = x;
    mat.m_data[13] = y;
    mat.m_data[14] = z;
    return mat;
  }
  
  static PMatrix4 rotateX(float angle); // angle in degrees
  static PMatrix4 rotateY(float angle); // angle in degrees
  static PMatrix4 rotateZ(float angle); // angle in degrees
  static PMatrix4 rotate(float angle, const PVector3& axis); // angle in degrees, axis must be normalized
  static PMatrix4 scale(float x, float y, float z);
  
  PMatrix4 operator*(const PMatrix4& other) const;
  PMatrix4& operator*=(const PMatrix4& other);
  PVector3 operator*(const PVector3& vec) const;
  
  float* data() { return m_data; }
  const float* data() const { return m_data; }
};

class PBoundBox
{
private:
  PVector3 m_min, m_max;

public:
  PBoundBox() : m_min(0, 0, 0), m_max(0, 0, 0) {}
  PBoundBox(const PVector3& min, const PVector3& max) : m_min(min), m_max(max) {}
  
  PVector3 min() const { return m_min; }
  PVector3 max() const { return m_max; }
  
  PBoundBox operator+(const PVector3& point) const;
  PBoundBox operator+(const PBoundBox& other) const;
  
  PVector3 center() const;
  PVector3 size() const;
  void expand(const PVector3& point);
  void expand(const PBoundBox& boundbox);
  void expand(const PPoint3& point);
};

class PColorRGBA
{
private:
  float m_r, m_g, m_b, m_a;

public:
  PColorRGBA() : m_r(0), m_g(0), m_b(0), m_a(1) {}
  PColorRGBA(float r, float g, float b, float a = 1) : m_r(r), m_g(g), m_b(b), m_a(a) {}
  
  float r() const { return m_r; }
  float g() const { return m_g; }
  float b() const { return m_b; }
  float a() const { return m_a; }
  
  void setR(float r) { m_r = r; }
  void setG(float g) { m_g = g; }
  void setB(float b) { m_b = b; }
  void setA(float a) { m_a = a; }
};

#endif // MATH_TYPES_H