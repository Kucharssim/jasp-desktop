#ifndef BOUNDCONTROLRESTRIKTORTEXTAREA_H
#define BOUNDCONTROLRESTRIKTORTEXTAREA_H

#include "boundcontroltextarea.h"
#include "restriktorsyntaxhighlighter.h"

class BoundControlRestriktorTextArea : public BoundControlTextArea
{
public:
	BoundControlRestriktorTextArea(TextAreaBase* textArea);

	bool		isJsonValid(const Json::Value& optionValue)	override;
	Json::Value	createJson()								override;
	void		bindTo(const Json::Value &value)			override;

	void		checkSyntax()								override;
	QString		rScriptDoneHandler(const QString &result)	override;

protected:
	QString	_textEncoded;
	RestriktorSyntaxHighlighter*	_restriktorHighlighter	= nullptr;
};

#endif // BOUNDCONTROLRESTRIKTORTEXTAREA_H
