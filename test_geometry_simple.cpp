#include "geometry.h"
#include <iostream>

int main() {
    // 创建一个实体来测试四面体
    PEntity tetrahedron(1, "tetrahedron");
    tetrahedron.setErrorCallback([](const std::string& message) {
        std::cerr << "Error: " << message << std::endl;
    });
    tetrahedron.setInfoCallback([](const std::string& message) {
        std::cout << "Info: " << message << std::endl;
    });
    
    // 加载四面体文件
    std::cout << "Loading tetrahedron.cme..." << std::endl;
    tetrahedron.load("E:\\Qt\\CMeshStudio\\build\\script\\tetrahedron.cme");
    std::cout << "Tetrahedron vertices: " << tetrahedron.mesh().vertices().size() << std::endl;
    std::cout << "Tetrahedron indices: " << tetrahedron.mesh().indices().size() << std::endl;
    
    // 创建一个实体来测试八面体
    PEntity octahedron(2, "octahedron");
    octahedron.setErrorCallback([](const std::string& message) {
        std::cerr << "Error: " << message << std::endl;
    });
    octahedron.setInfoCallback([](const std::string& message) {
        std::cout << "Info: " << message << std::endl;
    });
    
    // 加载八面体文件
    std::cout << "\nLoading octahedron.cme..." << std::endl;
    octahedron.load("E:\\Qt\\CMeshStudio\\build\\script\\octahedron.cme");
    std::cout << "Octahedron vertices: " << octahedron.mesh().vertices().size() << std::endl;
    std::cout << "Octahedron indices: " << octahedron.mesh().indices().size() << std::endl;
    
    // 创建一个实体来测试十二面体
    PEntity dodecahedron(3, "dodecahedron");
    dodecahedron.setErrorCallback([](const std::string& message) {
        std::cerr << "Error: " << message << std::endl;
    });
    dodecahedron.setInfoCallback([](const std::string& message) {
        std::cout << "Info: " << message << std::endl;
    });
    
    // 加载十二面体文件
    std::cout << "\nLoading dodecahedron.cme..." << std::endl;
    dodecahedron.load("E:\\Qt\\CMeshStudio\\build\\script\\dodecahedron.cme");
    std::cout << "Dodecahedron vertices: " << dodecahedron.mesh().vertices().size() << std::endl;
    std::cout << "Dodecahedron indices: " << dodecahedron.mesh().indices().size() << std::endl;
    
    return 0;
}