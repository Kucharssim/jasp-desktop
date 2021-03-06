#include "listmodelfiltereddataentry.h"
#include "analysis/analysisform.h"
#include "tableviewbase.h"
#include "utilities/qutils.h"
#include "log.h"
#include "analysis/jaspcontrol.h"
#include "data/columnsmodel.h"

ListModelFilteredDataEntry::ListModelFilteredDataEntry(TableViewBase * parent)
	: ListModelTableViewBase(parent)
{
	_keepRowsOnReset = false;
	setAcceptedRowsTrue();

	connect(this,				&ListModelFilteredDataEntry::filterChanged,		this, &ListModelFilteredDataEntry::runFilter				);
	connect(DataSetTableModel::singleton(),	&DataSetTableModel::modelReset,		this, &ListModelFilteredDataEntry::dataSetChangedHandler,	Qt::QueuedConnection	);
	connect(_tableView,			SIGNAL(filterSignal(QString)),					this, SLOT(setFilter(QString))								);
	connect(_tableView,			SIGNAL(colNameSignal(QString)),					this, SLOT(setColName(QString))								);
	connect(_tableView,			SIGNAL(extraColSignal(QString)),				this, SLOT(setExtraCol(QString))							);

}

void ListModelFilteredDataEntry::dataSetChangedHandler()
{
	setAcceptedRowsTrue();
	runFilter(_filter);
}

void ListModelFilteredDataEntry::setFilter(QString filter)
{
	if (_filter == filter)
		return;

	_filter = filter;
	emit filterChanged(_filter);
}

void ListModelFilteredDataEntry::runFilter(QString filter)
{
	//std::cout << "ListModelFilteredDataEntry::runFilter(" << filter.toStdString() << ")" << std::endl;

	if (getDataSetRowCount() > 0)
		runRScript(	"filterResult <- {" + filter + "};"										"\n"
					"if(!is.logical(filterResult)) filterResult <- rep(TRUE, rowcount);"	"\n"
					"return(paste0(sep=' ', collapse='', as.character(filterResult)));"		"\n"
		);
}

size_t ListModelFilteredDataEntry::getDataSetRowCount() const
{
	return size_t(DataSetTableModel::singleton()->rowCount());
}

void ListModelFilteredDataEntry::rScriptDoneHandler(const QString & result)
{
	Log::log() << "ListModelFilteredDataEntry::rScriptDoneHandler: " << result << std::endl;

	QStringList values = result.split(' ');

	size_t dataSetRows = getDataSetRowCount();

	auto newRows = std::vector<bool>(dataSetRows, true);

	if (dataSetRows == 0)
		return;

	size_t i = 0;
	for (const QString & value : values)
		if (value == "TRUE" || value == "FALSE")
		{
			if(i < dataSetRows)
				newRows[i] = value == "TRUE";
			i++;
		}


	setAcceptedRows(i == dataSetRows ? newRows : std::vector<bool>(dataSetRows, true));
}

void ListModelFilteredDataEntry::setAcceptedRows(std::vector<bool> newRows)
{
	//std::cout << "setAcceptedRows(# newRows == " << newRows.size() << ")" << std::endl;
	bool changed = newRows.size() != _acceptedRows.size();

	if (changed)
		_acceptedRows = newRows;
	else
		for (size_t i=0; i<_acceptedRows.size(); i++)
			if (_acceptedRows[i] != newRows[i])
			{
				_acceptedRows[i] = newRows[i];
				changed = true;
			}

	if (changed)
	{
		emit acceptedRowsChanged();
		fillTable();
	}
}

void ListModelFilteredDataEntry::itemChanged(int column, int row, QVariant value, QString)
{
	if (column != _editableColumn)
		return;

	//std::cout << "ListModelFilteredDataEntry::itemChanged(" << column << ", " << row << ", " << value << ")" << std::endl;

	//If changing this function also take a look at it's counterpart in ListModelTableViewBase
	if (column > -1 && column < columnCount() && row > -1 && row < _tableTerms.rowNames.length())
	{
		if (_tableTerms.values[0][row] != value)
		{
			bool gotLarger							= _tableTerms.values[0][row].toString().size() != value.toString().size();
			_tableTerms.values[0][row]							= value.toDouble();
			_enteredValues[_filteredRowToData[size_t(row)]] = value.toDouble();

			emit dataChanged(index(row, column), index(row, column), { Qt::DisplayRole });

			if (gotLarger)
				emit headerDataChanged(Qt::Orientation::Horizontal, column, column);


		}
	}
}

Qt::ItemFlags ListModelFilteredDataEntry::flags(const QModelIndex & index) const
{

	return Qt::ItemIsEnabled | (index.column() == _editableColumn ? (Qt::ItemIsSelectable | Qt::ItemIsEditable) : Qt::NoItemFlags );
}


void ListModelFilteredDataEntry::sourceTermsReset()
{
	//std::cout << "ListModelFilteredDataEntry::sourceTermsChanged(Terms *, Terms *)" << std::endl;

	Terms sourceTerms		= getSourceTerms();
	QString colName			= (_editableColumn >= 0 && _editableColumn < _tableTerms.colNames.size()) ? _tableTerms.colNames[_editableColumn] : "";
	_dataColumns			= sourceTerms.asQList();
	_tableTerms.colNames	= _dataColumns;

	if (_extraCol != "")
		_tableTerms.colNames.push_back(_extraCol);

	if (!colName.isEmpty())
	{
		_editableColumn		= _tableTerms.colNames.size();
		_tableTerms.colNames.push_back(colName);
	}
	else
		_editableColumn = -1;

	fillTable();
}

void ListModelFilteredDataEntry::initTableTerms(const TableTerms& terms)
{
	//std::cout << "ListModelFilteredDataEntry::initValues(OptionsTable * bindHere)" << std::endl;

	if (terms.values.size() > 1)
		Log::log() << "Too many values in ListModelFilteredDataEntry" << std::endl;

	if (terms.values.size() == 0)
	{
		fillTable();
		return;
	}

	_tableTerms = terms;
	setFilter(_tableTerms.filter);
	setColName(_tableTerms.colName);
	setExtraCol(_tableTerms.extraCol);

	_acceptedRows = std::vector<bool>(getDataSetRowCount(), false);

	_enteredValues.clear();

	_dataColumns	= _tableTerms.colNames;

	if (_extraCol != "")
		_tableTerms.colNames.push_back(_extraCol);

	_editableColumn = _colName.isEmpty() ? -1 : _tableTerms.colNames.size();

	if (!_colName.isEmpty()) _tableTerms.colNames.push_back(_colName);

	int valIndex = 0;
	for (int rowIndex : _tableTerms.rowIndices)
	{
		size_t row = static_cast<size_t>(rowIndex) - 1;

		_enteredValues[row] = _tableTerms.values[0][valIndex++].toDouble();
		_acceptedRows[row]	= true;
	}

	fillTable();
}

void ListModelFilteredDataEntry::fillTable()
{
	beginResetModel();

	_filteredRowToData.clear();
	_tableTerms.rowNames.clear();
	_tableTerms.values.clear();

	size_t dataRows = getDataSetRowCount();

	if (_acceptedRows.size() != dataRows)
		_acceptedRows = std::vector<bool>(dataRows, true);


	_tableTerms.values.push_back({});

	for (size_t row=0; row<dataRows; row++)
		if (_acceptedRows[row])
		{
			_filteredRowToData.push_back(row);
			_tableTerms.values[0].push_back(_enteredValues[row]);
			_tableTerms.rowNames.push_back(tq(std::to_string(row + 1)));

		}

	_editableColumn = _colName.isEmpty() ? -1 : (columnCount() - 1);
	endResetModel();

	emit columnCountChanged();
	emit rowCountChanged();
}

QVariant ListModelFilteredDataEntry::data(const QModelIndex &index, int role) const
{
	int		column	= index.column(),
			row		= index.row();

	if(row < 0 || row > _tableTerms.rowNames.size())
		return QVariant();

	if (role != Qt::DisplayRole)
		return ListModelTableViewBase::data(index, role);

	if(column == _editableColumn)
		return QVariant(_tableTerms.values[0][row]);

	if(getDataSetRowCount() == 0 || column > _tableTerms.colNames.size() || column < 0)
		return QVariant();

	std::string colName = _tableTerms.colNames[column].toStdString();
	size_t rowData		= _filteredRowToData[static_cast<size_t>(row)];
	DataSetTableModel* dataSetModel = DataSetTableModel::singleton();
	int colIndex = dataSetModel->getColumnIndex(colName);

	return dataSetModel->data(dataSetModel->index(int(rowData), colIndex), role);
}


int ListModelFilteredDataEntry::getMaximumColumnWidthInCharacters(size_t column) const
{
	int colIndex = int(column);

	if (colIndex == _editableColumn)
		return ListModelTableViewBase::getMaximumColumnWidthInCharacters(0);


	DataSetTableModel* dataSetModel = DataSetTableModel::singleton();

	if (!(dataSetModel->columnCount() >= 0 || colIndex > _tableTerms.colNames.size() || colIndex < 0))
	{
		std::string colName		= _tableTerms.colNames[colIndex].toStdString();
		int			colIndex	= dataSetModel->getColumnIndex(colName);

		if (colIndex > -1)
			return int(dataSetModel->getMaximumColumnWidthInCharacters(colIndex));
	}


	return 6;
}

void ListModelFilteredDataEntry::setColName(QString colName)
{
	if (_colName == colName)
		return;

	if (_colName.isEmpty())
	{
		_tableTerms.colNames.push_back(colName);
		_editableColumn = _tableTerms.colNames.size() - 1;
	}
	else if (colName.isEmpty())
	{
		if (_tableTerms.colNames.size() > 0) _tableTerms.colNames.pop_back();
		_editableColumn = -1;
	}
	else if (_tableTerms.colNames.size() > _editableColumn)
	{
		if (_editableColumn >= 0)
			_tableTerms.colNames[_editableColumn]	= colName;
		else
			Log::log() << "Warning: editableColumn is negative!" << std::endl;
	}

	_colName = colName;
	emit colNameChanged(_colName);
	refresh();

	if (_editableColumn >= 0)
		emit headerDataChanged(Qt::Horizontal, _editableColumn, _editableColumn);

}

void ListModelFilteredDataEntry::setExtraCol(QString extraCol)
{
	if (_extraCol == extraCol)
		return;

	//std::cout << "ListModelFilteredDataEntry::setExtraCol("<< extraCol.toStdString() <<")" << std::endl;

	QString oldExtraCol = _extraCol;

	_extraCol = extraCol;


	beginResetModel();
	if (extraCol == "" && _tableTerms.colNames.size() > _editableColumn && _editableColumn > 0 && _tableTerms.colNames[_editableColumn - 1] == oldExtraCol)
	{
		//std::cout << "Leegmaken!" << std::endl;

		_tableTerms.colNames.erase(_tableTerms.colNames.begin() + _editableColumn - 1);
		_editableColumn--;

		//emit headerDataChanged(Qt::Horizontal, _editableColumn, _colNames.size() + 1);
		//emit dataChanged(index(0, _editableColumn), index(static_cast<int>(getDataSetRowCount()), _colNames.size() + 1));

	}
	else if (oldExtraCol == "" && _tableTerms.colNames.size() > 0)
	{
		//std::cout << "Volmaken!" << std::endl;

		if (_editableColumn >= 0 && !_colName.isEmpty())
		{
			_tableTerms.colNames[_editableColumn] = extraCol;
			_editableColumn++;
			_tableTerms.colNames.push_back(_colName);
		}
		else
			_tableTerms.colNames.push_back(extraCol);


		//emit headerDataChanged(Qt::Horizontal, _editableColumn - 1, _colNames.size());
		//emit dataChanged(index(0, _editableColumn - 1), index(static_cast<int>(getDataSetRowCount()), _colNames.size()));

	}
	else if (oldExtraCol != "" && extraCol != "" && _tableTerms.colNames.size() > 0)
	{
		_tableTerms.colNames[_tableTerms.colNames.size() - 1] = extraCol;
		//emit headerDataChanged(Qt::Horizontal, _editableColumn - 1, _editableColumn - 1);
		//emit dataChanged(index(0, _editableColumn - 1), index(static_cast<int>(getDataSetRowCount()), _editableColumn - 1));
	}

	endResetModel();

	emit columnCountChanged();
	emit extraColChanged(_extraCol);
}

void ListModelFilteredDataEntry::refreshModel()
{
	ListModel::refresh();

	runFilter(_filter);
}
