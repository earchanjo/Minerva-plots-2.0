#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <stdio.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QFile>
/*PROGAMA PARA PLOTAGEM DOS DADOS RECEBIDOS DO MOBFOG EM TEMPO REAL.
 *E UTILIZADO A BIBLIOTECA QCUSTOMPLOT PARA MANIPULAR O GRAFICO.
 *
 * ATUALIZACAO DO QUE FALTA:
 * 1- Criar a funcao de save dos dados recebidos ao pressionar o botao save
 * 2- Terminar a funcao "on_btn_add_clicked()" responsavel por comecar a conexao e plotagem dos dados;
 *
 *
 * PROBLEMAS:
 * 1 - Programa nao inicia. Para de funcionar assim que compila
 *
 * OBS: Estou cogitando em retirar os radiobuttons de escolha, e colocar o combo mesmo
 *
*/


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->plot->addGraph();
    // Colocando os botoes de conectar e parar indisponiveis ate obter algo na lista de devices
    ui->btn_add->setEnabled(false);
    ui->btn_stop->setEnabled(false);

    on_dev_update_clicked();

    // Configurando o grafico para ter dois eixos
    ui->plot->yAxis->setTickLabels(false);
    connect(ui->plot->yAxis2, SIGNAL(rangeChanged(QCPRange)), ui->plot->yAxis, SLOT(setRange(QCPRange))); // left axis only mirrors inner right axis
    ui->plot->yAxis2->setVisible(true);
    ui->plot->axisRect()->addAxis(QCPAxis::atRight);
    ui->plot->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(30); // add some padding to have space for tags
    ui->plot->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(30); // add some padding to have space for tags

    // Criando oos graficos
    mGraph1 = ui->plot->addGraph(ui->plot->xAxis, ui->plot->axisRect()->axis(QCPAxis::atRight, 0));
    mGraph2 = ui->plot->addGraph(ui->plot->xAxis, ui->plot->axisRect()->axis(QCPAxis::atRight, 1));
    mGraph1->setPen(QPen(QColor(250, 120, 0)));
    mGraph2->setPen(QPen(QColor(0, 180, 60)));

    // Criando as tags que acompanharao os valores do grafico no eixo vertical
    mTag1 = new AxisTag(mGraph1->valueAxis());
    mTag1->setPen(mGraph1->pen());
    mTag2 = new AxisTag(mGraph2->valueAxis());
    mTag2->setPen(mGraph2->pen());

    connect(&mDataTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    mDataTimer.start(40);

    // Configuracao da conexao serial com arduino

    arduino = new QSerialPort(this);
    procSerial = new comserial(arduino);

    // Criacao do arquivo
    QFile arquivo("mobfog.txt");



  }


  MainWindow::~MainWindow()
  {
    delete ui;
  }

void MainWindow::fun_plot(QVector<double> temp, QVector<double> Y1, QVector<double> Y2)
  {
    // calculate and add a new data point to each graph:
    mGraph1->addData(temp, Y1);
    mGraph2->addData(temp, Y2);

    // make key axis range scroll with the data:
    ui->plot->xAxis->rescale();
    mGraph1->rescaleValueAxis(false, true);
    mGraph2->rescaleValueAxis(false, true);
    ui->plot->xAxis->setRange(ui->plot->xAxis->range().upper, 100, Qt::AlignRight);

    // update the vertical axis tag positions and texts to match the rightmost data point of the graphs:
    double graph1Value = mGraph1->dataMainValue(mGraph1->dataCount()-1);
    double graph2Value = mGraph2->dataMainValue(mGraph2->dataCount()-1);
    mTag1->updatePosition(graph1Value);
    mTag2->updatePosition(graph2Value);
    mTag1->setText(QString::number(graph1Value, 'f', 2));
    mTag2->setText(QString::number(graph2Value, 'f', 2));

    ui->plot->replot();
}
    /*Funcao Read_Serial() para ler a porta serial e ir separando os dados para cada vetor de variaveis
    Ira ler a porta serial e alocar em cada vetor correspondente os dados medidos nos sensores da seguinte maneira:
    . serial[0] = altidude;
    . serial[1] = pressao;
    . serial[2] = temperatura;
    . serial[3] = aceleracao;
    */

void MainWindow::Read_Serial()
{
   QString dados_recebidos = procSerial->Read(); //lembrando que procSerial é o objeto comserial que manipula a porta serial
   QVector<float> dados;
   dados.push_back(dados_recebidos.toFloat());

   qv_altidude.push_back(dados[0]);
   qv_pressao.push_back(dados[1]);
   qv_temperatura.push_back(dados[2]);
   qv_aceleracao.push_back(dados[3]);

}
//inutil
void MainWindow::addPoint()
{

}
//inutil
void MainWindow::clearData()
{

}


// Funcao para comecar a ler os dados da porta serial
void MainWindow::on_btn_add_clicked()
{
    bool statusOpenSerial;
    QVector<float> x, y1, y2;
   // bool radio_btn_graph1, radio_btn_graph2;

        statusOpenSerial = procSerial->Conectar(ui->combo->currentText(),9600);
        if (statusOpenSerial) {
            ui->btn_stop->setEnabled(true);
            ui->btn_add->setEnabled(false);
            qDebug() << "### Porta serial aberta com sucesso!";
        }
        else {
            qDebug() << "Falha ao abrir conexão serial.";
        }
    Read_Serial();
    /*Ira analisar qual opcao foi escolhida no combo menu, para colocar na funcao de plotagem*/
    if (ui->combo_green->currentText() == "Altitude"){
        y1 = qv_altidude;
    }
    else if (ui->combo_green->currentText() == "Pressao"){
        y1 = qv_pressao;
    }
    else if (ui->combo_green->currentText() == "Temperatura"){
        y1 = qv_temperatura;
    }
    else if (ui->combo_green->currentText() == "Aceleracao"){
        y1 = qv_aceleracao;
    }

    if (ui->combo_orange->currentText() == "Altitude"){
        y2 = qv_altidude;
    }
    else if (ui->combo_orange->currentText() == "Pressao"){
        y2 = qv_pressao;
    }
    else if (ui->combo_orange->currentText() == "Temperatura"){
        y2 = qv_temperatura;
    }
    else if (ui->combo_orange->currentText() == "Aceleracao"){
        y2 = qv_aceleracao;
    }

    // checkar qual sera os dados do eixo X, segundos ou id dos pacotes que chegam
    // obs: perguntar a HB como capta os id packages e coloca nos vetores

    if (ui->rd_idpackages->isChecked()){

    }
    else if (ui->rd_seg->isChecked()){}

    }

//Limpa os vetores de dados e o grafico
void MainWindow::on_btn_clear_clicked()
{
    ui->plot->clearGraphs();
    ui->plot->replot();

    qv_altidude.clear();
    qv_pressao.clear();
    qv_temperatura.clear();
    qv_aceleracao.clear();
}

// Parar a conexao e plotagem
void MainWindow::on_btn_stop_clicked()
{
    bool statusCloseSerial;


        statusCloseSerial = procSerial->Desconectar();

        /* BUG: Existe um bug no close do QtSerialPort, já conhecido pela comunidade onde
         * quando usado com waitForReadyRead, da um erro 12 Timeout no SerialPort e não encerra a conexão
         * porém é reportado o erro mas o device é encerrado.
         * Para contornar o problema no Desconectar eu verifiquei com isOpen logo apos fechar a conexão
         * serial.
         */

        if (statusCloseSerial) {
            ui->btn_stop->setEnabled(false);
            ui->btn_add->setEnabled(true);
            qDebug() << "### Porta serial fechada com sucesso!";
        }
        else {
            qDebug() << "### Falha ao fechar conexão serial.";
        }
}

// Atualizar a lista de portas disponiveis para conexao
void MainWindow::on_dev_update_clicked()
{
    bool statusCloseSerial;


        statusCloseSerial = procSerial->Desconectar();

        /* BUG: Existe um bug no close do QtSerialPort, já conhecido pela comunidade onde
         * quando usado com waitForReadyRead, da um erro 12 Timeout no SerialPort e não encerra a conexão
         * porém é reportado o erro mas o device é encerrado.
         * Para contornar o problema no Desconectar eu verifiquei com isOpen logo apos fechar a conexão
         * serial.
         */

        if (statusCloseSerial) {


                /* Load Device PortSerial available */
                QStringList DispSeriais = procSerial->CarregarDispositivos();

                /* Inserting in ComboxBox the Devices */
                ui->combo->clear();
                ui->combo->addItems(DispSeriais);

                /* Enable PushButton "Conectar" if any port is found.
                 * If an error occurs, it is reported in the Log
                 */
                if(ui->combo->count() > 0) {
                    ui->btn_add->setEnabled(true);
                    ui->btn_stop->setEnabled(false);
                   // ui->teLog->append("### Porta serial pronta para ser utilizada.");
                }
                else {
                    ui->btn_stop->setEnabled(false);
                    ui->btn_add->setEnabled(false);
                    //ui->pbLed->setEnabled(false);
                    //ui->teLog->append("### Nenhuma porta serial foi detectada!"); }
        }
        /*else {
            ui->teLog->append("### Falha ao fechar conexão serial.");
        }*/
}
}
// Salvar os arquivos
void MainWindow::on_btn_save_clicked()
{

}
