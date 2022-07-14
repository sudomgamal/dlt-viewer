#ifndef ADVANCEDOPTIONS_H
#define ADVANCEDOPTIONS_H

#include <QDialog>

namespace Ui {
class advancedOptions;
}

class advancedOptions : public QDialog
{
    Q_OBJECT

public:
    explicit advancedOptions(QWidget *parent = nullptr);
    ~advancedOptions();

    void updateConfigurationPath(const QString& path);

private slots:
    void on_pushButton_clicked();

private:
    Ui::advancedOptions *ui;
    QString m_configurationPath;
};

#endif // ADVANCEDOPTIONS_H
