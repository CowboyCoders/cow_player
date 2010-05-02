#ifndef ___about_dialog_h___
#define ___about_dialog_h___

#include <QDialog>
#include "ui_about_dialog.h"

class about_dialog : public QDialog {
    Q_OBJECT
public:
    about_dialog(QWidget *parent = 0) :
        QDialog(parent),
        ui(new Ui::about_dialog)
    {
        ui->setupUi(this);
    }
    
    ~about_dialog()
    {
        delete ui;
    }

private:
    Ui::about_dialog *ui;
};

#endif // ___about_dialog_h___
