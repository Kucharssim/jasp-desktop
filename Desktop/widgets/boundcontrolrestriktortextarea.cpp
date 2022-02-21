#include "boundcontrolrestriktortextarea.h"
#include "textareabase.h"
#include "log.h"
#include "columnencoder.h"
#include "analysis/analysisform.h"
#include <QQuickTextDocument>

BoundControlRestriktorTextArea::BoundControlRestriktorTextArea(TextAreaBase *textArea)
	: BoundControlTextArea(textArea)
{

	QVariant textDocumentVariant = textArea->property("textDocument");
	QQuickTextDocument* textDocumentQQuick = textDocumentVariant.value<QQuickTextDocument *>();
	if(textDocumentQQuick)
	{
		QTextDocument* doc = textDocumentQQuick->textDocument();
		_restriktorHighlighter = new RestriktorSyntaxHighlighter(doc);
	}
	else
		Log::log()	<< "No document object found!" << std::endl;
}

void BoundControlRestriktorTextArea::bindTo(const Json::Value &value)
{
	if (value.type() != Json::objectValue) return;
	BoundControlBase::bindTo(value);
}

Json::Value BoundControlRestriktorTextArea ::createMeta()
{
	Json::Value meta(BoundControlBase::createMeta());

	meta["shouldEncode"] = true;

	return meta;
}

Json::Value BoundControlRestriktorTextArea::createJson()
{
	std::string text = _textArea->text().toStdString();

	return text;
}

bool BoundControlRestriktorTextArea::isJsonValid(const Json::Value &value)
{
	if (!value.isObject())						return false;
	if (!value.isString())						return false;

	return true;
}
