/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file filterdialog.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "msgfilterdialog.h"
#include "ui_msgfilterdialog.h"
#include <QMessageBox>
#include <QCloseEvent>
#include "dltuiutils.h"

FilterDialog::FilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterDialog)
{
    ui->setupUi(this);

    connect(ui->lineEdit_msgIdMax, SIGNAL(textChanged(const QString&)), this, SLOT(checkMsgIdValid(const QString&)));
    connect(ui->lineEdit_msgIdMin, SIGNAL(textChanged(const QString&)), this, SLOT(checkMsgIdValid(const QString&)));
    connect(ui->checkBoxMessageId, SIGNAL(stateChanged(int)), this, SLOT(on_checkboxMessageId_stateChanged(int)));
    connect(ui->lineEditRegexSearch, SIGNAL(textChanged(const QString&)), this, SLOT(on_checkRegex(const QString&)));
    connect(ui->lineEditRegexReplace, SIGNAL(textChanged(const QString&)), this, SLOT(on_checkRegex(const QString&)));

    ui->lineEdit_msgIdMax->setInputMask("nnhhhhhhhh");
    ui->lineEdit_msgIdMin->setInputMask("nnhhhhhhhh");
    ui->plainTextEdit_msgIdRule->viewport()->setAutoFillBackground(false);
    QFont font=  ui->plainTextEdit_msgIdRule->font();
    font.setFamily("monospace [Consolas]");
    font.setFixedPitch(true);
    font.setStyleHint(QFont::Monospace);
    ui->plainTextEdit_msgIdRule->setFont(font);
    on_checkboxMessageId_stateChanged(0);
}

FilterDialog::~FilterDialog()
{
    delete ui;
}

int FilterDialog::getType()
{
    return 0;
}

void FilterDialog::setEnableRegexp_Appid(bool state)
{
    ui->checkBoxRegexp_Appid->setChecked(state);
}

bool FilterDialog::getEnableRegexp_Appid()
{
    return (ui->checkBoxRegexp_Appid->checkState() == Qt::Checked);
}

void FilterDialog::setEnableRegexp_Context(bool state)
{
    ui->checkBoxRegexp_Context->setChecked(state);
}

bool FilterDialog::getEnableRegexp_Context()
{
    return (ui->checkBoxRegexp_Context->checkState() == Qt::Checked);
}

void FilterDialog::setEnableRegexp_Header(bool state)
{
    ui->checkBoxRegexp_Header->setChecked(state);
}

bool FilterDialog::getEnableRegexp_Header()
{
    return (ui->checkBoxRegexp_Header->checkState() == Qt::Checked);
}

void FilterDialog::setEnableRegexp_Payload(bool state)
{
    ui->checkBoxRegexp_Payload->setChecked(state);
}

bool FilterDialog::getEnableRegexp_Payload()
{
    return (ui->checkBoxRegexp_Payload->checkState() == Qt::Checked);
}

void FilterDialog::setIgnoreCase_Header(bool state)
{
    ui->checkBox_IgnoreCase_Header->setChecked(state);
}

bool FilterDialog::getIgnoreCase_Header()
{
    return (ui->checkBox_IgnoreCase_Header->checkState() == Qt::Checked);
}

void FilterDialog::setIgnoreCase_Payload(bool state)
{
    ui->checkBox_IgnoreCase_Payload->setChecked(state);
}

bool FilterDialog::getIgnoreCase_Payload()
{
    return (ui->checkBox_IgnoreCase_Payload->checkState() == Qt::Checked);
}

void FilterDialog::setEcuId(QString id)
{
    ui->lineEditEcuId->setText(id);
}

QString FilterDialog::getEcuId()
{
    return ui->lineEditEcuId->text();
}

void FilterDialog::setEnableEcuId(bool state)
{
    ui->checkBoxEcuId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableEcuId()
{
    return (ui->checkBoxEcuId->checkState() == Qt::Checked);
}

void FilterDialog::setApplicationId(QString id)
{
    ui->lineEditApplicationId->setText(id);
}

QString FilterDialog::getApplicationId()
{
    return ui->lineEditApplicationId->text();
}

void FilterDialog::setEnableApplicationId(bool state)
{
    ui->checkBoxApplicationId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableApplicationId()
{
    return (ui->checkBoxApplicationId->checkState() == Qt::Checked);
}

void FilterDialog::setContextId(QString id)
{
    ui->lineEditContextId->setText(id);
}

QString FilterDialog::getContextId()
{
    return ui->lineEditContextId->text();
}

void FilterDialog::setEnableContextId(bool state)
{
    ui->checkBoxContextId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableContextId()
{
    return (ui->checkBoxContextId->checkState() == Qt::Checked);
}

void FilterDialog::setHeaderText(QString id)
{
    ui->lineEditHeaderText->setText(id);
}

QString FilterDialog::getHeaderText()
{
    return ui->lineEditHeaderText->text();
}

void FilterDialog::setEnableHeaderText(bool state)
{
    ui->checkBoxHeaderText->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableHeaderText()
{
    return (ui->checkBoxHeaderText->checkState() == Qt::Checked);
}

void FilterDialog::setPayloadText(QString id)
{
    ui->lineEditPayloadText->setText(id);
}

QString FilterDialog::getPayloadText()
{
    return ui->lineEditPayloadText->text();
}

void FilterDialog::setEnablePayloadText(bool state)
{
    ui->checkBoxPayloadText->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnablePayloadText()
{
    return (ui->checkBoxPayloadText->checkState() == Qt::Checked);
}


void FilterDialog::setEnableMessageId(bool state)
{
    ui->checkBoxMessageId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableMessageId()
{
    return (ui->checkBoxMessageId->checkState() == Qt::Checked);
}

unsigned int  FilterDialog::getMessageId_min()
{
    return msgIdMin;
}

unsigned int  FilterDialog::getMessageId_max()
{
    if (msgIdMax==msgIdMin) return 0;
    return msgIdMax;
}

void FilterDialog::setMessageId_min(unsigned int min)
{
    ui->lineEdit_msgIdMin->setText(QString("%1").arg(min));
}

void FilterDialog::setMessageId_max(unsigned int max)
{
    ui->lineEdit_msgIdMax->setText(QString("%1").arg(max));
}

void FilterDialog::setLogLevelMax(int value)
{
    ui->comboBoxLogLevelMax->setCurrentIndex(value);
}

int FilterDialog::getLogLevelMax()
{
    return ui->comboBoxLogLevelMax->currentIndex();
}

void FilterDialog::setEnableLogLevelMax(bool state)
{
    ui->checkBoxLogLevelMax->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableLogLevelMax()
{
    return (ui->checkBoxLogLevelMax->checkState() == Qt::Checked);
}

void FilterDialog::setLogLevelMin(int value)
{
    ui->comboBoxLogLevelMin->setCurrentIndex(value);
}

int FilterDialog::getLogLevelMin()
{
    return ui->comboBoxLogLevelMin->currentIndex();
}

void FilterDialog::setEnableLogLevelMin(bool state)
{
    ui->checkBoxLogLevelMin->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableLogLevelMin()
{
    return (ui->checkBoxLogLevelMin->checkState() == Qt::Checked);
}

void FilterDialog::setEnableCtrlMsgs(bool state)
{
    ui->checkBoxCtrlMsgs->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableCtrlMsgs()
{
    return (ui->checkBoxCtrlMsgs->checkState() == Qt::Checked);
}

void FilterDialog::setEnableRegexSearchReplace(bool state){
    return ui->checkBoxRegexSearchReplace->setChecked(state);
}

bool FilterDialog::getEnableRegexSearchReplace(){
    return ui->checkBoxRegexSearchReplace->checkState() == Qt::Checked;
}

void FilterDialog::setRegexSearchText(const QString& str){
    ui->lineEditRegexSearch->setText(str);
}

QString FilterDialog::getRegexSearchText(){
    return ui->lineEditRegexSearch->text();
}

void FilterDialog::setRegexReplaceText(const QString& str){
    ui->lineEditRegexReplace->setText(str);
}

QString FilterDialog::getRegexReplaceText(){
    return ui->lineEditRegexReplace->text();
}

void FilterDialog::on_lineEditApplicationId_textEdited(const QString &)
{
    if (ui->lineEditApplicationId->text().length())
        ui->checkBoxApplicationId->setCheckState(Qt::Checked);
    else
        ui->checkBoxApplicationId->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_lineEditEcuId_textEdited(const QString &)
{
    if (ui->lineEditEcuId->text().length())
        ui->checkBoxEcuId->setCheckState(Qt::Checked);
    else
        ui->checkBoxEcuId->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_lineEditContextId_textEdited(const QString &)
{
    if (ui->lineEditContextId->text().length())
        ui->checkBoxContextId->setCheckState(Qt::Checked);
    else
        ui->checkBoxContextId->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_lineEditHeaderText_textEdited(const QString &)
{
    if (ui->lineEditHeaderText->text().length())
        ui->checkBoxHeaderText->setCheckState(Qt::Checked);
    else
        ui->checkBoxHeaderText->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_lineEditPayloadText_textEdited(const QString &)
{
    if (ui->lineEditPayloadText->text().length())
        ui->checkBoxPayloadText->setCheckState(Qt::Checked);
    else
        ui->checkBoxPayloadText->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_comboBoxLogLevelMax_currentIndexChanged(int )
{
    ui->checkBoxLogLevelMax->setCheckState(Qt::Checked);
}

void FilterDialog::on_comboBoxLogLevelMin_currentIndexChanged(int )
{
    ui->checkBoxLogLevelMin->setCheckState(Qt::Checked);
}

void FilterDialog::on_checkRegex(const QString&)
{
    if (ui->lineEditRegexSearch->text().length() || ui->lineEditRegexReplace->text().length())
        ui->checkBoxRegexSearchReplace->setCheckState(Qt::Checked);
    else
        ui->checkBoxRegexSearchReplace->setCheckState(Qt::Unchecked);
}

void FilterDialog::validate()
{
    QString
            error =  "Could not parse %1 regular expression. \n";
    error += "Please correct the error or remove the regular expression.\n";
    error += "Expression: '%2' \n";
    error += "Error: %3 ";

    if (!getEnableRegexp_Appid() && 4 < ui->lineEditApplicationId->text().length())
    {
        ui->lineEditApplicationId->selectAll();
        QMessageBox::warning(this, "Warning", "Application Id is more than four characters in length (and not in RegExp mode).");
        ui->lineEditApplicationId->setFocus();
        return;
    }

    if(!(getEnableRegexp_Context()||getEnableRegexp_Header()||getEnableRegexp_Payload()))
    {
        emit accept();
        return;
    }

    QRegExp rx;
    rx.setPattern(getPayloadText());
    if(!rx.isValid()) {
        QMessageBox::warning(this, "Warning", error.arg("PAYLOAD").arg(rx.pattern()).arg(rx.errorString()));
        return;
    }

    rx.setPattern(getHeaderText());
    if(!rx.isValid()) {
        QMessageBox::warning(this, "Warning", error.arg("HEADER").arg(rx.pattern()).arg(rx.errorString()));
        return;
    }

    rx.setPattern(getContextId());
    if(!rx.isValid()) {
        QMessageBox::warning(this, "Warning", error.arg("CONTEXTID").arg(rx.pattern()).arg(rx.errorString()));
        return;
    }

    emit accept();
}

void FilterDialog::checkMsgIdValid(const QString&)
{

    QString hint;
    QString rule="<= ID < ";
    bool ok_min;
    bool ok_max;

    msgIdMin = ui->lineEdit_msgIdMin->text().toUInt(&ok_min,10);
    if (ok_min)
    {
        ui->lineEdit_msgIdMin->setStyleSheet("border: 2px solid blue");
    }
    else
    {
        msgIdMin = ui->lineEdit_msgIdMin->text().toUInt(&ok_min,16);
        ui->lineEdit_msgIdMin->setStyleSheet("border: 2px solid green");
    }
    if (!ok_min)
    {
        ui->lineEdit_msgIdMin->setStyleSheet("border: 2px solid red");
    }

    msgIdMax = ui->lineEdit_msgIdMax->text().toUInt(&ok_max,10);
    if (ok_max)
    {
        ui->lineEdit_msgIdMax->setStyleSheet("border: 2px solid blue");
    }
    else
    {
        msgIdMax = ui->lineEdit_msgIdMax->text().toUInt(&ok_max,16);
        ui->lineEdit_msgIdMax->setStyleSheet("border: 2px solid green");
    }
    if (!ok_max)
    {
        ui->lineEdit_msgIdMax->setStyleSheet("border: 2px solid red");
    }

    if ((!ok_max) && (!ok_min))
    {
        hint.append("[MIN & MAX invalid]");
    }
    else
    {
        if (!ok_min)
            hint.append("[MIN invalid]");
        if (!ok_max)
            hint.append("[MAX invalid]");
    }
    if (msgIdMax==0)
    {
        rule="== ID //";
        ui->lineEdit_msgIdMax->setStyleSheet("border: 2px solid grey");
    }
    else
        if (msgIdMin>=msgIdMax)
        {
            ui->lineEdit_msgIdMax->setStyleSheet("border: 2px dotted grey");
            hint.append("[MAX<=MIN !]");
            rule= "== ID //";
        }
    if (ui->checkBoxMessageId->checkState()==Qt::Checked)
    {
        ui->plainTextEdit_msgIdRule->setVisible(true);
    }
    else
    {
        ui->plainTextEdit_msgIdRule->setVisible(false);
        ui->lineEdit_msgIdMax->setStyleSheet("border: 1px solid black");
        ui->lineEdit_msgIdMin->setStyleSheet("border: 1px solid black");
    }

    ui->plainTextEdit_msgIdRule->setPlainText(
                QString("%1  %2 %3 %7 \n0x%4  %5 0x%6").\
                arg(msgIdMin,10,10,QLatin1Char(' ')).arg(rule).arg(msgIdMax,10,10,QLatin1Char(' ')).\
                arg(msgIdMin, 8,16,QLatin1Char('0')).arg(rule).arg(msgIdMax, 8,16,QLatin1Char('0')).\
                arg(hint)\
                );

}

void FilterDialog::on_checkboxMessageId_stateChanged(int)
{
    checkMsgIdValid("");
}
