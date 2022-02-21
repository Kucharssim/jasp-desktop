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

	Json::Value	createMeta()								override;

protected:
	RestriktorSyntaxHighlighter*	_restriktorHighlighter	= nullptr;
};

#endif // BOUNDCONTROLRESTRIKTORTEXTAREA_H
