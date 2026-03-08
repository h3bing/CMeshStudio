#include "geometry.h"
#include "tccengine.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <cmath>

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
}

void PEntity::setErrorCallback(std::function<void(const std::string&)> callback) {
  m_errorCallbackFunc = callback;
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
  // 尝试从其他属性组中查找
  for (const auto& [groupName, group] : m_propertyGroups) {
    if (groupName == "常规") continue;
    const auto& properties = group.properties();
    auto propIt = properties.find(key);
    if (propIt != properties.end()) {
      return propIt->second;
    }
  }
  // 返回一个空的 variant，表示属性不存在
  return PPropertyValue();
}

PPropertyValue PEntity::getProperty(const std::string& groupName, const std::string& key) const {
  auto it = m_propertyGroups.find(groupName);
  if (it != m_propertyGroups.end()) {
    return it->second.getProperty(key);
  }
  // 返回一个空的 variant，表示属性不存在
  return PPropertyValue();
}

void PEntity::setProperty(const std::string& key, const PPropertyValue& value) {
  // 默认添加到"常规"组
  m_propertyGroups["常规"].setProperty(key, value);
  m_dirty = true;
}

void PEntity::setProperty(const std::string& groupName, const std::string& key, const PPropertyValue& value) {
  // 确保属性组存在，并且名称正确
  if (m_propertyGroups.find(groupName) == m_propertyGroups.end()) {
    m_propertyGroups.insert({groupName, PPropertyGroup(groupName)});
  }
  m_propertyGroups[groupName].setProperty(key, value);
  m_dirty = true;
}

PPropertyGroup& PEntity::getPropertyGroup(const std::string& groupName) {
  // 确保属性组存在，并且名称正确
  if (m_propertyGroups.find(groupName) == m_propertyGroups.end()) {
    m_propertyGroups.insert({groupName, PPropertyGroup(groupName)});
  }
  return m_propertyGroups[groupName];
}

PBoundBox PEntity::boundingBox() const {
  return m_mesh.boundingBox();
}

void PEntity::rebuild() {
  // Set current entity for C API
  g_currentEntity = this;
  
  try {
    // Clear existing geometry
    clearVertices();
    clearIndices();
    
    // Add function declarations to the script
    std::string fullScript = "// Function declarations\n";
    fullScript += "float cgeo_sin(float x);\n";
    fullScript += "float cgeo_cos(float x);\n";
    fullScript += "float cgeo_sqrt(float x);\n\n";
    fullScript += "void cgeo_add_vertex(float x, float y, float z);\n";
    fullScript += "void cgeo_set_normal(float x, float y, float z);\n";
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
    
    // Get global TCC engine
    PTCCEngine* tccEngine = PTCCEngineManager::getInstance();
    
    // Set error callback for this entity
    PTCCEngineManager::setCurrentErrorCallback([this](const std::string& message) {
      if (m_errorCallbackFunc) {
        m_errorCallbackFunc(message);
      }
    });
    
    // Compile and execute script
    if (tccEngine) {
      // Log script compilation start
      if (m_infoCallbackFunc) {
        m_infoCallbackFunc("开始编译脚本: " + name());
      }
      
      // Log script content for debugging
      if (m_infoCallbackFunc) {
        m_infoCallbackFunc("脚本内容: " + fullScript);
      }
      
      bool compiled = tccEngine->compile(fullScript);
      if (compiled) {
        if (m_infoCallbackFunc) {
          m_infoCallbackFunc("脚本编译成功: " + name());
        }
        
        bool executed = tccEngine->execute();
        if (!executed) {
          // Handle execution error
          std::string error = "脚本执行错误: " + tccEngine->errorMessage();
          if (m_errorCallbackFunc) {
            m_errorCallbackFunc(error);
          }
          if (m_infoCallbackFunc) {
            m_infoCallbackFunc(error);
          }
        } else {
          // Log entity geometry information to UI console
          std::string info = "几何体生成成功: " + name() + "\n" +
                            "顶点数量: " + std::to_string(mesh().vertices().size()) + "\n" +
                            "索引数量: " + std::to_string(mesh().indices().size());
          if (m_infoCallbackFunc) {
            m_infoCallbackFunc(info);
          }
          
          // Log vertex coordinates for debugging
          if (m_infoCallbackFunc && mesh().vertices().size() > 0) {
            std::string verticesInfo = "顶点坐标: ";
            for (size_t i = 0; i < std::min(size_t(5), mesh().vertices().size()); i++) {
              const auto& vertex = mesh().vertices()[i];
              PVector3 pos = vertex.position();
              verticesInfo += "(" + std::to_string(pos.x()) + ", " + std::to_string(pos.y()) + ", " + std::to_string(pos.z()) + ") ";
            }
            if (mesh().vertices().size() > 5) {
              verticesInfo += "...";
            }
            m_infoCallbackFunc(verticesInfo);
          }
        }
      } else {
        // Handle compilation error
        std::string error = "脚本编译错误: " + tccEngine->errorMessage();
        if (m_errorCallbackFunc) {
          m_errorCallbackFunc(error);
        }
        if (m_infoCallbackFunc) {
          m_infoCallbackFunc(error);
        }
      }
    } else {
      std::string error = "TCC引擎未初始化!";
      if (m_errorCallbackFunc) {
        m_errorCallbackFunc(error);
      }
    }
    
    // Clear error callback
    PTCCEngineManager::clearCurrentErrorCallback();
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
  
  // Reset current entity
  g_currentEntity = nullptr;
  
  m_dirty = false;
}

void PEntity::load(const std::string& filename) {
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
    
    // 加载实体数据
    if (root.contains("entities") && root["entities"].is_array() && root["entities"].size() > 0) {
      const auto& entityJson = root["entities"][0];
      
      // 加载名称
      if (entityJson.contains("name")) {
        m_name = entityJson["name"];
      }
      
      // 加载属性组
      if (entityJson.contains("propertyGroups")) {
        m_propertyGroups.clear();
        for (const auto& [groupName, groupJson] : entityJson["propertyGroups"].items()) {
          // 首先创建属性组
          m_propertyGroups.insert({groupName, PPropertyGroup(groupName)});
          
          if (groupJson.contains("properties")) {
            for (const auto& [key, value] : groupJson["properties"].items()) {
              try {
                if (value.is_number_float()) {
                  setProperty(groupName, key, PPropertyValue(value.get<float>()));
                } else if (value.is_number_integer()) {
                  setProperty(groupName, key, PPropertyValue(value.get<int>()));
                } else if (value.is_boolean()) {
                  setProperty(groupName, key, PPropertyValue(value.get<bool>()));
                } else if (value.is_string()) {
                  setProperty(groupName, key, PPropertyValue(value.get<std::string>()));
                } else if (value.is_array()) {
                  // 处理数组类型属性
                  std::vector<float> vec;
                  for (const auto& elem : value) {
                    if (elem.is_number()) {
                      vec.push_back(elem.get<float>());
                    }
                  }
                  setProperty(groupName, key, PPropertyValue(vec));
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
              setProperty(key, PPropertyValue(value.get<float>()));
            } else if (value.is_number_integer()) {
              setProperty(key, PPropertyValue(value.get<int>()));
            } else if (value.is_boolean()) {
              setProperty(key, PPropertyValue(value.get<bool>()));
            } else if (value.is_string()) {
              setProperty(key, PPropertyValue(value.get<std::string>()));
            } else if (value.is_array()) {
              // 处理数组类型属性
              std::vector<float> vec;
              for (const auto& elem : value) {
                if (elem.is_number()) {
                  vec.push_back(elem.get<float>());
                }
              }
              setProperty(key, PPropertyValue(vec));
            }
          } catch (const std::exception& e) {
            // 忽略单个属性的错误，继续处理其他属性
          }
        }
      }
      
      // 加载脚本
      if (entityJson.contains("script")) {
        try {
          setScriptSource(entityJson["script"]);
        } catch (const std::exception& e) {
          // 忽略脚本加载错误
        }
      }
      
      // 加载变换矩阵
      if (entityJson.contains("transform")) {
        try {
          const auto& transformJson = entityJson["transform"];
          if (transformJson.is_array() && transformJson.size() == 16) {
            for (int i = 0; i < 16; i++) {
              m_transform.data()[i] = transformJson[i].get<float>();
            }
          }
        } catch (const std::exception& e) {
          // 忽略变换矩阵加载错误
        }
      }
      
      // 重建几何
      rebuild();
      
      // 调用信息回调
      if (m_infoCallbackFunc) {
        m_infoCallbackFunc("加载实体文件成功: " + filename);
      }
    } else {
      throw std::runtime_error("无效的实体文件: 缺少entities数组");
    }
  } catch (const std::exception& e) {
    // 调用错误回调
    if (m_errorCallbackFunc) {
      m_errorCallbackFunc(std::string("加载实体文件错误: ") + e.what());
    }
  } catch (...) {
    // 调用错误回调
    if (m_errorCallbackFunc) {
      m_errorCallbackFunc("加载实体文件未知错误");
    }
  }
}

void PEntity::save(const std::string& filename) const {
  try {
    nlohmann::json root;
    root["version"] = "1.0";
    
    nlohmann::json entities;
    nlohmann::json entityJson;
    
    entityJson["id"] = m_id;
    entityJson["name"] = m_name;
    
    // 保存属性组
    nlohmann::json propertyGroups;
    for (const auto& [groupName, group] : m_propertyGroups) {
      nlohmann::json groupJson;
      groupJson["name"] = group.name();
      
      nlohmann::json properties;
      for (const auto& [key, value] : group.properties()) {
        if (std::holds_alternative<float>(value)) {
          properties[key] = std::get<float>(value);
        } else if (std::holds_alternative<int>(value)) {
          properties[key] = std::get<int>(value);
        } else if (std::holds_alternative<bool>(value)) {
          properties[key] = std::get<bool>(value);
        } else if (std::holds_alternative<std::string>(value)) {
          properties[key] = std::get<std::string>(value);
        } else if (std::holds_alternative<std::vector<float>>(value)) {
          properties[key] = std::get<std::vector<float>>(value);
        }
      }
      groupJson["properties"] = properties;
      propertyGroups[groupName] = groupJson;
    }
    entityJson["propertyGroups"] = propertyGroups;
    
    // 保存脚本
    entityJson["script"] = m_scriptSource;
    
    // 保存变换矩阵
    nlohmann::json transform;
    for (int i = 0; i < 16; i++) {
      transform.push_back(m_transform.data()[i]);
    }
    entityJson["transform"] = transform;
    
    entities.push_back(entityJson);
    root["entities"] = entities;
    
    // 写入文件
    std::ofstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("无法打开文件: " + filename);
    }
    file << std::setw(2) << root << std::endl;
    
    // 调用信息回调
    if (m_infoCallbackFunc) {
      m_infoCallbackFunc("保存实体文件成功: " + filename);
    }
  } catch (const std::exception& e) {
    // 调用错误回调
    if (m_errorCallbackFunc) {
      m_errorCallbackFunc(std::string("保存实体文件错误: ") + e.what());
    }
  } catch (...) {
    // 调用错误回调
    if (m_errorCallbackFunc) {
      m_errorCallbackFunc("保存实体文件未知错误");
    }
  }
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
    
    if (root.contains("entities")) {
      for (const auto& entityJson : root["entities"]) {
        try {
          int id = m_nextId++;
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
  
  void cgeo_set_normal(float x, float y, float z) {
    if (g_currentEntity) {
      auto& vertices = g_currentEntity->mesh().vertices();
      if (!vertices.empty()) {
        PVertex& vertex = const_cast<PVertex&>(vertices.back());
        vertex.setNormal(PVector3(x, y, z));
      }
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
      // 首先尝试从默认的"常规"组获取
      auto it = g_currentEntity->propertyGroups().find("常规");
      if (it != g_currentEntity->propertyGroups().end()) {
        const auto& properties = it->second.properties();
        auto propIt = properties.find(name);
        if (propIt != properties.end()) {
          const auto& value = propIt->second;
          if (std::holds_alternative<float>(value)) {
            return std::get<float>(value);
          } else if (std::holds_alternative<int>(value)) {
            return static_cast<float>(std::get<int>(value));
          }
        }
      }
      
      // 如果在"常规"组中找不到，尝试从其他属性组中获取
      for (const auto& [groupName, group] : g_currentEntity->propertyGroups()) {
        if (groupName == "常规") continue; // 跳过已经检查过的"常规"组
        
        const auto& properties = group.properties();
        auto propIt = properties.find(name);
        if (propIt != properties.end()) {
          const auto& value = propIt->second;
          if (std::holds_alternative<float>(value)) {
            return std::get<float>(value);
          } else if (std::holds_alternative<int>(value)) {
            return static_cast<float>(std::get<int>(value));
          }
        }
      }
    }
    
    // 如果找不到属性，返回一个默认值
    if (strcmp(name, "大小") == 0) {
      return 1.0f;
    } else if (strcmp(name, "半径") == 0) {
      return 1.0f;
    } else if (strcmp(name, "高度") == 0) {
      return 2.0f;
    } else if (strcmp(name, "分段数") == 0) {
      return 32.0f;
    } else if (strcmp(name, "管半径") == 0) {
      return 0.3f;
    } else if (strcmp(name, "管分段数") == 0) {
      return 16.0f;
    }
    
    return 1.0f; // 默认为1.0f，而不是0.0f，这样即使没有找到属性，几何体也能生成
  }
  
  void cgeo_set_prop_float(const char* name, float value) {
    if (g_currentEntity) {
      g_currentEntity->setProperty(name, PPropertyValue(value));
    }
  }
  
  int cgeo_get_prop_int(const char* name) {
    if (g_currentEntity) {
      // 首先尝试从默认的"常规"组获取
      auto it = g_currentEntity->propertyGroups().find("常规");
      if (it != g_currentEntity->propertyGroups().end()) {
        const auto& properties = it->second.properties();
        auto propIt = properties.find(name);
        if (propIt != properties.end()) {
          const auto& value = propIt->second;
          if (std::holds_alternative<int>(value)) {
            return std::get<int>(value);
          } else if (std::holds_alternative<float>(value)) {
            return static_cast<int>(std::get<float>(value));
          }
        }
      }
      
      // 如果在"常规"组中找不到，尝试从其他属性组中获取
      for (const auto& [groupName, group] : g_currentEntity->propertyGroups()) {
        if (groupName == "常规") continue; // 跳过已经检查过的"常规"组
        
        const auto& properties = group.properties();
        auto propIt = properties.find(name);
        if (propIt != properties.end()) {
          const auto& value = propIt->second;
          if (std::holds_alternative<int>(value)) {
            return std::get<int>(value);
          } else if (std::holds_alternative<float>(value)) {
            return static_cast<int>(std::get<float>(value));
          }
        }
      }
    }
    return 0;
  }
  
  void cgeo_set_prop_int(const char* name, int value) {
    if (g_currentEntity) {
      g_currentEntity->setProperty(name, PPropertyValue(value));
    }
  }
  
  int cgeo_get_prop_bool(const char* name) {
    if (g_currentEntity) {
      // 首先尝试从默认的"常规"组获取
      auto it = g_currentEntity->propertyGroups().find("常规");
      if (it != g_currentEntity->propertyGroups().end()) {
        const auto& properties = it->second.properties();
        auto propIt = properties.find(name);
        if (propIt != properties.end()) {
          const auto& value = propIt->second;
          if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value) ? 1 : 0;
          } else if (std::holds_alternative<int>(value)) {
            return std::get<int>(value) != 0 ? 1 : 0;
          } else if (std::holds_alternative<float>(value)) {
            return std::get<float>(value) != 0.0f ? 1 : 0;
          }
        }
      }
      
      // 如果在"常规"组中找不到，尝试从其他属性组中获取
      for (const auto& [groupName, group] : g_currentEntity->propertyGroups()) {
        if (groupName == "常规") continue; // 跳过已经检查过的"常规"组
        
        const auto& properties = group.properties();
        auto propIt = properties.find(name);
        if (propIt != properties.end()) {
          const auto& value = propIt->second;
          if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value) ? 1 : 0;
          } else if (std::holds_alternative<int>(value)) {
            return std::get<int>(value) != 0 ? 1 : 0;
          } else if (std::holds_alternative<float>(value)) {
            return std::get<float>(value) != 0.0f ? 1 : 0;
          }
        }
      }
    }
    return 0;
  }
  
  void cgeo_set_prop_bool(const char* name, int value) {
    if (g_currentEntity) {
      g_currentEntity->setProperty(name, PPropertyValue(value != 0));
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
      // 首先尝试从默认的"常规"组获取
      auto it = g_currentEntity->propertyGroups().find("常规");
      if (it != g_currentEntity->propertyGroups().end()) {
        const auto& properties = it->second.properties();
        auto propIt = properties.find(name);
        if (propIt != properties.end()) {
          const auto& value = propIt->second;
          if (std::holds_alternative<std::vector<float>>(value)) {
            auto vec = std::get<std::vector<float>>(value);
            for (int i = 0; i < size && i < static_cast<int>(vec.size()); ++i) {
              values[i] = vec[i];
            }
            return;
          }
        }
      }
      
      // 如果在"常规"组中找不到，尝试从其他属性组中获取
      for (const auto& [groupName, group] : g_currentEntity->propertyGroups()) {
        if (groupName == "常规") continue; // 跳过已经检查过的"常规"组
        
        const auto& properties = group.properties();
        auto propIt = properties.find(name);
        if (propIt != properties.end()) {
          const auto& value = propIt->second;
          if (std::holds_alternative<std::vector<float>>(value)) {
            auto vec = std::get<std::vector<float>>(value);
            for (int i = 0; i < size && i < static_cast<int>(vec.size()); ++i) {
              values[i] = vec[i];
            }
            return;
          }
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
  
  // Set error
  void cgeo_set_error(int error, const char* message) {
    g_lastError = error;
    if (message) {
      strncpy(g_errorMessage, message, sizeof(g_errorMessage) - 1);
      g_errorMessage[sizeof(g_errorMessage) - 1] = '\0';
    } else {
      g_errorMessage[0] = '\0';
    }
  }
  
  // Clear error
  void cgeo_clear_error() {
    g_lastError = 0;
    g_errorMessage[0] = '\0';
  }
  
  // Math functions
  float cgeo_sin(float x) {
    return std::sin(x);
  }
  
  float cgeo_cos(float x) {
    return std::cos(x);
  }
  
  float cgeo_sqrt(float x) {
    return std::sqrt(x);
  }
}
