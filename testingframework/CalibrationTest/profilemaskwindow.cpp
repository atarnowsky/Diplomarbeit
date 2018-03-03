#include "profilemaskwindow.h"
#include "ui_profilemaskwindow.h"
#include <QPushButton>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>

ProfilemaskWindow::ProfilemaskWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfilemaskWindow)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->labelRunning->setVisible(false);

    QSettings s;
    ui->lineSuffix->setText(s.value("Suffix").toString());
    ui->lineName->setText(s.value("Name").toString());
    ui->spinDistance->setValue(s.value("Distance").toInt());
    ui->editNotes->document()->setPlainText(s.value("Notes").toString());
    ui->lineCalibrationSet->setText(s.value("CalibrationSet").toString());
    ui->lineTestSet->setText(s.value("TestSet").toString());
    ui->checkPauseSets->setChecked(s.value("PauseSets").toBool());
    ui->lineWorkingDir->setText(s.value("WorkingDirectory").toString());

    connect(ui->lineName, SIGNAL(textChanged(QString)), this, SLOT(validateEntries()));
    connect(ui->lineCalibrationSet, SIGNAL(textChanged(QString)), this, SLOT(validateEntries()));
    connect(ui->lineTestSet, SIGNAL(textChanged(QString)), this, SLOT(validateEntries()));
    connect(ui->lineWorkingDir, SIGNAL(textChanged(QString)), this, SLOT(validateEntries()));

    connect(ui->buttonCalibrationSet, SIGNAL(clicked()), this, SLOT(changeCalibrationSet()));
    connect(ui->buttonTestSet, SIGNAL(clicked()), this, SLOT(changeTestSet()));
    connect(ui->buttonWorkingDir, SIGNAL(clicked()), this, SLOT(changeWorkingDir()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(startTest()));

    validateEntries();

    cap = 0;
    win = new CalibrationWindow();

    QString title("<div align=\"center\" style=\"color:#222222\">");
    title += "<div style=\"font-size:16px;font-weight:bold;\">CalibrationTestSuite</div>";
    title += "<div style=\"font-size:10px;color:orange;font-weight:bold\">Waiting for test specifications";
    title += "</div></div>";

    win->setTitle(title);
    win->show();
    this->raise();
}

ProfilemaskWindow::~ProfilemaskWindow()
{
    delete ui;
}

void ProfilemaskWindow::reject()
{
    QDialog::reject();
    delete cap;
    delete win;
}

void ProfilemaskWindow::startTest()
{
    QString add;
    if(ui->lineSuffix->text().length() > 0)
        add = " - " + ui->lineSuffix->text();
    outputPath = ui->lineWorkingDir->text() + "/" + ui->lineName->text() + add;
    outputPath = QDir::cleanPath(outputPath);
    if(!QDir().mkpath(outputPath))
        return;


    QFile info(outputPath + "/subject.info");
    if(info.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QString text("Name:\t");
        text += ui->lineName->text();
        text += "\nSuffix:\t";
        text += ui->lineSuffix->text();
        text += "\nDistance to monitor:\t";
        text += QString::number(ui->spinDistance->value());
        text += "\nCalibration set:\t";
        text += ui->lineCalibrationSet->text();
        text += "\nTest set:\t";
        text += ui->lineTestSet->text();
        text += "\nNotes:\n";
        text += ui->editNotes->document()->toPlainText();

        info.write(text.toLocal8Bit());
        info.close();
    }


    win->disconnect(this);
    connect(win, SIGNAL(finished(QVector<CaptureFrame>)), this, SLOT(firstFinished(QVector<CaptureFrame>)));

    QFile data(ui->lineCalibrationSet->text());
    if(!data.open(QIODevice::ReadOnly))
        return;

    QString dataText(data.readAll());
    if(!win->parsePointData(dataText))
        return;

    this->hide();

    // TODO: Add ui element for device number
    cap = new VideoCapture(0, outputPath + "/calibration");
    cap->start();

    win->setVideoCapture(cap);

    QString title("<div align=\"center\" style=\"color:#222222\">");
    title += "<div style=\"font-size:16px;font-weight:bold;\">CalibrationTestSuite</div>";
    title += "<div style=\"font-size:10px;color:green;font-weight:bold\">Warming up camera";
    title += "</div></div>";
    win->setTitle(title);

    QTime time;
    time.start();
    while(time.elapsed() < 3 * 1000)
        qApp->processEvents();

    title = "<div align=\"center\" style=\"color:#222222\">";
    title += "<div style=\"font-size:16px;font-weight:bold;\">CalibrationTestSuite</div>";
    title += "<div style=\"font-size:10px;\"><b>Output:</b>";
    title += outputPath + "/calibration.avi";
    title += "</div></div>";
    win->setTitle(title);

    win->nextPoint();
}

void ProfilemaskWindow::firstFinished(QVector<CaptureFrame> result)
{
    win->disconnect(this);
    connect(win, SIGNAL(finished(QVector<CaptureFrame>)), this, SLOT(secondFinished(QVector<CaptureFrame>)));

    win->centerPoint();

    cap->stop();
    while(cap->isRunning())
    {
        cap->stop();
        qApp->processEvents();
    }

    delete cap;
    cap = 0;

    QFile output(outputPath + "/calibration.log");
    if(output.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        foreach(CaptureFrame c, result)
        {
            QString line;
            line += QString::number(c.frameNumber);
            line += ";";
            line += QString::number(c.position.x());
            line += ";";
            line += QString::number(c.position.y());
            line += "\n";
            output.write(line.toLocal8Bit());
        }
        output.close();
    }

    QFile data(ui->lineTestSet->text());
    if(!data.open(QIODevice::ReadOnly))
        return;

    QString dataText(data.readAll());
    if(!win->parsePointData(dataText))
        return;

    if(ui->checkPauseSets->isChecked())
        QMessageBox::information(this, "Calibration finished", "Press enter for next test pattern.");

    // TODO: Add ui element for device number
    cap = new VideoCapture(0, outputPath + "/testpattern");
    cap->start();

    win->setVideoCapture(cap);

    QString title("<div align=\"center\" style=\"color:#222222\">");
    title += "<div style=\"font-size:16px;font-weight:bold;\">CalibrationTestSuite</div>";
    title += "<div style=\"font-size:10px;color:green;font-weight:bold\">Warming up camera";
    title += "</div></div>";
    win->setTitle(title);

    QTime time;
    time.start();
    while(time.elapsed() < 3 * 1000)
        qApp->processEvents();

    title = "<div align=\"center\" style=\"color:#222222\">";
    title += "<div style=\"font-size:16px;font-weight:bold;\">CalibrationTestSuite</div>";
    title += "<div style=\"font-size:10px;\"><b>Output:</b>";
    title += outputPath + "/testpattern.avi";
    title += "</div></div>";
    win->setTitle(title);

    win->nextPoint();
}

void ProfilemaskWindow::secondFinished(QVector<CaptureFrame> result)
{
    win->centerPoint();

    cap->stop();
    while(cap->isRunning())
    {
        cap->stop();
        qApp->processEvents();
    }

    delete cap;
    cap = 0;

    QFile output(outputPath + "/testpattern.log");
    if(output.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        foreach(CaptureFrame c, result)
        {
            QString line;
            line += QString::number(c.frameNumber);
            line += ";";
            line += QString::number(c.position.x());
            line += ";";
            line += QString::number(c.position.y());
            line += "\n";
            output.write(line.toLocal8Bit());
        }
        output.close();
    }

    QString title("<div align=\"center\" style=\"color:#222222\">");
    title += "<div style=\"font-size:16px;font-weight:bold;\">CalibrationTestSuite</div>";
    title += "<div style=\"font-size:10px;color:orange;font-weight:bold\">Test finished - Waiting for new test specifications";
    title += "</div></div>";

    win->setTitle(title);

    this->show();
}

void ProfilemaskWindow::validateEntries()
{
    bool ok = true;

    if(ui->lineName->text().length() == 0)
    {
        ui->lineName->setStyleSheet("background-color:#FFDDDD;");
        ok = false;
    }
    else
        ui->lineName->setStyleSheet("");

    if(!QFile(ui->lineTestSet->text()).exists())
    {
        ui->lineTestSet->setStyleSheet("background-color:#FFDDDD;");
        ok = false;
    }
    else
        ui->lineTestSet->setStyleSheet("");

    if(!QFile(ui->lineCalibrationSet->text()).exists())
    {
        ui->lineCalibrationSet->setStyleSheet("background-color:#FFDDDD;");
        ok = false;
    }
    else
        ui->lineCalibrationSet->setStyleSheet("");

    if(!QDir(ui->lineWorkingDir->text()).exists())
    {
        ui->lineWorkingDir->setStyleSheet("background-color:#FFDDDD;");
        ok = false;
    }
    else
        ui->lineWorkingDir->setStyleSheet("");


    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);

    if(ok)
    {
        QSettings s;
        s.setValue("Suffix", ui->lineSuffix->text());
        s.setValue("Name", ui->lineName->text());
        s.setValue("Distance", ui->spinDistance->value());
        s.setValue("Notes", ui->editNotes->document()->toPlainText());
        s.setValue("CalibrationSet", ui->lineCalibrationSet->text());
        s.setValue("TestSet", ui->lineTestSet->text());
        s.setValue("PauseSets", ui->checkPauseSets->isChecked());
        s.setValue("WorkingDirectory", ui->lineWorkingDir->text());
    }
}

void ProfilemaskWindow::changeCalibrationSet()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select point-set");
    if(!filename.isEmpty())
        ui->lineCalibrationSet->setText(filename);
}

void ProfilemaskWindow::changeTestSet()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select point-set");
    if(!filename.isEmpty())
        ui->lineTestSet->setText(filename);
}

void ProfilemaskWindow::changeWorkingDir()
{
    QString dirname = QFileDialog::getExistingDirectory(this, "Select working directory");
    if(!dirname.isEmpty())
        ui->lineWorkingDir->setText(dirname);
}
