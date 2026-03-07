#include "geometry.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

// PPropertyGroup implementations
PPropertyValue PPropertyGroup::getProperty(const std::string& key) const {
  auto it = m_properties.find(key);
  return it != m_properties.end() ? it->second : PPropertyValue(0.0f);
}

void PPropertyGroup::setProperty(const std::string& key, const PPropertyValue& value) {
  m_properties[key] = value;
}

// PEdge implementations
PBoundBox PEdge::boundingBox() const {
  PBoundBox box;
  for (const auto& vertex : m_vertices) {
    box.expand(vertex.position());
  }
  return box;
}

// PMesh implementations
PBoundBox PMesh::boundingBox() const {
  PBoundBox box;
  for (const auto& vertex : m_vertices) {
    box.expand(vertex.position());
  }
  return box;
}

// Global variables for C API
static PEntity* g_currentEntity = nullptr;
static PDocument* g_currentDocument = nullptr;
static PMatrix4 g_currentMatrix;
static PBoundBox g_currentBBox;
static PColorRGBA g_currentColor;
static int g_lastError = 0;
static char g_errorMessage[256] = "";

PEntity::PEntity(int id, const std::string& name) 
  : m_id(id), m_name(name), m_transform(PMatrix4::identity()), m_dirty(false) {
  // 添加默认属性组
  m_propertyGroups["常规"] = PPropertyGroup("常规");
  m_propertyGroups["外观"] = PPropertyGroup("外观");
  m_propertyGroups["位姿"] = PPropertyGroup("位姿");
  m_propertyGroups["参数"] = PPropertyGroup("参数");
  
  // 初始化TCC引擎
  m_tccEngine = std::make_unique<PTCCEngine>();
  m_tccEngine->initialize();
}

void PEntity::setErrorCallback(std::function<void(const std::string&)> callback) {
  if (m_tccEngine) {
    m_tccEngine->setErrorCallback(callback);
  }
}

void PEntity::setInfoCallback(std::function<void(const std::string&)> callback) {
  m_infoCallbackFunc = callback;
}

PPropertyValue PEntity::getProperty(const std::string& key) const {
  // 默认从"常规"组获取
  auto it = m_propertyGroups.find("常规");
  if (it != m_propertyGroups.end()) {
    return it->second.getProperty(key);
  }
  return PPropertyValue(0.0f);
}

PPropertyValue PEntity::getProperty(const std::string& groupName, const std::string& key) const {
  auto it = m_propertyGroups.find(groupName);
  if (it != m_propertyGroups.end()) {
    return it->second.getProperty(key);
  }
  return PPropertyValue(0.0f);
}

void PEntity::setProperty(const std::string& key, const PPropertyValue& value) {
  // 默认添加到"常规"组
  m_propertyGroups["常规"].setProperty(key, value);
  m_dirty = true;
}

void PEntity::setProperty(const std::string& groupName, const std::string& key, const PPropertyValue& value) {
  m_propertyGroups[groupName].setProperty(key, value);
  m_dirty = true;
}

PPropertyGroup& PEntity::getPropertyGroup(const std::string& groupName) {
  return m_propertyGroups[groupName];
}

PBoundBox PEntity::boundingBox() const {
  return m_mesh.boundingBox();
}

void PEntity::rebuild() {
  try {
    // Clear existing geometry
    clearVertices();
    clearIndices();
    
    // Set current entity for C API
    g_currentEntity = this;
    
    // Add function declarations to the script
    std::string fullScript = "// Function declarations\n";
    fullScript += "void cgeo_add_vertex(float x, float y, float z);\n";
    fullScript += "void cgeo_add_index(int index);\n";
    fullScript += "void cgeo_clear_vertices();\n";
    fullScript += "void cgeo_clear_indices();\n";
    fullScript += "void cgeo_add_edge_vertex(float x, float y, float z);\n";
    fullScript += "void cgeo_clear_edge_vertices();\n";
    fullScript += "float cgeo_get_prop_float(const char* name);\n";
    fullScript += "void cgeo_set_prop_float(const char* name, float value);\n";
    fullScript += "int cgeo_get_prop_int(const char* name);\n";
    fullScript += "void cgeo_set_prop_int(const char* name, int value);\n";
    fullScript += "int cgeo_get_prop_bool(const char* name);\n";
    fullScript += "void cgeo_set_prop_bool(const char* name, int value);\n";
    fullScript += "void cgeo_get_prop_vector(const char* name, float* values, int size);\n";
    fullScript += "void cgeo_set_prop_vector(const char* name, const float* values, int size);\n";
    fullScript += "void cgeo_set_color(float r, float g, float b, float a);\n";
    fullScript += "void cgeo_get_color(float* r, float* g, float* b, float* a);\n";
    fullScript += "void cgeo_matrix_identity();\n";
    fullScript += "void cgeo_matrix_translate(float x, float y, float z);\n";
    fullScript += "void cgeo_matrix_rotate_x(float angle);\n";
    fullScript += "void cgeo_matrix_rotate_y(float angle);\n";
    fullScript += "void cgeo_matrix_rotate_z(float angle);\n";
    fullScript += "void cgeo_matrix_scale(float x, float y, float z);\n";
    fullScript += "float cgeo_vector_length(float x, float y, float z);\n";
    fullScript += "void cgeo_vector_normalize(float* x, float* y, float* z);\n";
    fullScript += "float cgeo_vector_dot(float x1, float y1, float z1, float x2, float y2, float z2);\n";
    fullScript += "void cgeo_vector_cross(float x1, float y1, float z1, float x2, float y2, float z2, float* out_x, float* out_y, float* out_z);\n";
    fullScript += "void cgeo_bbox_reset();\n";
    fullScript += "void cgeo_bbox_add_point(float x, float y, float z);\n";
    fullScript += "void cgeo_bbox_get_min(float* x, float* y, float* z);\n";
    fullScript += "void cgeo_bbox_get_max(float* x, float* y, float* z);\n";
    fullScript += "void cgeo_bbox_get_center(float* x, float* y, float* z);\n";
    fullScript += "void cgeo_bbox_get_size(float* x, float* y, float* z);\n";
    fullScript += "void cgeo_save_document(const char* filename);\n";
    fullScript += "void cgeo_load_document(const char* filename);\n";
    fullScript += "int cgeo_get_last_error();\n";
    fullScript += "const char* cgeo_get_error_message();\n\n";

    fullScript += m_scriptSource;
    
    // Re-initialize TCC engine to avoid function redefinition errors
    m_tccEngine = std::make_unique<PTCCEngine>();
    
    // Set error callback
    m_tccEngine->setErrorCallback([this](const std::string& message) {
      if (m_errorCallbackFunc) {
        m_errorCallbackFunc(message);
      }
    });
    
    bool initialized = m_tccEngine->initialize();
    if (!initialized) {
      std::string error = "TCC引擎初始化失败: " + m_tccEngine->errorMessage();
      if (m_errorCallbackFunc) {
        m_errorCallbackFunc(error);
      }
      return;
    }
    
    // Compile and execute script
    if (m_tccEngine) {
      bool compiled = m_tccEngine->compile(fullScript);
      if (compiled) {
        bool executed = m_tccEngine->execute();
        if (!executed) {
          // Handle execution error
          std::string error = "脚本执行错误: " + m_tccEngine->errorMessage();
          if (m_errorCallbackFunc) {
            m_errorCallbackFunc(error);
          }
        } else {
          // Log entity geometry information to UI console
          std::string info = "几何体生成成功: " + name() + "\n" +
                            "顶点数量: " + std::to_string(mesh().vertices().size()) + "\n" +
                            "索引数量: " + std::to_string(mesh().indices().size());
          if (m_infoCallbackFunc) {
            m_infoCallbackFunc(info);
          }
        }
      } else {
        // Handle compilation error
        std::string error = "脚本编译错误: " + m_tccEngine->errorMessage();
        if (m_errorCallbackFunc) {
          m_errorCallbackFunc(error);
        }
      }
    } else {
      std::string error = "TCC引擎未初始化!";
      if (m_errorCallbackFunc) {
        m_errorCallbackFunc(error);
      }
    }
  } catch (const std::exception& e) {
    std::string error = "异常: " + std::string(e.what());
    if (m_errorCallbackFunc) {
      m_errorCallbackFunc(error);
    }
  } catch (...) {
    std::string error = "未知异常";
    if (m_errorCallbackFunc) {
      m_errorCallbackFunc(error);
    }
  }
  
  m_dirty = false;
}

std::shared_ptr<PEntity> PDocument::createEntity(const std::string& name) {
  int id = m_nextId++;
  auto entity = std::make_shared<PEntity>(id, name);
  m_entities[id] = entity;
  return entity;
}

void PDocument::removeEntity(int id) {
  m_entities.erase(id);
}

std::shared_ptr<PEntity> PDocument::getEntity(int id) const {
  auto it = m_entities.find(id);
  return it != m_entities.end() ? it->second : nullptr;
}

void PDocument::save(const std::string& filename) const {
  try {
    nlohmann::json root;
    root["version"] = "1.0";
    
    nlohmann::json entitiesArray = nlohmann::json::array();
    for (const auto& [id, entity] : m_entities) {
      try {
        nlohmann::json entityJson;
        entityJson["id"] = entity->id();
        entityJson["name"] = entity->name();
        
        nlohmann::json propertyGroupsJson;
        for (const auto& [groupName, group] : entity->propertyGroups()) {
          try {
            nlohmann::json groupJson;
            groupJson["name"] = group.name();
            nlohmann::json propertiesJson;
            for (const auto& [key, value] : group) {
              try {
                std::visit([&propertiesJson, &key](auto&& arg) {
                  propertiesJson[key] = arg;
                }, value);
              } catch (const std::exception& e) {
                // 忽略单个属性的错误，继续处理其他属性
              }
            }
            groupJson["properties"] = propertiesJson;
            propertyGroupsJson[groupName] = groupJson;
          } catch (const std::exception& e) {
            // 忽略单个属性组的错误，继续处理其他属性组
          }
        }
        entityJson["propertyGroups"] = propertyGroupsJson;
        
        try {
          entityJson["script"] = entity->scriptSource();
        } catch (const std::exception& e) {
          // 忽略脚本保存错误
        }
        
        // Save transform
        try {
          nlohmann::json transformJson;
          const auto& transform = entity->transform();
          for (int i = 0; i < 16; i++) {
            transformJson.push_back(transform.data()[i]);
          }
          entityJson["transform"] = transformJson;
        } catch (const std::exception& e) {
          // 忽略变换保存错误
        }
        
        entitiesArray.push_back(entityJson);
      } catch (const std::exception& e) {
        // 忽略单个实体的错误，继续处理其他实体
      }
    }
    
    root["entities"] = entitiesArray;
    
    std::ofstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("无法打开文件进行写入: " + filename);
    }
    
    try {
      file << root.dump(2);
    } catch (const std::exception& e) {
      throw std::runtime_error("文件写入错误: " + std::string(e.what()));
    }
  } catch (const std::exception& e) {
    // 这里可以添加错误回调，将错误信息传递给UI
    std::cerr << "文件保存错误: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "文件保存未知错误" << std::endl;
  }
}

PBoundBox PDocument::boundingBox() const {
  PBoundBox box;
  for (const auto& [id, entity] : m_entities) {
    box.expand(entity->boundingBox());
  }
  return box;
}

void PDocument::load(const std::string& filename) {
  try {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("无法打开文件: " + filename);
    }
    
    nlohmann::json root;
    try {
      file >> root;
    } catch (const nlohmann::json::parse_error& e) {
      throw std::runtime_error("JSON解析错误: " + std::string(e.what()));
    }
    
    m_entities.clear();
    m_nextId = 1;
    
    if (root.contains("entities")) {
      for (const auto& entityJson : root["entities"]) {
        try {
          int id = entityJson["id"];
          std::string name = entityJson["name"];
          auto entity = std::make_shared<PEntity>(id, name);
          
          if (entityJson.contains("propertyGroups")) {
            for (const auto& [groupName, groupJson] : entityJson["propertyGroups"].items()) {
              if (groupJson.contains("properties")) {
                for (const auto& [key, value] : groupJson["properties"].items()) {
                  try {
                    if (value.is_number_float()) {
                      entity->setProperty(groupName, key, PPropertyValue(value.get<float>()));
                    } else if (value.is_number_integer()) {
                      entity->setProperty(groupName, key, PPropertyValue(value.get<int>()));
                    } else if (value.is_boolean()) {
                      entity->setProperty(groupName, key, PPropertyValue(value.get<bool>()));
                    } else if (value.is_string()) {
                      entity->setProperty(groupName, key, PPropertyValue(value.get<std::string>()));
                    } else if (value.is_array()) {
                      // 处理数组类型属性
                      std::vector<float> vec;
                      for (const auto& elem : value) {
                        if (elem.is_number()) {
                          vec.push_back(elem.get<float>());
                        }
                      }
                      entity->setProperty(groupName, key, PPropertyValue(vec));
                    }
                  } catch (const std::exception& e) {
                    // 忽略单个属性的错误，继续处理其他属性
                  }
                }
              }
            }
          } else if (entityJson.contains("properties")) {
            // 兼容旧格式
            for (const auto& [key, value] : entityJson["properties"].items()) {
              try {
                if (value.is_number_float()) {
                  entity->setProperty(key, PPropertyValue(value.get<float>()));
                } else if (value.is_number_integer()) {
                  entity->setProperty(key, PPropertyValue(value.get<int>()));
                } else if (value.is_boolean()) {
                  entity->setProperty(key, PPropertyValue(value.get<bool>()));
                } else if (value.is_string()) {
                  entity->setProperty(key, PPropertyValue(value.get<std::string>()));
                } else if (value.is_array()) {
                  // 处理数组类型属性
                  std::vector<float> vec;
                  for (const auto& elem : value) {
                    if (elem.is_number()) {
                      vec.push_back(elem.get<float>());
                    }
                  }
                  entity->setProperty(key, PPropertyValue(vec));
                }
              } catch (const std::exception& e) {
                // 忽略单个属性的错误，继续处理其他属性
              }
            }
          }
          
          if (entityJson.contains("script")) {
            try {
              entity->setScriptSource(entityJson["script"]);
            } catch (const std::exception& e) {
              // 忽略脚本加载错误
            }
          }
          
          // Load transform
          if (entityJson.contains("transform")) {
            try {
              PMatrix4 transform;
              const auto& transformJson = entityJson["transform"];
              if (transformJson.is_array() && transformJson.size() == 16) {
                for (int i = 0; i < 16; i++) {
                  transform.data()[i] = transformJson[i].get<float>();
                }
                entity->setTransform(transform);
              }
            } catch (const std::exception& e) {
              // 忽略变换加载错误
            }
          }
          
          m_entities[id] = entity;
          if (id >= m_nextId) {
            m_nextId = id + 1;
          }
          
          // Rebuild geometry after loading
          entity->rebuild();
          
        } catch (const std::exception& e) {
          // 忽略单个实体的错误，继续处理其他实体
        }
      }
    }
  } catch (const std::exception& e) {
    // 这里可以添加错误回调，将错误信息传递给UI
    std::cerr << "文件加载错误: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "文件加载未知错误" << std::endl;
  }
}

// C API implementations
extern "C" {
  // Entity management
  void cgeo_add_vertex(float x, float y, float z) {
    if (g_currentEntity) {
      g_currentEntity->addVertex(PVertex(PVector3(x, y, z)));
    }
  }
  
  void cgeo_add_index(int index) {
    if (g_currentEntity) {
      g_currentEntity->addIndex(index);
    }
  }
  
  void cgeo_clear_vertices() {
    if (g_currentEntity) {
      g_currentEntity->clearVertices();
    }
  }
  
  void cgeo_clear_indices() {
    if (g_currentEntity) {
      g_currentEntity->clearIndices();
    }
  }
  
  // Property management
  float cgeo_get_prop_float(const char* name) {
    if (g_currentEntity) {
      auto value = g_currentEntity->getProperty(name);
      if (std::holds_alternative<float>(value)) {
        return std::get<float>(value);
      }
    }
    return 0.0f;
  }
  
  void cgeo_set_prop_float(const char* name, float value) {
    if (g_currentEntity) {
      g_currentEntity->setProperty(name, PPropertyValue(value));
    }
  }
  
  int cgeo_get_prop_int(const char* name) {
    if (g_currentEntity) {
      auto value = g_currentEntity->getProperty(name);
      if (std::holds_alternative<int>(value)) {
        return std::get<int>(value);
      }
    }
    return 0;
  }
  
  void cgeo_set_prop_int(const char* name, int value) {
    if (g_currentEntity) {
      g_currentEntity->setProperty(name, PPropertyValue(value));
    }
  }
  
  bool cgeo_get_prop_bool(const char* name) {
    if (g_currentEntity) {
      auto value = g_currentEntity->getProperty(name);
      if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
      }
    }
    return false;
  }
  
  void cgeo_set_prop_bool(const char* name, bool value) {
    if (g_currentEntity) {
      g_currentEntity->setProperty(name, PPropertyValue(value));
    }
  }
  
  // Matrix operations
  void cgeo_matrix_identity() {
    g_currentMatrix = PMatrix4::identity();
  }
  
  void cgeo_matrix_translate(float x, float y, float z) {
    g_currentMatrix *= PMatrix4::translate(x, y, z);
  }
  
  void cgeo_matrix_rotate_x(float angle) {
    g_currentMatrix *= PMatrix4::rotateX(angle);
  }
  
  void cgeo_matrix_rotate_y(float angle) {
    g_currentMatrix *= PMatrix4::rotateY(angle);
  }
  
  void cgeo_matrix_rotate_z(float angle) {
    g_currentMatrix *= PMatrix4::rotateZ(angle);
  }
  
  void cgeo_matrix_scale(float x, float y, float z) {
    g_currentMatrix *= PMatrix4::scale(x, y, z);
  }
  
  // Vector operations
  float cgeo_vector_length(float x, float y, float z) {
    PVector3 vec(x, y, z);
    return vec.length();
  }
  
  void cgeo_vector_normalize(float* x, float* y, float* z) {
    if (x && y && z) {
      PVector3 vec(*x, *y, *z);
      PVector3 normalized = vec.normalize();
      *x = normalized.x();
      *y = normalized.y();
      *z = normalized.z();
    }
  }
  
  float cgeo_vector_dot(float x1, float y1, float z1, float x2, float y2, float z2) {
    PVector3 vec1(x1, y1, z1);
    PVector3 vec2(x2, y2, z2);
    return vec1.dot(vec2);
  }
  
  void cgeo_vector_cross(float x1, float y1, float z1, float x2, float y2, float z2, float* out_x, float* out_y, float* out_z) {
    if (out_x && out_y && out_z) {
      PVector3 vec1(x1, y1, z1);
      PVector3 vec2(x2, y2, z2);
      PVector3 cross = vec1.cross(vec2);
      *out_x = cross.x();
      *out_y = cross.y();
      *out_z = cross.z();
    }
  }
  
  // Bounding box operations
  void cgeo_bbox_reset() {
    g_currentBBox = PBoundBox();
  }
  
  void cgeo_bbox_add_point(float x, float y, float z) {
    g_currentBBox.expand(PVector3(x, y, z));
  }
  
  void cgeo_bbox_get_min(float* x, float* y, float* z) {
    if (x && y && z) {
      PVector3 min = g_currentBBox.min();
      *x = min.x();
      *y = min.y();
      *z = min.z();
    }
  }
  
  void cgeo_bbox_get_max(float* x, float* y, float* z) {
    if (x && y && z) {
      PVector3 max = g_currentBBox.max();
      *x = max.x();
      *y = max.y();
      *z = max.z();
    }
  }
  
  void cgeo_bbox_get_center(float* x, float* y, float* z) {
    if (x && y && z) {
      PVector3 center = g_currentBBox.center();
      *x = center.x();
      *y = center.y();
      *z = center.z();
    }
  }
  
  void cgeo_bbox_get_size(float* x, float* y, float* z) {
    if (x && y && z) {
      PVector3 size = g_currentBBox.size();
      *x = size.x();
      *y = size.y();
      *z = size.z();
    }
  }
  
  // Edge management
  void cgeo_add_edge_vertex(float x, float y, float z) {
    // Note: This is a placeholder. In a real implementation, you would need to
    // manage edges separately, similar to how vertices and indices are managed.
    // For now, we'll just add it as a regular vertex.
    if (g_currentEntity) {
      g_currentEntity->addVertex(PVertex(PVector3(x, y, z)));
    }
  }
  
  void cgeo_clear_edge_vertices() {
    // Note: This is a placeholder. In a real implementation, you would need to
    // manage edges separately, similar to how vertices and indices are managed.
    // For now, we'll just clear regular vertices.
    if (g_currentEntity) {
      g_currentEntity->clearVertices();
    }
  }
  
  // Property management for vector types
  void cgeo_get_prop_vector(const char* name, float* values, int size) {
    if (g_currentEntity && values) {
      auto value = g_currentEntity->getProperty(name);
      if (std::holds_alternative<std::vector<float>>(value)) {
        auto vec = std::get<std::vector<float>>(value);
        for (int i = 0; i < size && i < static_cast<int>(vec.size()); ++i) {
          values[i] = vec[i];
        }
      }
    }
  }
  
  void cgeo_set_prop_vector(const char* name, const float* values, int size) {
    if (g_currentEntity && values) {
      std::vector<float> vec(values, values + size);
      g_currentEntity->setProperty(name, PPropertyValue(vec));
    }
  }
  
  // Color operations
  void cgeo_set_color(float r, float g, float b, float a) {
    g_currentColor = PColorRGBA(r, g, b, a);
  }
  
  void cgeo_get_color(float* r, float* g, float* b, float* a) {
    if (r && g && b && a) {
      *r = g_currentColor.r();
      *g = g_currentColor.g();
      *b = g_currentColor.b();
      *a = g_currentColor.a();
    }
  }
  
  // Document operations
  void cgeo_save_document(const char* filename) {
    if (g_currentDocument) {
      g_currentDocument->save(filename);
    }
  }
  
  void cgeo_load_document(const char* filename) {
    if (g_currentDocument) {
      g_currentDocument->load(filename);
    }
  }
  
  // Error handling
  int cgeo_get_last_error() {
    return g_lastError;
  }
  
  const char* cgeo_get_error_message() {
    return g_errorMessage;
  }
}
