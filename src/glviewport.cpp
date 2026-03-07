#include "glviewport.h"
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPoint>

PGLViewport::PGLViewport(QWidget* parent) 
  : QOpenGLWidget(parent), m_selectedEntityId(-1), m_showAxes(true),
    m_mousePressed(false), m_mouseButton(Qt::NoButton),
    m_cameraDistance(10.0f), m_cameraYaw(0.0f), m_cameraPitch(0.0f),
    m_perspective(true), // 默认使用透视投影
    m_orthoSize(5.0f), // 默认正交投影大小
    m_renderMode(SOLID), // 默认使用实体渲染
    m_shaderProgram(0), m_vertexShader(0), m_fragmentShader(0) {}

void PGLViewport::updateCamera() {
  // 重置视图矩阵
  m_viewMatrix.setToIdentity();
  
  // 应用相机变换
  m_viewMatrix.translate(0.0f, 0.0f, -m_cameraDistance);
  m_viewMatrix.rotate(m_cameraPitch, 1.0f, 0.0f, 0.0f);
  m_viewMatrix.rotate(m_cameraYaw, 0.0f, 1.0f, 0.0f);
}

void PGLViewport::mousePressEvent(QMouseEvent* event) {
  m_mousePressed = true;
  m_mouseButton = event->button();
  m_lastMousePos = event->pos();
  setCursor(Qt::ClosedHandCursor);
}

void PGLViewport::mouseMoveEvent(QMouseEvent* event) {
  if (!m_mousePressed) return;
  
  QPoint delta = event->pos() - m_lastMousePos;
  
  if (m_mouseButton == Qt::LeftButton) {
    // 旋转相机
    m_cameraYaw += delta.x() * 0.5f;
    m_cameraPitch -= delta.y() * 0.5f;
    
    // 限制俯仰角
    if (m_cameraPitch > 89.0f) m_cameraPitch = 89.0f;
    if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;
  } else if (m_mouseButton == Qt::RightButton) {
    // 平移相机
    m_viewMatrix.translate(-delta.x() * 0.01f, delta.y() * 0.01f, 0.0f);
  }
  
  m_lastMousePos = event->pos();
  updateCamera();
  update();
}

void PGLViewport::mouseReleaseEvent(QMouseEvent* event) {
  m_mousePressed = false;
  m_mouseButton = Qt::NoButton;
  setCursor(Qt::ArrowCursor);
}

void PGLViewport::wheelEvent(QWheelEvent* event) {
  if (m_perspective) {
    // 透视投影模式：调整相机距离
    m_cameraDistance -= event->angleDelta().y() * 0.01f;
    if (m_cameraDistance < 1.0f) m_cameraDistance = 1.0f;
    if (m_cameraDistance > 100.0f) m_cameraDistance = 100.0f;
    updateCamera();
  } else {
    // 正交投影模式：调整正交投影的大小
    // 基于当前大小进行缩放，实现更平滑的效果
    m_orthoSize *= (1.0f - event->angleDelta().y() * 0.001f);
    if (m_orthoSize < 0.1f) m_orthoSize = 0.1f;
    if (m_orthoSize > 50.0f) m_orthoSize = 50.0f;
    
    // 更新投影矩阵
    m_projectionMatrix.setToIdentity();
    float aspect = (float)width() / height();
    m_projectionMatrix.ortho(-m_orthoSize * aspect, m_orthoSize * aspect, -m_orthoSize, m_orthoSize, 0.1f, 1000.0f);
  }
  
  update();
}

void PGLViewport::setStandardView(int viewIndex) {
  switch (viewIndex) {
    case 0: // 顶视图
      m_cameraYaw = 0.0f;
      m_cameraPitch = -90.0f;
      break;
    case 1: // 前视图
      m_cameraYaw = 0.0f;
      m_cameraPitch = 0.0f;
      break;
    case 2: // 侧视图
      m_cameraYaw = 90.0f;
      m_cameraPitch = 0.0f;
      break;
    case 3: // 透视图
      m_cameraYaw = 45.0f;
      m_cameraPitch = 45.0f;
      break;
    default:
      break;
  }
  m_cameraDistance = 10.0f;
  updateCamera();
  update();
}

void PGLViewport::resetView() {
  m_cameraYaw = 0.0f;
  m_cameraPitch = 0.0f;
  m_cameraDistance = 10.0f;
  updateCamera();
  update();
}

void PGLViewport::setPerspective(bool perspective) {
  m_perspective = perspective;
  // 重新计算投影矩阵
  resizeGL(width(), height());
  // 重置相机参数，确保切换投影模式后视图大小合适
  if (m_perspective) {
    m_cameraDistance = 10.0f;
    updateCamera();
  } else {
    m_orthoSize = 5.0f; // 重置正交投影大小
    // 更新投影矩阵
    m_projectionMatrix.setToIdentity();
    float aspect = (float)width() / height();
    m_projectionMatrix.ortho(-m_orthoSize * aspect, m_orthoSize * aspect, -m_orthoSize, m_orthoSize, 0.1f, 1000.0f);
  }
  update();
}

void PGLViewport::setRenderMode(int mode) {
  switch (mode) {
    case 0:
      m_renderMode = POINTS;
      break;
    case 1:
      m_renderMode = WIREFRAME;
      break;
    case 2:
      m_renderMode = MESH;
      break;
    case 3:
      m_renderMode = SOLID;
      break;
    default:
      m_renderMode = SOLID;
      break;
  }
  update();
}

PGLViewport::~PGLViewport() {
  makeCurrent();
  
  // Clean up shaders
  if (m_shaderProgram) {
    glDeleteProgram(m_shaderProgram);
  }
  
  // Clean up buffers
  for (auto& [id, vao] : m_vaos) {
    glDeleteVertexArrays(1, &vao);
  }
  for (auto& [id, vbo] : m_vbos) {
    glDeleteBuffers(1, &vbo);
  }
  for (auto& [id, ebo] : m_ebos) {
    glDeleteBuffers(1, &ebo);
  }
  
  doneCurrent();
}

void PGLViewport::setDocument(std::shared_ptr<PDocument> document) {
  m_document = document;
  update();
}

void PGLViewport::initializeGL() {
  initializeOpenGLFunctions();
  
  // Set clear color
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  
  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  
  // Compile shaders
  compileShaders();
  
  // Create buffers
  createBuffers();
  
  // Initialize camera
  updateCamera();
}

void PGLViewport::resizeGL(int w, int h) {
  // Set viewport
  glViewport(0, 0, w, h);
  
  // Update projection matrix
  m_projectionMatrix.setToIdentity();
  if (m_perspective) {
    m_projectionMatrix.perspective(45.0f, (float)w / h, 0.1f, 1000.0f);
  } else {
    // 正交投影
    float aspect = (float)w / h;
    m_projectionMatrix.ortho(-m_orthoSize * aspect, m_orthoSize * aspect, -m_orthoSize, m_orthoSize, 0.1f, 1000.0f);
  }
}

void PGLViewport::paintGL() {
  // Clear buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // Use shader program
  glUseProgram(m_shaderProgram);
  
  // Update buffers
  updateBuffers();
  
  // Render entities
  if (m_document) {
    for (const auto& [id, entity] : m_document->entities()) {
      renderEntity(entity);
    }
  }
  
  // Render axes if enabled
  if (m_showAxes) {
    renderAxes();
  }
}

void PGLViewport::compileShaders() {
  try {
    // Vertex shader source
    const char* vertexShaderSource = R"(
      #version 330 core
      layout (location = 0) in vec3 aPos;
      
      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;
      
      void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
      }
    )";
    
    // Fragment shader source
    const char* fragmentShaderSource = R"(
      #version 330 core
      out vec4 FragColor;
      
      uniform vec4 color;
      
      void main() {
        FragColor = color;
      }
    )";
    
    // Create vertex shader
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (m_vertexShader == 0) {
      throw std::runtime_error("Failed to create vertex shader");
    }
    glShaderSource(m_vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(m_vertexShader);
    
    // Check for vertex shader errors
    int success;
    char infoLog[512];
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(m_vertexShader, 512, nullptr, infoLog);
      throw std::runtime_error(std::string("Vertex shader compilation failed: ") + infoLog);
    }
    
    // Create fragment shader
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (m_fragmentShader == 0) {
      throw std::runtime_error("Failed to create fragment shader");
    }
    glShaderSource(m_fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(m_fragmentShader);
    
    // Check for fragment shader errors
    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(m_fragmentShader, 512, nullptr, infoLog);
      throw std::runtime_error(std::string("Fragment shader compilation failed: ") + infoLog);
    }
    
    // Create shader program
    m_shaderProgram = glCreateProgram();
    if (m_shaderProgram == 0) {
      throw std::runtime_error("Failed to create shader program");
    }
    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);
    
    // Check for linking errors
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
      throw std::runtime_error(std::string("Shader program linking failed: ") + infoLog);
    }
    
    // Delete shaders
    glDeleteShader(m_vertexShader);
    glDeleteShader(m_fragmentShader);
  } catch (const std::exception& e) {
    qDebug() << "Shader compilation error:" << e.what();
    // 这里可以添加错误回调，将错误信息传递给UI
  } catch (...) {
    qDebug() << "Unknown shader compilation error";
  }
}

void PGLViewport::createBuffers() {
  // Buffers will be created in updateBuffers
}

void PGLViewport::updateBuffers() {
  if (!m_document) return;
  
  try {
    for (const auto& [id, entity] : m_document->entities()) {
      try {
        // Check if buffers exist for this entity
        if (m_vaos.find(id) == m_vaos.end()) {
          // Create VAO
          unsigned int vao;
          glGenVertexArrays(1, &vao);
          if (vao == 0) {
            throw std::runtime_error("Failed to generate VAO");
          }
          m_vaos[id] = vao;
          
          // Create VBO
          unsigned int vbo;
          glGenBuffers(1, &vbo);
          if (vbo == 0) {
            throw std::runtime_error("Failed to generate VBO");
          }
          m_vbos[id] = vbo;
          
          // Create EBO
          unsigned int ebo;
          glGenBuffers(1, &ebo);
          if (ebo == 0) {
            throw std::runtime_error("Failed to generate EBO");
          }
          m_ebos[id] = ebo;
        }
        
        // Bind VAO
        glBindVertexArray(m_vaos[id]);
        
        // Bind VBO and upload data
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos[id]);
        const auto& vertices = entity->vertices();
        if (!vertices.empty()) {
          glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PVector3), vertices.data(), GL_STATIC_DRAW);
          
          // Bind EBO and upload data
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos[id]);
          const auto& indices = entity->indices();
          if (!indices.empty()) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
          }
          
          // Set vertex attributes
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PVector3), (void*)0);
          glEnableVertexAttribArray(0);
        }
        
        // Unbind VAO
        glBindVertexArray(0);
      } catch (const std::exception& e) {
        qDebug() << "Buffer update error for entity" << id << ":" << e.what();
        // 忽略单个实体的错误，继续处理其他实体
      }
    }
  } catch (const std::exception& e) {
    qDebug() << "Buffer update error:" << e.what();
  } catch (...) {
    qDebug() << "Unknown buffer update error";
  }
}

void PGLViewport::renderEntity(const std::shared_ptr<PEntity>& entity) {
  try {
    const auto& vertices = entity->vertices();
    const auto& indices = entity->indices();
    
    if (vertices.empty() || indices.empty()) return;
    
    // Get entity ID
    int id = entity->id();
    
    // Check if buffers exist
    if (m_vaos.find(id) == m_vaos.end()) return;
    
    // Bind VAO
    glBindVertexArray(m_vaos[id]);
    
    // Set model matrix from entity transform
    QMatrix4x4 modelMatrix;
    PMatrix4 transform = entity->transform();
    
    // Convert PMatrix4 to QMatrix4x4
    const float* data = transform.data();
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        modelMatrix(i, j) = data[j * 4 + i]; // Column-major order
      }
    }
    
    // Apply position, rotation, and scale from properties if available
    try {
      // Get position from properties
      auto pos = entity->getProperty("位姿", "位置");
      if (std::holds_alternative<std::vector<float>>(pos)) {
        auto vec = std::get<std::vector<float>>(pos);
        if (vec.size() >= 3) {
          modelMatrix.translate(vec[0], vec[1], vec[2]);
        }
      }
      
      // Get rotation from properties
      auto rot = entity->getProperty("位姿", "旋转");
      if (std::holds_alternative<std::vector<float>>(rot)) {
        auto vec = std::get<std::vector<float>>(rot);
        if (vec.size() >= 3) {
          modelMatrix.rotate(vec[0], 1.0f, 0.0f, 0.0f);
          modelMatrix.rotate(vec[1], 0.0f, 1.0f, 0.0f);
          modelMatrix.rotate(vec[2], 0.0f, 0.0f, 1.0f);
        }
      }
      
      // Get scale from properties
      auto scale = entity->getProperty("位姿", "缩放");
      if (std::holds_alternative<std::vector<float>>(scale)) {
        auto vec = std::get<std::vector<float>>(scale);
        if (vec.size() >= 3) {
          modelMatrix.scale(vec[0], vec[1], vec[2]);
        }
      }
    } catch (...) {
      // Ignore property access errors
    }
    
    // Get uniform locations
    int modelLoc = glGetUniformLocation(m_shaderProgram, "model");
    int viewLoc = glGetUniformLocation(m_shaderProgram, "view");
    int projectionLoc = glGetUniformLocation(m_shaderProgram, "projection");
    
    if (modelLoc == -1 || viewLoc == -1 || projectionLoc == -1) {
      throw std::runtime_error("Failed to get uniform locations");
    }
    
    // Set uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix.data());
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, m_viewMatrix.data());
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, m_projectionMatrix.data());
    
    // Set color from properties if available
    float r = 1.0f, g = 0.5f, b = 0.2f, a = 1.0f;
    try {
      auto color = entity->getProperty("外观", "颜色");
      if (std::holds_alternative<std::vector<float>>(color)) {
        auto vec = std::get<std::vector<float>>(color);
        if (vec.size() >= 3) {
          r = vec[0];
          g = vec[1];
          b = vec[2];
        }
        if (vec.size() >= 4) {
          a = vec[3];
        }
      }
    } catch (...) {
      // Use default color if property access fails
    }
    
    int colorLoc = glGetUniformLocation(m_shaderProgram, "color");
    if (colorLoc == -1) {
      throw std::runtime_error("Failed to get color uniform location");
    }
    glUniform4f(colorLoc, r, g, b, a);
    
    // Draw based on render mode
    switch (m_renderMode) {
      case POINTS:
        glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, 0);
        break;
      case WIREFRAME:
        // 使用线框模式渲染三角形
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
      case MESH:
        // 使用线框模式渲染三角形
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
      case SOLID:
      default:
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        break;
    }
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      throw std::runtime_error(std::string("OpenGL error during rendering: ") + std::to_string(error));
    }
    
    // Unbind VAO
    glBindVertexArray(0);
  } catch (const std::exception& e) {
    qDebug() << "Render entity error:" << e.what();
    // 忽略单个实体的渲染错误，继续渲染其他实体
  } catch (...) {
    qDebug() << "Unknown render entity error";
  }
}

void PGLViewport::renderAxes() {
  // Create axis vertices
  float axisLength = 2.0f;
  float vertices[] = {
    // X-axis (red)
    0.0f, 0.0f, 0.0f,
    axisLength, 0.0f, 0.0f,
    // Y-axis (green)
    0.0f, 0.0f, 0.0f,
    0.0f, axisLength, 0.0f,
    // Z-axis (blue)
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, axisLength
  };
  
  // Create VAO and VBO for axes
  unsigned int vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  // Set vertex attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  // Get uniform locations
  int modelLoc = glGetUniformLocation(m_shaderProgram, "model");
  int viewLoc = glGetUniformLocation(m_shaderProgram, "view");
  int projectionLoc = glGetUniformLocation(m_shaderProgram, "projection");
  
  // Set model matrix (identity)
  QMatrix4x4 modelMatrix;
  
  // Set uniforms
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix.data());
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, m_viewMatrix.data());
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, m_projectionMatrix.data());
  
  // Draw X-axis (red)
  glUniform4f(glGetUniformLocation(m_shaderProgram, "color"), 1.0f, 0.0f, 0.0f, 1.0f);
  glDrawArrays(GL_LINES, 0, 2);
  
  // Draw Y-axis (green)
  glUniform4f(glGetUniformLocation(m_shaderProgram, "color"), 0.0f, 1.0f, 0.0f, 1.0f);
  glDrawArrays(GL_LINES, 2, 2);
  
  // Draw Z-axis (blue)
  glUniform4f(glGetUniformLocation(m_shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);
  glDrawArrays(GL_LINES, 4, 2);
  
  // Clean up
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
}
