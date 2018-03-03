#ifndef PROFILEMASKWINDOW_H
#define PROFILEMASKWINDOW_H

#include <QDialog>
#include "calibrationwindow.h"
#include "videocapture.h"

namespace Ui {
    class ProfilemaskWindow;
}

class ProfilemaskWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ProfilemaskWindow(QWidget *parent = 0);
    ~ProfilemaskWindow();

public slots:
    void reject();

protected slots:
    void startTest();
    void validateEntries();

    void changeCalibrationSet();
    void changeTestSet();
    void changeWorkingDir();

    void firstFinished(QVector<CaptureFrame> result);
    void secondFinished(QVector<CaptureFrame> result);


private:
    Ui::ProfilemaskWindow *ui;
    CalibrationWindow *win;
    VideoCapture *cap;
    QString outputPath;
};

#endif // PROFILEMASKWINDOW_H
