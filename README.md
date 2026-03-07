# CMeshStudio

CMeshStudio是一个基于Qt和OpenGL的3D建模工具，支持通过C脚本生成几何模型，提供直观的用户界面和丰富的渲染选项。

## 功能特性

### 核心功能
- **几何生成**：通过C脚本生成3D几何模型
- **实体管理**：创建、删除和编辑实体
- **属性编辑**：为实体添加和修改属性
- **脚本编辑**：内置C语法高亮的脚本编辑器
- **文档管理**：保存和加载项目文件

### 渲染功能
- **多种渲染模式**：点、线框、网格线、实体
- **标准视向**：顶视图、前视图、侧视图、透视图
- **相机控制**：鼠标操作（旋转、平移、缩放）
- **投影模式**：透视和正交投影切换
- **坐标轴显示**：可控制的三维坐标轴

### 技术特性
- **C++17**：使用现代C++标准
- **Qt 6**：跨平台GUI框架
- **OpenGL 3.3+**：硬件加速渲染
- **TCC**：内嵌C编译器，支持实时脚本执行
- **nlohmann/json**：JSON序列化和反序列化

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
- `cgeo_add_index(int index)`：添加索引
- `cgeo_clear_vertices()`：清除所有顶点
- `cgeo_clear_indices()`：清除所有索引

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
├── CMakeLists.txt      # CMake配置文件
└── README.md           # 项目说明
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
- 缺少材质和光照系统

## 未来计划

- 添加模型导入/导出功能
- 实现材质和光照系统
- 增加更多几何生成算法
- 优化渲染性能
- 添加 undo/redo 功能
- 支持更多脚本语言

## 许可证

本项目使用MIT许可证。

## 联系方式

如有问题或建议，请联系项目维护者：

- 邮箱：H3bing@163.com
- GitHub仓库：https://github.com/H3bing/CMeshStudio

---

**CMeshStudio - 让3D建模更简单**