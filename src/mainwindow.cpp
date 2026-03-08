#include "mainwindow.h"
#include "csyntaxhighlighter.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QFont>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>

PMainWindow::PMainWindow(QWidget* parent) 
  : QMainWindow(parent), m_selectedEntityId(-1) {
  // Create document
  m_document = std::make_shared<PDocument>();
  
  // Setup UI
  setupUI();
  
  // Setup menu bar
  setupMenuBar();
  
  // Set window properties
  setWindowTitle("CMeshStudio");
  setGeometry(100, 100, 1600, 800);
}

PMainWindow::~PMainWindow() {
}

void PMainWindow::setupUI() {
  // Create central widget
  QWidget* centralWidget = new QWidget(this);
  
  // Create main splitter
  QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
  
  // Left panel (2 parts)
  QWidget* leftPanel = new QWidget();
  QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
  
  // Top left: Document
  QGroupBox* sceneTreeGroup = new QGroupBox("文档");
  QVBoxLayout* sceneTreeLayout = new QVBoxLayout(sceneTreeGroup);
  
  // Scene tree buttons
  QHBoxLayout* sceneTreeButtons = new QHBoxLayout();
  QPushButton* newBtn = new QPushButton("新建");
  QPushButton* openBtn = new QPushButton("打开");
  QPushButton* saveBtn = new QPushButton("保存");
  QPushButton* saveAsBtn = new QPushButton("另存为");
  QPushButton* newEntityBtn = new QPushButton("新实体");
  QPushButton* deleteEntityBtn = new QPushButton("删除实体");
  
  // Set button widths to make them narrower
  int buttonWidth = 80;
  newBtn->setFixedWidth(buttonWidth);
  openBtn->setFixedWidth(buttonWidth);
  saveBtn->setFixedWidth(buttonWidth);
  saveAsBtn->setFixedWidth(buttonWidth);
  newEntityBtn->setFixedWidth(buttonWidth);
  deleteEntityBtn->setFixedWidth(buttonWidth);
  sceneTreeButtons->addWidget(newBtn);
  sceneTreeButtons->addWidget(openBtn);
  sceneTreeButtons->addWidget(saveBtn);
  sceneTreeButtons->addWidget(saveAsBtn);
  sceneTreeButtons->addWidget(newEntityBtn);
  sceneTreeButtons->addWidget(deleteEntityBtn);
  
  // Scene tree widget
  m_sceneTree = new QTreeWidget();
  m_sceneTree->setHeaderLabel("实体列表");
  
  sceneTreeLayout->addLayout(sceneTreeButtons);
  sceneTreeLayout->addWidget(m_sceneTree);
  
  // Bottom left: Property grid
  QGroupBox* propertyGroup = new QGroupBox("属性编辑器");
  QVBoxLayout* propertyLayout = new QVBoxLayout(propertyGroup);
  
  m_propertyLabel = new QLabel("请选择一个实体来编辑属性");
  
  // Create property table
  m_propertyEditor = new QTableWidget();
  m_propertyEditor->setColumnCount(2);
  m_propertyEditor->setHorizontalHeaderLabels({"属性", "值"});
  m_propertyEditor->setEnabled(false);
  
  propertyLayout->addWidget(m_propertyLabel);
  propertyLayout->addWidget(m_propertyEditor);
  
  leftLayout->addWidget(sceneTreeGroup, 2);
  leftLayout->addWidget(propertyGroup, 1);
  
  // Middle panel: GL Viewport
  QGroupBox* viewportGroup = new QGroupBox("视图端口");
  QVBoxLayout* viewportLayout = new QVBoxLayout(viewportGroup);
  
  // Viewport controls
  QHBoxLayout* viewportControls = new QHBoxLayout();
  
  // Render mode dropdown
  QLabel* renderModeLabel = new QLabel("渲染模式:");
  QComboBox* renderModeComboBox = new QComboBox();
  renderModeComboBox->addItem("点");
  renderModeComboBox->addItem("线框");
  renderModeComboBox->addItem("网格线");
  renderModeComboBox->addItem("实体");
  
  // Standard view dropdown
  QLabel* viewLabel = new QLabel("标准视向:");
  QComboBox* viewComboBox = new QComboBox();
  viewComboBox->addItem("顶视图");
  viewComboBox->addItem("前视图");
  viewComboBox->addItem("侧视图");
  viewComboBox->addItem("透视图");
  
  // Reset view button
  QPushButton* resetViewBtn = new QPushButton("初始视向");
  
  // Show axes checkbox
  QCheckBox* showAxesCheckBox = new QCheckBox("显示坐标轴");
  showAxesCheckBox->setChecked(true); // 默认显示坐标轴
  
  // Projection mode dropdown
  QLabel* projectionLabel = new QLabel("投影方式:");
  QComboBox* projectionComboBox = new QComboBox();
  projectionComboBox->addItem("透视投影");
  projectionComboBox->addItem("正交投影");
  projectionComboBox->setCurrentIndex(0); // 默认使用透视投影
  
  // Rebuild geometry button
  QPushButton* rebuildBtn = new QPushButton("重建几何");
  
  viewportControls->addWidget(renderModeLabel);
  viewportControls->addWidget(renderModeComboBox);
  viewportControls->addWidget(viewLabel);
  viewportControls->addWidget(viewComboBox);
  viewportControls->addWidget(resetViewBtn);
  viewportControls->addWidget(showAxesCheckBox);
  viewportControls->addWidget(projectionLabel);
  viewportControls->addWidget(projectionComboBox);
  viewportControls->addWidget(rebuildBtn);
  viewportControls->addStretch();
  
  // GL Viewport
  m_glViewport = new PGLViewport();
  m_glViewport->setDocument(m_document);
  
  viewportLayout->addLayout(viewportControls);
  viewportLayout->addWidget(m_glViewport);
  
  // Right panel (2 parts)
  QWidget* rightPanel = new QWidget();
  QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
  
  // Top right: Script editor
  QGroupBox* scriptGroup = new QGroupBox("脚本编辑器");
  QVBoxLayout* scriptLayout = new QVBoxLayout(scriptGroup);
  
  // Script editor toolbar
  QHBoxLayout* scriptToolbar = new QHBoxLayout();
  QPushButton* loadScriptBtn = new QPushButton("加载");
  QPushButton* saveScriptBtn = new QPushButton("保存");
  QPushButton* runScriptBtn = new QPushButton("运行");
  QPushButton* aboutBtn = new QPushButton("关于");
  scriptToolbar->addWidget(loadScriptBtn);
  scriptToolbar->addWidget(saveScriptBtn);
  scriptToolbar->addWidget(runScriptBtn);
  scriptToolbar->addStretch();
  scriptToolbar->addWidget(aboutBtn);
  
  m_scriptEditor = new QTextEdit();
  m_scriptEditor->setEnabled(true);
  m_scriptEditor->setReadOnly(false);
  m_scriptEditor->setPlainText("void generate() {\n  // 在这里添加几何生成代码\n}\n");
  
  // Set font for better code display
  QFont font;
  font.setFamily("Courier New");
  font.setFixedPitch(true);
  font.setPointSize(10);
  m_scriptEditor->setFont(font);
  
  // Enable C syntax highlighting
  CSyntaxHighlighter* highlighter = new CSyntaxHighlighter(m_scriptEditor->document());
  
  scriptLayout->addLayout(scriptToolbar);
  scriptLayout->addWidget(m_scriptEditor);
  
  // Bottom right: Console
  QGroupBox* consoleGroup = new QGroupBox("控制台");
  QVBoxLayout* consoleLayout = new QVBoxLayout(consoleGroup);
  
  m_console = new QTextEdit();
  m_console->setReadOnly(true);
  
  consoleLayout->addWidget(m_console);
  
  rightLayout->addWidget(scriptGroup, 2);
  rightLayout->addWidget(consoleGroup, 1);
  
  // Add panels to main splitter
  mainSplitter->addWidget(leftPanel);
  mainSplitter->addWidget(viewportGroup);
  mainSplitter->addWidget(rightPanel);
  
  // Set initial sizes
  QList<int> sizes;
  sizes << 250 << 750 << 400; // 左侧边栏更窄，右侧边栏更宽
  mainSplitter->setSizes(sizes);
  
  // Set central widget
  setCentralWidget(mainSplitter);
  
  // Connect signals and slots
  connect(newEntityBtn, &QPushButton::clicked, this, &PMainWindow::onNewEntity);
  connect(deleteEntityBtn, &QPushButton::clicked, this, &PMainWindow::onDeleteEntity);
  connect(m_sceneTree, &QTreeWidget::itemSelectionChanged, [this]() {
    QList<QTreeWidgetItem*> selectedItems = m_sceneTree->selectedItems();
    if (!selectedItems.isEmpty()) {
      onSceneTreeItemSelected(selectedItems.first(), 0);
    }
  });
  connect(m_propertyEditor, &QTableWidget::cellChanged, this, &PMainWindow::onPropertyChanged);
  connect(m_scriptEditor, &QTextEdit::textChanged, this, &PMainWindow::onScriptChanged);
  connect(rebuildBtn, &QPushButton::clicked, this, &PMainWindow::onRebuildGeometry);
  connect(loadScriptBtn, &QPushButton::clicked, this, &PMainWindow::onLoadScript);
  connect(saveScriptBtn, &QPushButton::clicked, this, &PMainWindow::onSaveScript);
  connect(runScriptBtn, &QPushButton::clicked, this, &PMainWindow::onRunScript);
  connect(aboutBtn, &QPushButton::clicked, this, &PMainWindow::onAbout);
  connect(showAxesCheckBox, &QCheckBox::toggled, m_glViewport, &PGLViewport::setShowAxes);
  connect(renderModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), m_glViewport, &PGLViewport::setRenderMode);
  connect(viewComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), m_glViewport, &PGLViewport::setStandardView);
  connect(resetViewBtn, &QPushButton::clicked, m_glViewport, &PGLViewport::resetView);
  connect(projectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
    m_glViewport->setPerspective(index == 0); // 0: 透视投影, 1: 正交投影
  });
  
  // Update scene tree
  updateSceneTree();
}

void PMainWindow::onNewEntity() {
  // Open file dialog to select or create entity file
  QString scriptDir = QCoreApplication::applicationDirPath() + "/script";
  QDir dir(scriptDir);
  if (!dir.exists()) {
    dir.mkpath(scriptDir);
  }
  
  QString fileName = QFileDialog::getOpenFileName(
    this, 
    "选择实体文件", 
    scriptDir, 
    "CMeshStudio 实体文件 (*.cme);;All Files (*)"
  );
  
  if (!fileName.isEmpty()) {
    // 检查文件扩展名
    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "cme") {
      // 加载单个实体文件
      auto entity = m_document->createEntity(fileInfo.baseName().toStdString());
      
      // Set error and info callbacks
      entity->setErrorCallback([this](const std::string& message) {
        this->logToConsole("TCC 错误: " + QString::fromStdString(message));
      });
      entity->setInfoCallback([this](const std::string& message) {
        this->logToConsole("INFO: " + QString::fromStdString(message));
      });
      
      // 加载实体文件
      entity->load(fileName.toStdString());
      
      // Update scene tree
      updateSceneTree();
      
      // Select the new entity
      if (m_sceneTree->topLevelItemCount() > 0) {
        QTreeWidgetItem* item = m_sceneTree->topLevelItem(m_sceneTree->topLevelItemCount() - 1);
        m_sceneTree->setCurrentItem(item);
      }
      
      logToConsole("加载实体文件: " + fileName);
    } else if (extension == "cmd") {
      // 加载多个实体文档
      m_document->load(fileName.toStdString());
      
      // Set error and info callbacks for all entities
      for (const auto& [id, entity] : m_document->entities()) {
        entity->setErrorCallback([this](const std::string& message) {
          this->logToConsole("TCC 错误: " + QString::fromStdString(message));
        });
        entity->setInfoCallback([this](const std::string& message) {
          this->logToConsole("INFO: " + QString::fromStdString(message));
        });
      }
      
      // Update scene tree
      updateSceneTree();
      
      // Select the first entity
      if (m_sceneTree->topLevelItemCount() > 0) {
        QTreeWidgetItem* item = m_sceneTree->topLevelItem(0);
        m_sceneTree->setCurrentItem(item);
      }
      
      logToConsole("加载文档文件: " + fileName);
    } else {
      logToConsole("不支持的文件格式: " + fileName);
    }
  } else {
    // Create new entity
    auto entity = m_document->createEntity("新实体");
    
    // Set error and info callbacks
    entity->setErrorCallback([this](const std::string& message) {
      this->logToConsole("TCC 错误: " + QString::fromStdString(message));
    });
    entity->setInfoCallback([this](const std::string& message) {
      this->logToConsole("INFO: " + QString::fromStdString(message));
    });
    
    // Set default script
    entity->setScriptSource("void generate() {\n  float size = cgeo_get_prop_float(\"size\");\n  if (size <= 0) size = 1.0f;\n  \n  // Create a cube\n  cgeo_add_vertex(-size, -size, -size);\n  cgeo_add_vertex(size, -size, -size);\n  cgeo_add_vertex(size, size, -size);\n  cgeo_add_vertex(-size, size, -size);\n  cgeo_add_vertex(-size, -size, size);\n  cgeo_add_vertex(size, -size, size);\n  cgeo_add_vertex(size, size, size);\n  cgeo_add_vertex(-size, size, size);\n  \n  // Front face\n  cgeo_add_index(0); cgeo_add_index(1); cgeo_add_index(2);\n  cgeo_add_index(0); cgeo_add_index(2); cgeo_add_index(3);\n  \n  // Back face\n  cgeo_add_index(4); cgeo_add_index(6); cgeo_add_index(5);\n  cgeo_add_index(4); cgeo_add_index(7); cgeo_add_index(6);\n  \n  // Left face\n  cgeo_add_index(4); cgeo_add_index(5); cgeo_add_index(1);\n  cgeo_add_index(4); cgeo_add_index(1); cgeo_add_index(0);\n  \n  // Right face\n  cgeo_add_index(3); cgeo_add_index(2); cgeo_add_index(6);\n  cgeo_add_index(3); cgeo_add_index(6); cgeo_add_index(7);\n  \n  // Top face\n  cgeo_add_index(7); cgeo_add_index(6); cgeo_add_index(2);\n  cgeo_add_index(7); cgeo_add_index(2); cgeo_add_index(3);\n  \n  // Bottom face\n  cgeo_add_index(4); cgeo_add_index(0); cgeo_add_index(1);\n  cgeo_add_index(4); cgeo_add_index(1); cgeo_add_index(5);\n}\n");
    
    // Set default property
    entity->setProperty("size", 1.0f);
    
    // Update scene tree
    updateSceneTree();
    
    // Select the new entity
    QTreeWidgetItem* item = m_sceneTree->topLevelItem(m_sceneTree->topLevelItemCount() - 1);
    m_sceneTree->setCurrentItem(item);
    
    logToConsole("创建新实体: " + QString::fromStdString(entity->name()));
  }
}

void PMainWindow::onDeleteEntity() {
  if (m_selectedEntityId == -1) {
    QMessageBox::information(this, "信息", "未选择实体");
    return;
  }
  
  // Remove entity
  m_document->removeEntity(m_selectedEntityId);
  m_selectedEntityId = -1;
  
  // Update UI
  updateSceneTree();
  updatePropertyEditor();
  updateScriptEditor();
  
  logToConsole("删除实体 ID: " + QString::number(m_selectedEntityId));
}

void PMainWindow::onSceneTreeItemSelected(QTreeWidgetItem* item, int column) {
  if (!item) {
    m_selectedEntityId = -1;
    updatePropertyEditor();
    updateScriptEditor();
    return;
  }
  
  // Get entity ID from item data
  m_selectedEntityId = item->data(0, Qt::UserRole).toInt();
  
  // Update property and script editors
  updatePropertyEditor();
  updateScriptEditor();
  
  // Update GL viewport selection
  m_glViewport->setSelectedEntityId(m_selectedEntityId);
}

void PMainWindow::onPropertyChanged(int row, int column) {
  if (m_selectedEntityId == -1) return;
  if (column != 1) return; // Only handle value changes
  
  auto entity = m_document->getEntity(m_selectedEntityId);
  if (!entity) return;
  
  // Get property name and value
  QTableWidgetItem* keyItem = m_propertyEditor->item(row, 0);
  QTableWidgetItem* valueItem = m_propertyEditor->item(row, 1);
  if (!keyItem || !valueItem) return;
  
  std::string key = keyItem->text().toStdString();
  QString valueStr = valueItem->text();
  
  // Determine the property group
  std::string groupName = "常规"; // Default group
  // Find the group header above this row
  for (int i = row - 1; i >= 0; --i) {
    QTableWidgetItem* item = m_propertyEditor->item(i, 0);
    if (item && item->text().startsWith("[")) {
      QString groupText = item->text();
      groupName = groupText.mid(1, groupText.length() - 2).toStdString();
      break;
    }
  }
  
  // Handle different types of property values
  if (valueStr.startsWith("[")) {
    // Handle vector type (e.g., color, position, etc.)
    // Remove brackets and split by commas
    QString cleanStr = valueStr.mid(1, valueStr.length() - 2);
    QStringList parts = cleanStr.split(",");
    std::vector<float> vec;
    for (const QString& part : parts) {
      vec.push_back(part.trimmed().toFloat());
    }
    entity->setProperty(groupName, key, PPropertyValue(vec));
  } else if (valueStr == "true" || valueStr == "false") {
    // Handle boolean type
    bool boolValue = (valueStr == "true");
    entity->setProperty(groupName, key, PPropertyValue(boolValue));
  } else if (valueStr.toInt() != 0 || valueStr == "0") {
    // Handle integer type
    int intValue = valueStr.toInt();
    entity->setProperty(groupName, key, PPropertyValue(intValue));
  } else if (valueStr.toFloat() != 0.0f || valueStr == "0.0" || valueStr == "0") {
    // Handle float type
    float floatValue = valueStr.toFloat();
    entity->setProperty(groupName, key, PPropertyValue(floatValue));
  } else {
    // Handle string type
    entity->setProperty(groupName, key, PPropertyValue(valueStr.toStdString()));
  }
  
  // Rebuild geometry with new property value
  entity->rebuild();
  
  // Update GL viewport
  m_glViewport->update();
  
  logToConsole("更新实体属性: " + QString::fromStdString(entity->name()));
  logToConsole("重建实体几何: " + QString::fromStdString(entity->name()));
}

void PMainWindow::onScriptChanged() {
  if (m_selectedEntityId == -1) return;
  
  auto entity = m_document->getEntity(m_selectedEntityId);
  if (!entity) return;
  
  entity->setScriptSource(m_scriptEditor->toPlainText().toStdString());
  
  logToConsole("更新实体脚本: " + QString::fromStdString(entity->name()));
}

void PMainWindow::onRebuildGeometry() {
  if (m_selectedEntityId == -1) {
    // Rebuild all entities
    for (const auto& [id, entity] : m_document->entities()) {
      entity->rebuild();
    }
    logToConsole("重建所有实体几何");
  } else {
    // Rebuild selected entity
    auto entity = m_document->getEntity(m_selectedEntityId);
    if (entity) {
      entity->rebuild();
      logToConsole("重建实体几何: " + QString::fromStdString(entity->name()));
    }
  }
  
  // Update GL viewport
  m_glViewport->update();
}

void PMainWindow::updateSceneTree() {
  // Clear existing items
  m_sceneTree->clear();
  
  // Add entities to scene tree
  for (const auto& [id, entity] : m_document->entities()) {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, QString::fromStdString(entity->name()) + " (" + QString::number(id) + ")");
    item->setData(0, Qt::UserRole, id);
    m_sceneTree->addTopLevelItem(item);
  }
}

void PMainWindow::updatePropertyEditor() {
  if (m_selectedEntityId == -1) {
    m_propertyLabel->setText("请选择一个实体来编辑属性");
    m_propertyEditor->setEnabled(false);
    m_propertyEditor->setRowCount(0);
    return;
  }
  
  auto entity = m_document->getEntity(m_selectedEntityId);
  if (!entity) {
    m_propertyLabel->setText("未找到实体");
    m_propertyEditor->setEnabled(false);
    m_propertyEditor->setRowCount(0);
    return;
  }
  
  m_propertyLabel->setText("实体属性: " + QString::fromStdString(entity->name()));
  m_propertyEditor->setEnabled(true);
  
  // Clear existing rows
  m_propertyEditor->setRowCount(0);
  
  // Add properties to table
  for (const auto& [groupName, group] : entity->propertyGroups()) {
    // Add group header
    int row = m_propertyEditor->rowCount();
    m_propertyEditor->insertRow(row);
    QTableWidgetItem* groupHeaderItem = new QTableWidgetItem("[" + QString::fromStdString(groupName) + "]");
    groupHeaderItem->setFlags(groupHeaderItem->flags() & ~Qt::ItemIsEditable);
    groupHeaderItem->setBackground(QColor(240, 240, 240));
    m_propertyEditor->setItem(row, 0, groupHeaderItem);
    m_propertyEditor->setSpan(row, 0, 1, 2);
    
    // Add properties in this group
    for (const auto& [key, value] : group) {
      row = m_propertyEditor->rowCount();
      m_propertyEditor->insertRow(row);
      
      QTableWidgetItem* keyItem = new QTableWidgetItem(key.c_str());
      keyItem->setFlags(keyItem->flags() & ~Qt::ItemIsEditable);
      m_propertyEditor->setItem(row, 0, keyItem);
      
      QString valueStr;
      std::visit([&valueStr](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, float>) {
          valueStr = QString::number(arg);
        } else if constexpr (std::is_same_v<T, int>) {
          valueStr = QString::number(arg);
        } else if constexpr (std::is_same_v<T, bool>) {
          valueStr = arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::string>) {
          valueStr = QString::fromStdString(arg);
        } else if constexpr (std::is_same_v<T, std::vector<float>>) {
          valueStr = "[";
          for (size_t i = 0; i < arg.size(); ++i) {
            valueStr += QString::number(arg[i]);
            if (i < arg.size() - 1) {
              valueStr += ", ";
            }
          }
          valueStr += "]";
        }
      }, value);
      
      QTableWidgetItem* valueItem = new QTableWidgetItem(valueStr);
      m_propertyEditor->setItem(row, 1, valueItem);
    }
  }
  
  // Resize columns to fit content
  m_propertyEditor->resizeColumnsToContents();
  
  // Set minimum column widths
  m_propertyEditor->setColumnWidth(0, 150); // 加宽“属性”列
  m_propertyEditor->setColumnWidth(1, 150); // 加宽“值”列
}

void PMainWindow::updateScriptEditor() {
  if (m_selectedEntityId == -1) {
    m_scriptEditor->setEnabled(false);
    m_scriptEditor->setPlainText("void generate() {\n  // 在这里添加几何生成代码\n}\n");
    return;
  }
  
  auto entity = m_document->getEntity(m_selectedEntityId);
  if (!entity) {
    m_scriptEditor->setEnabled(false);
    m_scriptEditor->setPlainText("void generate() {\n  // 在这里添加几何生成代码\n}\n");
    return;
  }
  
  m_scriptEditor->setEnabled(true);
  m_scriptEditor->setReadOnly(false);
  m_scriptEditor->setPlainText(entity->scriptSource().c_str());
}

void PMainWindow::logToConsole(const QString& message) {
  m_console->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + ": " + message);
}

void PMainWindow::onLoadScript() {
  QString fileName = QFileDialog::getOpenFileName(this, "加载脚本", "", "C Files (*.c);;All Files (*)");
  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in(&file);
      m_scriptEditor->setPlainText(in.readAll());
      file.close();
      logToConsole("加载脚本文件: " + fileName);
    } else {
      logToConsole("错误: 无法打开脚本文件: " + fileName);
    }
  }
}

void PMainWindow::onSaveScript() {
  QString fileName = QFileDialog::getSaveFileName(this, "保存脚本", "", "C Files (*.c);;All Files (*)");
  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      out << m_scriptEditor->toPlainText();
      file.close();
      logToConsole("保存脚本文件: " + fileName);
    } else {
      logToConsole("错误: 无法保存脚本文件: " + fileName);
    }
  }
}

void PMainWindow::setupMenuBar() {
  // Create menu bar
  QMenuBar* menuBar = new QMenuBar(this);
  setMenuBar(menuBar);
  
  // Create help menu
  QMenu* helpMenu = menuBar->addMenu("帮助");
  
  // Create about action
  QAction* aboutAction = new QAction(QIcon::fromTheme("help-about"), "关于", this);
  connect(aboutAction, &QAction::triggered, this, &PMainWindow::onAbout);
  helpMenu->addAction(aboutAction);
}

void PMainWindow::onAbout() {
  // Create about dialog
  QDialog* aboutDialog = new QDialog(this);
  aboutDialog->setWindowTitle("关于 CMeshStudio");
  aboutDialog->setFixedSize(450, 350);
  
  // Create layout
  QVBoxLayout* layout = new QVBoxLayout(aboutDialog);
  
  // Add logo or icon
  QLabel* logoLabel = new QLabel(aboutDialog);
  logoLabel->setPixmap(QIcon::fromTheme("application-x-executable").pixmap(64, 64));
  logoLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(logoLabel);
  
  // Add application name and version
  QLabel* nameLabel = new QLabel("CMeshStudio", aboutDialog);
  nameLabel->setAlignment(Qt::AlignCenter);
  QFont nameFont = nameLabel->font();
  nameFont.setBold(true);
  nameFont.setPointSize(16);
  nameLabel->setFont(nameFont);
  layout->addWidget(nameLabel);
  
  QLabel* versionLabel = new QLabel("版本 1.0.0", aboutDialog);
  versionLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(versionLabel);
  
  // Add separator
  QFrame* separator = new QFrame(aboutDialog);
  separator->setFrameShape(QFrame::HLine);
  separator->setFrameShadow(QFrame::Sunken);
  layout->addWidget(separator);
  
  // Add description
  QLabel* descriptionLabel = new QLabel("CMeshStudio 是一个基于 Qt 和 OpenGL 的 3D 几何建模工具，支持脚本化几何生成。", aboutDialog);
  descriptionLabel->setWordWrap(true);
  descriptionLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(descriptionLabel);
  
  // Add email
  QLabel* emailLabel = new QLabel("邮箱: H3bing@163.com", aboutDialog);
  emailLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(emailLabel);
  
  // Add dependencies
  QLabel* dependenciesLabel = new QLabel("第三方依赖库:", aboutDialog);
  dependenciesLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(dependenciesLabel);
  
  QLabel* dependenciesListLabel = new QLabel("- Qt 6.10.1 (https://www.qt.io/)\n- OpenGL 3.3+ (https://www.opengl.org/)\n- TCC (Tiny C Compiler) (https://bellard.org/tcc/)\n- nlohmann/json (https://github.com/nlohmann/json)", aboutDialog);
  dependenciesListLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(dependenciesListLabel);
  
  // Add GitHub repository
  QLabel* githubLabel = new QLabel("GitHub 仓库:", aboutDialog);
  githubLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(githubLabel);
  
  QLabel* githubUrlLabel = new QLabel("https://github.com/H3bing/CMeshStudio", aboutDialog);
  githubUrlLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(githubUrlLabel);
  
  // Add button
  QPushButton* okButton = new QPushButton("确定", aboutDialog);
  connect(okButton, &QPushButton::clicked, aboutDialog, &QDialog::accept);
  layout->addWidget(okButton, 0, Qt::AlignCenter);
  
  // Show dialog
  aboutDialog->exec();
}

void PMainWindow::onRunScript() {
  onRebuildGeometry();
  logToConsole("运行脚本");
}
