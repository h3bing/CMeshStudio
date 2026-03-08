#include "tccengine.h"
#include "libtcc.h"
#include "geometry.h"
#include <cstring>
#include <iostream>
#include <Windows.h>

// PHandleManager static members
std::map<int, std::shared_ptr<void>> PHandleManager::m_handles;
int PHandleManager::m_nextHandle = 1000;

int PHandleManager::addHandle(std::shared_ptr<void> object) {
  int handle = m_nextHandle++;
  m_handles[handle] = object;
  return handle;
}

std::shared_ptr<void> PHandleManager::getHandle(int handle) {
  auto it = m_handles.find(handle);
  if (it != m_handles.end()) {
    return it->second;
  }
  return nullptr;
}

void PHandleManager::removeHandle(int handle) {
  m_handles.erase(handle);
}

bool PHandleManager::isValidHandle(int handle) {
  return m_handles.find(handle) != m_handles.end();
}

// PTCCEngine implementation
PTCCEngine::PTCCEngine() : m_tccState(nullptr) {
}

PTCCEngine::~PTCCEngine() {
  if (m_tccState) {
    tcc_delete(m_tccState);
  }
}

void PTCCEngine::errorCallback(void* opaque, const char* msg) {
  PTCCEngine* engine = static_cast<PTCCEngine*>(opaque);
  if (engine) {
    engine->m_errorMessage += msg;
    // 调用错误回调函数
    if (engine->m_errorCallbackFunc) {
      engine->m_errorCallbackFunc(std::string(msg));
    }
  }
  std::cerr << "TCC Error: " << msg << std::endl;
}

#include <cmath>

void PTCCEngine::registerSymbols() {
  if (!m_tccState) return;
  
  // Register geometry C API functions
  tcc_add_symbol(m_tccState, "cgeo_add_vertex", (void*)cgeo_add_vertex);
  tcc_add_symbol(m_tccState, "cgeo_set_normal", (void*)cgeo_set_normal);
  tcc_add_symbol(m_tccState, "cgeo_add_index", (void*)cgeo_add_index);
  tcc_add_symbol(m_tccState, "cgeo_clear_vertices", (void*)cgeo_clear_vertices);
  tcc_add_symbol(m_tccState, "cgeo_clear_indices", (void*)cgeo_clear_indices);
  tcc_add_symbol(m_tccState, "cgeo_get_prop_float", (void*)cgeo_get_prop_float);
  tcc_add_symbol(m_tccState, "cgeo_set_prop_float", (void*)cgeo_set_prop_float);
  tcc_add_symbol(m_tccState, "cgeo_get_prop_int", (void*)cgeo_get_prop_int);
  tcc_add_symbol(m_tccState, "cgeo_set_prop_int", (void*)cgeo_set_prop_int);
  tcc_add_symbol(m_tccState, "cgeo_get_prop_bool", (void*)cgeo_get_prop_bool);
  tcc_add_symbol(m_tccState, "cgeo_set_prop_bool", (void*)cgeo_set_prop_bool);
  tcc_add_symbol(m_tccState, "cgeo_matrix_identity", (void*)cgeo_matrix_identity);
  tcc_add_symbol(m_tccState, "cgeo_matrix_translate", (void*)cgeo_matrix_translate);
  tcc_add_symbol(m_tccState, "cgeo_matrix_rotate_x", (void*)cgeo_matrix_rotate_x);
  tcc_add_symbol(m_tccState, "cgeo_matrix_rotate_y", (void*)cgeo_matrix_rotate_y);
  tcc_add_symbol(m_tccState, "cgeo_matrix_rotate_z", (void*)cgeo_matrix_rotate_z);
  tcc_add_symbol(m_tccState, "cgeo_matrix_scale", (void*)cgeo_matrix_scale);
  tcc_add_symbol(m_tccState, "cgeo_vector_length", (void*)cgeo_vector_length);
  tcc_add_symbol(m_tccState, "cgeo_vector_normalize", (void*)cgeo_vector_normalize);
  tcc_add_symbol(m_tccState, "cgeo_vector_dot", (void*)cgeo_vector_dot);
  tcc_add_symbol(m_tccState, "cgeo_vector_cross", (void*)cgeo_vector_cross);
  tcc_add_symbol(m_tccState, "cgeo_bbox_reset", (void*)cgeo_bbox_reset);
  tcc_add_symbol(m_tccState, "cgeo_bbox_add_point", (void*)cgeo_bbox_add_point);
  tcc_add_symbol(m_tccState, "cgeo_bbox_get_min", (void*)cgeo_bbox_get_min);
  tcc_add_symbol(m_tccState, "cgeo_bbox_get_max", (void*)cgeo_bbox_get_max);
  tcc_add_symbol(m_tccState, "cgeo_bbox_get_center", (void*)cgeo_bbox_get_center);
  tcc_add_symbol(m_tccState, "cgeo_bbox_get_size", (void*)cgeo_bbox_get_size);
  
  // Register math functions
  tcc_add_symbol(m_tccState, "cgeo_sin", (void*)cgeo_sin);
  tcc_add_symbol(m_tccState, "cgeo_cos", (void*)cgeo_cos);
  tcc_add_symbol(m_tccState, "cgeo_sqrt", (void*)cgeo_sqrt);
}

bool PTCCEngine::initialize() {
  // Create TCC state
  m_tccState = tcc_new();
  if (!m_tccState) {
    m_errorMessage = "Failed to create TCC state";
    return false;
  }
  
  // Set error callback
  tcc_set_error_func(m_tccState, this, errorCallback);
  
  // Set output type to memory (JIT mode)
  tcc_set_output_type(m_tccState, TCC_OUTPUT_MEMORY);
  
  // Set library path - use absolute path
  std::string libPath = "E:\\Qt\\CMeshStudio\\extlib\\libtcc\\lib";
  tcc_set_lib_path(m_tccState, libPath.c_str());
  
  // Also add current directory as library path
  tcc_add_library_path(m_tccState, ".");
  
  // Add MinGW include path for math.h
  tcc_add_include_path(m_tccState, "E:\\Qt\\Tools\\mingw1310_64\\include");
  
  // Register symbols
  registerSymbols();
  
  // Add math library
  tcc_add_library(m_tccState, "m");
  
  return true;
}

bool PTCCEngine::compile(const std::string& code) {
  if (!m_tccState) {
    m_errorMessage = "TCC state not initialized";
    return false;
  }
  
  // Clear previous error message
  m_errorMessage.clear();
  
  // Compile the code
  int result = tcc_compile_string(m_tccState, code.c_str());
  if (result != 0) {
    if (m_errorMessage.empty()) {
      m_errorMessage = "Compilation failed";
    }
    return false;
  }
  
  return true;
}

bool PTCCEngine::execute() {
  if (!m_tccState) {
    m_errorMessage = "TCC state not initialized";
    return false;
  }
  
  // Relocate the code - use TCC_RELOCATE_AUTO to let TCC allocate memory automatically
  int result = tcc_relocate(m_tccState, TCC_RELOCATE_AUTO);
  if (result != 0) {
    m_errorMessage = "Relocation failed";
    return false;
  }
  
  // Get the generate function
  typedef void (*GenerateFunc)();
  GenerateFunc generate = (GenerateFunc)tcc_get_symbol(m_tccState, "generate");
  if (!generate) {
    m_errorMessage = "generate() function not found";
    return false;
  }
  
  // Execute the generate function with exception handling
  try {
    generate();
    return true;
  } catch (const std::exception& e) {
    m_errorMessage = "Runtime exception: " + std::string(e.what());
    return false;
  } catch (...) {
    m_errorMessage = "Unknown runtime exception";
    return false;
  }
}

void PTCCEngine::addIncludePath(const std::string& path) {
  if (m_tccState) {
    tcc_add_include_path(m_tccState, path.c_str());
  }
}

void PTCCEngine::addLibraryPath(const std::string& path) {
  if (m_tccState) {
    tcc_add_library_path(m_tccState, path.c_str());
  }
}

void PTCCEngine::addLibrary(const std::string& library) {
  if (m_tccState) {
    tcc_add_library(m_tccState, library.c_str());
  }
}
