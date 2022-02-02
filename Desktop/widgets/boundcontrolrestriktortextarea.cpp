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

	_textArea->setText(tq(value["syntaxOriginal"].asString()));

	checkSyntax();
}

Json::Value BoundControlRestriktorTextArea::createJson()
{
	Json::Value result;
	std::string text = _textArea->text().toStdString();

	result["syntaxOriginal"]	=	text;
	result["syntax"]			=	text;

	return result;
}

bool BoundControlRestriktorTextArea::isJsonValid(const Json::Value &value)
{
	if (!value.isObject())						return false;
	if (!value["syntaxOriginal"].isString())	return false;
	if (!value["syntax"].isString())			return false;

	return true;
}

void BoundControlRestriktorTextArea::checkSyntax()
{
	QString text = _textArea->text();
	_textEncoded = tq(ColumnEncoder::columnEncoder()->encodeAll(fq(text)));

	// Here we will call a syntax checker from jaspAnova/restriktor, but that is not written yet...
	_textArea->runRScript("TRUE", true);
}

QString BoundControlRestriktorTextArea::rScriptDoneHandler(const QString &result)
{
	if (!result.isEmpty())
		return result;

	Json::Value boundValue(Json::objectValue);

	boundValue["syntaxOriginal"]	=	_textArea->text().toStdString();
	boundValue["syntax"]			=	_textEncoded.toStdString();

	setBoundValue(boundValue, !_control->form()->analysisObj()->wasUpgraded());
	return QString();
}
