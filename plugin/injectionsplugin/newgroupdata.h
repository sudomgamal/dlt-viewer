#ifndef NEWGROUPDATA_H
#define NEWGROUPDATA_H

#include <QDialog>

namespace Ui {
class newGroupData;
}

class newGroupData : public QDialog
{
    Q_OBJECT

public:
    explicit newGroupData(QWidget *parent = nullptr);
    ~newGroupData();
    QString getNewGroupName();

private slots:
    void on_pushButton_clicked();

private:
    Ui::newGroupData *ui;
    QString m_newGroupName;
};

#endif // NEWGROUPDATA_H
