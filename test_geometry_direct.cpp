#include "geometry.h"
#include <iostream>

int main() {
    // 创建一个实体来测试四面体
    PEntity tetrahedron(1, "tetrahedron");
    
    // 加载四面体文件
    std::cout << "Loading tetrahedron.cme..." << std::endl;
    tetrahedron.load("E:\\Qt\\CMeshStudio\\build\\script\\tetrahedron.cme");
    std::cout << "Tetrahedron vertices: " << tetrahedron.mesh().vertices().size() << std::endl;
    std::cout << "Tetrahedron indices: " << tetrahedron.mesh().indices().size() << std::endl;
    
    // 打印顶点坐标
    std::cout << "Tetrahedron vertices:" << std::endl;
    for (const auto& vertex : tetrahedron.mesh().vertices()) {
        PVector3 pos = vertex.position();
        std::cout << "  (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    }
    
    return 0;
}