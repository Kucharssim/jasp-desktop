#include "restriktorsyntaxhighlighter.h"

RestriktorSyntaxHighlighter::RestriktorSyntaxHighlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{

	HighlightingRule rule;

	// operators
	operatorFormat.setForeground(Qt::darkGreen);
	QStringList operatorPatterns;
	operatorPatterns << "\\=" << "\\<" << "\\>" << "\\:" << "\\+" << "\\-" << "\\*" << "\\/" << "\\;";
	for(const QString &pattern : operatorPatterns) {
		rule.pattern = QRegularExpression(pattern);
		rule.format = operatorFormat;
		highlightingRules.append(rule);
	}

	//comments
	commentFormat.setForeground(Qt::darkGray);
	commentFormat.setFontItalic(true);
	rule.pattern = QRegularExpression("#[^\n]*");
	rule.format = commentFormat;
	highlightingRules.append(rule);
}

void RestriktorSyntaxHighlighter::highlightBlock(const QString &text)
{
	for(const HighlightingRule &rule : highlightingRules)
	{
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext())
		{
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}
}
