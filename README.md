# CMeshStudio

CMeshStudio是一个基于Qt和OpenGL的3D建模工具，支持通过C脚本生成几何模型，提供直观的用户界面和丰富的渲染选项。

## 功能特性

### 核心功能
- **几何生成**：通过C脚本生成3D几何模型
- **实体管理**：创建、删除和编辑实体
- **属性编辑**：为实体添加和修改属性
- **脚本编辑**：内置C语法高亮的脚本编辑器
- **文档管理**：保存和加载项目文件
- **全局TCC引擎**：优化的脚本编译和执行系统
- **模板系统**：创建可重用的几何生成脚本，减少代码冗余

### 渲染功能
- **多种渲染模式**：点、线框、网格线、实体
- **标准视向**：顶视图、前视图、侧视图、透视图
- **相机控制**：鼠标操作（旋转、平移、缩放）
- **投影模式**：透视和正交投影切换
- **坐标轴显示**：可控制的三维坐标轴
- **光照系统**：基础光照计算和法向量支持
- **渲染缓存**：优化的几何数据缓存机制

### 技术特性
- **C++17**：使用现代C++标准
- **Qt 6**：跨平台GUI框架
- **OpenGL 3.3+**：硬件加速渲染
- **TCC**：内嵌C编译器，支持实时脚本执行
- **nlohmann/json**：JSON序列化和反序列化
- **智能指针**：现代内存管理

## 安装说明

### 系统要求
- Windows 10或更高版本
- Qt 6.10.1或更高版本
- MinGW 13.1.0或更高版本
- CMake 3.30.5或更高版本

### 编译步骤
1. 克隆或下载项目到本地
2. 打开命令行工具，进入项目目录
3. 运行以下命令：

```powershell
# 创建构建目录
mkdir build
cd build

# 配置CMake
E:\Qt\Tools\CMake_64\bin\cmake.exe .. -G Ninja

# 编译项目
E:\Qt\Tools\CMake_64\bin\cmake.exe --build . --config Release
```

## 使用方法

### 基本操作
1. **创建实体**：点击左侧工具栏的"新实体"按钮
2. **编辑属性**：在属性编辑器中修改实体属性
3. **编写脚本**：在脚本编辑器中编写C代码生成几何
4. **运行脚本**：点击脚本编辑器工具栏的"运行"按钮
5. **调整视图**：使用鼠标操作调整相机视角
6. **保存项目**：使用文件菜单保存项目

### 脚本示例

```c
void generate() {
  float size = cgeo_get_prop_float("size");
  if (size <= 0) size = 1.0f;
  
  // 创建一个立方体
  cgeo_add_vertex(-size, -size, -size);
  cgeo_add_vertex(size, -size, -size);
  cgeo_add_vertex(size, size, -size);
  cgeo_add_vertex(-size, size, -size);
  cgeo_add_vertex(-size, -size, size);
  cgeo_add_vertex(size, -size, size);
  cgeo_add_vertex(size, size, size);
  cgeo_add_vertex(-size, size, size);
  
  // 前 face
  cgeo_add_index(0); cgeo_add_index(1); cgeo_add_index(2);
  cgeo_add_index(0); cgeo_add_index(2); cgeo_add_index(3);
  
  // 后 face
  cgeo_add_index(4); cgeo_add_index(6); cgeo_add_index(5);
  cgeo_add_index(4); cgeo_add_index(7); cgeo_add_index(6);
  
  // 左 face
  cgeo_add_index(4); cgeo_add_index(5); cgeo_add_index(1);
  cgeo_add_index(4); cgeo_add_index(1); cgeo_add_index(0);
  
  // 右 face
  cgeo_add_index(3); cgeo_add_index(2); cgeo_add_index(6);
  cgeo_add_index(3); cgeo_add_index(6); cgeo_add_index(7);
  
  // 顶 face
  cgeo_add_index(7); cgeo_add_index(6); cgeo_add_index(2);
  cgeo_add_index(7); cgeo_add_index(2); cgeo_add_index(3);
  
  // 底 face
  cgeo_add_index(4); cgeo_add_index(0); cgeo_add_index(1);
  cgeo_add_index(4); cgeo_add_index(1); cgeo_add_index(5);
}
```

### 脚本API

#### 几何操作
- `cgeo_add_vertex(float x, float y, float z)`：添加顶点
- `cgeo_set_normal(float x, float y, float z)`：设置顶点法向量
- `cgeo_add_index(int index)`：添加索引
- `cgeo_clear_vertices()`：清除所有顶点
- `cgeo_clear_indices()`：清除所有索引

#### 数学函数
- `cgeo_sin(float x)`：正弦函数
- `cgeo_cos(float x)`：余弦函数
- `cgeo_sqrt(float x)`：平方根函数

#### 属性操作
- `cgeo_get_prop_float(const char* name)`：获取浮点属性
- `cgeo_set_prop_float(const char* name, float value)`：设置浮点属性
- `cgeo_get_prop_int(const char* name)`：获取整数属性
- `cgeo_set_prop_int(const char* name, int value)`：设置整数属性
- `cgeo_get_prop_bool(const char* name)`：获取布尔属性
- `cgeo_set_prop_bool(const char* name, bool value)`：设置布尔属性

#### 矩阵操作
- `cgeo_matrix_identity()`：设置单位矩阵
- `cgeo_matrix_translate(float x, float y, float z)`：平移矩阵
- `cgeo_matrix_rotate_x(float angle)`：绕X轴旋转
- `cgeo_matrix_rotate_y(float angle)`：绕Y轴旋转
- `cgeo_matrix_rotate_z(float angle)`：绕Z轴旋转
- `cgeo_matrix_scale(float x, float y, float z)`：缩放矩阵

#### 向量操作
- `cgeo_vector_length(float x, float y, float z)`：计算向量长度
- `cgeo_vector_normalize(float* x, float* y, float* z)`：归一化向量
- `cgeo_vector_dot(float x1, float y1, float z1, float x2, float y2, float z2)`：计算点积
- `cgeo_vector_cross(float x1, float y1, float z1, float x2, float y2, float z2, float* out_x, float* out_y, float* out_z)`：计算叉积

#### 包围盒操作
- `cgeo_bbox_reset()`：重置包围盒
- `cgeo_bbox_add_point(float x, float y, float z)`：添加点到包围盒
- `cgeo_bbox_get_min(float* x, float* y, float* z)`：获取包围盒最小值
- `cgeo_bbox_get_max(float* x, float* y, float* z)`：获取包围盒最大值
- `cgeo_bbox_get_center(float* x, float* y, float* z)`：获取包围盒中心
- `cgeo_bbox_get_size(float* x, float* y, float* z)`：获取包围盒大小

#### 错误处理
- `cgeo_get_last_error()`：获取最后错误代码
- `cgeo_get_error_message()`：获取错误消息

## 模板系统

### 概述

模板系统是CMeshStudio的一个新特性，它允许你创建可重用的几何生成脚本，这些脚本可以被多个实体共享，从而减少代码冗余并提高开发效率。

### 模板系统的优势

- **代码复用**：创建一次模板，多次使用
- **一致性**：确保使用相同模板的实体具有一致的几何生成逻辑
- **易于维护**：修改模板会自动更新所有使用该模板的实体
- **减少文件大小**：避免在每个实体中存储相同的脚本代码

### 模板文件格式

模板系统使用扩展的JSON格式，在`.cme`文件中添加了一个`templates`部分：

```json
{
  "version": "3.0",
  "templates": [
    {
      "id": "cube",
      "name": "立方体",
      "script": "void generate() {\n  float size = cgeo_get_prop_float(\"size\");\n  if (size <= 0) size = 1.0f;\n  \n  cgeo_clear_vertices();\n  cgeo_clear_indices();\n  \n  cgeo_add_vertex(-size, -size, -size);\n  cgeo_add_vertex(size, -size, -size);\n  cgeo_add_vertex(size, size, -size);\n  cgeo_add_vertex(-size, size, -size);\n  cgeo_add_vertex(-size, -size, size);\n  cgeo_add_vertex(size, -size, size);\n  cgeo_add_vertex(size, size, size);\n  cgeo_add_vertex(-size, size, size);\n  \n  cgeo_add_index(0); cgeo_add_index(1); cgeo_add_index(2);\n  cgeo_add_index(0); cgeo_add_index(2); cgeo_add_index(3);\n  cgeo_add_index(4); cgeo_add_index(6); cgeo_add_index(5);\n  cgeo_add_index(4); cgeo_add_index(7); cgeo_add_index(6);\n  cgeo_add_index(4); cgeo_add_index(5); cgeo_add_index(1);\n  cgeo_add_index(4); cgeo_add_index(1); cgeo_add_index(0);\n  cgeo_add_index(3); cgeo_add_index(2); cgeo_add_index(6);\n  cgeo_add_index(3); cgeo_add_index(6); cgeo_add_index(7);\n  cgeo_add_index(7); cgeo_add_index(6); cgeo_add_index(2);\n  cgeo_add_index(7); cgeo_add_index(2); cgeo_add_index(3);\n  cgeo_add_index(4); cgeo_add_index(0); cgeo_add_index(1);\n  cgeo_add_index(4); cgeo_add_index(1); cgeo_add_index(5);\n}"
    }
  ],
  "entities": [
    {
      "id": 1,
      "name": "立方体1",
      "templateId": "cube",
      "propertyGroups": {
        "参数": {
          "name": "参数",
          "properties": {
            "size": 1.0
          }
        }
      },
      "transform": [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
      ]
    }
  ]
}
```

### 使用模板系统

1. **创建模板**：在`.cme`文件的`templates`部分定义模板，每个模板包含`id`、`name`和`script`字段
2. **使用模板**：在实体中使用`templateId`字段引用模板，而不是直接在实体中定义`script`字段
3. **共享属性**：模板可以访问实体的属性，就像直接在实体中定义的脚本一样

### 模板示例

以下是一个包含多种几何体模板的示例：

```json
{
  "version": "3.0",
  "templates": [
    {
      "id": "cube",
      "name": "立方体",
      "script": "void generate() {\n  float size = cgeo_get_prop_float(\"size\");\n  if (size <= 0) size = 1.0f;\n  \n  cgeo_clear_vertices();\n  cgeo_clear_indices();\n  \n  cgeo_add_vertex(-size, -size, -size);\n  cgeo_add_vertex(size, -size, -size);\n  cgeo_add_vertex(size, size, -size);\n  cgeo_add_vertex(-size, size, -size);\n  cgeo_add_vertex(-size, -size, size);\n  cgeo_add_vertex(size, -size, size);\n  cgeo_add_vertex(size, size, size);\n  cgeo_add_vertex(-size, size, size);\n  \n  cgeo_add_index(0); cgeo_add_index(1); cgeo_add_index(2);\n  cgeo_add_index(0); cgeo_add_index(2); cgeo_add_index(3);\n  cgeo_add_index(4); cgeo_add_index(6); cgeo_add_index(5);\n  cgeo_add_index(4); cgeo_add_index(7); cgeo_add_index(6);\n  cgeo_add_index(4); cgeo_add_index(5); cgeo_add_index(1);\n  cgeo_add_index(4); cgeo_add_index(1); cgeo_add_index(0);\n  cgeo_add_index(3); cgeo_add_index(2); cgeo_add_index(6);\n  cgeo_add_index(3); cgeo_add_index(6); cgeo_add_index(7);\n  cgeo_add_index(7); cgeo_add_index(6); cgeo_add_index(2);\n  cgeo_add_index(7); cgeo_add_index(2); cgeo_add_index(3);\n  cgeo_add_index(4); cgeo_add_index(0); cgeo_add_index(1);\n  cgeo_add_index(4); cgeo_add_index(1); cgeo_add_index(5);\n}"
    },
    {
      "id": "sphere",
      "name": "球体",
      "script": "void generate() {\n  float radius = cgeo_get_prop_float(\"半径\");\n  if (radius <= 0) radius = 1.0f;\n  \n  int segments = cgeo_get_prop_int(\"分段数\");\n  if (segments < 3) segments = 3;\n  \n  cgeo_clear_vertices();\n  cgeo_clear_indices();\n  \n  for (int i = 0; i <= segments; i++) {\n    float theta = 3.14159f * i / segments;\n    float sinTheta = cgeo_sin(theta);\n    float cosTheta = cgeo_cos(theta);\n    \n    for (int j = 0; j <= segments; j++) {\n      float phi = 2.0f * 3.14159f * j / segments;\n      float sinPhi = cgeo_sin(phi);\n      float cosPhi = cgeo_cos(phi);\n      \n      float x = radius * sinTheta * cosPhi;\n      float y = radius * cosTheta;\n      float z = radius * sinTheta * sinPhi;\n      \n      cgeo_add_vertex(x, y, z);\n      \n      float nx = sinTheta * cosPhi;\n      float ny = cosTheta;\n      float nz = sinTheta * sinPhi;\n      cgeo_set_normal(nx, ny, nz);\n    }\n  }\n  \n  for (int i = 0; i < segments; i++) {\n    for (int j = 0; j < segments; j++) {\n      int first = i * (segments + 1) + j;\n      int second = first + segments + 1;\n      \n      cgeo_add_index(first);\n      cgeo_add_index(second);\n      cgeo_add_index(first + 1);\n      \n      cgeo_add_index(second);\n      cgeo_add_index(second + 1);\n      cgeo_add_index(first + 1);\n    }\n  }\n}"
    },
    {
      "id": "torus",
      "name": "圆环体",
      "script": "void generate() {\n  float radius = cgeo_get_prop_float(\"半径\");\n  if (radius <= 0) radius = 1.0f;\n  \n  float tubeRadius = cgeo_get_prop_float(\"管半径\");\n  if (tubeRadius <= 0) tubeRadius = 0.3f;\n  \n  int segments = cgeo_get_prop_int(\"分段数\");\n  if (segments < 3) segments = 3;\n  \n  int tubeSegments = cgeo_get_prop_int(\"管分段数\");\n  if (tubeSegments < 3) tubeSegments = 3;\n  \n  cgeo_clear_vertices();\n  cgeo_clear_indices();\n  \n  for (int i = 0; i <= segments; i++) {\n    float angle1 = 2.0f * 3.14159f * i / segments;\n    float cos1 = cgeo_cos(angle1);\n    float sin1 = cgeo_sin(angle1);\n    \n    for (int j = 0; j <= tubeSegments; j++) {\n      float angle2 = 2.0f * 3.14159f * j / tubeSegments;\n      float cos2 = cgeo_cos(angle2);\n      float sin2 = cgeo_sin(angle2);\n      \n      float x = (radius + tubeRadius * cos2) * cos1;\n      float y = (radius + tubeRadius * cos2) * sin1;\n      float z = tubeRadius * sin2;\n      \n      cgeo_add_vertex(x, y, z);\n      \n      float nx = cos2 * cos1;\n      float ny = cos2 * sin1;\n      float nz = sin2;\n      cgeo_set_normal(nx, ny, nz);\n    }\n  }\n  \n  for (int i = 0; i < segments; i++) {\n    for (int j = 0; j < tubeSegments; j++) {\n      int first = i * (tubeSegments + 1) + j;\n      int second = first + tubeSegments + 1;\n      \n      cgeo_add_index(first);\n      cgeo_add_index(second);\n      cgeo_add_index(first + 1);\n      \n      cgeo_add_index(second);\n      cgeo_add_index(second + 1);\n      cgeo_add_index(first + 1);\n    }\n  }\n}"
    }
  ],
  "entities": [
    {
      "id": 1,
      "name": "立方体1",
      "templateId": "cube",
      "propertyGroups": {
        "参数": {
          "name": "参数",
          "properties": {
            "size": 1.0
          }
        },
        "外观": {
          "name": "外观",
          "properties": {
            "颜色": [1.0, 0.5, 0.2, 1.0]
          }
        },
        "位姿": {
          "name": "位姿",
          "properties": {
            "位置": [0.0, 0.0, 0.0],
            "旋转": [0.0, 0.0, 0.0],
            "缩放": [1.0, 1.0, 1.0]
          }
        }
      },
      "transform": [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
      ]
    },
    {
      "id": 2,
      "name": "球体1",
      "templateId": "sphere",
      "propertyGroups": {
        "参数": {
          "name": "参数",
          "properties": {
            "半径": 1.0,
            "分段数": 32
          }
        },
        "外观": {
          "name": "外观",
          "properties": {
            "颜色": [1.0, 0.0, 0.0, 1.0]
          }
        },
        "位姿": {
          "name": "位姿",
          "properties": {
            "位置": [2.0, 0.0, 0.0],
            "旋转": [0.0, 0.0, 0.0],
            "缩放": [1.0, 1.0, 1.0]
          }
        }
      },
      "transform": [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        2.0, 0.0, 0.0, 1.0
      ]
    },
    {
      "id": 3,
      "name": "圆环体1",
      "templateId": "torus",
      "propertyGroups": {
        "参数": {
          "name": "参数",
          "properties": {
            "半径": 1.0,
            "管半径": 0.3,
            "分段数": 32,
            "管分段数": 16
          }
        },
        "外观": {
          "name": "外观",
          "properties": {
            "颜色": [0.0, 1.0, 0.0, 1.0]
          }
        },
        "位姿": {
          "name": "位姿",
          "properties": {
            "位置": [-2.0, 0.0, 0.0],
            "旋转": [0.0, 0.0, 0.0],
            "缩放": [1.0, 1.0, 1.0]
          }
        }
      },
      "transform": [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        -2.0, 0.0, 0.0, 1.0
      ]
    }
  ]
}
```

## 项目结构

```
CMeshStudio/
├── include/            # 头文件
│   ├── csyntaxhighlighter.h
│   ├── geometry.h
│   ├── glviewport.h
│   ├── mainwindow.h
│   ├── math_types.h
│   └── tccengine.h
├── src/                # 源代码
│   ├── csyntaxhighlighter.cpp
│   ├── geometry.cpp
│   ├── glviewport.cpp
│   ├── main.cpp
│   ├── mainwindow.cpp
│   ├── math_types.cpp
│   └── tccengine.cpp
├── extlib/             # 第三方库
│   ├── earcut.hpp-master/    # 多边形三角化库
│   ├── libtcc/               # Tiny C Compiler
│   ├── nlohmann-json-develop/ # JSON库
│   └── stb-master/           # 单头文件库
├── build/              # 构建目录
│   └── script/         # 示例脚本文件
├── CMakeLists.txt      # CMake配置文件
├── README.md           # 项目说明
└── 脚本编写指南.md      # 脚本编写指南
```

## 开发环境配置

### Qt配置
- 确保Qt 6.10.1或更高版本已安装
- 在系统环境变量中设置Qt的bin目录路径

### CMake配置
- 使用E:\Qt\Tools\CMake_64\bin\cmake.exe
- 支持Ninja构建系统

### MinGW配置
- 使用E:\Qt\Tools\mingw1310_64\bin\g++.exe
- 支持C++17标准

## 已知问题

- 渲染性能在处理复杂模型时可能需要优化

## 未来计划

- 添加模型导入/导出功能
- 实现更高级的材质系统
- 增加更多几何生成算法
- 优化渲染性能
- 添加 undo/redo 功能
- 集成AI辅助功能
- 构建云协作平台

## 许可证

本项目使用MIT许可证。

## 联系方式

如有问题或建议，请联系项目维护者：

- 邮箱：H3bing@163.com
- GitHub仓库：https://github.com/H3bing/CMeshStudio

---

**CMeshStudio - 让3D建模更简单**