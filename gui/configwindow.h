#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H
#include "ui_configwindow.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <QTabWidget>
#include <QMap>

#include <QTimer>
#include <QPixmap>
#include <QPainter>

#include <QStringListModel>

#include "Logger.h"
#include "detector_threshold.h"
#include "detector_color.h"
#include "params.h"

typedef QMap<QString, QVariant> Settings;

using namespace cv;

namespace Ui {
	class configWindow;
}

class configWindow : public QTabWidget
{
	Q_OBJECT

public:
	explicit configWindow(Logger* logger, QWidget *parent = 0);
	double pixelsToMeters(int px);
	~configWindow();

signals:
	void configurationUpdated(Settings);

	public slots:

	void load(Settings);

	void refreshCaptureDevices();
	void maskValueChanged();
	void on_testButton_clicked();
	void on_refreshTrackingButton_clicked();
	void on_browseButton_clicked();
	void on_refreshCheckbox_stateChanged(int arg1);
	void on_deviceCombobox_activated(int);

	private slots:
	void on_revertButton_clicked();

	void on_applyButton_clicked();

	void on_okayButton_clicked();

	void on_skipCombo_currentIndexChanged(int index);

	void trackFrame();

	void trackingParamsChanged();

	void updateTrackingView();

	void on_maskButton_toggled(bool checked);

	void on_trackingCombobox_currentIndexChanged(int index);

	void on_resolutionBox_toggled(bool checked);

	void captureResolutionChanged();

	void addAction();
	void addAction(Action action);
	void addArea();
	void addArea(Area area);
	void addCounter();
	void addCounter(Counter counter);
	void removeThisActionRow();
	void removeThisAreaRow();
	void removeThisCounterRow();
	void clearAllActions();

	void actionAreaSetPressed();
	void actionActionSetPressed();

	void triggersChanged(QString id);

	void areaUpdate(int row, Area area);
	void actionUpdate(int row, Action area);

	void on_refreshPortButton_clicked();

	void on_portComboBox_activated(const QString &arg1);

	void on_modeComboBox_activated(int index);

	void on_colorModeBox_activated(int index);

	void on_shockModeCombobox_activated(int index);

private:
	Ui::configWindow *ui;
	Logger* logger;

	std::unique_ptr<Detector> detector;
	void resetDetector();

	Settings compileSettings();
	Settings lastSettings;

	QPixmap testFrame;
	Mat trackingFrame;
	QTimer refreshTimer;
	VideoCapture capture;

	void closeEvent(QCloseEvent*);
	void showEvent(QShowEvent *);
	QString videoFilename;

	QStringListModel actionTriggers;

	QMap<int, Action> partialActions;
	QMap<int, Area> partialAreas;

	QList<Action> getActionsFromUI();
	QList<Area> getAreasFromUI();
	QList<Counter> getCountersFromUI();
};

#endif // CONFIGWINDOW_H
