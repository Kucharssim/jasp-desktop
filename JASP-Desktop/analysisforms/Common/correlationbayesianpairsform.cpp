//
// Copyright (C) 2013-2018 University of Amsterdam
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

#include "correlationbayesianpairsform.h"
#include "ui_correlationbayesianpairsform.h"

#include "widgets/tablemodelvariablesassigned.h"

CorrelationBayesianPairsForm::CorrelationBayesianPairsForm(QWidget *parent) :
	AnalysisForm("CorrelationBayesianPairsForm", parent),
	ui(new Ui::CorrelationBayesianPairsForm)
{
	ui->setupUi(this);

	_availableVariablesModel.setSupportedDropActions(Qt::MoveAction);
	_availableVariablesModel.setSupportedDragActions(Qt::CopyAction);
	_availableVariablesModel.setVariableTypesSuggested(Column::ColumnTypeScale);
	_availableVariablesModel.setVariableTypesAllowed(Column::ColumnTypeScale | Column::ColumnTypeOrdinal | Column::ColumnTypeNominal);

	ui->availableFields->setModel(&_availableVariablesModel);
	ui->availableFields->setDefaultDropAction(Qt::MoveAction);
	ui->availableFields->setDoubleClickTarget(ui->pairs);

	TableModelPairsAssigned *model = new TableModelPairsAssigned(this);
	model->setSource(&_availableVariablesModel);
	model->setVariableTypesSuggested(Column::ColumnTypeScale);
	model->setVariableTypesAllowed(Column::ColumnTypeScale | Column::ColumnTypeOrdinal | Column::ColumnTypeNominal);
	ui->pairs->setModel(model);

	ui->assignButton->setSourceAndTarget(ui->availableFields, ui->pairs);


    ui->availableCondVars->setModel(&_availableVariablesModel);

    _condVariables = new TableModelVariablesAssigned(this);
    _condVariables->setVariableTypesSuggested(Column::ColumnTypeScale);
    _condVariables->setVariableTypesAllowed(Column::ColumnTypeNominal | Column::ColumnTypeOrdinal | Column::ColumnTypeScale);
    _condVariables->setSource(&_availableVariablesModel);
    ui->condVars->setModel(_condVariables);
    ui->availableCondVars->setDoubleClickTarget(ui->condVars);
    ui->assignCondVars->setSourceAndTarget(ui->availableCondVars, ui->condVars);

    //ui->_availableCondVariables->setModel(&_availableVariablesModel);
    //_availableCondVariables = new TableModelVariablesAvailable();

    //_availableCondVariables->setSource(_availableVariablesModel);
    //ui->availableCondVars->setModel(&_availableCondVariables);

    //_condVariables = new TableModelVariablesAssigned(this);
    //_condVariables->setSource(&_availableCondVariables);
    //_condVariables->setVariableTypesAllowed(Column::ColumnTypeNominal | Column::ColumnTypeOrdinal | Column::ColumnTypeScale);
    //_condVariables->setVariableTypesSuggested(Column::ColumnTypeScale | Column::ColumnTypeOrdinal);
    //ui->condVars->setModel(_condVariables);

    //ui->parCorContainer->hide();

#ifndef JASP_DEBUG
	ui->plotSequentialAnalysisRobustness->hide();
    ui->ciValueContainer->hide();
#else
    ui->plotSequentialAnalysisRobustness->hide(); // delete
    ui->ciValueContainer->hide(); //delete
    //ui->plotSequentialAnalysisRobustness->setStyleSheet("background-color: pink;"); //uncomment
    //ui->ciValueContainer->setStyleSheet("background-color: pink;"); // uncomment
#endif
	ui->priorWidth->setLabel("Beta* prior width");
}

CorrelationBayesianPairsForm::~CorrelationBayesianPairsForm()
{
	delete ui;
}
