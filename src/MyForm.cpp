#include "MyForm.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QIcon>
#include <QRadioButton>
#include <QPushButton>
#include <QSlider>
#include <QSignalBlocker>
#include <cmath>

namespace {
float zoomSliderToScale(int value) {
  constexpr float minScale = 0.25f;
  constexpr float defaultScale = 1.0f;
  constexpr float maxScale = 4.0f;
  constexpr int midpoint = 100;

  if (value <= midpoint) {
    const float t = float(value) / float(midpoint);
    return maxScale + (defaultScale - maxScale) * t;
  }

  const float t = float(value - midpoint) / float(midpoint);
  return defaultScale + (minScale - defaultScale) * t;
}

int zoomScaleToSlider(float zoomScale) {
  constexpr float minScale = 0.25f;
  constexpr float defaultScale = 1.0f;
  constexpr float maxScale = 4.0f;
  constexpr int midpoint = 100;

  if (zoomScale >= defaultScale) {
    const float t = (zoomScale - defaultScale) / (maxScale - defaultScale);
    return int(std::lround(float(midpoint) * (1.0f - t)));
  }

  const float t = (defaultScale - zoomScale) / (defaultScale - minScale);
  return midpoint + int(std::lround(float(midpoint) * t));
}
}

MyForm::MyForm(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);

  // Per el layout: el viewport creix, el panell es queda fix
  ui.horizontalLayout->setStretch(0, 1);
  ui.horizontalLayout->setStretch(1, 0);

  setupGameOverDialog();

  // Per el panell → escena: el que l'usuari toca envia ordres al widget
  connect(ui.startButton, &QPushButton::clicked, this, [this]() {
    ui.gameOverDialog->hide();
    ui.widget->startGame();
  });

  connect(ui.firstPersonRadio, &QRadioButton::toggled, ui.widget, &MySceneWidget::setCameraFirstPerson);
  connect(ui.psiSlider, &QSlider::valueChanged, this, [this](int degrees) {
    constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;
    ui.widget->setCameraPsi(float(degrees) * kDegToRad);
  });
  connect(ui.thetaSlider, &QSlider::valueChanged, this, [this](int degrees) {
    constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;
    ui.widget->setCameraTheta(float(degrees) * kDegToRad);
  });
  connect(ui.zoomSlider, &QSlider::valueChanged, this, [this](int value) {
    ui.widget->setCameraZoomScale(zoomSliderToScale(value));
  });
  connect(ui.coinsRotationCheck, &QCheckBox::toggled, ui.widget, &MySceneWidget::setCoinsRotationEnabled);
  connect(ui.nightModeCheck, &QCheckBox::toggled, ui.widget, &MySceneWidget::setNightMode);

  auto applyFocusColor = [this]() {
    ui.widget->setFocusColorMix(
        float(ui.focusBlueSlider->value()) / 100.0f,
        float(ui.focusRedSlider->value()) / 100.0f,
        float(ui.focusYellowSlider->value()) / 100.0f);
  };
  connect(ui.focusBlueSlider, &QSlider::valueChanged, this, applyFocusColor);
  connect(ui.focusRedSlider, &QSlider::valueChanged, this, applyFocusColor);
  connect(ui.focusYellowSlider, &QSlider::valueChanged, this, applyFocusColor);

  // Per l'escena → panell: actualitzo labels i sincronitzo controls
  connect(ui.widget, &MySceneWidget::coinsUpdated, this, [this](int collected, int total) {
    ui.coinsLabel->setText(QString::number(collected) + "/" + QString::number(total) + " monedes");
  });

  connect(ui.widget, &MySceneWidget::statusMessageChanged, this, [this](const QString &message) {
    ui.statusLabel->setText(message);
  });

  connect(ui.widget, &MySceneWidget::gameOver, this, &MyForm::showGameOverDialog);

  connect(ui.gameOverDialog, &QDialog::accepted, ui.widget, &MySceneWidget::startGame);

  connect(ui.widget, &MySceneWidget::cameraStateChanged, this, [this](float psi, float theta, float zoomScale, int cameraMode) {
    QSignalBlocker blockPsi(ui.psiSlider);
    QSignalBlocker blockTheta(ui.thetaSlider);
    QSignalBlocker blockZoom(ui.zoomSlider);
    QSignalBlocker blockCamera(ui.firstPersonRadio);
    QSignalBlocker blockCamera2(ui.thirdPersonRadio);
    constexpr float kRadToDeg = 180.0f / 3.14159265358979323846f;
    const auto toSliderDegrees = [](float radians) {
      constexpr float kRadToDeg = 180.0f / 3.14159265358979323846f;
      float degrees = radians * kRadToDeg;
      degrees = std::fmod(degrees, 360.0f);
      if (degrees < 0.0f) degrees += 360.0f;
      return int(std::lround(degrees));
    };
    ui.psiSlider->setValue(toSliderDegrees(psi));
    ui.thetaSlider->setValue(int(std::lround(theta * kRadToDeg)));
    ui.zoomSlider->setValue(zoomScaleToSlider(zoomScale));
    const bool isFirstPerson = (cameraMode == 1);
    ui.firstPersonRadio->setChecked(isFirstPerson);
    ui.thirdPersonRadio->setChecked(!isFirstPerson);
    ui.psiSlider->setEnabled(!isFirstPerson);
    ui.thetaSlider->setEnabled(!isFirstPerson);
    ui.zoomSlider->setEnabled(!isFirstPerson);
    ui.psiLabel->setEnabled(!isFirstPerson);
    ui.thetaLabel->setEnabled(!isFirstPerson);
    ui.zoomLabel->setEnabled(!isFirstPerson);
  });

  connect(ui.widget, &MySceneWidget::coinsRotationStateChanged, this, [this](bool activat) {
    QSignalBlocker blockCoins(ui.coinsRotationCheck);
    ui.coinsRotationCheck->setChecked(activat);
  });

  connect(ui.widget, &MySceneWidget::nightModeChanged, this, [this](bool activat) {
    QSignalBlocker blockNight(ui.nightModeCheck);
    ui.nightModeCheck->setChecked(activat);
  });

  // Per els valors per defecte abans de començar
  ui.coinsLabel->setText("0/0 monedes");
  ui.statusLabel->setText("Prem Començar partida");
  ui.thirdPersonRadio->setChecked(true);
  ui.coinsRotationCheck->setChecked(true);
}

void MyForm::setupGameOverDialog() {
  ui.gameOverDialog->setModal(true);

  // Botó Acceptar (reinicia la partida)
  if (QPushButton *okButton = ui.gameOverButtonBox->button(QDialogButtonBox::Ok)) {
    okButton->setText(QStringLiteral("Acceptar"));
    okButton->setIcon(QIcon());
    okButton->setDefault(true);
    okButton->setAutoDefault(true);
    okButton->setStyleSheet(
        QStringLiteral("QPushButton {"
                       "  background-color: #2d6cdf;"
                       "  color: white;"
                       "  font-weight: bold;"
                       "  padding: 8px 16px;"
                       "  border: none;"
                       "  border-radius: 4px;"
                       "}"
                       "QPushButton:hover { background-color: #3a7af0; }"
                       "QPushButton:pressed { background-color: #2458b8; }"));
  }

  // Per el botó Tancar (només tanca el diàleg)
  if (QPushButton *closeButton = ui.gameOverButtonBox->button(QDialogButtonBox::Close)) {
    closeButton->setText(QStringLiteral("Tancar"));
    closeButton->setIcon(QIcon());
  }
}

void MyForm::showGameOverDialog() {
  // Centrar el diàleg sobre el viewport abans de mostrar-lo
  ui.gameOverDialog->adjustSize();
  const QPoint center = ui.widget->mapToGlobal(ui.widget->rect().center());
  ui.gameOverDialog->move(center.x() - ui.gameOverDialog->width() / 2, center.y() - ui.gameOverDialog->height() / 2);
  ui.gameOverDialog->exec();
}
