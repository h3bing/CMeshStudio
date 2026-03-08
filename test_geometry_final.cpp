#include "geometry.h"
#include <iostream>

int main() {
    // 创建一个实体来测试四面体
    PEntity tetrahedron(1, "tetrahedron");
    
    // 设置错误和信息回调
    tetrahedron.setErrorCallback([](const std::string& message) {
        std::cerr << "Error: " << message << std::endl;
    });
    tetrahedron.setInfoCallback([](const std::string& message) {
        std::cout << "Info: " << message << std::endl;
    });
    
    // 加载四面体文件
    std::cout << "Loading tetrahedron.cme..." << std::endl;
    tetrahedron.load("E:\\Qt\\CMeshStudio\\build\\script\\tetra