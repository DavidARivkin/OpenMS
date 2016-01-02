#ifndef MASSRANGEDIALOG_H
#define MASSRANGEDIALOG_H

#include <QDialog>

namespace Ui {
class MassRangeDialog;
}

class MassRangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MassRangeDialog(QWidget *parent = 0);
    ~MassRangeDialog();

    void setMax(int);
    void setMin(int);
    int getMax(void);
    int getMin(void);

private:
    Ui::MassRangeDialog *ui;
};

#endif // MASSRANGEDIALOG_H
