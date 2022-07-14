#include "newgroupdata.h"
#include "ui_newgroupdata.h"
#include <QDebug>

newGroupData::newGroupData(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::newGroupData)
{
    ui->setupUi(this);
}

newGroupData::~newGroupData()
{
    delete ui;
}

void newGroupData::on_pushButton_clicked()
{
    m_newGroupName = ui->m_newGroupName->toPlainText();
    qDebug() << "updaed m_newGroupName to: " << m_newGroupName;
    close();
}

QString newGroupData::getNewGroupName()
{
    //std::remove_if(m_newGroupName.begin(), m_newGroupName.end(), std::isspace);
    m_newGroupName = m_newGroupName.remove(QRegExp("[ ,;\\`{}()\n]."));
    qDebug() << "returning m_newGroupName: " << m_newGroupName;
    return m_newGroupName;
}
