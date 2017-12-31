#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    int last_alphabet_length = 0;
    int current_alphabet_length = 0;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void update_alphabet_text();
    void update_input_text();
    void adjust_probabilities();
private slots:
    void on_btnGen_clicked();
    void on_textAlphabet_textChanged();
    void on_btnAdjust_clicked();
    void on_textInput_textChanged();
    void on_radioNoCoding_clicked();
    void on_radioBlock_clicked();
    void on_radioHuffman_clicked();
    void on_radioArith_clicked();
    void on_radioShannon_clicked();
    void on_radioLZ77_clicked();
    void on_radioLZW_clicked();
    void on_adjustCheckbox_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
