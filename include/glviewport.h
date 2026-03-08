#ifndef GLVIEWPORT_H
#define GLVIEWPORT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include "geometry.h"

class PGLViewport : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
private:
  QMatrix4x4 m_projectionMatrix;
  QMatrix4x4 m_viewMatrix;
  std::shared_ptr<PDocument> m_document;
  int m_selectedEntityId;
  bool m_showAxes; // 控制是否显示坐标轴
  
  // 鼠标操作相关
  bool m_mousePressed;
  Qt::MouseButton m_mouseButton;
  QPoint m_lastMousePos;
  
  // 相机参数
  float m_cameraDistance;
  float m_cameraYaw;
  float m_cameraPitch;
  QVector3D m_cameraPosition; // 相机位置，用于平移
  bool m_perspective; // 透视/正交投影模式
  float m_orthoSize; // 正交投影大小
  
  // 渲染模式
  enum RenderMode {
    POINTS,
    WIREFRAME,
    MESH,
    SOLID
  } m_renderMode;
  
  // Shader programs
  unsigned int m_shaderProgram;
  unsigned int m_vertexShader;
  unsigned int m_fragmentShader;
  
  // VBOs and VAOs
  std::map<int, unsigned int> m_vaos;
  std::map<int, unsigned int> m_vbos;
  std::map<int, unsigned int> m_ebos;
  
  // Geometry cache to avoid reloading unchanged data
  std::map<int, size_t> m_vertexHashCache;
  std::map<int, size_t> m_indexHashCache;

public:
  PGLViewport(QWidget* parent = nullptr);
  ~PGLViewport();
  
  void setDocument(std::shared_ptr<PDocument> document);
  void setSelectedEntityId(int id) { m_selectedEntityId = id; }
  void setShowAxes(bool show) { m_showAxes = show; update(); }
  bool showAxes() const { return m_showAxes; }
  
  // 视向控制
  void setStandardView(int viewIndex); // 0: 顶视图, 1: 前视图, 2: 侧视图, 3: 透视图
  void resetView();
  void setPerspective(bool perspective);
  bool isPerspective() const { return m_perspective; }
  
  // 渲染模式控制
  void setRenderMode(int mode); // 0: 点, 1: 线框, 2: 网格线, 3: 实体
  
protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  
private:
  void compileShaders();
  void createBuffers();
  void updateBuffers();
  void renderEntity(const std::shared_ptr<PEntity>& entity);
  void renderAxes();
  void updateCamera();
}; 

#endif // GLVIEWPORT_H