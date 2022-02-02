#ifndef RESTRIKTORSYNTAXHIGHLIGHTER_H
#define RESTRIKTORSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCursor>
#include <QRegularExpression>

class RestriktorSyntaxHighlighter : public QSyntaxHighlighter
{
public:
	RestriktorSyntaxHighlighter(QTextDocument *parent);
	virtual void highlightBlock(const QString &text) override;
private:
	struct HighlightingRule
	{
		QRegularExpression pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> highlightingRules;
	QTextCharFormat operatorFormat;
	QTextCharFormat commentFormat;
};

#endif // RESTRIKTORSYNTAXHIGHLIGHTER_H
