# CME 文件扩展规范 - 支持无限几何体

## 概述

CME（CMeshStudio Entity）文件是JSON格式的数据文件，用于保存几何实体及其属性。本规范扩展了原有的CME文件格式，使其支持无限几何体组合。

## 文件结构

### 基础结构

```json
{
  "version": "3.0",                    // 文件格式版本
  "metadata": {                        // 元数据
    "creator": "创建者信息",
    "created": "创建时间戳",
    "description": "文件描述"
  },
  "templates": [                       // 模板数组 - 可重用的几何生成脚本
    {
      "id": "cube",                 // 模板唯一标识符
      "name": "立方体",             // 模板名称
      "script": "脚本代码"           // C脚本
    }
  ],
  "entities": [                        // 几何体数组 - 支持无限多个
    {
      "id": 1,                       // 唯一标识符
      "name": "几何体名称",
      "templateId": "cube",         // 模板ID（可选，替代script字段）
      "propertyGroups": {...},       // 属性组
      "script": "脚本代码",          // C脚本（当templateId不存在时使用）
      "transform": [...]             // 变换矩阵
    }
  ],
  "groups": {...},                    // 几何体分组
  "relations": [...],                 // 几何体关系
  "patterns": {...}                   // 生成模式
}
```

### 1. 几何体实体 (entities)

每个几何体实体现在可以完全独立：

```json
{
  "id": 1,
  "name": "锥形塔",
  "propertyGroups": {
    "常规": {
      "name": "常规",
      "properties": {
        "可见": true,
        "名称": "锥形塔",
        "ID": 1
      }
    },
    "外观": {
      "name": "外观",
      "properties": {
        "颜色": [0.5, 0.6, 0.7, 1.0],
        "高光颜色": [0.8, 0.9, 1.0],
        "反光度": 32.0,
        "反射率": 0.3,
        "透明度": 0.0
      }
    },
    "位姿": {
      "name": "位姿",
      "properties": {
        "位置": [0.0, 0.0, 0.0],
        "旋转": [0.0, 0.0, 0.0],
        "缩放": [1.0, 1.0, 1.0]
      }
    },
    "参数": {
      "name": "参数",
      "properties": {
        "自定义参数1": 1.0,
        "自定义参数2": 10
      }
    }
  },
  "script": "void generate() {\n  // 几何生成脚本\n}",
  "transform": [
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
  ]
}
```

### 2. 分组系统 (groups)

支持将多个几何体组织成逻辑组：

```json
"groups": {
  "主建筑": [1, 2, 3, 4],
  "装饰元素": [5, 6, 7],
  "景观组件": [8, 9, 10, 11, 12],
  "细节构件": [13, 14, 15, 16, 17, 18, 19, 20]
}
```

**分组优势：**
- 批量操作（隐藏/显示/移动）
- 层级管理
- 简化复杂场景的组织

### 3. 关系系统 (relations)

定义几何体之间的父子关系：

```json
"relations": [
  {
    "parent": 1,
    "child": 2,
    "type": "position",                 // 位置关联
    "constraints": {
      "x_offset": 5.0,
      "y_offset": 0.0,
      "z_offset": 0.0
    }
  },
  {
    "parent": 1,
    "child": 3,
    "type": "scale",                    // 缩放关联
    "multiplier": 0.5
  },
  {
    "parent": 2,
    "child": 4,
    "type": "rotation",                 // 旋转关联
    "axis": "y",
    "follow_parent": true
  }
]
```

**关系类型：**
- `position` - 位置跟随
- `rotation` - 旋转跟随
- `scale` - 缩放跟随
- `relative_position` - 相对位置
- `constraint` - 约束关系

### 4. 生成模式 (patterns)

定义几何体的生成规则，用于批量创建：

```json
"patterns": {
  "树木群组": {
    "baseEntity": 1,
    "distribution": "random",           // 分布模式
    "count": 50,                        // 生成数量
    "area": {
      "type": "circle",
      "radius": 100.0
    },
    "variations": {
      "大小变化": [0.8, 1.0, 1.2],
      "高度变化": [0.7, 1.0, 1.3],
      "旋转变化": [-15, 0, 15]
    }
  },
  "建筑网格": {
    "baseEntity": 2,
    "distribution": "grid",
    "gridSize": [10, 10],
    "spacing": {
      "x": 20.0,
      "z": 25.0
    },
    "variations": {
      "大小变化": [0.5, 1.0, 1.5, 2.0],
      "高度变化": [0.3, 0.7, 1.0, 1.4, 1.8]
    }
  },
  "螺旋装饰": {
    "baseEntity": 3,
    "distribution": "spiral",
    "turns": 3,
    "radius": 50.0,
    "pitch": 5.0
  }
}
```

## 无限几何体的实现策略

### 1. 性能优化

```json
{
  "entities": [...],
  "optimization": {
    "levelOfDetail": {
      "enable": true,
      "distances": [10.0, 50.0, 100.0],
      "reductionRatio": [1.0, 0.5, 0.1]
    },
    "culling": {
      "frustumCulling": true,
      "occlusionCulling": true,
      "distanceCulling": {
        "maxDistance": 1000.0,
        "fadeOut": true
      }
    },
    "instancing": {
      "enable": true,
      "batchSize": 1000
    }
  }
}
```

### 2. 流式加载

```json
{
  "streaming": {
    "enable": true,
    "chunkSize": 50,                    // 每块50个几何体
    "loadDistance": 100.0,              // 加载距离
    "unloadDistance": 150.0,            // 卸载距离
    "priorityEntities": [1, 2, 3, 4]    // 优先加载的几何体
  }
}
```

### 3. 内存管理

```json
{
  "memory": {
    "maxVertices": 1000000,             // 最大顶点数
    "maxIndices": 2000000,              // 最大索引数
    "cacheSize": 512,                   // 缓存大小(MB)
    "garbageCollection": {
      "enable": true,
      "interval": 30                    // 清理间隔(秒)
    }
  }
}
```

## 扩展案例

### 案例 1: 城市建筑群

自动随机生成不同高度和旋转的建筑物：

- 基础建筑为锥体或立方体
- 应用随机变换
- 形成自然的建筑群效果
- 支持无限扩展

### 案例 2: 自然环境

```json
"entities": [
  {
    "id": 1,
    "template": "tree.base",
    "variations": {
      "trunkHeight": [2.0, 4.0, 6.0],
      "crownRadius": [1.0, 1.5, 2.0],
      "leafDensity": [0.5, 0.7, 0.9]
    }
  }
  // 可以添加数千个这样的树木模板
]
```

### 案例 3: 机制艺术

```json
"patterns": {
  "螺旋星系": {
    "baseEntity": 1,
    "distribution": "spiral",
    "arms": 4,
    "turns": 10,
    "particlesPerArm": 1000
  }
}
```

## 向后兼容性

### 版本 1.0 → 3.0 迁移

```json
// 旧版本 (1.0)
{
  "version": "1.0",
  "entities": [{...}]
}

// 新版本 (3.0)
{
  "version": "3.0",
  "metadata": {
    "description": "从版本1.0迁移的文件"
  },
  "entities": [{...}],
  "groups": {},
  "relations": [],
  "patterns": {}
}
```

## 最佳实践

### 1. 几何体数量管理
- 单个文件建议最多 10000 个几何体
- 使用分组管理大量几何体
- 实现 LOD (细节层次) 系统

### 2. 脚本优化
- 减少重复计算
- 使用参数化脚本
- 缓存计算结果

### 3. 文件组织
- 将相关几何体分组
- 使用清晰的命名规则
- 添加详细的元数据

### 4. 错误处理
```json
{
  "validation": {
    "schemaVersion": "3.0",
    "rules": {
      "uniqueIds": true,
      "validScripts": true,
      "transformMatrix": "4x4"
    }
  }
}
```

## 结语

扩展后的CME文件格式现在能够支持：

1. ✅ **无限几何体数量** - 理论上没有限制
2. ✅ **智能分组管理** - 便于组织复杂场景
3. ✅ **关系系统** - 几何体间的智能关联
4. ✅ **生成模式** - 批量创建和变化
5. ✅ **性能优化** - LOD和流式加载
6. ✅ **内存管理** - 自动资源回收

通过这些扩展，CMeshStudio现在可以用来创建从简单几何体到复杂场景的任何内容！