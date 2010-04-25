#ifndef SELECT_PROGRAM_DIALOG_H
#define SELECT_PROGRAM_DIALOG_H

#include <QDialog>

#include <cow/cow.hpp>


namespace Ui {
    class select_program_dialog;
}

/**
 * Shows a dialog with a list of program names 
 * and lets the user choose of them.
 */
class select_program_dialog : public QDialog {
    Q_OBJECT
public:
    /**
     * Creates and initializes a new dialog
     *
     * @param cow_client A pointer to client which the main application uses
     * @param parent A pointer to the creator of the window
     */
    select_program_dialog(QWidget *parent = 0);
    ~select_program_dialog();

    /**
     * Fills the list widget with program names which are
     * being fetched from the a program_table_server.
     */
    void populate_list();
    
    const libcow::program_info* selected_program()
    {
        if(connected_ && selected_program_index_ >= 0)
        {
            return &prog_table_.at(selected_program_index_);
        } else {
            return 0;
        }
    }

    /**
     * Checks if the list widget has been populated with program names.
     *
     * @return true if the list is already populatd, otherwise false
     */
    bool is_populated()
    {
        return is_populated_;
    }

    void set_program_table_url(const std::string& url) { program_table_url_ = url; }

protected:
    void changeEvent(QEvent *e);

private:
    Ui::select_program_dialog *ui;

    libcow::program_table prog_table_;

    std::string program_table_url_;

    int selected_program_index_;

    bool is_populated_;
    bool connected_;

private slots:
    void on_buttonBox_accepted();
};

#endif // SELECT_PROGRAM_DIALOG_H
