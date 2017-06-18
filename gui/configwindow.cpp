#include "configwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <initguid.h>
#include <windows.h>
#include <dshow.h>

#include "detector_color.h"


configWindow::configWindow(Logger* logger, QWidget *parent) :
	QTabWidget(parent),
	ui(new Ui::configWindow)
{
	ui->setupUi(this);

	this->logger = logger;

	connect(ui->maskXBox, SIGNAL(valueChanged(int)), this, SLOT(maskValueChanged()));
	connect(ui->maskYBox, SIGNAL(valueChanged(int)), this, SLOT(maskValueChanged()));
	connect(ui->maskRadiusBox, SIGNAL(valueChanged(int)), this, SLOT(maskValueChanged()));

	connect(ui->skipSpin, SIGNAL(valueChanged(int)), ui->skipSlider, SLOT(setValue(int)));
	connect(ui->skipSlider, SIGNAL(valueChanged(int)), ui->skipSpin, SLOT(setValue(int)));

	connect(ui->thresholdSpin, SIGNAL(valueChanged(int)), ui->thresholdSlider, SLOT(setValue(int)));
	connect(ui->thresholdSlider, SIGNAL(valueChanged(int)), ui->thresholdSpin, SLOT(setValue(int)));

	connect(ui->videoLabel, SIGNAL(posChangedX(int)), ui->maskXBox, SLOT(setValue(int)));
	connect(ui->videoLabel, SIGNAL(posChangedY(int)), ui->maskYBox, SLOT(setValue(int)));
	connect(ui->videoLabel, SIGNAL(radiusChanged(int)), ui->maskRadiusBox, SLOT(setValue(int)));

	connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(trackFrame()));

	connect(ui->refreshDevicesButton, SIGNAL(clicked(bool)), this, SLOT(refreshCaptureDevices()));

	ui->ratFrontSelector->setTitle("Rat Front");
	ui->ratBackSelector->setTitle("Rat Back");
	ui->robotFrontSelector->setTitle("Robot Front");
	ui->robotBackSelector->setTitle("Robot Back");

	connect(ui->resolutionHeightSpin, SIGNAL(editingFinished()), this, SLOT(captureResolutionChanged()));
	connect(ui->resolutionWidthSpin, SIGNAL(editingFinished()), this, SLOT(captureResolutionChanged()));

	QStringList list;
	list.append("[START]");
	list.append("[END]");
	triggerNames.setStringList(list);

	ui->ratBackSelector->setVisible(false);
	ui->robotBackSelector->setVisible(false);

	on_refreshPortButton_clicked();
	refreshCaptureDevices();
}

configWindow::~configWindow()
{
	delete ui;
}

void configWindow::load(Settings settings)
{
	int length = settings.value("experiment/duration").toInt();
	int s = length % 60;
	length /= 60;
	int m = length % 60;
	length /= 60;
	int h = length % 24;

	ui->lengthEdit->setTime(QTime(h, m, s));
	ui->modeComboBox->setCurrentIndex(settings.value("experiment/mode").toInt());
	on_modeComboBox_activated(ui->modeComboBox->currentIndex());
	ui->timeoutStopBox->setChecked(settings.value("experiment/stopAfterTimeout").toBool());

	ui->arenaSpinbox->setValue(settings.value("arena/PWM").toInt());
	ui->arenaDirectionCombobox->setCurrentText(settings.value("arena/direction").toString());

	int deviceIndex = settings.value("video/device").toInt();
	if (deviceIndex == -1) {
		videoFilename = settings.value("video/filename").toString();
		refreshCaptureDevices();
		ui->deviceCombobox->setCurrentIndex(ui->deviceCombobox->count() - 1);
	}
	else {
		refreshCaptureDevices();
		ui->deviceCombobox->setCurrentIndex(deviceIndex);
	}

	ui->resolutionBox->setChecked(settings.value("video/resolution/enabled").toBool());
	ui->resolutionWidthSpin->setValue(settings.value("video/resolution/width").toInt());
	ui->resolutionHeightSpin->setValue(settings.value("video/resolution/height").toInt());

	ui->maskXBox->setValue(settings.value("arena/X").toDouble());
	ui->maskYBox->setValue(settings.value("arena/Y").toDouble());
	ui->maskRadiusBox->setValue(settings.value("arena/radius").toInt());
	ui->arenaSizeBox->setValue(settings.value("arena/size").toDouble());

	//ui->multipleCombo->setCurrentIndex(settings.value("faults/multipleReaction").toInt());
	ui->skipCombo->setCurrentIndex(settings.value("faults/skipReaction").toInt());
	ui->skipSpin->setValue(settings.value("faults/skipDistance").toInt());
	ui->skipTimeoutBox->setValue(settings.value("faults/skipTimeout").toInt() / 1000.0);

	ui->trackingCombobox->setCurrentIndex(settings.value("tracking/type").toInt());

	ui->thresholdSpin->setValue(settings.value("tracking/threshold/threshold").toInt());
	ui->ratMinSize->setValue(settings.value("tracking/threshold/minRat").toDouble());
	ui->ratMaxSize->setValue(settings.value("tracking/threshold/maxRat").toDouble());
	ui->robotMinSize->setValue(settings.value("tracking/threshold/minRobot").toDouble());
	ui->robotMaxSize->setValue(settings.value("tracking/threshold/maxRobot").toDouble());

	ui->globminBox->setValue(settings.value("tracking/color/global_minimum_size").toDouble());
	ui->globmaxBox->setValue(settings.value("tracking/color/global_maximum_size").toDouble());
	ui->hueToleranceSlider->setValue(settings.value("tracking/color/hue_tolerance").toInt());

	ui->valSlider->setValue(settings.value("tracking/color/value_threshold").toInt());
	ui->satSlider->setValue(settings.value("tracking/color/saturation_threshold").toInt());

	ui->colorModeBox->setCurrentIndex(settings.value("tracking/color/mode").toInt());

	pointRange ratFrontRange;
	ratFrontRange.hue = settings.value("tracking/color/ratFront/hue").toInt();
	ratFrontRange.minimum_size = settings.value("tracking/color/ratFront/minimum_size").toDouble();
	ratFrontRange.maximum_size = settings.value("tracking/color/ratFront/maximum_size").toDouble();
	ui->ratFrontSelector->setRange(ratFrontRange);

	pointRange ratBackRange;
	ratBackRange.hue = settings.value("tracking/color/ratBack/hue").toInt();
	ratBackRange.minimum_size = settings.value("tracking/color/ratBack/minimum_size").toDouble();
	ratBackRange.maximum_size = settings.value("tracking/color/ratBack/maximum_size").toDouble();
	ui->ratBackSelector->setRange(ratBackRange);

	pointRange robotFrontRange;
	robotFrontRange.hue = settings.value("tracking/color/robotFront/hue").toInt();
	robotFrontRange.minimum_size = settings.value("tracking/color/robotFront/minimum_size").toDouble();
	robotFrontRange.maximum_size = settings.value("tracking/color/robotFront/maximum_size").toDouble();
	ui->robotFrontSelector->setRange(robotFrontRange);

	pointRange robotBackRange;
	robotBackRange.hue = settings.value("tracking/color/robotBack/hue").toInt();
	robotBackRange.minimum_size = settings.value("tracking/color/robotBack/minimum_size").toDouble();
	robotBackRange.maximum_size = settings.value("tracking/color/robotBack/maximum_size").toDouble();
	ui->robotBackSelector->setRange(robotBackRange);

	ui->synchroBox->setChecked(settings.value("output/sync_enabled").toBool());
	ui->invertBox->setChecked(settings.value("output/sync_inverted").toBool());
	ui->shockBox->setChecked(settings.value("output/shock").toBool());

	ui->shockModeCombobox->setCurrentIndex(settings.value("shock/mode").toInt());
	ui->initialShockSpinBox->setValue(settings.value("shock/initialShock").toInt());
	ui->entryBox->setValue(settings.value("shock/EntranceLatency").toInt());
	ui->interBox->setValue(settings.value("shock/InterShockLatency").toInt());
	ui->durationBox->setValue(settings.value("shock/ShockDuration").toInt());
	ui->refractoryBox->setValue(settings.value("shock/OutsideRefractory").toInt());

	ui->directoryEdit->setText(settings.value("system/defaultDirectory").toString());
	ui->filenameEdit->setText(settings.value("system/defaultFilename").toString());

	ui->updateBox->setValue(settings.value("system/updateInterval").toInt());

	ui->portComboBox->setCurrentText(settings.value("hardware/device").toString());
	on_portComboBox_activated(ui->portComboBox->currentText());

	clearAllActions();

	QList<Counter> tmpCounters = settings.value("actions/counters").value<QList<Counter>>();
	for (Counter counter : tmpCounters) {
		addCounter(counter);
	}

	QList<Area> tmpAreas = settings.value("actions/areas").value<QList<Area>>();
	for (Area area : tmpAreas) {
		addArea(area);
	}

	QList<Action> tmpActions = settings.value("actions/actions").value<QList<Action>>();
	for (Action action : tmpActions) {
		addAction(action);
	}


	Settings newsettings = compileSettings();
	if (lastSettings != newsettings) {
		lastSettings = newsettings;
		emit(configurationUpdated(newsettings));
	}
}

Settings configWindow::compileSettings()
{
	Settings settings;

	QTime time = ui->lengthEdit->time();
	settings.insert("experiment/duration", time.hour() * 60 * 60 + time.minute() * 60 + time.second());
	settings.insert("experiment/mode", ui->modeComboBox->currentIndex());
	settings.insert("experiment/stopAfterTimeout", ui->timeoutStopBox->isChecked());

	settings.insert("system/defaultDirectory", ui->directoryEdit->text());
	settings.insert("system/defaultFilename", ui->filenameEdit->text());
	settings.insert("system/updateInterval", ui->updateBox->value());

	settings.insert("arena/PWM", ui->arenaSpinbox->value());
	settings.insert("arena/direction", ui->arenaDirectionCombobox->currentText());

	int deviceIndex = ui->deviceCombobox->currentIndex();
	if (deviceIndex == ui->deviceCombobox->count() - 1) {
		settings.insert("video/device", -1);
		settings.insert("video/filename", videoFilename);
	}
	else {
		settings.insert("video/device", deviceIndex);
	}

	settings.insert("video/resolution/enabled", ui->resolutionBox->isChecked());
	settings.insert("video/resolution/width", ui->resolutionWidthSpin->value());
	settings.insert("video/resolution/height", ui->resolutionHeightSpin->value());

	//settings.insert("faults/multipleReaction", ui->multipleCombo->currentIndex());
	settings.insert("faults/skipReaction", ui->skipCombo->currentIndex());
	settings.insert("faults/skipDistance", ui->skipSpin->value());
	settings.insert("faults/skipTimeout", (int)(ui->skipTimeoutBox->value() * 1000));

	settings.insert("arena/X", ui->maskXBox->value());
	settings.insert("arena/Y", ui->maskYBox->value());
	settings.insert("arena/radius", ui->maskRadiusBox->value());
	settings.insert("arena/size", ui->arenaSizeBox->value());

	settings.insert("tracking/type", ui->trackingCombobox->currentIndex());

	settings.insert("tracking/threshold/threshold", ui->thresholdSpin->value());
	settings.insert("tracking/threshold/minRat", ui->ratMinSize->value());
	settings.insert("tracking/threshold/maxRat", ui->ratMaxSize->value());
	settings.insert("tracking/threshold/minRobot", ui->robotMinSize->value());
	settings.insert("tracking/threshold/maxRobot", ui->robotMaxSize->value());

	settings.insert("tracking/color/global_minimum_size", ui->globminBox->value());
	settings.insert("tracking/color/global_maximum_size", ui->globmaxBox->value());
	settings.insert("tracking/color/hue_tolerance", ui->hueToleranceSlider->value());

	settings.insert("tracking/color/saturation_threshold", ui->satSlider->value());
	settings.insert("tracking/color/value_threshold", ui->valSlider->value());

	settings.insert("tracking/color/mode", ui->colorModeBox->currentIndex());

	pointRange ratFrontRange = ui->ratFrontSelector->getColorRange();
	settings.insert("tracking/color/ratFront/hue", ratFrontRange.hue);
	settings.insert("tracking/color/ratFront/maximum_size", ratFrontRange.maximum_size);
	settings.insert("tracking/color/ratFront/minimum_size", ratFrontRange.minimum_size);

	pointRange ratBackRange = ui->ratBackSelector->getColorRange();
	settings.insert("tracking/color/ratBack/hue", ratBackRange.hue);
	settings.insert("tracking/color/ratBack/maximum_size", ratBackRange.maximum_size);
	settings.insert("tracking/color/ratBack/minimum_size", ratBackRange.minimum_size);

	pointRange robotFrontRange = ui->robotFrontSelector->getColorRange();
	settings.insert("tracking/color/robotFront/hue", robotFrontRange.hue);
	settings.insert("tracking/color/robotFront/maximum_size", robotFrontRange.maximum_size);
	settings.insert("tracking/color/robotFront/minimum_size", robotFrontRange.minimum_size);

	pointRange robotBackRange = ui->robotBackSelector->getColorRange();
	settings.insert("tracking/color/robotBack/hue", robotBackRange.hue);
	settings.insert("tracking/color/robotBack/maximum_size", robotBackRange.maximum_size);
	settings.insert("tracking/color/robotBack/minimum_size", robotBackRange.minimum_size);

	settings.insert("output/sync_enabled", ui->synchroBox->isChecked());
	settings.insert("output/sync_inverted", ui->invertBox->isChecked());
	settings.insert("output/shock", ui->shockBox->isChecked());

	settings.insert("shock/mode", ui->shockModeCombobox->currentIndex());
	settings.insert("shock/initialShock", ui->initialShockSpinBox->value());
	settings.insert("shock/EntranceLatency", ui->entryBox->value());
	settings.insert("shock/InterShockLatency", ui->interBox->value());
	settings.insert("shock/ShockDuration", ui->durationBox->value());
	settings.insert("shock/OutsideRefractory", ui->refractoryBox->value());

	settings.insert("hardware/device", ui->portComboBox->currentText());

	settings.insert("actions/areas", QVariant::fromValue(getAreasFromUI()));
	settings.insert("actions/actions", QVariant::fromValue(getActionsFromUI()));
	settings.insert("actions/counters", QVariant::fromValue(getCountersFromUI()));

	return settings;
}

void configWindow::on_revertButton_clicked()
{
	load(lastSettings);
}

void configWindow::on_applyButton_clicked()
{
	Settings settings = compileSettings();
	lastSettings = settings;
	emit(configurationUpdated(settings));
}

void configWindow::on_okayButton_clicked()
{
	on_applyButton_clicked();
	close();
}

void configWindow::showEvent(QShowEvent *event) {
	// If we're not running from a file
	if (ui->deviceCombobox->currentIndex() != ui->deviceCombobox->count() - 1) {
		ui->lengthEdit->setEnabled(true);
		ui->timeoutStopBox->setEnabled(true);
		ui->resolutionBox->setEnabled(true);
		capture.open(ui->deviceCombobox->currentIndex());
	}
	else {
		ui->lengthEdit->setEnabled(false);
		ui->timeoutStopBox->setEnabled(false);
		ui->resolutionBox->setChecked(false);
		ui->resolutionBox->setEnabled(false);
		if (!videoFilename.isEmpty())
			capture.open(videoFilename.toStdString());
	}

	if (ui->resolutionBox->isChecked()) {
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, ui->resolutionHeightSpin->value());
		capture.set(CV_CAP_PROP_FRAME_WIDTH, ui->resolutionWidthSpin->value());
	}
	else {
		ui->resolutionHeightSpin->setValue(capture.get(CV_CAP_PROP_FRAME_HEIGHT));
		ui->resolutionWidthSpin->setValue(capture.get(CV_CAP_PROP_FRAME_WIDTH));
	}

	if (event) event->accept();
}



void configWindow::closeEvent(QCloseEvent *event) {
	ui->refreshCheckbox->setChecked(false);

	if (compileSettings() != lastSettings) {
		QMessageBox reallyDialog;
		reallyDialog.setIcon(QMessageBox::Warning);
		reallyDialog.setModal(true);
		reallyDialog.setText("Wait! You've got unsaved changes in experiment configuration.");
		reallyDialog.setInformativeText("Do you want to save the changes?");
		reallyDialog.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		switch (reallyDialog.exec()) {
		case QMessageBox::Save:
			on_applyButton_clicked();
			break;
		case QMessageBox::Discard:
			on_revertButton_clicked();
			break;
		case QMessageBox::Cancel:
			setCurrentIndex(0);
			event->ignore();
			return;
			break;
		}
	}

	capture.release();

	ui->ratFrontSelector->closeDialog();
	ui->ratBackSelector->closeDialog();
	ui->robotFrontSelector->closeDialog();
	ui->robotBackSelector->closeDialog();

	event->accept();
}

double configWindow::pixelsToMeters(int px) {
	double diameter_real = ui->arenaSizeBox->value();
	double radius_px = ui->maskRadiusBox->value();
	double result = diameter_real * (px / (radius_px * 2));
	return result;
}

#include "configwindow_general.cpp"

#include "configwindow_capture.cpp"

#include "configwindow_tracking.cpp"

#include "configwindow_actions.cpp"
