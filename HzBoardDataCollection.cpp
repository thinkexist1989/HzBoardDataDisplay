#include <QApplication>
#include <QButtonGroup>
#include <QIcon>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include "HzBoardDataCollection.h"
#include "chartdir.h"
#include <math.h>
#include <stdio.h>

#include <QDebug>
#include <QScreen>

#define AVETIMES  10
#define INFODISPLAY "Hz Board Data Collection v1.1"

int cnt = 0;
float TIMES = 0;
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyleSheet("* {font-family:arial;font-size:11px}");
    DataCollection demo;
    demo.show();
    return app.exec();
}


static const int DataInterval = 200;

DataCollection::DataCollection(QWidget *parent) :
    QDialog(parent),m_RcvPtr(0),m_GetPtr(0),bDataA(false),bDataB(false),bDataC(false)
{
    //
    // Set up the GUI
    //
    QScreen *screen=QGuiApplication::primaryScreen ();
    qDebug()<<screen->availableGeometry();
    QRect mm=screen->availableGeometry() ;
    qDebug()<<mm.width ()<<mm.height ();//主屏幕分辨率的大小
    TIMES = mm.width()/1920.0;

    //setFixedSize(740, 285);
    setFixedSize(740*TIMES, 393*TIMES);
    setWindowTitle(INFODISPLAY);
    setWindowIcon(QIcon(":/vout.png"));

    // The frame on the left side
    QFrame *frame = new QFrame(this);
    frame->setGeometry(4*TIMES, 4*TIMES, 120*TIMES, 380*TIMES);
    frame->setFrameShape(QFrame::StyledPanel);

    // Run push button
    QPushButton *runPB = new QPushButton(QIcon(":/play.png"), "Run", frame);
    runPB->setGeometry(4*TIMES, 8*TIMES, 112*TIMES, 28*TIMES);
    runPB->setStyleSheet("QPushButton { text-align:left; padding:5px}");
    runPB->setCheckable(true);

    // Freeze push button
    QPushButton *freezePB = new QPushButton(QIcon(":/pause.png"), "Pause", frame);
    freezePB->setGeometry(4*TIMES, 36*TIMES, 112*TIMES, 28*TIMES);
    freezePB->setStyleSheet("QPushButton { text-align:left; padding:5px}");
    freezePB->setCheckable(true);

    // The Run/Freeze buttons form a button group
    QButtonGroup *runFreezeControl = new QButtonGroup(frame);
    runFreezeControl->addButton(runPB, 1);
    runFreezeControl->addButton(freezePB, 0);
    connect(runFreezeControl, SIGNAL(buttonPressed(int)), SLOT(onRunFreezeChanged(int)));

    /**************** 串口号选择ComboBox******************/
    (new QLabel("Serial Port", frame))->setGeometry(6*TIMES, 80*TIMES, 108*TIMES, 16*TIMES);
    QComboBox *serialPort = new QComboBox(frame);
    serialPort->setGeometry(6*TIMES, 96*TIMES, 108*TIMES, 21*TIMES);

    foreach(const QSerialPortInfo &qspinfo, QSerialPortInfo::availablePorts()){
               serialPort->addItem(qspinfo.portName());
               //ui->textEdit->append("portname   " + qspinfo.portName());
              // ui->textEdit->append("description:   " + qspinfo.description());
              // ui->textEdit->append("manufacture    "+ qspinfo.manufacturer());
              // ui->textEdit->append("serialnumber   "+ qspinfo.serialNumber());
              // ui->textEdit->append("systemlocation "+qspinfo.serialNumber());
    }
    serialPort->setCurrentIndex(0);
    m_PortName = serialPort->currentText();

    memset(m_RcvBuf,0,256);
    connect(serialPort, SIGNAL(currentIndexChanged(QString)),SLOT(onSerialPortChanged(QString)));

    /*************** 波特率选择ComboBox******************/
    (new QLabel("Baud Rate", frame))->setGeometry(6*TIMES, 120*TIMES, 108*TIMES, 16*TIMES);
    QComboBox *baudRate = new QComboBox(frame);
    baudRate->setGeometry(6*TIMES, 136*TIMES, 108*TIMES, 21*TIMES);
    baudRate->addItems(QStringList()<<"9600"<<"19200"<<"38400"<<"115200");
    baudRate->setCurrentIndex(3);
    m_BaudRate = QSerialPort::Baud115200;
    connect(baudRate, SIGNAL(currentIndexChanged(QString)),SLOT(onBaudRateChanged(QString)));

    /*************** 校验位选择ComboBox******************/
    (new QLabel("Parity Bits", frame))->setGeometry(6*TIMES, 160*TIMES, 108*TIMES, 16*TIMES);
    QComboBox *parityBits = new QComboBox(frame);
    parityBits->setGeometry(6*TIMES, 176*TIMES, 108*TIMES, 21*TIMES);
    parityBits->addItems(QStringList()<<"No Parity"<<"Odd Parity"<<"Even Parity");
    parityBits->setCurrentIndex(0);
    m_ParityBits = QSerialPort::NoParity;

    connect(parityBits, SIGNAL(currentIndexChanged(int)),SLOT(onParityBitsChanged(int)));

    /*************** 数据位选择ComboBox******************/
    (new QLabel("Data Bits", frame))->setGeometry(6*TIMES, 200*TIMES, 108*TIMES, 16*TIMES);
    QComboBox *dataBits = new QComboBox(frame);
    dataBits->setGeometry(6*TIMES, 216*TIMES, 108*TIMES, 21*TIMES);
    dataBits->addItems(QStringList()<<"4 bits"<<"5 bits"<<"6 bits"<<"7 bits"<<"8 bits");
    dataBits->setCurrentIndex(4);
    m_DataBits = QSerialPort::Data8;

    connect(dataBits, SIGNAL(currentIndexChanged(int)),SLOT(onDataBitsChanged(int)));

    /*************** 停止位选择ComboBox******************/
    (new QLabel("Stop Bits", frame))->setGeometry(6*TIMES, 240*TIMES, 108*TIMES, 16*TIMES);
    QComboBox *stopBits = new QComboBox(frame);
    stopBits->setGeometry(6*TIMES, 256*TIMES, 108*TIMES, 21*TIMES);
    stopBits->addItems(QStringList()<<"1 bit"<<"1.5 bits"<<"2 bits");
    stopBits->setCurrentIndex(0);
    m_StopBits = QSerialPort::OneStop;

    connect(stopBits, SIGNAL(currentIndexChanged(int)),SLOT(onStopBitsChanged(int)));


    // V1 Average Value display
    (new QLabel("Ave1", frame))->setGeometry(6*TIMES, 300*TIMES, 31*TIMES, 21*TIMES);
    m_ValueA = new QLabel(frame);
    m_ValueA->setGeometry(55*TIMES, 300*TIMES, 59*TIMES, 21*TIMES);
    m_ValueA->setFrameShape(QFrame::StyledPanel);
    m_ValueA->setText(QString::number(0,'f',4));

    // V2 Average Value display
    (new QLabel("Ave2", frame))->setGeometry(6*TIMES, 325*TIMES, 31*TIMES, 21*TIMES);
    m_ValueB = new QLabel(frame);
    m_ValueB->setGeometry(55*TIMES, 325*TIMES, 59*TIMES, 21*TIMES);
    m_ValueB->setFrameShape(QFrame::StyledPanel);
    m_ValueB->setText(QString::number(0,'f',4));

    // V3 Average Value display
    (new QLabel("Ave3", frame))->setGeometry(6*TIMES, 350*TIMES, 31*TIMES, 21*TIMES);
    m_ValueC = new QLabel(frame);
    m_ValueC->setGeometry(55*TIMES, 350*TIMES, 59*TIMES, 21*TIMES);
    m_ValueC->setFrameShape(QFrame::StyledPanel);
    m_ValueC->setText(QString::number(0,'f',4));



    // Chart Viewer
    m_ChartViewer = new QChartViewer(this);
    m_ChartViewer->setGeometry(132*TIMES, 8*TIMES, 600*TIMES, 378*TIMES);
    connect(m_ChartViewer, SIGNAL(viewPortChanged()), SLOT(drawChart()));


    /************** 数据显示CheckBox*************/
    QCheckBox *checkA = new QCheckBox(frame);
    checkA->setGeometry(35*TIMES, 300*TIMES, 21*TIMES, 21*TIMES);
    checkA->setStyleSheet("QCheckBox::indicator::checked{border: 1px solid grey;background: red}");

    QCheckBox *checkB = new QCheckBox(frame);
    checkB->setGeometry(35*TIMES, 325*TIMES, 21*TIMES, 21*TIMES);
    checkB->setStyleSheet("QCheckBox::indicator::checked{border: 1px solid grey;background: green}");

    QCheckBox *checkC = new QCheckBox(frame);
    checkC->setGeometry(35*TIMES, 350*TIMES, 21*TIMES, 21*TIMES);
    checkC->setStyleSheet("QCheckBox::indicator::checked{border: 1px solid grey;background: blue}");

    QButtonGroup *checkControl = new QButtonGroup(m_ChartViewer);
    checkControl->addButton(checkA, 0);
    checkControl->addButton(checkB, 1);
    checkControl->addButton(checkC, 2);
    checkControl->setExclusive(false); //不互斥

    checkA->setChecked(true);
    showA = true;

    checkB->setChecked(true);
    showB = true;

    checkC->setChecked(true);
    showC = true;

    connect(checkControl, SIGNAL(buttonPressed(int)), SLOT(onCheckChanged(int)));


    // Clear data arrays to Chart::NoValue
    for (int i = 0; i < sampleSize; i++)
        m_timeStamps[i] = m_dataSeriesA[i] = m_dataSeriesB[i] = m_dataSeriesC[i] = Chart::NoValue;

    // Set m_nextDataTime to the current time. It is used by the real time random number
    // generator so it knows what timestamp should be used for the next data point.
    m_nextDataTime = QDateTime::currentDateTime();

    // Set up the data acquisition mechanism. In this demo, we just use a timer to get a
    // sample every 250ms.
//    QTimer *dataRateTimer = new QTimer(this);
//    dataRateTimer->start(DataInterval);
//    connect(dataRateTimer, SIGNAL(timeout()), SLOT(getData()));

    // Set up the chart update timer
    m_ChartUpdateTimer = new QTimer(this);
    connect(m_ChartUpdateTimer, SIGNAL(timeout()), SLOT(updateChart()));
    m_ChartUpdateTimer->start(100);

    //Get Data From Hz Board
    m_GetDataTimer = new QTimer(this);
    connect(m_GetDataTimer,SIGNAL(timeout()),SLOT(getData()));

    // Can start now
    //updatePeriod->setCurrentIndex(3);
    //runPB->click();
    dataFile = new QFile("data.csv");
    if(dataFile->open(QIODevice::ReadWrite))
        qDebug()<< "the data is saved in data.csv";
    else
        qDebug()<<"file open failed.";

    dataFile->write("Vout1\t\tVout2\t\tVout3\t\t\n");
}

DataCollection::~DataCollection()
{
    dataFile->close();
}

double DataCollection::ave(double *a, int n)
{
    double sum=0;
    for (int i=0;i<AVETIMES;i++)
       sum+=a[n-1-i];
    return sum/AVETIMES;
}

//
// A utility to shift a new data value into a data array
//
static void shiftData(double *data, int len, double newValue)
{
    memmove(data, data + 1, sizeof(*data) * (len - 1));
    data[len - 1] = newValue;
}

//
// Shift new data values into the real time data series
//
void DataCollection::getData()
{
    m_SerialPort->write("v5");
}

void DataCollection::processData(double dataA, double dataB, double dataC)
{
    // The current time
    QDateTime now = QDateTime::currentDateTime();

    // This is our formula for the random number generator
    do
    {
        // We need the currentTime in millisecond resolution
        double currentTime = Chart::chartTime2(m_nextDataTime.toTime_t())
                             + m_nextDataTime.time().msec() / 1000.0;

        // Get a data sample

        // Shift the values into the arrays
        shiftData(m_dataSeriesA, sampleSize, dataA);
        shiftData(m_dataSeriesB, sampleSize, dataB);
        shiftData(m_dataSeriesC, sampleSize, dataC);
        shiftData(m_timeStamps, sampleSize, currentTime);

        m_nextDataTime = m_nextDataTime.addMSecs(DataInterval);
    }
    while (m_nextDataTime < now);

    //
    // We provide some visual feedback to the latest numbers generated, so you can see the
    // data being generated.
    //
//    m_ValueA->setText(QString::number(m_dataSeriesA[sampleSize - 1], 'f', 2));
//    m_ValueB->setText(QString::number(m_dataSeriesB[sampleSize - 1], 'f', 2));
//    m_ValueC->setText(QString::number(m_dataSeriesC[sampleSize - 1], 'f', 2));
      cnt++;
      if(cnt >= AVETIMES){
          cnt = 0;
          m_ValueA->setText(QString::number(ave(m_dataSeriesA,sampleSize), 'f', 4));
          m_ValueB->setText(QString::number(ave(m_dataSeriesB,sampleSize), 'f', 4));
          m_ValueC->setText(QString::number(ave(m_dataSeriesC,sampleSize), 'f', 4));
      }
      char buf[256]={0};
      sprintf(buf,"%f\t%f\t%f\n",dataA,dataB,dataC);
      dataFile->write(buf);
}

//
// The Run or Freeze button is pressed
//
void DataCollection::onRunFreezeChanged(int run)
{
    if (run){
     //   m_ChartUpdateTimer->start();
        m_SerialPort = new QSerialPort(m_PortName);
       // m_SerialPort.setPortName(m_PortName);
        m_SerialPort->open(QIODevice::ReadWrite);
        m_SerialPort->setBaudRate(m_BaudRate);
        m_SerialPort->setDataBits(m_DataBits);
        m_SerialPort->setParity(m_ParityBits);
        m_SerialPort->setStopBits(m_StopBits);
        m_SerialPort->setFlowControl(QSerialPort::NoFlowControl);

//        m_SerialPort.clearError();
//        m_SerialPort.clear();

       // m_SerialPort.setRequestToSend(false);
      //  m_SerialPort.setDataTerminalReady(false);

        connect(m_SerialPort,SIGNAL(readyRead()),this,SLOT(onSerialReadyRead()));

        m_GetDataTimer->start(DataInterval);


    }
    else{
     //   m_ChartUpdateTimer->stop();
        m_SerialPort->close();
        m_GetDataTimer->stop();
    }

}

void DataCollection::onCheckChanged(int val)
{
    switch(val){
    case 0:
        showA = !showA;
        break;
    case 1:
        showB = !showB;
        break;
    case 2:
        showC = !showC;
        break;
    default:break;
    }
}

//
// User changes the chart update period
//
void DataCollection::onSerialPortChanged(QString text)
{
    //m_ChartUpdateTimer->start(text.toInt());
    m_PortName = text;
}

void DataCollection::onBaudRateChanged(QString text)
{
    m_BaudRate = text.toInt();
}

void DataCollection::onParityBitsChanged(int val)
{
    switch(val){
    case 0:
        m_ParityBits = QSerialPort::NoParity;
        break;
    default:break;
    }
}

void DataCollection::onDataBitsChanged(int val)
{
    switch(val){
    case 4:
        m_DataBits = QSerialPort::Data8;
        break;
    default:break;
    }
}

void DataCollection::onStopBitsChanged(int val)
{
    switch(val){
    case 0:
        m_StopBits = QSerialPort::OneStop;
        break;
    default:break;
    }
}

void DataCollection::onSerialReadyRead()
{
    char databuf[256] = {0};
    char temp[256] = {0};
    int a = m_SerialPort->readLine(temp,256);
    for(int i = 0; i< a; i++){
        m_RcvBuf[(m_RcvPtr + i)&0xFF] = temp[i];
    }
    m_RcvPtr = (m_RcvPtr + a)&0xFF;
   // qDebug()<< a;
   // int i = 0;
    int j = 0;
    int isNum = 0;
    double dataA=0,dataB=0,dataC=0;
    while(m_GetPtr != m_RcvPtr){
        if(isNum == 0){
            if(m_RcvBuf[(m_GetPtr-4)&0xFF] == 'V'){
                switch(m_RcvBuf[m_GetPtr]){
                case '1':
                    isNum = 1;
                    break;
                case '2':
                    isNum = 2;
                    break;
                case '3':
                    isNum = 3;
                    break;
                case '4':
                    isNum = 4;
                    break;
                default:break;
                }
                //i=i++;
            }
        }
        else{
            if(isdigit(m_RcvBuf[m_GetPtr])){
                databuf[j] = m_RcvBuf[m_GetPtr];
                j++;
            }
            else if(m_RcvBuf[m_GetPtr] == '.'){
                databuf[j] = m_RcvBuf[m_GetPtr];
                j++;
            }
            else if(m_RcvBuf[m_GetPtr] == ','){
                double fval = atof(databuf);
                switch(isNum){
                case 1:
                    dataA = fval;
                    bDataA = true;
                    break;
                case 2:
                    dataB = fval;
                    bDataB = true;
                    //qDebug()<<fval;
                    break;
                case 3:
                    dataC = fval;
                    bDataC = true;
                    break;
                case 4:
                    break;
                default:break;
                }
                isNum = 0;
                j=0;
                memset(databuf,0,256);
            }
        }
        m_GetPtr = (m_GetPtr+1)&0xFF;
    }
    if(bDataA&&bDataB&&bDataC)
        processData(dataA,dataB,dataC);
        bDataA=bDataB=bDataC=false;
   // updateChart();

}

//
// Update the chart. Instead of drawing the chart directly, we call updateViewPort, which
// will trigger a ViewPortChanged signal. We update the chart in the signal handler
// "drawChart". This can take advantage of the built-in rate control in QChartViewer to
// ensure a smooth user interface, even for extremely high update rate. (See the
// documentation on QChartViewer.setUpdateInterval).
//
void DataCollection::updateChart()
{
    m_ChartViewer->updateViewPort(true, false);
}

//
// Draw chart
//
void DataCollection::drawChart()
{
    // Create an XYChart object 600 x 270 pixels in size, with light grey (f4f4f4)
    // background, black (000000) border, 1 pixel raised effect, and with a rounded frame.
    XYChart *c = new XYChart(600*TIMES, 378*TIMES, 0xf4f4f4, 0x000000, 1);
    QColor bgColor = palette().color(backgroundRole()).rgb();
    c->setRoundedFrame((bgColor.red() << 16) + (bgColor.green() << 8) + bgColor.blue());

    // Set the plotarea at (55, 62) and of size 520 x 175 pixels. Use white (ffffff)
    // background. Enable both horizontal and vertical grids by setting their colors to
    // grey (cccccc). Set clipping mode to clip the data lines to the plot area.
    c->setPlotArea(55*TIMES, 62*TIMES, 520*TIMES, 290*TIMES, 0xffffff, -1, -1, 0xcccccc, 0xcccccc);
    c->setClipping();

    // Add a title to the chart using 15 pts Times New Roman Bold Italic font, with a light
    // grey (dddddd) background, black (000000) border, and a glass like raised effect.
    c->addTitle(INFODISPLAY, "timesbi.ttf", 15*TIMES
        )->setBackground(0xdddddd, 0x000000, Chart::glassEffect());

    // Add a legend box at the top of the plot area with 9pts Arial Bold font. We set the
    // legend box to the same width as the plot area and use grid layout (as opposed to
    // flow or top/down layout). This distributes the 3 legend icons evenly on top of the
    // plot area.
    LegendBox *b = c->addLegend2(55*TIMES, 33*TIMES, 3, "arialbd.ttf", 9*TIMES);
    b->setBackground(Chart::Transparent, Chart::Transparent);
    b->setWidth(520*TIMES);

    // Configure the y-axis with a 10pts Arial Bold axis title
    c->yAxis()->setTitle("Voltage (V)", "arialbd.ttf", 10*TIMES);

    // Configure the x-axis to auto-scale with at least 75 pixels between major tick and
    // 15  pixels between minor ticks. This shows more minor grid lines on the chart.
    c->xAxis()->setTickDensity(75, 15);
    c->yAxis()->setTickDensity(15);

    // Set the axes width to 2 pixels
    c->xAxis()->setWidth(2);
    c->yAxis()->setWidth(2);

    // Now we add the data to the chart.
    double lastTime = m_timeStamps[sampleSize - 1];
    if (lastTime != Chart::NoValue)
    {
        // Set up the x-axis to show the time range in the data buffer
        c->xAxis()->setDateScale(lastTime - DataInterval * sampleSize / 1000, lastTime);

        // Set the x-axis label format
        c->xAxis()->setLabelFormat("{value|hh:nn:ss}");

        // Create a line layer to plot the lines
        LineLayer *layer = c->addLineLayer();

        // The x-coordinates are the timeStamps.
        layer->setXData(DoubleArray(m_timeStamps, sampleSize));

        // The 3 data series are used to draw 3 lines. Here we put the latest data values
        // as part of the data set name, so you can see them updated in the legend box.
        char buffer[1024];

        sprintf(buffer, "Vout1: <*bgColor=FFCCCC*> %.6f ", m_dataSeriesA[sampleSize - 1]);
        if(showA)
            layer->addDataSet(DoubleArray(m_dataSeriesA, sampleSize), 0xff0000, buffer);

        sprintf(buffer, "Vout2(DAC): <*bgColor=CCFFCC*> %.6f ", m_dataSeriesB[sampleSize - 1]);
        if(showB)
            layer->addDataSet(DoubleArray(m_dataSeriesB, sampleSize), 0x00cc00, buffer);

        sprintf(buffer, "Vout3(AV): <*bgColor=CCCCFF*> %.6f ", m_dataSeriesC[sampleSize - 1]);
        if(showC)
            layer->addDataSet(DoubleArray(m_dataSeriesC, sampleSize), 0x0000ff, buffer);
    }

    // Set the chart image to the WinChartViewer
    m_ChartViewer->setChart(c);

    m_ChartViewer->setImageMap(
        c->getHTMLImageMap("", "", "title='Vout{={dataSet}+1}:{={value}*1000}mV'"));
    delete c;
}
