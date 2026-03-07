#ifndef C_SYNTAX_HIGHLIGHTER_H
#define C_SYNTAX_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class CSyntaxHighlighter : public QSyntaxHighlighter
{
public:
  CSyntaxHighlighter(QTextDocument* parent = nullptr);
  
protected:
  void highlightBlock(const QString& text) override;
  
private:
  struct HighlightingRule
  {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  
  QVector<HighlightingRule> m_highlightingRules;
  
  QTextCharFormat m_keywordFormat;
  QTextCharFormat m_typeFormat;
  QTextCharFormat m_commentFormat;
  QTextCharFormat m_stringFormat;
  QTextCharFormat m_numberFormat;
  QTextCharFormat m_booleanFormat;
  QTextCharFormat m_macroFormat;
  QTextCharFormat m_enumFormat;
  QTextCharFormat m_preprocessorFormat;
  QTextCharFormat m_functionFormat;
  QTextCharFormat m_operatorFormat;
  QTextCharFormat m_bracketFormat;
};

#endif // C_SYNTAX_HIGHLIGHTER_H