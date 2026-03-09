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
    m_cameraPosition(0.0f, 0.0f, 0.0f), // 初始相机位置
    m_perspective(true), // 默认使用透视投影
    m_orthoSize(5.0f), // 默认正交投影大小
    m_renderMode(SOLID), // 默认使用实体渲染
    m_shaderProgram(0), m_vertexShader(0), m_fragmentShader(0),
    m_backgroundColor(26, 26, 26) {} // 默认背景色

void PGLViewport::updateCamera() {
  // 重置视图矩阵
  m_viewMatrix.setToIdentity();
  
  // 应用相机变换
  m_viewMatrix.translate(m_cameraPosition); // 应用平移
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
    float scale = m_cameraDistance * 0.001f; // 平移速度与相机距离相关
    m_cameraPosition.setX(m_cameraPosition.x() - delta.x() * scale);
    m_cameraPosition.setY(m_cameraPosition.y() + delta.y() * scale);
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
  m_cameraPosition = QVector3D(0.0f, 0.0f, 0.0f); // 重置相机位置
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

void PGLViewport::setBackgroundColor(const QColor& color) {
  m_backgroundColor = color;
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
  glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), m_backgroundColor.blueF(), m_backgroundColor.alphaF());
  
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
  try {
    // Update clear color
    glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), m_backgroundColor.blueF(), m_backgroundColor.alphaF());
    
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use shader program
    glUseProgram(m_shaderProgram);
    
    // Update buffers
    updateBuffers();
    
    // Render entities
    if (m_document) {
      for (const auto& [id, entity] : m_document->entities()) {
        try {
          renderEntity(entity);
        } catch (const std::exception& e) {
          qDebug() << "Render error for entity" << id << ":" << e.what();
          // 忽略单个实体的渲染错误，继续处理其他实体
        } catch (...) {
          qDebug() << "Unknown render error for entity" << id;
        }
      }
    }
    
    // Render axes if enabled
    if (m_showAxes) {
      try {
        renderAxes();
      } catch (const std::exception& e) {
        qDebug() << "Render axes error:" << e.what();
      } catch (...) {
        qDebug() << "Unknown render axes error";
      }
    }
  } catch (const std::exception& e) {
    qDebug() << "PaintGL error:" << e.what();
  } catch (...) {
    qDebug() << "Unknown PaintGL error";
  }
}

void PGLViewport::compileShaders() {
  try {
    // Vertex shader source
    const char* vertexShaderSource = "#version 330 core\n"
                                    "layout (location = 0) in vec3 aPos;\n"
                                    "layout (location = 1) in vec3 aNormal;\n"
                                    "\n"
                                    "uniform mat4 model;\n"
                                    "uniform mat4 view;\n"
                                    "uniform mat4 projection;\n"
                                    "\n"
                                    "out vec3 Normal;\n"
                                    "out vec3 FragPos;\n"
                                    "\n"
                                    "void main() {\n"
                                    "  gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                                    "  FragPos = vec3(model * vec4(aPos, 1.0));\n"
                                    "  Normal = mat3(transpose(inverse(model))) * aNormal;\n"
                                    "}\n";
    
    // Fragment shader source
    const char* fragmentShaderSource = "#version 330 core\n"
                                      "out vec4 FragColor;\n"
                                      "\n"
                                      "uniform vec4 color;\n"
                                      "\n"
                                      "in vec3 Normal;\n"
                                      "in vec3 FragPos;\n"
                                      "\n"
                                      "uniform vec3 lightPos = vec3(1.0, 1.0, 1.0);\n"
                                      "uniform vec3 viewPos = vec3(0.0, 0.0, 5.0);\n"
                                      "uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);\n"
                                      "\n"
                                      "void main() {\n"
                                      "  // 环境光\n"
                                      "  float ambientStrength = 0.1;\n"
                                      "  vec3 ambient = ambientStrength * lightColor;\n"
                                      "  \n"
                                      "  // 漫反射\n"
                                      "  vec3 norm = normalize(Normal);\n"
                                      "  vec3 lightDir = normalize(lightPos - FragPos);\n"
                                      "  float diff = max(dot(norm, lightDir), 0.0);\n"
                                      "  vec3 diffuse = diff * lightColor;\n"
                                      "  \n"
                                      "  // 镜面反射\n"
                                      "  float specularStrength = 0.5;\n"
                                      "  vec3 viewDir = normalize(viewPos - FragPos);\n"
                                      "  vec3 reflectDir = reflect(-lightDir, norm);\n"
                                      "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
                                      "  vec3 specular = specularStrength * spec * lightColor;\n"
                                      "  \n"
                                      "  // 最终颜色\n"
                                      "  vec3 result = (ambient + diffuse + specular) * vec3(color);\n"
                                      "  FragColor = vec4(result, color.w);\n"
                                      "}\n";
    
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

// Helper function to compute hash for vertex data
size_t computeVertexHash(const std::vector<PVertex>& vertices) {
  std::size_t hash = 0;
  for (const auto& vertex : vertices) {
    PVector3 pos = vertex.position();
    PVector3 norm = vertex.normal();
    hash ^= std::hash<float>()(pos.x()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<float>()(pos.y()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<float>()(pos.z()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<float>()(norm.x()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<float>()(norm.y()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<float>()(norm.z()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  }
  return hash;
}

// Helper function to compute hash for index data
size_t computeIndexHash(const std::vector<unsigned int>& indices) {
  std::size_t hash = 0;
  for (unsigned int index : indices) {
    hash ^= std::hash<unsigned int>()(index) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  }
  return hash;
}

void PGLViewport::updateBuffers() {
  if (!m_document) return;
  
  try {
    for (const auto& [id, entity] : m_document->entities()) {
      try {
        const auto& meshVertices = entity->mesh().vertices();
        const auto& indices = entity->mesh().indices();
        
        // Compute hashes for current geometry
        size_t vertexHash = computeVertexHash(meshVertices);
        size_t indexHash = computeIndexHash(indices);
        
        // Check if geometry has changed
        bool geometryChanged = true;
        if (m_vertexHashCache.find(id) != m_vertexHashCache.end() && 
            m_indexHashCache.find(id) != m_indexHashCache.end()) {
          if (m_vertexHashCache[id] == vertexHash && m_indexHashCache[id] == indexHash) {
            geometryChanged = false;
          }
        }
        
        if (!geometryChanged) {
          // Geometry hasn't changed, skip buffer update
          continue;
        }
        
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
        if (!meshVertices.empty()) {
          // Create a temporary vector to hold vertex positions and normals
          std::vector<float> vertices;
          vertices.reserve(meshVertices.size() * 6); // 3 for position + 3 for normal
          for (const auto& vertex : meshVertices) {
            PVector3 pos = vertex.position();
            PVector3 norm = vertex.normal();
            // Add position
            vertices.push_back(pos.x());
            vertices.push_back(pos.y());
            vertices.push_back(pos.z());
            // Add normal
            vertices.push_back(norm.x());
            vertices.push_back(norm.y());
            vertices.push_back(norm.z());
          }
          
          glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
          
          // Bind EBO and upload data
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos[id]);
          if (!indices.empty()) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
          }
          
          // Set vertex attributes
          // Position attribute
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
          glEnableVertexAttribArray(0);
          // Normal attribute
          glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
          glEnableVertexAttribArray(1);
        }
        
        // Unbind VAO
        glBindVertexArray(0);
        
        // Update cache
        m_vertexHashCache[id] = vertexHash;
        m_indexHashCache[id] = indexHash;
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
    
    // Calculate camera position from view matrix
    QMatrix4x4 viewInverse = m_viewMatrix.inverted();
    QVector3D cameraPos = viewInverse.column(3).toVector3D();
    
    // Set light position to camera position (light follows camera)
    int lightPosLoc = glGetUniformLocation(m_shaderProgram, "lightPos");
    if (lightPosLoc != -1) {
      glUniform3f(lightPosLoc, cameraPos.x(), cameraPos.y(), cameraPos.z());
    }
    
    // Set view position to camera position
    int viewPosLoc = glGetUniformLocation(m_shaderProgram, "viewPos");
    if (viewPosLoc != -1) {
      glUniform3f(viewPosLoc, cameraPos.x(), cameraPos.y(), cameraPos.z());
    }
    
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
        // 先渲染实体
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        // 再渲染线框
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUniform4f(glGetUniformLocation(m_shaderProgram, "color"), 0.0f, 0.0f, 0.0f, 1.0f); // 黑色线框
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

unsigned int PGLViewport::createSimpleShaderProgram() {
  // 顶点着色器源码
  const char* vertexShaderSource = "#version 330 core\n"
                                  "layout (location = 0) in vec3 aPos;\n"
                                  "\n"
                                  "uniform mat4 model;\n"
                                  "uniform mat4 view;\n"
                                  "uniform mat4 projection;\n"
                                  "\n"
                                  "void main() {\n"
                                  "  gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                                  "}\n";
  
  // 片段着色器源码
  const char* fragmentShaderSource = "#version 330 core\n"
                                    "out vec4 FragColor;\n"
                                    "\n"
                                    "uniform vec4 color;\n"
                                    "\n"
                                    "void main() {\n"
                                    "  FragColor = color;\n"
                                    "}\n";
  
  // 创建顶点着色器
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);
  
  // 检查顶点着色器错误
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    qDebug() << "Vertex shader compilation failed:" << infoLog;
    return 0;
  }
  
  // 创建片段着色器
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);
  
  // 检查片段着色器错误
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    qDebug() << "Fragment shader compilation failed:" << infoLog;
    return 0;
  }
  
  // 创建着色器程序
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  
  // 检查链接错误
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
    qDebug() << "Shader program linking failed:" << infoLog;
    return 0;
  }
  
  // 删除着色器
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  
  return shaderProgram;
}

void PGLViewport::renderAxes() {
  // 创建简单的着色器程序（无光照）
  unsigned int simpleShaderProgram = createSimpleShaderProgram();
  if (simpleShaderProgram == 0) {
    return;
  }
  
  // 使用简单着色器程序
  glUseProgram(simpleShaderProgram);
  
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
  int modelLoc = glGetUniformLocation(simpleShaderProgram, "model");
  int viewLoc = glGetUniformLocation(simpleShaderProgram, "view");
  int projectionLoc = glGetUniformLocation(simpleShaderProgram, "projection");
  
  // Set model matrix (identity)
  QMatrix4x4 modelMatrix;
  
  // Set uniforms
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix.data());
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, m_viewMatrix.data());
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, m_projectionMatrix.data());
  
  // Draw X-axis (red)
  glUniform4f(glGetUniformLocation(simpleShaderProgram, "color"), 1.0f, 0.0f, 0.0f, 1.0f);
  glDrawArrays(GL_LINES, 0, 2);
  
  // Draw Y-axis (green)
  glUniform4f(glGetUniformLocation(simpleShaderProgram, "color"), 0.0f, 1.0f, 0.0f, 1.0f);
  glDrawArrays(GL_LINES, 2, 2);
  
  // Draw Z-axis (blue)
  glUniform4f(glGetUniformLocation(simpleShaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);
  glDrawArrays(GL_LINES, 4, 2);
  
  // Clean up
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(simpleShaderProgram);
  
  // 切换回原来的着色器程序
  glUseProgram(m_shaderProgram);
}
