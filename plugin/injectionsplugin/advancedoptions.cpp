#include "advancedoptions.h"
#include "ui_advancedoptions.h"
#include <QDesktopServices>

advancedOptions::advancedOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::advancedOptions)
{
    ui->setupUi(this);
}

void advancedOptions::updateConfigurationPath(const QString& path)
{
    ui->confPath->setText(path);
    m_configurationPath = path;
}

advancedOptions::~advancedOptions()
{
    delete ui;
}

void advancedOptions::on_pushButton_clicked()
{
    QString urlString = "file:///" + m_configurationPath;
    //QUrl url("file:///D:" ,QUrl::TolerantMode);
    QUrl url(urlString ,QUrl::TolerantMode);
    //QDesktopServices::openUrl(QUrl("file:///C:/Program Files", QUrl::TolerantMode));
    QDesktopServices::openUrl(url);
}
