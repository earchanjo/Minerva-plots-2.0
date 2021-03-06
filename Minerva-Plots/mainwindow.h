#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "axistag.h"
#include "comserial.h"

namespace Ui {
class MainWindow;
}
class QSerialPort;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addPoint();
    void clearData();

private slots:
    void on_btn_add_clicked();
    void on_btn_clear_clicked();
    void on_btn_stop_clicked();
    void fun_plot(QVector<double> temp, QVector<double> x, QVector<double> y);
    void Read_Serial();
    void LerArquivo(QFile &arquivo);
    void on_dev_update_clicked();
    void PassaDados();
    void resetPlot();

    //void on_combo_activated(const QString &arg1);

    void on_btn_save_clicked();

    void on_btn_open_file_clicked();

    void on_combo_green_currentTextChanged(const QString &arg1);

    void on_combo_orange_currentTextChanged(const QString &arg1);

    void on_rd_idpackages_toggled(bool checked);

    void on_rd_seg_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    QSerialPort *arduino;
    comserial *procSerial;
    QString arduino_nome_porta;
    bool arduino_disponivel;
    QPointer<QCPGraph> mGraph1;
    QPointer<QCPGraph> mGraph2;
    AxisTag *mTag1;
    AxisTag *mTag2;
    QTimer mDataTimer;
    QVector<double> qv_altidude, qv_pressao, qv_temperatura, qv_aceleracao;
    QVector<double> qv_temp,qv_id;
    QVector<double> x, y1, y2;
    QFile arquivo_final;
    QString dados_recebidos_global;
};

#endif // MAINWINDOW_H
