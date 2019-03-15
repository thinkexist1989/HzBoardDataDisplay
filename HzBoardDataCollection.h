#ifndef DATACOLLECTION_H
#define DATACOLLECTION_H

#include <QDialog>
#include <QDateTime>
#include <QTimer>
#include <QLabel>
#include <QString>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QFile>

#include "qchartviewer.h"


// The number of samples per data series used in this demo
const int sampleSize = 240;

class DataCollection : public QDialog {
    Q_OBJECT
public:
    DataCollection(QWidget *parent = 0);
    ~DataCollection();

private:
    double m_timeStamps[sampleSize];	// The timestamps for the data series
    double m_dataSeriesA[sampleSize];	// The values for the data series A
    double m_dataSeriesB[sampleSize];	// The values for the data series B
    double m_dataSeriesC[sampleSize];	// The values for the data series C

    QDateTime m_nextDataTime;           // Used by the random number generator to generate realtime data.

    QLabel *m_ValueA;                   // Label to display the realtime value A
    QLabel *m_ValueB;                   // Label to display the realtime value B
    QLabel *m_ValueC;                   // Label to display the realtime value C

    QChartViewer *m_ChartViewer;        // QChartViewer control
    QTimer *m_ChartUpdateTimer;         // The chart update timer

    /*****Serial Variants******/
    QString m_PortName;                 //serial port name
    int     m_BaudRate;
    QSerialPort::Parity     m_ParityBits;
    QSerialPort::DataBits     m_DataBits;
    QSerialPort::StopBits     m_StopBits;

    QSerialPort* m_SerialPort;

    char m_RcvBuf[256];
    int  m_RcvPtr;
    int  m_GetPtr;

    bool bDataA;
    bool bDataB;
    bool bDataC;

    bool showA;
    bool showB;
    bool showC;

    //Get Data From Hz Board
    QTimer *m_GetDataTimer;

public:
    QFile *dataFile;

    double ave(double* a,int n);
    void processData(double dataA, double dataB, double dataC);    //process the data

private slots:
    void onRunFreezeChanged(int);       // The "Run" or "Freeze" button has been pressed
    void onCheckChanged(int);


    void onSerialPortChanged(QString);// The chart update timer interval has changed.
    void onBaudRateChanged(QString); //The BaudRate has changed
    void onParityBitsChanged(int); //The BaudRate has changed
    void onDataBitsChanged(int); //The BaudRate has changed
    void onStopBitsChanged(int); //The BaudRate has changed

    void onSerialReadyRead();    //Serial ready to read

    void getData();                     // Get new data values
    void updateChart();                 // Update the chart.
    void drawChart();                   // Draw the chart.
};

#endif // DataCollection_H
