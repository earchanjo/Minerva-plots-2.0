#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <stdio.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
/*PROGAMA PARA PLOTAGEM DOS DADOS RECEBIDOS DO MOBFOG EM TEMPO REAL.
 *E UTILIZADO A BIBLIOTECA QCUSTOMPLOT PARA MANIPULAR O GRAFICO.
 *
 * ATUALIZACAO:
 * 1 - Descomenteia linha da chamada o read_serial(), para que o botao de plot funcionasse;
 * 2 - Retirei o retorno de read_serial(). Voltou a ficar void;
 * 3 - Criei uma variavel global dados_recebidos_global onde ficara os dados que entram direto da porta serial,
 *      para la na funcao de salvar ela seja adicionada no arquivo_final, que é o arquivo que o programa salva.
 *
 *
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
    mGraph1->setPen(QPen(QColor(0, 180, 60)));
    mGraph2->setPen(QPen(QColor(250, 120, 0)));

    // Criando as tags que acompanharao os valores do grafico no eixo vertical
    mTag1 = new AxisTag(mGraph1->valueAxis());
    mTag1->setPen(mGraph1->pen());
    mTag2 = new AxisTag(mGraph2->valueAxis());
    mTag2->setPen(mGraph2->pen());

    //connect(&mDataTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    mDataTimer.start(40);

    // Configuracao da conexao serial com arduino

    arduino = new QSerialPort(this);
    procSerial = new comserial(arduino);
    connect(arduino, SIGNAL(readyRead()), this, SLOT(Read_Serial()));
    on_dev_update_clicked();

    // Criacao do arquivo
   QFile arquivo_final("mobfog.txt");

   //configurando valores iniciais dos eixos
   PassaDados();



  }


  MainWindow::~MainWindow()
  {
    delete ui;
  }

void MainWindow::fun_plot(QVector<double> X, QVector<double> Y1, QVector<double> Y2)
  {
    // calculate and add a new data point to each graph:
    if (!Y1.isEmpty() && !Y2.isEmpty() && !X.isEmpty())
    {
        mGraph1->addData(X, Y1);
        mGraph2->addData(X, Y2);


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
   dados_recebidos_global.push_back(dados_recebidos);
   dados_recebidos_global.push_back("\r\n"); //pula linha no arquivo
   QStringList lista_separada = dados_recebidos.split(";");
   QVector<double> dados;
   foreach(QString s, lista_separada){
            dados.push_back(s.toDouble());
       }

   if(dados.length() == 6){
       qv_altidude.push_back(dados[0]);
       qv_pressao.push_back(dados[1]);
       qv_temperatura.push_back(dados[2]);
       qv_aceleracao.push_back(dados[3]);
       qv_temp.push_back(dados[4]);
       qv_id.push_back(dados[5]);

       PassaDados();

       fun_plot(x, y1, y2);
   }

}

void MainWindow::LerArquivo(QFile &arquivo)
{
    qv_temp.clear();
    qv_id.clear();
    qv_altidude.clear();
    qv_pressao.clear();
    qv_temperatura.clear();
    qv_aceleracao.clear();

    if(!arquivo.open(QIODevice::ReadOnly)){
        QTextStream arq(&arquivo);
        while (!arq.atEnd()) {
            QString linha = arq.readLine();
            QStringList lista_separada = linha.split(";");
            QVector<double> dados;

            if(lista_separada.length() == 6){


                foreach(QString s, lista_separada){
                         dados.push_back(s.toDouble());
                }

                qv_altidude.push_back(dados[0]);
                qv_pressao.push_back(dados[1]);
                qv_temperatura.push_back(dados[2]);
                qv_aceleracao.push_back(dados[3]);
                qv_temp.push_back(dados[4]);
                qv_id.push_back(dados[5]);

            }

        }
    }
}
// Funcao para comecar a ler os dados da porta serial
void MainWindow::on_btn_add_clicked()
{
    bool statusOpenSerial;
    //QVector<double> x, y1, y2, dados1;
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
    //Read_Serial();// verificar necessidade
    /*Ira analisar qual opcao foi escolhida no combo menu, para colocar na funcao de plotagem*/

    };




//Limpa os vetores de dados e o grafico
void MainWindow::on_btn_clear_clicked()
{
    resetPlot();

    qv_altidude.clear();
    qv_pressao.clear();
    qv_temperatura.clear();
    qv_aceleracao.clear();
    qv_temp.clear();
    qv_id.clear();

    x.clear();
    y1.clear();
    y2.clear();
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
    QString filename = QFileDialog::getSaveFileName(this,tr("Save"),"C://",tr("Text File (*.txt);;CSV Files (*.csv);;All Files(*.*)"));
    if(filename.isEmpty()){
        qDebug() << "Nenhum nome para o arquivo";
    }else{
        QFile arq(filename);
        if(!arq.open(QIODevice::WriteOnly)){
            qDebug() << "Impossivel abrir o arquivo";
        }else{
            arq.write(dados_recebidos_global.toUtf8()); //converto para para UTF8 data para poder adicionar ao arquivo.
            arq.close();
            dados_recebidos_global.clear();
            qDebug() << "Salvo!";
        } /*Para salvar o arquivo lembre-se colocar aquela variavel globar vector que vai armazenar tudo da porta serial*/
    }

}
//Botao para selecionar o arquivo de leitura para plotar
void MainWindow::on_btn_open_file_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Open File"),"C://","All files (*.*);;Text file (*.txt)");
    QFile arquivo(filename);
    arquivo.setFileName(filename);
    if(!arquivo.exists()){
        qDebug() << "Arquivo nao existe!!";
    }
    else
    {
        arquivo.open(QIODevice::ReadOnly | QIODevice::Text);
        if(!arquivo.isOpen()){
            qDebug() << "Nao foi possivel abrir o arquivo selecionado";
        }else{
            LerArquivo(arquivo);

            resetPlot();

            PassaDados();

            fun_plot(x, y1, y2);
        }
    }

}

void MainWindow::resetPlot()
{
    ui->plot->clearGraphs();
    ui->plot->replot();
    mGraph1 = ui->plot->addGraph(ui->plot->xAxis, ui->plot->axisRect()->axis(QCPAxis::atRight, 0));
    mGraph2 = ui->plot->addGraph(ui->plot->xAxis, ui->plot->axisRect()->axis(QCPAxis::atRight, 1));
    mGraph1->setPen(QPen(QColor(0, 180, 60)));
    mGraph2->setPen(QPen(QColor(250, 120, 0)));
}

void MainWindow::PassaDados()
{
    if (ui->rd_idpackages->isChecked())
    {
        x=qv_id;
    }
    else if (ui->rd_seg->isChecked())
    {
        x=qv_temp;
    }

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
}

void MainWindow::on_combo_green_currentTextChanged(const QString &newText)
{
    resetPlot();

    if (newText == "Altitude"){
        y1 = qv_altidude;
    }
    else if (newText == "Pressao"){
        y1 = qv_pressao;
    }
    else if (newText == "Temperatura"){
        y1 = qv_temperatura;
    }
    else if (newText == "Aceleracao"){
        y1 = qv_aceleracao;
    }

    fun_plot(x,y1,y2);
}

void MainWindow::on_combo_orange_currentTextChanged(const QString &newText)
{
    resetPlot();

    if (newText == "Altitude"){
        y2 = qv_altidude;
    }
    else if (newText == "Pressao"){
        y2 = qv_pressao;
    }
    else if (newText == "Temperatura"){
        y2 = qv_temperatura;
    }
    else if (newText == "Aceleracao"){
        y2 = qv_aceleracao;
    }

    fun_plot(x,y1,y2);
}

void MainWindow::on_rd_idpackages_toggled(bool checked)
{
    resetPlot();

    if (checked)
    {
        x=qv_id;
    }

    fun_plot(x,y1,y2);
}

void MainWindow::on_rd_seg_toggled(bool checked)
{
    resetPlot();

    if (checked)
    {
        x=qv_temp;
    }

    fun_plot(x,y1,y2);
}
