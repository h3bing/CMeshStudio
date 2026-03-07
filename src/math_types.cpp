#include "math_types.h"

// Matrix multiplication (column-major)
PMatrix4 PMatrix4::operator*(const PMatrix4& other) const {
  PMatrix4 result;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      result.m_data[j * 4 + i] = 0;
      for (int k = 0; k < 4; k++) {
        result.m_data[j * 4 + i] += m_data[k * 4 + i] * other.m_data[j * 4 + k];
      }
    }
  }
  return result;
}

// Vector transformation (column-major)
PVector3 PMatrix4::operator*(const PVector3& vec) const {
  float x = m_data[0] * vec.x() + m_data[4] * vec.y() + m_data[8] * vec.z() + m_data[12];
  float y = m_data[1] * vec.x() + m_data[5] * vec.y() + m_data[9] * vec.z() + m_data[13];
  float z = m_data[2] * vec.x() + m_data[6] * vec.y() + m_data[10] * vec.z() + m_data[14];
  float w = m_data[3] * vec.x() + m_data[7] * vec.y() + m_data[11] * vec.z() + m_data[15];
  return PVector3(x / w, y / w, z / w);
}

// Rotation around X-axis (angle in degrees)
PMatrix4 PMatrix4::rotateX(float angle) {
  float radians = angle * M_PI / 180.0f;
  float c = std::cos(radians);
  float s = std::sin(radians);
  PMatrix4 mat = identity();
  mat.m_data[5] = c;
  mat.m_data[6] = -s;
  mat.m_data[9] = s;
  mat.m_data[10] = c;
  return mat;
}

// Rotation around Y-axis (angle in degrees)
PMatrix4 PMatrix4::rotateY(float angle) {
  float radians = angle * M_PI / 180.0f;
  float c = std::cos(radians);
  float s = std::sin(radians);
  PMatrix4 mat = identity();
  mat.m_data[0] = c;
  mat.m_data[8] = s;
  mat.m_data[2] = -s;
  mat.m_data[10] = c;
  return mat;
}

// Rotation around Z-axis (angle in degrees)
PMatrix4 PMatrix4::rotateZ(float angle) {
  float radians = angle * M_PI / 180.0f;
  float c = std::cos(radians);
  float s = std::sin(radians);
  PMatrix4 mat = identity();
  mat.m_data[0] = c;
  mat.m_data[4] = -s;
  mat.m_data[1] = s;
  mat.m_data[5] = c;
  return mat;
}

// Scaling
PMatrix4 PMatrix4::scale(float x, float y, float z) {
  PMatrix4 mat = identity();
  mat.m_data[0] = x;
  mat.m_data[5] = y;
  mat.m_data[10] = z;
  return mat;
}

// Rotation around arbitrary axis (angle in degrees, axis must be normalized)
PMatrix4 PMatrix4::rotate(float angle, const PVector3& axis) {
  float radians = degreesToRadians(angle);
  float c = std::cos(radians);
  float s = std::sin(radians);
  float oneMinusC = 1.0f - c;
  
  float x = axis.x();
  float y = axis.y();
  float z = axis.z();
  
  PMatrix4 mat;
  mat.m_data[0] = c + x * x * oneMinusC;
  mat.m_data[1] = x * y * oneMinusC - z * s;
  mat.m_data[2] = x * z * oneMinusC + y * s;
  mat.m_data[3] = 0;
  
  mat.m_data[4] = y * x * oneMinusC + z * s;
  mat.m_data[5] = c + y * y * oneMinusC;
  mat.m_data[6] = y * z * oneMinusC - x * s;
  mat.m_data[7] = 0;
  
  mat.m_data[8] = z * x * oneMinusC - y * s;
  mat.m_data[9] = z * y * oneMinusC + x * s;
  mat.m_data[10] = c + z * z * oneMinusC;
  mat.m_data[11] = 0;
  
  mat.m_data[12] = 0;
  mat.m_data[13] = 0;
  mat.m_data[14] = 0;
  mat.m_data[15] = 1;
  
  return mat;
}

// Matrix multiplication assignment operator
PMatrix4& PMatrix4::operator*=(const PMatrix4& other) {
  *this = *this * other;
  return *this;
}

// Bounding box methods
PBoundBox PBoundBox::operator+(const PVector3& point) const {
  PBoundBox box = *this;
  box.expand(point);
  return box;
}

PBoundBox PBoundBox::operator+(const PBoundBox& other) const {
  PBoundBox box = *this;
  box.expand(other.m_min);
  box.expand(other.m_max);
  return box;
}

PVector3 PBoundBox::center() const {
  return PVector3((m_min.x() + m_max.x()) / 2, (m_min.y() + m_max.y()) / 2, (m_min.z() + m_max.z()) / 2);
}

PVector3 PBoundBox::size() const {
  return m_max - m_min;
}

void PBoundBox::expand(const PVector3& point) {
  if (point.x() < m_min.x()) m_min.setX(point.x());
  if (point.y() < m_min.y()) m_min.setY(point.y());
  if (point.z() < m_min.z()) m_min.setZ(point.z());
  if (point.x() > m_max.x()) m_max.setX(point.x());
  if (point.y() > m_max.y()) m_max.setY(point.y());
  if (point.z() > m_max.z()) m_max.setZ(point.z());
}

void PBoundBox::expand(const PPoint3& point) {
  PVector3 vec(point.x(), point.y(), point.z());
  expand(vec);
}

void PBoundBox::expand(const PBoundBox& boundbox) {
  expand(boundbox.min());
  expand(boundbox.max());
}
