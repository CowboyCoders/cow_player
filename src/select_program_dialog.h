#ifndef SELECT_PROGRAM_DIALOG_H
#define SELECT_PROGRAM_DIALOG_H

#include <QDialog>

#include <cow/cow_client.hpp>
#include <cow/program_info.hpp>


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
    select_program_dialog(libcow::cow_client* client, QWidget *parent = 0);
    ~select_program_dialog();

    /**
     * Fills the list widget with program names which are
     * being fetched from the a program_table_server.
     */
    void populate_list();
    
    /**
     * Returns which movie id the user selected in the list,
     * or -1 if the user didn't select anything.
     *
     * @return the movie id, otherwise -1
     */
    int selected_id()
    {
        return id_;
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


protected:
    void changeEvent(QEvent *e);

private:
    Ui::select_program_dialog *ui;
    libcow::cow_client* client_;
    int id_;
    bool is_populated_;

private slots:
    void on_buttonBox_accepted();
};

#endif // SELECT_PROGRAM_DIALOG_H
