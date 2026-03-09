#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "math_types.h"
#include "tccengine.h"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <variant>

// 属性元数据类，用于存储属性的额外信息
class PPropertyMeta
{
private:
  std::string m_displayName;      // 显示名称
  std::string m_description;      // 描述
  bool m_readOnly = false;         // 是否只读
  
  // 范围信息
  bool m_hasRange = false;
  float m_minValue = 0.0f;
  float m_maxValue = 1.0f;
  
  // 枚举信息
  bool m_isEnum = false;
  std::map<int, std::string> m_enumValues; // 枚举值映射
  
  // 数组信息
  bool m_isArray = false;
  int m_arraySize = 0;
  
public:
  PPropertyMeta() {}
  PPropertyMeta(const std::string& displayName) : m_displayName(displayName) {}
  
  // Getters and setters
  const std::string& displayName() const { return m_displayName; }
  void setDisplayName(const std::string& displayName) { m_displayName = displayName; }
  
  const std::string& description() const { return m_description; }
  void setDescription(const std::string& description) { m_description = description; }
  
  bool readOnly() const { return m_readOnly; }
  void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
  
  // Range methods
  bool hasRange() const { return m_hasRange; }
  void setRange(float min, float max) {
    m_hasRange = true;
    m_minValue = min;
    m_maxValue = max;
  }
  float minValue() const { return m_minValue; }
  float maxValue() const { return m_maxValue; }
  
  // Enum methods
  bool isEnum() const { return m_isEnum; }
  void addEnumValue(int value, const std::string& name) {
    m_isEnum = true;
    m_enumValues[value] = name;
  }
  const std::map<int, std::string>& enumValues() const { return m_enumValues; }
  
  // Array methods
  bool isArray() const { return m_isArray; }
  void setArraySize(int size) {
    m_isArray = true;
    m_arraySize = size;
  }
  int arraySize() const { return m_arraySize; }
};

// 属性类型
typedef std::variant<float, int, bool, std::string, std::vector<float>> PPropertyValue;

// 属性组
class PPropertyGroup
{
private:
  std::string m_name;
  std::map<std::string, PPropertyValue> m_properties;
  std::map<std::string, PPropertyMeta> m_propertyMetas; // 属性元数据

public:
  PPropertyGroup() : m_name("常规") {}
  PPropertyGroup(const std::string& name) : m_name(name) {}
  
  std::string name() const { return m_name; }
  void setName(const std::string& name) { m_name = name; }
  
  PPropertyValue getProperty(const std::string& key) const;
  void setProperty(const std::string& key, const PPropertyValue& value);
  const std::map<std::string, PPropertyValue>& properties() const { return m_properties; }
  
  // 属性元数据
  PPropertyMeta& getPropertyMeta(const std::string& key);
  const PPropertyMeta* getPropertyMeta(const std::string& key) const;
  
  // 支持范围for循环
  auto begin() const { return m_properties.begin(); }
  auto end() const { return m_properties.end(); }
};

// 顶点类
class PVertex
{
private:
  PVector3 m_position;
  PVector3 m_normal;
  PVector3 m_tangent;

public:
  PVertex() : m_position(0, 0, 0), m_normal(0, 1, 0), m_tangent(1, 0, 0) {}
  PVertex(const PVector3& position) : m_position(position), m_normal(0, 1, 0), m_tangent(1, 0, 0) {}
  PVertex(const PVector3& position, const PVector3& normal, const PVector3& tangent) 
    : m_position(position), m_normal(normal), m_tangent(tangent) {}
  
  PVector3 position() const { return m_position; }
  PVector3 normal() const { return m_normal; }
  PVector3 tangent() const { return m_tangent; }
  
  void setPosition(const PVector3& position) { m_position = position; }
  void setNormal(const PVector3& normal) { m_normal = normal; }
  void setTangent(const PVector3& tangent) { m_tangent = tangent; }
};

// 边类（多段折线）
class PEdge
{
private:
  std::vector<PVertex> m_vertices;

public:
  PEdge() {}
  PEdge(const std::vector<PVertex>& vertices) : m_vertices(vertices) {}
  
  const std::vector<PVertex>& vertices() const { return m_vertices; }
  void addVertex(const PVertex& vertex) { m_vertices.push_back(vertex); }
  void clearVertices() { m_vertices.clear(); }
  
  PBoundBox boundingBox() const;
};

// 网格类
class PMesh
{
private:
  std::vector<PVertex> m_vertices;
  std::vector<unsigned int> m_indices;

public:
  PMesh() {}
  PMesh(const std::vector<PVertex>& vertices, const std::vector<unsigned int>& indices) 
    : m_vertices(vertices), m_indices(indices) {}
  
  const std::vector<PVertex>& vertices() const { return m_vertices; }
  const std::vector<unsigned int>& indices() const { return m_indices; }
  
  void addVertex(const PVertex& vertex) { m_vertices.push_back(vertex); }
  void addIndex(unsigned int index) { m_indices.push_back(index); }
  void clearVertices() { m_vertices.clear(); }
  void clearIndices() { m_indices.clear(); }
  
  PBoundBox boundingBox() const;
};

class PDocument;

class PEntity
{
private:
  int m_id;
  std::string m_name;
  int m_templateId; // 模板ID
  std::string m_scriptSource; // 脚本源代码
  PDocument* m_document; // 文档指针
  std::map<std::string, PPropertyGroup> m_propertyGroups;
  PMesh m_mesh;
  PMatrix4 m_transform;
  bool m_dirty;
  std::function<void(const std::string&)> m_errorCallbackFunc;
  std::function<void(const std::string&)> m_infoCallbackFunc;

public:
  PEntity(int id, const std::string& name, PDocument* document = nullptr);
  
  int id() const { return m_id; }
  std::string name() const { return m_name; }
  void setName(const std::string& name) { m_name = name; m_dirty = true; }
  
  // 模板相关
  int templateId() const { return m_templateId; }
  void setTemplateId(int templateId) { m_templateId = templateId; m_dirty = true; }
  
  // 脚本相关
  const std::string& scriptSource() const { return m_scriptSource; }
  void setScriptSource(const std::string& script) { m_scriptSource = script; m_dirty = true; }
  
  // 文档相关
  void setDocument(PDocument* document) { m_document = document; }
  PDocument* document() const { return m_document; }
  
  // 设置错误回调函数
  void setErrorCallback(std::function<void(const std::string&)> callback);
  // 设置信息回调函数
  void setInfoCallback(std::function<void(const std::string&)> callback);
  
  PPropertyValue getProperty(const std::string& key) const;
  PPropertyValue getProperty(const std::string& groupName, const std::string& key) const;
  void setProperty(const std::string& key, const PPropertyValue& value);
  void setProperty(const std::string& groupName, const std::string& key, const PPropertyValue& value);
  const std::map<std::string, PPropertyGroup>& propertyGroups() const { return m_propertyGroups; }
  PPropertyGroup& getPropertyGroup(const std::string& groupName);
  
  const PMesh& mesh() const { return m_mesh; }
  PMesh& mesh() { return m_mesh; }
  void addVertex(const PVertex& vertex) { m_mesh.addVertex(vertex); }
  void addIndex(int index) { m_mesh.addIndex(index); }
  void clearVertices() { m_mesh.clearVertices(); }
  void clearIndices() { m_mesh.clearIndices(); }
  
  // 兼容旧代码
  const std::vector<PVector3> vertices() const {
    std::vector<PVector3> vecs;
    for (const auto& vertex : m_mesh.vertices()) {
      vecs.push_back(vertex.position());
    }
    return vecs;
  }
  const std::vector<unsigned int>& indices() const { return m_mesh.indices(); }
  
  PMatrix4 transform() const { return m_transform; }
  void setTransform(const PMatrix4& transform) { m_transform = transform; }
  
  bool isDirty() const { return m_dirty; }
  void setDirty(bool dirty) { m_dirty = dirty; }
  
  PBoundBox boundingBox() const;
  void rebuild();
  
  // 加载单个实体文件
  void load(const std::string& filename);
  // 保存单个实体文件
  void save(const std::string& filename) const;
  // 导出为STL文件
  void exportSTL(const std::string& filename) const;
};

// 脚本模板类
class PScriptTemplate
{
private:
  int m_id;
  std::string m_name;
  std::string m_script;

public:
  PScriptTemplate(int id, const std::string& name, const std::string& script)
    : m_id(id), m_name(name), m_script(script) {}
  
  int id() const { return m_id; }
  std::string name() const { return m_name; }
  void setName(const std::string& name) { m_name = name; }
  std::string script() const { return m_script; }
  void setScript(const std::string& script) { m_script = script; }
};

class PDocument
{
private:
  std::map<int, std::shared_ptr<PEntity>> m_entities;
  std::map<int, std::shared_ptr<PScriptTemplate>> m_templates;
  int m_nextEntityId;
  int m_nextTemplateId;

public:
  PDocument() : m_nextEntityId(1), m_nextTemplateId(1000) {}
  
  // 实体管理
  std::shared_ptr<PEntity> createEntity(const std::string& name);
  void removeEntity(int id);
  std::shared_ptr<PEntity> getEntity(int id) const;
  const std::map<int, std::shared_ptr<PEntity>>& entities() const { return m_entities; }
  
  // 模板管理
  std::shared_ptr<PScriptTemplate> createTemplate(const std::string& name, const std::string& script);
  std::shared_ptr<PScriptTemplate> createTemplate(int id, const std::string& name, const std::string& script);
  void removeTemplate(int id);
  void updateTemplate(int id, const std::string& name, const std::string& script);
  std::shared_ptr<PScriptTemplate> getTemplate(int id) const;
  const std::map<int, std::shared_ptr<PScriptTemplate>>& templates() const { return m_templates; }
  
  PBoundBox boundingBox() const;
  
  void save(const std::string& filename) const;
  void load(const std::string& filename);
  
  // ID生成
  int generateEntityId() { return m_nextEntityId++; }
  int generateTemplateId() { return m_nextTemplateId++; }
};

// C API for scripts
extern "C" {
  // Entity management
  void cgeo_add_vertex(float x, float y, float z);
  void cgeo_set_normal(float x, float y, float z);
  void cgeo_add_index(int index);
  void cgeo_clear_vertices();
  void cgeo_clear_indices();
  
  // Edge management
  void cgeo_add_edge_vertex(float x, float y, float z);
  void cgeo_clear_edge_vertices();
  
  // Property management
  float cgeo_get_prop_float(const char* name);
  void cgeo_set_prop_float(const char* name, float value);
  int cgeo_get_prop_int(const char* name);
  void cgeo_set_prop_int(const char* name, int value);
  int cgeo_get_prop_bool(const char* name);
  void cgeo_set_prop_bool(const char* name, int value);
  void cgeo_get_prop_vector(const char* name, float* values, int size);
  void cgeo_set_prop_vector(const char* name, const float* values, int size);
  
  // Color operations
  void cgeo_set_color(float r, float g, float b, float a);
  void cgeo_get_color(float* r, float* g, float* b, float* a);
  
  // Matrix operations
  void cgeo_matrix_identity();
  void cgeo_matrix_translate(float x, float y, float z);
  void cgeo_matrix_rotate_x(float angle);
  void cgeo_matrix_rotate_y(float angle);
  void cgeo_matrix_rotate_z(float angle);
  void cgeo_matrix_scale(float x, float y, float z);
  
  // Vector operations
  float cgeo_vector_length(float x, float y, float z);
  void cgeo_vector_normalize(float* x, float* y, float* z);
  float cgeo_vector_dot(float x1, float y1, float z1, float x2, float y2, float z2);
  void cgeo_vector_cross(float x1, float y1, float z1, float x2, float y2, float z2, float* out_x, float* out_y, float* out_z);
  
  // Bounding box operations
  void cgeo_bbox_reset();
  void cgeo_bbox_add_point(float x, float y, float z);
  void cgeo_bbox_get_min(float* x, float* y, float* z);
  void cgeo_bbox_get_max(float* x, float* y, float* z);
  void cgeo_bbox_get_center(float* x, float* y, float* z);
  void cgeo_bbox_get_size(float* x, float* y, float* z);
  
  // Document operations
  void cgeo_save_document(const char* filename);
  void cgeo_load_document(const char* filename);
  
  // Math functions
  float cgeo_sin(float x);
  float cgeo_cos(float x);
  float cgeo_sqrt(float x);
  
  // Error handling
  int cgeo_get_last_error();
  const char* cgeo_get_error_message();
  void cgeo_set_error(int error, const char* message);
  void cgeo_clear_error();
}

#endif // GEOMETRY_H