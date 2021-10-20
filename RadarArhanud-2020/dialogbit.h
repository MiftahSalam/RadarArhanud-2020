#ifndef DIALOGBIT_H
#define DIALOGBIT_H

#include <QDialog>

namespace Ui {
class DialogBIT;
}

class DialogBIT : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBIT(QWidget *parent = nullptr);
    ~DialogBIT();

private slots:
    void on_pushButtonBITRadar_clicked();

    void on_pushButtonBITAnthenaSW_clicked();

    void on_pushButtonBITTilting_clicked();

    void on_pushButtonBITLora_clicked();

    void on_pushButtonBITNav_clicked();

private:
    Ui::DialogBIT *ui;
};

#endif // DIALOGBIT_H
