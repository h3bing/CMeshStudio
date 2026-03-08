#include "csyntaxhighlighter.h"

CSyntaxHighlighter::CSyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
{
  // Initialize formats
  m_keywordFormat.setForeground(Qt::darkBlue);
  m_keywordFormat.setFontWeight(QFont::Bold);
  
  m_typeFormat.setForeground(Qt::darkGreen);
  m_typeFormat.setFontWeight(QFont::Bold);
  
  m_commentFormat.setForeground(Qt::gray);
  m_commentFormat.setFontItalic(true);
  
  m_stringFormat.setForeground(Qt::red);
  
  m_numberFormat.setForeground(Qt::darkMagenta);
  
  m_preprocessorFormat.setForeground(Qt::darkCyan);
  m_preprocessorFormat.setFontWeight(QFont::Bold);
  
  m_functionFormat.setForeground(Qt::darkMagenta);
  m_functionFormat.setFontWeight(QFont::Bold);
  
  m_operatorFormat.setForeground(Qt::darkYellow);
  
  m_bracketFormat.setForeground(Qt::black);
  m_bracketFormat.setFontWeight(QFont::Bold);
  
  m_booleanFormat.setForeground(Qt::darkRed);
  m_booleanFormat.setFontWeight(QFont::Bold);
  
  m_macroFormat.setForeground(Qt::magenta);
  m_macroFormat.setFontWeight(QFont::Bold);
  
  m_enumFormat.setForeground(Qt::darkYellow);
  m_enumFormat.setFontWeight(QFont::Bold);
  
  // Add highlighting rules
  QStringList keywordPatterns = {
    "\\bvoid\\b", "\\bint\\b", "\\bfloat\\b", "\\bdouble\\b", "\\bchar\\b",
    "\\bbool\\b", "\\bif\\b", "\\belse\\b", "\\bfor\\b", "\\bwhile\\b",
    "\\bdo\\b", "\\bswitch\\b", "\\bcase\\b", "\\bdefault\\b", "\\breturn\\b",
    "\\bbreak\\b", "\\bcontinue\\b", "\\bstruct\\b", "\\bunion\\b", "\\benum\\b",
    "\\bconst\\b", "\\bstatic\\b", "\\bextern\\b", "\\bvolatile\\b", "\\btypedef\\b",
    "\\bsigned\\b", "\\bunsigned\\b", "\\bshort\\b", "\\blong\\b", "\\bregister\\b",
    "\\bauto\\b", "\\brestrict\\b", "\\binline\\b", "\\bnoreturn\\b", "\\bthread_local\\b",
    "\\bsize_t\\b", "\\bssize_t\\b", "\\bptrdiff_t\\b", "\\bintptr_t\\b", "\\buintptr_t\\b",
    "\\bFILE\\b", "\\bfpos_t\\b", "\\bclock_t\\b", "\\btime_t\\b", "\\bva_list\\b"
  };
  
  for (const QString& pattern : keywordPatterns) {
    HighlightingRule rule;
    rule.pattern = QRegularExpression(pattern);
    rule.format = m_keywordFormat;
    m_highlightingRules.append(rule);
  }
  
  QStringList typePatterns = {
    "\\bint\\b", "\\bfloat\\b", "\\bdouble\\b", "\\bchar\\b", "\\bbool\\b",
    "\\bsigned\\b", "\\bunsigned\\b", "\\bshort\\b", "\\blong\\b",
    "\\bvoid\\b", "\\bsize_t\\b", "\\bssize_t\\b", "\\bptrdiff_t\\b",
    "\\bintptr_t\\b", "\\buintptr_t\\b"
  };
  
  for (const QString& pattern : typePatterns) {
    HighlightingRule rule;
    rule.pattern = QRegularExpression(pattern);
    rule.format = m_typeFormat;
    m_highlightingRules.append(rule);
  }
  
  // Boolean constants
  QStringList booleanPatterns = {"\\btrue\\b", "\\bfalse\\b"};
  for (const QString& pattern : booleanPatterns) {
    HighlightingRule rule;
    rule.pattern = QRegularExpression(pattern);
    rule.format = m_booleanFormat;
    m_highlightingRules.append(rule);
  }
  
  // String rule - using raw string literal to avoid escape issues
  HighlightingRule stringRule;
  stringRule.pattern = QRegularExpression(R"("(?:\\.|[^"\\])*")");
  stringRule.format = m_stringFormat;
  m_highlightingRules.append(stringRule);
  
  // Character literal rule
  HighlightingRule charRule;
  charRule.pattern = QRegularExpression(R"('(?:\\.|[^'\\])*')");
  charRule.format = m_stringFormat;
  m_highlightingRules.append(charRule);
  
  // Number rule (decimal, hex, octal, scientific notation)
  HighlightingRule numberRule;
  numberRule.pattern = QRegularExpression("\\b(?:0x[0-9a-fA-F]+|0[0-7]*|\\d+(?:\\.\\d*)?(?:[eE][+-]?\\d+)?|\\.\\d+(?:[eE][+-]?\\d+)?)\\b");
  numberRule.format = m_numberFormat;
  m_highlightingRules.append(numberRule);
  
  // Comment rule (single line)
  HighlightingRule commentRule;
  commentRule.pattern = QRegularExpression("//[\\s\\S]*$");
  commentRule.format = m_commentFormat;
  m_highlightingRules.append(commentRule);
  
  // Preprocessor directive rule
  HighlightingRule preprocessorRule;
  preprocessorRule.pattern = QRegularExpression("^\\s*#\\s*(?:define|include|if|ifdef|ifndef|else|elif|endif|pragma|error|warning|line|undef|ifdef|ifndef|defined)\\b");
  preprocessorRule.format = m_preprocessorFormat;
  m_highlightingRules.append(preprocessorRule);
  
  // Macro rule (identifiers defined with #define)
  HighlightingRule macroRule;
  macroRule.pattern = QRegularExpression("\\b[A-Z_][A-Z0-9_]*\\b");
  macroRule.format = m_macroFormat;
  m_highlightingRules.append(macroRule);
  
  // Function name rule (improved to handle more cases)
  HighlightingRule functionRule;
  functionRule.pattern = QRegularExpression("\\b[a-zA-Z_][a-zA-Z0-9_]*\\s*(?=\\()");
  functionRule.format = m_functionFormat;
  m_highlightingRules.append(functionRule);
  
  // Operator rule (enhanced to include compound operators)
  HighlightingRule operatorRule;
  operatorRule.pattern = QRegularExpression("[+\\-*/%=!<>&|^~\\?:]|\\+=|\\-=|\\*=|\\/=|\\%=|\\&=|\\|=|\\^=|\\<<=|\\>>=|\\==|\\!=|\\<=|\\>=|\\&&|\\|||\\<<|\\>>|\\++|\\--");
  operatorRule.format = m_operatorFormat;
  m_highlightingRules.append(operatorRule);
  
  // Bracket rule
  HighlightingRule bracketRule;
  bracketRule.pattern = QRegularExpression(R"([()\[\]{}])");
  bracketRule.format = m_bracketFormat;
  m_highlightingRules.append(bracketRule);
  
  // Enumeration constant rule
  HighlightingRule enumRule;
  enumRule.pattern = QRegularExpression("\\b[A-Z_][a-zA-Z0-9_]*\\b(?=\\s*,|\\s*\\})");
  enumRule.format = m_enumFormat;
  m_highlightingRules.append(enumRule);
}

void CSyntaxHighlighter::highlightBlock(const QString& text)
{
  for (const HighlightingRule& rule : m_highlightingRules) {
    QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
  
  // Handle multi-line comments
  setCurrentBlockState(0);
  
  int startIndex = 0;
  if (previousBlockState() != 1)
    startIndex = text.indexOf("/*");
  
  while (startIndex >= 0) {
    int endIndex = text.indexOf("*/", startIndex);
    int commentLength;
    if (endIndex == -1) {
      setCurrentBlockState(1);
      commentLength = text.length() - startIndex;
    } else {
      commentLength = endIndex - startIndex + 2;
    }
    setFormat(startIndex, commentLength, m_commentFormat);
    startIndex = text.indexOf("/*", startIndex + commentLength);
  }
}
