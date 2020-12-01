//
// Copyright (C) 2013-2020 University of Amsterdam
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.
//

#include "listmodellabelvalueterms.h"
#include "log.h"

ListModelLabelValueTerms::ListModelLabelValueTerms(JASPListControl* listView, const JASPListControl::LabelValueMap& values)
	: ListModelTermsAvailable(listView)
{
	_setLabelValues(values);
}

QVariant ListModelLabelValueTerms::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	const Terms& myTerms = terms();
	size_t row_t = size_t(row);

	if (row < 0 || row_t >= myTerms.size())
		return QVariant();

	if (role == ListModel::ValueRole)
	{
		QString label = myTerms.at(row_t).asQString();
		if (_labelToValueMap.contains(label))
			return _labelToValueMap[label];
		else
			return label;
	}

	return ListModelTermsAvailable::data(index, role);
}

void ListModelLabelValueTerms::resetTermsFromSourceModels(bool )
{
	beginResetModel();

	setLabelValuesFromSource();

	endResetModel();
}

void ListModelLabelValueTerms::initTerms(const Terms &terms, const ListModel::RowControlsOptions &)
{
	// TODO: this is called when a DropDown has no source and is filled with the available columns names...
	// The Available column names should be a source as another one.
	beginResetModel();

	JASPListControl::LabelValueMap values;

	if (_listView->addEmptyValue())
		values.push_back(std::make_pair(_listView->placeholderText(), ""));

	for (const Term& term : terms)
		values.push_back(std::make_pair(term.asQString(), term.asQString()));

	_setLabelValues(values);

	endResetModel();
}


std::vector<std::string> ListModelLabelValueTerms::getValues()
{
	std::vector<std::string> values;
	const Terms& terms = this->terms();
	for (const Term& term : terms)
	{
		QString label = term.asQString();
		QString value = _labelToValueMap.contains(label) ? _labelToValueMap[label] : label;
		values.push_back(value.toStdString());
	}

	return values;
}

QString ListModelLabelValueTerms::getValue(const QString &label)
{
	return _labelToValueMap.contains(label) ? _labelToValueMap[label] : label;
}

QString ListModelLabelValueTerms::getLabel(const QString &value)
{
	return _valueToLabelMap.contains(value) ? _valueToLabelMap[value] : value;
}


int ListModelLabelValueTerms::getIndexOfValue(const QString &value)
{
	int index = 0;
	QString label = getLabel(value);
	for (const Term& term : _terms)
	{
		if (term.asQString() == label)
			return index;
		index++;
	}

	return -1;
}

void ListModelLabelValueTerms::_setLabelValues(const JASPListControl::LabelValueMap &labelvalues)
{
	_valueToLabelMap.clear();
	_labelToValueMap.clear();
	_terms.clear();

	for (const auto& labelValue :labelvalues)
	{
		const QString& label = labelValue.first;
		const QString& value = labelValue.second;
		_terms.add(label);
		_valueToLabelMap[value] = label;
		_labelToValueMap[label] = value;
	}
}

void ListModelLabelValueTerms::setLabelValuesFromSource()
{
	JASPListControl::LabelValueMap values;

	if (_listView->addEmptyValue())
		values.push_back(std::make_pair(_listView->placeholderText(), ""));

	for (const std::pair<JASPListControl::SourceType *, Terms>& source : _listView->getTermsPerSource())
	{
		ListModel* sourceModel = source.first->model;
		ListModelLabelValueTerms* labelValueSourceModel = qobject_cast<ListModelLabelValueTerms*>(sourceModel);
		for (const Term& term : source.second)
		{
			QString label = term.asQString();
			QString value = labelValueSourceModel ? labelValueSourceModel->getValue(label) : label;
			values.push_back(std::make_pair(label, value));
		}
	}

	_setLabelValues(values);
}
