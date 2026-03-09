#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QDateTime>
#include <QSplitter>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QListWidget>
#include <QLineEdit>
#include "geometry.h"
#include "glviewport.h"

class PMainWindow : public QMainWindow
{
private:
  std::shared_ptr<PDocument> m_document;
  
  // UI components
  QTreeWidget* m_sceneTree;
  QTextEdit* m_scriptEditor;
  QTextEdit* m_console;
  PGLViewport* m_glViewport;
  QTableWidget* m_propertyEditor;
  QLabel* m_propertyLabel;
  QListWidget* m_templateList;
  
  // Current selected entity
  int m_selectedEntityId;

public:
  PMainWindow(QWidget* parent = nullptr);
  ~PMainWindow();
  
private slots:
  void onNewEntity();
  void onDeleteEntity();
  void onSceneTreeItemSelected(QTreeWidgetItem* item, int column);
  void onPropertyChanged(int row, int column);
  void onScriptChanged();
  void onRebuildGeometry();
  void onLoadScript();
  void onSaveScript();
  void onRunScript();
  void onExportSTL();
  void onNewTemplate();
  void onEditTemplate();
  void onDeleteTemplate();
  
private:
  void setupUI();
  void setupMenuBar();
  void updateSceneTree();
  void updatePropertyEditor();
  void updateScriptEditor();
  void updateTemplateList();
  void logToConsole(const QString& message);
  void onAbout();
};

#endif // MAINWINDOW_H