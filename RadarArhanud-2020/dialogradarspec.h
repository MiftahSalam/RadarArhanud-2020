#ifndef DIALOGRADARSPEC_H
#define DIALOGRADARSPEC_H

#include <QDialog>

namespace Ui {
class DialogRadarSpec;
}

class DialogRadarSpec : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRadarSpec(QWidget *parent = nullptr);
    ~DialogRadarSpec();

private:
    Ui::DialogRadarSpec *ui;
};

#endif // DIALOGRADARSPEC_H
