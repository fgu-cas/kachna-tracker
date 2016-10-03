#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "debugwindow.h"
#include "counterwindow.hpp"

#include <iostream>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QTimer>
#include <QPainter>
#include <cmath>

using namespace cv;


kachnatracker::kachnatracker(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::kachnatracker) {

	ui->setupUi(this);


	configWin.setWindowModality(Qt::ApplicationModal);
	connect(&configWin, SIGNAL(configurationUpdated(Settings)), this, SLOT(onConfigurationUpdated(Settings)));
	connect(ui->actionSave_tracks, SIGNAL(triggered()), this, SLOT(saveTracks()));
	connect(ui->actionView_counters, SIGNAL(triggered()), this, SLOT(showCounterWindow()));

	appSettings.reset(new QSettings("FGU AV", "Kachna Tracker", this));

	QString fileName = appSettings->value("lastUsedSettings").toString();
	if (fileName.isEmpty()) {
		loadSettings(QCoreApplication::applicationDirPath() + "experiment.ini");

		QMessageBox alert;
		alert.setText(tr("<b>Is this the first time you're running Kachna Tracker?</b>"));
		alert.setInformativeText(tr("No valid settings were found, please make sure to correctly set up"
			" the experiment parameters and export them before proceeding."));
		alert.setStandardButtons(QMessageBox::Ok);
		alert.exec();
		configWin.show();
	}
	else {
		loadSettings(fileName);
		dirty = false;
	}

	reset();

	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(requestUpdate()));
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateTick()));

	experiment = 0;
}

kachnatracker::~kachnatracker() {
	delete ui;
}

void kachnatracker::loadSettings(QString fileName) {

	QSettings experimentIni(fileName, QSettings::IniFormat, this);
	Settings settings;

	QStringList keys = experimentIni.allKeys();
	for (int i = 0; i < keys.size(); i++) {
		QString key = keys.value(i);
		settings.insert(key, experimentIni.value(key));
	}

	configWin.load(settings);
	appSettings->setValue("lastUsedSettings", fileName);

	QString version = experimentIni.value("general/version").toString();
	if (!version.isEmpty()) {
		int majorVersion = version.split(".")[0].toInt();
		if (majorVersion < MAJOR_VERSION) {
			QMessageBox alert;
			alert.setText(tr("<b>Hold on! You're using an old settings file!</b>"));
			alert.setInformativeText(tr("Please make sure that all settings are correct before proceeding with the experiment, "
				"then save the settings afterwards to get rid of this warning."));
			alert.setStandardButtons(QMessageBox::Ok);
			alert.exec();
			configWin.show();
		}
	}
}


void kachnatracker::on_actionConfigure_triggered() {
	configWin.show();
}

void kachnatracker::onConfigurationUpdated(Settings settings) {
	currentSettings = settings;
	dirty = true;
}

void kachnatracker::on_actionImportConfig_triggered() {

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		"",
		tr("Files (*.ini)"));
	if (fileName.isEmpty()) {
		return;
	}

	loadSettings(fileName);
}


void kachnatracker::on_actionExportConfig_triggered() {

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		"",
		tr("Files (*.ini)"));
	if (!fileName.isEmpty()) {
		if (!fileName.endsWith(".ini")) {
			fileName += ".ini";
		}
		QSettings settings(fileName, QSettings::IniFormat, this);
		QList<QString> keys = currentSettings.keys();
		for (int i = 0; i < keys.size(); i++) {
			QString key = keys[i];
			settings.setValue(key, currentSettings.value(key));
		}
		settings.setValue("general/version", QString("%1.%2")
			.arg(MAJOR_VERSION));
		appSettings->setValue("lastUsedSettings", fileName);
	}

	dirty = false;
}

void kachnatracker::experimentTimeout() {

	if (!experimentEnded) {
		QMessageBox *alert = new QMessageBox(this);
		alert->setAttribute(Qt::WA_DeleteOnClose);
		alert->setStandardButtons(QMessageBox::Ok);
		alert->setText(tr("Experiment ended!"));
		if (currentSettings.value("experiment/stopAfterTimeout").toBool() || !isLive) {
			updateTimer.stop();
			experiment->stop();
			alert->exec();
			ui->actionConfigure->setEnabled(true);
			saveTracks();
		}
		else {
			alert->setModal(false);
			alert->show();
		}
	}
}

void kachnatracker::saveTracks() {

	if (experiment != 0) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Rat track"),
			currentSettings.value("system/defaultDirectory").toString() + '/' +
			currentSettings.value("system/defaultFilename").toString() + "_rat",
			tr("Files (*.dat)"));
		if (!fileName.isEmpty()) {
			if (!fileName.endsWith(".dat")) {
				fileName += ".dat";
			}
			QFile file(fileName);
			if (file.open(QFile::WriteOnly)) {
				QString log = experiment->getLog(true);
				QTextStream out(&file);
				out << log;
			}
			file.close();
		}

		if (currentSettings.value("experiment/mode").toInt() == 1) {
			fileName = QFileDialog::getSaveFileName(this, tr("Robot track"),
				currentSettings.value("system/defaultDirectory").toString() + '/' +
				currentSettings.value("system/defaultFilename").toString() + "_rob",
				tr("Files (*.dat)"));
			if (!fileName.isEmpty()) {
				if (!fileName.endsWith(".dat")) {
					fileName += ".dat";
				}
				QFile file(fileName);
				if (file.open(QFile::WriteOnly)) {
					QString log = experiment->getLog(false);
					QTextStream out(&file);
					out << log;
				}
				file.close();
			}
		}
	}
}


void kachnatracker::on_startButton_clicked() {
	if (updateTimer.isActive()) {
		QMessageBox reallyDialog;
		reallyDialog.setIcon(QMessageBox::Warning);
		reallyDialog.setModal(true);
		reallyDialog.setText("Wait! There's an unfinished experiment in progress.");
		reallyDialog.setInformativeText("Are you sure you want to stop it?");
		reallyDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		if (reallyDialog.exec() == QMessageBox::Yes) {
			updateTimer.stop();
			experiment->stop();
			ui->actionConfigure->setEnabled(true);
			saveTracks();
		}
	}
	else {
		VideoCapture capture;
		int deviceIndex = currentSettings.value("video/device").toInt();
		if (deviceIndex == -1) {
			capture.open(currentSettings.value("video/filename").toString().toStdString());
			isLive = false;
			ui->progressBar->setValue(100);
			ui->timeLabel->setText("---");
		}
		else {
			capture.open(deviceIndex);
			isLive = true;
		}
		int cap_w = currentSettings.value("video/resolution/width").toInt();
		int cap_h = currentSettings.value("video/resolution/height").toInt();
		appSettings->setValue("lastSize", QSize(cap_w, cap_h));
		trackImage = QImage(cap_w, cap_h, QImage::Format_ARGB32_Premultiplied);
		//trackPixmap.fill(Qt::white);
		capture.release();

		QPainter painter(&trackImage);
		painter.setPen(Qt::black);
		int radius = currentSettings.value("arena/radius").toInt();
		arenaArea = QRect(QPoint(currentSettings.value("arena/X").toInt() - radius, currentSettings.value("arena/Y").toInt() - radius),
			QPoint(currentSettings.value("arena/X").toInt() + radius, currentSettings.value("arena/Y").toInt() + radius));
		painter.drawEllipse(arenaArea);
		painter.end();

		reset();

		ui->actionConfigure->setEnabled(false);

		experiment.reset(new Experiment(this, &currentSettings));
		if (!experiment->start()) {
			QMessageBox aboutBox;
			aboutBox.setText("<b>Hardware initialization failed!</b>");
			aboutBox.setInformativeText("An error occurred during hardware initialization. You may want to abort the experiment.");
			aboutBox.exec();
		}

		connect(ui->shockBox, SIGNAL(valueChanged(int)), experiment.get(), SLOT(setShockLevel(int)));

		updateTimer.start(currentSettings.value("system/updateInterval").toInt());
		elapsedTimer.start();
	}
}

void kachnatracker::requestUpdate() {
	Experiment::Update update = experiment->getUpdate();

	QPoint rat(update.keypoints.rat.pt.x, update.keypoints.rat.pt.y);
	QPoint lastRat;
	if (lastKeypoints.rat.size != 0) {
		lastRat = QPoint(lastKeypoints.rat.pt.x, lastKeypoints.rat.pt.y);
	}

	QPoint robot(update.keypoints.robot.pt.x, update.keypoints.robot.pt.y);
	QPoint lastRobot;
	if (lastKeypoints.robot.size != 0) {
		lastRobot = QPoint(lastKeypoints.robot.pt.x, lastKeypoints.robot.pt.y);
	}

	QPainter painter(&trackImage);

	if (rat.x() != -1) {
		if (!lastRat.isNull()) {
			painter.setPen(QColor(255, 140, 140));
			painter.setBrush(QBrush(QColor(255, 140, 140), Qt::SolidPattern));
			painter.drawLine(lastRat, rat);
			painter.drawEllipse(lastRat, 2, 2);
		}
		painter.setPen(Qt::red);
		painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
		painter.drawEllipse(rat, 2, 2);

		lastKeypoints.rat = update.keypoints.rat;
	}


	if (robot.x() != -1) {
		if (!lastRobot.isNull()) {
			painter.setPen(QColor(140, 140, 255));
			painter.setBrush(QBrush(QColor(140, 140, 255), Qt::SolidPattern));
			painter.drawLine(lastRobot, robot);
			painter.drawEllipse(lastRobot, 2, 2);
		}
		painter.setPen(Qt::blue);
		painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));
		painter.drawEllipse(robot, 2, 2);

		lastKeypoints.robot = update.keypoints.robot;
	}

	painter.end();

	QPixmap showPixmap(trackImage.size());
	QPainter *showPainter;


	if (showVideo) {
		Mat rgbFrame;
		cv::cvtColor(update.frame, rgbFrame, CV_BGR2RGB);
		showPixmap = QPixmap::fromImage(QImage((uchar*)rgbFrame.data,
			rgbFrame.cols,
			rgbFrame.rows,
			rgbFrame.step,
			QImage::Format_RGB888));
		showPainter = new QPainter(&showPixmap);
		showPainter->setOpacity(0.4);
	}
	else {
		showPainter = new QPainter(&showPixmap);
		showPixmap.fill(Qt::white);
	}

	showPainter->drawImage(QPoint(0, 0), trackImage);

	if (currentSettings.value("experiment/mode").toInt() == 0 ||
		robot.x() != -1 || robot.y() != -1) {
		showPainter->setPen(Qt::yellow);
		showPainter->setBrush(QBrush(Qt::yellow, Qt::FDiagPattern));

		QList<Area> areas = update.areas;
		for (Area area : areas) {
			if (!area.enabled) {
				continue;
			}
			if (area.type == Area::CIRCULAR_AREA) {
				int radius = area.radius;
				if (currentSettings.value("tracking/type").toInt() == 0) {
					showPainter->drawEllipse(robot, radius, radius);
				}
				else {
					int distance = currentSettings.value("shock/offsetDistance").toInt();
					int angle = currentSettings.value("shock/offsetAngle").toInt();
					QPoint shockPoint;
					shockPoint.setX(robot.x() + distance *
						sin((angle + update.keypoints.robot.angle)*CV_PI / 180));
					shockPoint.setY(robot.y() - distance *
						cos((angle + update.keypoints.robot.angle)*CV_PI / 180));
					showPainter->drawEllipse(shockPoint, radius, radius);
				}
			}
			else {
				int a = ((90 - area.angle - (area.range / 2))) * 16;
				int alen = area.range * 16;

				showPainter->drawPie(arenaArea, a, alen);
			}
		}


	}

	showPainter->end();
	ui->displayLabel->setPixmap(showPixmap);


	ui->goodFramesLCD->display(update.stats.goodFrames);
	ui->badFramesLCD->display(update.stats.badFrames);
	ui->encounterLabel->setText(QString::number(update.stats.entryCount));
	ui->shockLabel->setText(QString::number(update.stats.shockCount));
	ui->firstShockLabel->setText(QString::number(update.stats.initialShock / 1000) + " s");
}

void kachnatracker::updateTick() {
	if (isLive) {
		qint64 duration = currentSettings.value("experiment/duration").toInt() * 1000;
		qint64 elapsed = elapsedTimer.elapsed();
		ui->progressBar->setValue((elapsed*1.0 / duration) * 100);

		QString sign = "-";
		int time = (duration - elapsed) / 1000;
		if (time <= 0) {
			experimentTimeout();
			experimentEnded = true;
			time *= -1;
			sign = "+";
		}
		int s = time % 60;
		time /= 60;
		int m = time % 60;
		time /= 60;
		int h = time % 24;
		ui->timeLabel->setText(QString("%1%2:%3:%4").arg(sign)
			.arg(h, 2, 10, QChar('0'))
			.arg(m, 2, 10, QChar('0'))
			.arg(s, 2, 10, QChar('0')));
	}
}

void kachnatracker::closeEvent(QCloseEvent *closeEvent) {
	if (experiment != 0 && experiment->isRunning()) {
		QMessageBox reallyDialog;
		reallyDialog.setIcon(QMessageBox::Warning);
		reallyDialog.setText("Wait! An experiment is currently in progress!");
		reallyDialog.setInformativeText("Are you sure you want to quit the application?");
		reallyDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		switch (reallyDialog.exec()) {
		case QMessageBox::Yes:
			updateTimer.stop();
			experiment->stop();
			break;
		case QMessageBox::Cancel:
			closeEvent->ignore();
			return;
		}
	}
	else if (dirty) {
		QMessageBox reallyDialog;
		reallyDialog.setIcon(QMessageBox::Warning);
		reallyDialog.setModal(true);
		reallyDialog.setText("Wait! You've altered the experiment settings but haven't exported them.");
		reallyDialog.setInformativeText("Are you sure you want to quit the application without saving?");
		reallyDialog.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		switch (reallyDialog.exec()) {
		case QMessageBox::Save:
			on_actionExportConfig_triggered();
			break;
		case QMessageBox::Cancel:
			closeEvent->ignore();
			return;
		}
	}

	configWin.close();
	closeEvent->accept();
}

void kachnatracker::reset() {
	experimentEnded = false;

	lastKeypoints.rat = KeyPoint(0, 0, 0);
	lastKeypoints.robot = KeyPoint(0, 0, 0);

	QPixmap blank(appSettings->value("lastSize").toSize());
	blank.fill(Qt::white);
	ui->displayLabel->setPixmap(blank);

	ui->progressBar->setValue(0);

	ui->badFramesLCD->display(0);
	ui->badFramesLCD->display(0);

	if (currentSettings.value("shock/mode").toInt() == 0) {
		ui->shockBox->setValue(currentSettings.value("shock/initialShock").toInt());
	}
	else {
		ui->shockBox->setEnabled(false);
	}
}

void kachnatracker::on_actionSave_screenshot_triggered() {

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		"",
		tr("Images (*.png)"));
	if (!fileName.isEmpty()) {
		if (!fileName.endsWith(".png")) {
			fileName += ".png";
		}
		trackImage.save(fileName);
	}
}

void kachnatracker::on_actionVideo_tracking_toggled(bool state)
{
	showVideo = state;
}

void kachnatracker::showCounterWindow()
{
	CounterWindow* window = new CounterWindow(this);
	connect(experiment.get(), &Experiment::counterUpdate, window, &CounterWindow::updateView);
	connect(experiment.get(), &Experiment::finished, window, [=]() {
		window->done(0);
	});
	window->show();
}

void kachnatracker::on_actionAbout_triggered()
{
	QMessageBox aboutBox;
	aboutBox.setText("<b>Kachna Tracker</b>");
	aboutBox.setInformativeText("Version 3.4<br><br>https://github.com/fgu-cas/kachna-tracker");
	aboutBox.exec();
}

void kachnatracker::on_actionDebug_triggered()
{
	DebugWindow debugWindow;
	debugWindow.exec();
}
