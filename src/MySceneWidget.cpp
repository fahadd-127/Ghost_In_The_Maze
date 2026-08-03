// MySceneWidget.cpp
#include "MySceneWidget.h"
#include <iostream>
#include <stdio.h>
#include <utility>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>
#include <random>


#define CHECK() printOglError(__FILE__, __LINE__, __FUNCTION__)
#define DEBUG(text) std::cout << __FILE__ << " " << __LINE__ << " " << __FUNCTION__ << ":"<<text<<std::endl;


MySceneWidget::MySceneWidget(QWidget *parent): BaseGLWidget(parent) {
  // Connectar el timeout d'un QTimer al mètode rotateCoins
  timerMonedes.setInterval(40);
  connect(&timerMonedes, &QTimer::timeout, this, &MySceneWidget::rotateCoins);
  setMouseTracking(true);
}

MySceneWidget::~MySceneWidget() {
  timerMonedes.stop();
}

void MySceneWidget::rotateCoins() {
  angleGirMonedes += glm::radians(6.0f);
  if (angleGirMonedes > glm::two_pi<float>())
    angleGirMonedes -= glm::two_pi<float>();
  update();
}

void MySceneWidget::startGame() {
  // Inicialitzem les variables del joc
  gameStarted = true;
  gameWon = false;
  monedesRecollides = 0;
  cameraDistScale = 1.0f;
  cameraMode = CameraGeneral;
  cameraMorty = 0.0f;
  cameraPsi = 0.862f;
  cameraTheta = 0.895f;
  angleSol = glm::half_pi<float>();

  mortyFila = -1;
  mortyCol = -1;
  fantasmaFila = -1;
  fantasmaCol = -1;

  // Busquem la posició de Morty i el fantasma al laberint
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; ++j) {
      if (maze[i][j] == 2) {
        mortyFila = i;
        mortyCol = j;
      } else if (maze[i][j] == 3) {
        fantasmaFila = i;
        fantasmaCol = j;
      }
    }
  }

  // Inicialitzem la direcció del fantasma
  inicialitzaDireccioFantasma();

  // Inicialitzem les monedes cada vegada que es reinicia la partida
  inicialitzaMonedes();
  totalMonedes = int(posicionsMonedes.size());

  // Emetem els senyals perquè el MyForm actualitzi la UI
  emit coinsUpdated(monedesRecollides, totalMonedes);
  emit statusMessageChanged("Partida en curs...");
  emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
  emit coinsRotationStateChanged(timerMonedes.isActive());
  update();
}

void MySceneWidget::setCameraPsi(float valor) {
  cameraPsi = valor;
  emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
  update();
}

void MySceneWidget::setCameraTheta(float valor) {
  cameraTheta = valor;
  const float pitchLimit = glm::radians(89.0f);
  if (cameraTheta > pitchLimit) cameraTheta = pitchLimit;
  if (cameraTheta < 0.0f) cameraTheta = 0.0f;
  emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
  update();
}

// Funció per establir la distància de la càmera
void MySceneWidget::setCameraZoomScale(float valor) {
  cameraDistScale = valor;
  if (cameraDistScale < minCameraDistScale) cameraDistScale = minCameraDistScale;
  if (cameraDistScale > maxCameraDistScale) cameraDistScale = maxCameraDistScale;
  emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
  update();
}

// Funció per reduir la distància de la càmera
void MySceneWidget::zoomCameraIn() {
  if (cameraMode != CameraGeneral)
    return;
  setCameraZoomScale(cameraDistScale - 0.1f);
}

// Funció per augmentar la distància de la càmera
void MySceneWidget::zoomCameraOut() {
  if (cameraMode != CameraGeneral)
    return;
  setCameraZoomScale(cameraDistScale + 0.1f);
}

// Funció per establir el mode de càmera en primera persona
void MySceneWidget::setCameraFirstPerson(bool activat) {
  cameraMode = activat ? CameraFirstPerson : CameraGeneral;
  emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
  update();
}

// Funció per establir si les monedes giren
void MySceneWidget::setCoinsRotationEnabled(bool activat) {
  if (activat) {
    if (!timerMonedes.isActive()) timerMonedes.start();
  } else {
    timerMonedes.stop();
  }
  emit coinsRotationStateChanged(timerMonedes.isActive());
}

// Funció per establir el mode nocturn
void MySceneWidget::setNightMode(bool activat) {
  modeNocturn = activat;
  emit nightModeChanged(modeNocturn);
  update();
}

// Funció per gestionar el clic del ratolí
void MySceneWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mousePressed = true;
    lastMousePos = event->pos();
  }else if (event->button() == Qt::RightButton) {
    rightMousePressed = true;
    lastMousePos = event->pos();
  }
}

// Funció per gestionar el moviment del ratolí en el mode general
void MySceneWidget::mouseMoveEvent(QMouseEvent *event) {
  if (cameraMode != CameraGeneral) return;

  // Botó esquerre: rotació de la càmera al voltant de l'escena
  if (mousePressed) {
    QPoint delta = event->pos() - lastMousePos;
    lastMousePos = event->pos();

    const float sensitivity = 0.005f;
    cameraPsi += float(delta.x()) * sensitivity;
    cameraTheta -= float(delta.y()) * sensitivity;

    const float pitchLimit = glm::radians(89.0f);
    if (cameraTheta > pitchLimit) cameraTheta = pitchLimit;
    if (cameraTheta < 0.0f) cameraTheta = 0.0f;

    emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
    update();
    return;
    }else if (rightMousePressed) { 
    QPoint delta = event->pos() - lastMousePos;
    lastMousePos = event->pos();
    const float zoomSensitivity = 0.01f;
    cameraDistScale *= expf(-float(delta.y()) * zoomSensitivity);

    if (cameraDistScale < minCameraDistScale) cameraDistScale = minCameraDistScale;
    if (cameraDistScale > maxCameraDistScale) cameraDistScale = maxCameraDistScale;

    emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
    update();
    return;
  }
}

// Funció per gestionar l'alliberament del botó del ratolí
void MySceneWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mousePressed = false;
  } else if (event->button() == Qt::RightButton) {
    rightMousePressed = false;
  }
}

// Funció per calcular els extrems i l'alçada d'un model a partir del seu VBO de vèrtexs (per fer la caixa contenidora)
void MySceneWidget::boundingBox(Model &m, glm::vec3 &minP, glm::vec3 &maxP, float &height) const {
  int nFaces = int(m.faces().size());
  int nVerts = nFaces * 3;
  float *v = m.VBO_vertices();
  if (!v || nVerts == 0) {
    minP = glm::vec3(0.0f);
    maxP = glm::vec3(0.0f);
    height = 0.0f;
    return;
  }
  
  glm::vec3 min(v[0], v[1], v[2]);
  glm::vec3 max(v[0], v[1], v[2]);

  for (int i = 0; i < nVerts; ++i) {
    float x = v[3*i];
    float y = v[3*i+1];
    float z = v[3*i+2];

    min.x = std::min(min.x, x);
    min.y = std::min(min.y, y);
    min.z = std::min(min.z, z);


    max.x = std::max(max.x, x);
    max.y = std::max(max.y, y);
    max.z = std::max(max.z, z);
  }

  minP = min; 
  maxP = max;
  height = max.y - min.y;
}

// Funció per calcular el centre i el radi de l'esfera que engloba tota l'escena a partir dels mínims i màxims.
void MySceneWidget::calculaCentreEscenaIRadi(){
  const float margeTorres = 2.0f;
  float minEscenaX = -margeTorres;
  float maxEscenaX = float(N) + margeTorres;
  float minEscenaZ = -margeTorres;
  float maxEscenaZ = float(M) + margeTorres;

  // Aquesta alçada s'usarà per definir l'extrem superior en Y de la capsa de l'escena
  float alturaMaxModelEscalada = std::max({
    mortyScale * mortyModelHeight,
    fantasmaScale * fantasmaModelHeight,
    monedaScale * monedaModelHeight,
    torreScale * torreModelHeight
  });

  // Extrems de la capsa de l'escena en l'eix Y (terra = 0)
  float minEscenaY = 0.0f;
  float maxEscenaY = alturaMaxModelEscalada; 

  // Vectors amb els extrems complets de la capsa de l'escena
  glm::vec3 minEscena(minEscenaX, minEscenaY, minEscenaZ);
  glm::vec3 maxEscena(maxEscenaX, maxEscenaY, maxEscenaZ);

  // Centre de la capsa (mitjana entre mínim i màxim per component)
  sceneCenter = 0.5f * (minEscena + maxEscena);

  // Radi de l'esfera contenidora (distància del centre al punt més llunyà)
  radiEscena = glm::length(maxEscena - sceneCenter);
}

void MySceneWidget::initializeGL(){
    // Carreguem tots els models
    mortyModel.load("../models/Morty.obj");
    fantasmaModel.load("../models/Fantasma.obj");
    monedaModel.load("../models/Coin.obj");
    torreModel.load("../models/tower.obj");
      
    BaseGLWidget::initializeGL();

    wallMesh = new Mesh(this, vertexLoc, normalLoc, texUVLoc, matdiffLoc, matspecLoc, matambLoc, matshinLoc);
    wallMesh->LoadMesh("../models/block.obj");

    towerMesh = new Mesh(this, vertexLoc, normalLoc, texUVLoc, matdiffLoc, matspecLoc, matambLoc, matshinLoc);
    towerMesh->LoadMesh("../models/tower.obj");

    // Normalitzem el model de paret per omplir una cel·la de mida 1x1x1.
    {
      const glm::vec3 wallMin = wallMesh->GetBBMin();
      const glm::vec3 wallMax = wallMesh->GetBBMax();
      const float wallModelWidth = wallMax.x - wallMin.x;
      const float wallModelDepth = wallMax.z - wallMin.z;
      wallModelHeight = wallMax.y - wallMin.y;
      wallCentreBase = glm::vec3((wallMin.x + wallMax.x) * 0.5f, wallMin.y, (wallMin.z + wallMax.z) * 0.5f);
      const float desiredWallSize = 1.15f;
      wallScale = glm::vec3(1.0f);
      if (wallModelWidth > 1e-6f)
        wallScale.x = desiredWallSize / wallModelWidth;
      if (wallModelHeight > 1e-6f)
        wallScale.y = desiredWallSize / wallModelHeight;
      if (wallModelDepth > 1e-6f)
        wallScale.z = desiredWallSize / wallModelDepth;
    }

    // Calculem les mètriques de cada model per poder-los escalar i posicionar correctament a l'escena
    glm::vec3 min, max;
    float altura;
    boundingBox(mortyModel, min, max, altura);
    mortyModelHeight = altura;
    mortyCentreBase = glm::vec3((min.x + max.x)/2.0f, min.y, (min.z + max.z)/2.0f);
    const float desiredMortyHeight = 1.5f;
    mortyScale = desiredMortyHeight / mortyModelHeight;

    boundingBox(fantasmaModel, min, max, altura);
    fantasmaModelHeight = altura;
    fantasmaCentreBase = glm::vec3((min.x + max.x)/2.0f, min.y, (min.z + max.z)/2.0f);
    const float desiredFantasmaHeight = 0.65f;
    fantasmaScale = desiredFantasmaHeight / fantasmaModelHeight;

    boundingBox(monedaModel, min, max, altura);
    monedaModelHeight = altura;
    monedaCentreBase = glm::vec3((min.x + max.x)/2.0f, min.y, (min.z + max.z)/2.0f);
    const float desiredMonedaHeight = 0.5f;
    monedaScale = desiredMonedaHeight / monedaModelHeight;

    boundingBox(torreModel, min, max, altura);
    torreModelHeight = altura;
    torreCentreBase = glm::vec3((min.x + max.x)/2.0f, min.y, (min.z + max.z)/2.0f);
    const float desiredTorreHeight = 6.0f;
    torreScale = desiredTorreHeight / torreModelHeight;

    // Localitzem Morty i el fantasma al laberint.
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < M; ++j) {
        if (maze[i][j] == 2) {
          mortyFila = i;
          mortyCol = j;
        } else if (maze[i][j] == 3) {
          fantasmaFila = i;
          fantasmaCol = j;
        }
      }
    }

    // Inicialitzem la direcció del fantasma
    inicialitzaDireccioFantasma();

    // Un cop tenim les mètriques de tots els models, calculem el centre i el radi de la caixa contenidora de tota l'escena per ajustar la càmera i la projecció
    calculaCentreEscenaIRadi();

    // Generem les 10 monedes una sola vegada perquè no canviïn a cada fotograma
    inicialitzaMonedes();
    totalMonedes = int(posicionsMonedes.size());

    // Escrivim això per si el Morty no es troba al laberint; així el situem en una posició segura (1,1), que és un terra buit
    int safeMortyFila;
    int safeMortyCol;
    if (mortyFila >= 0) {
      safeMortyFila = mortyFila;
    } else {
      safeMortyFila = 1;
    }

    if (mortyCol >= 0) {
      safeMortyCol = mortyCol;
    } else {
      safeMortyCol = 1;
    }

    updateCamera(safeMortyFila, safeMortyCol);

    if (!timerMonedes.isActive())
      timerMonedes.start();

    emit coinsUpdated(monedesRecollides, totalMonedes);
    emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
    emit coinsRotationStateChanged(timerMonedes.isActive());

    DEBUG("InitializeGL");
}

// Funció per inicialitzar les monedes, que es fa cada vegada que es reinicia la partida
void MySceneWidget::inicialitzaMonedes(){
  // Reunim totes les caselles que són terra (valor 0 al laberint)
  std::vector<std::pair<int, int>> casellesValides;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; ++j) {
      if (maze[i][j] == 0) {
        casellesValides.push_back({i, j});
      }
    }
  }

  // Barregem les caselles vàlides i ens quedem amb les primeres 10
  posicionsMonedes.clear();
  std::mt19937 rng(QRandomGenerator::global()->generate());
  std::shuffle(casellesValides.begin(), casellesValides.end(), rng);
  const int nMonedes = std::min(10, int(casellesValides.size()));
  posicionsMonedes.insert(posicionsMonedes.end(), casellesValides.begin(), casellesValides.begin() + nMonedes);
}

void MySceneWidget::recullMonedaSiCal(int fila, int col) {
  for (auto it = posicionsMonedes.begin(); it != posicionsMonedes.end(); ++it) {
    if (it->first == fila && it->second == col) {
      posicionsMonedes.erase(it);
      ++monedesRecollides;
      emit coinsUpdated(monedesRecollides, totalMonedes);
      break;
    }
  }
}

// Funció per inicialitzar la direcció del fantasma
void MySceneWidget::inicialitzaDireccioFantasma() {
  fantasmaDirFila = 0;
  fantasmaDirCol = 0;
  if (fantasmaFila < 0 || fantasmaCol < 0)
    return;

  const std::pair<int, int> direccionsInicials[4] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
  for (const auto &dir : direccionsInicials) {
    const int nf = fantasmaFila + dir.first;
    const int nc = fantasmaCol + dir.second;
    if (nf >= 0 && nf < N && nc >= 0 && nc < M && maze[nf][nc] == 0) {
      fantasmaDirFila = dir.first;
      fantasmaDirCol = dir.second;
      break;
    }
  }
}

// Funció per moure el fantasma
void MySceneWidget::mouFantasma() {
  if (fantasmaFila < 0 || fantasmaCol < 0) return;

  auto esCasellaBuida = [&](int f, int c) {
    return f >= 0 && f < N && c >= 0 && c < M && maze[f][c] != 1 && maze[f][c] != 4;
  };

  const int filaEndavant = fantasmaFila + fantasmaDirFila;
  const int colEndavant = fantasmaCol + fantasmaDirCol;
  const bool potAnarEndavant = esCasellaBuida(filaEndavant, colEndavant);

  std::vector<std::pair<int, int>> direccionsLliures;
  const std::pair<int, int> direccions[4] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
  for (const auto &dir : direccions) {
    int nf = fantasmaFila + dir.first;
    int nc = fantasmaCol + dir.second;
    if (esCasellaBuida(nf, nc)) {
      direccionsLliures.push_back(dir);
    }
  }

  if (potAnarEndavant && direccionsLliures.size() <= 1) {
    fantasmaFila = filaEndavant;
    fantasmaCol = colEndavant;
    return;
  }

  if (direccionsLliures.empty()) return;

  int idx = QRandomGenerator::global()->bounded(int(direccionsLliures.size()));
  fantasmaDirFila = direccionsLliures[idx].first;
  fantasmaDirCol = direccionsLliures[idx].second;
  fantasmaFila += fantasmaDirFila;
  fantasmaCol += fantasmaDirCol;
}

// Funció per comprovar la col·lisió i la victòria
void MySceneWidget::comprovaColisioIVictoria(bool mortyHaCanviatCasella) {
  if (!gameStarted || mortyFila < 0 || mortyCol < 0)
    return;

  if (fantasmaFila == mortyFila && fantasmaCol == mortyCol) {
    gameStarted = false;
    gameWon = false;
    emit gameOver();
    return;
  }

  if (mortyHaCanviatCasella && maze[mortyFila][mortyCol] == 4 && totalMonedes > 0 && monedesRecollides >= totalMonedes) {
    gameStarted = false;
    gameWon = true;
    emit statusMessageChanged("Victòria!");
  }
}

// Funció per gestionar la pulsació de tecles
void MySceneWidget::keyPressEvent(QKeyEvent *event){
  makeCurrent();
  if (!gameStarted) {
    switch (event->key()) {
      case Qt::Key_Up:
      case Qt::Key_Left:
      case Qt::Key_Right:
        event->ignore();
        return;
      default:
        break;
    }
  }

  switch (event->key()) {
    case Qt::Key_C:
      if (cameraMode == CameraGeneral) {
        cameraMode = CameraFirstPerson;
      } else {
        cameraMode = CameraGeneral;
      }
      emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
      update();
      break;

    case Qt::Key_Left:
      cameraMorty += glm::half_pi<float>();
      if (gameStarted) {
        mouFantasma();
        comprovaColisioIVictoria(false);
      }
      update();
      break;

    case Qt::Key_Right:
      cameraMorty -= glm::half_pi<float>();
      if (gameStarted) {
        mouFantasma();
        comprovaColisioIVictoria(false);
      }
      update();
      break;

    // Gestió del moviment de Morty
    case Qt::Key_Up: {
      int mortyFilaActual = mortyFila;
      int mortyColActual = mortyCol;
      bool sHaMogut = false;

      if (mortyFilaActual >= 0) {
        int df = int(round(sinf(cameraMorty)));
        int dc = int(round(cosf(cameraMorty)));
        int nf = mortyFilaActual + df;
        int nc = mortyColActual + dc;
        const bool allCoinsCollected = (totalMonedes > 0 && monedesRecollides >= totalMonedes);
        const bool isExitCell = (nf >= 0 && nf < N && nc >= 0 && nc < M && maze[nf][nc] == 4);

        if (nf >= 0 && nf < N && nc >= 0 && nc < M && maze[nf][nc] != 1 && (!isExitCell || allCoinsCollected)) {
          mortyFila = nf;
          mortyCol = nc;
          sHaMogut = true;
        }
      }

      comprovaColisioIVictoria(sHaMogut);

      mouFantasma();

      if (sHaMogut && maze[mortyFila][mortyCol] != 4) {
        recullMonedaSiCal(mortyFila, mortyCol);
      }

      comprovaColisioIVictoria(sHaMogut);

      emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
      update();
      break;
    }

    case Qt::Key_N:
      modeNocturn = !modeNocturn;
      emit nightModeChanged(modeNocturn);
      update();
      break;

    case Qt::Key_O:
      if (!modeNocturn) {
        makeCurrent();
        angleSol += glm::radians(5.0f);
        if (angleSol > glm::pi<float>())
          angleSol = glm::pi<float>();
        update();
      }
      break;

    case Qt::Key_P:
      if (!modeNocturn) {
        makeCurrent();
        angleSol -= glm::radians(5.0f);
        if (angleSol < 0.0f)
          angleSol = 0.0f;
        update();
      }
      break;

    case Qt::Key_Plus:
    case Qt::Key_Equal:
      if (cameraMode == CameraGeneral) {
        cameraDistScale *= 0.9f;
        if (cameraDistScale < minCameraDistScale) cameraDistScale = minCameraDistScale;
        emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
        update();
      }
      break;

    case Qt::Key_Minus:
    case Qt::Key_Underscore:
      if (cameraMode == CameraGeneral) {
        cameraDistScale *= 1.1f;
        if (cameraDistScale > maxCameraDistScale) cameraDistScale = maxCameraDistScale;
        emit cameraStateChanged(cameraPsi, cameraTheta, cameraDistScale, int(cameraMode));
        update();
      }
      break;

    default:
      BaseGLWidget::keyPressEvent(event);
      break;
  }
}

// Funció per actualitzar la càmera segons la posició de Morty
void MySceneWidget::updateCamera(int mortyFila, int mortyCol){
    float ra;
      if (alt == 0) {
          ra = 1.0f;
      } else {
          ra = float(ample) / float(alt);
      }

  if (cameraMode == CameraGeneral) {
    // Càmera general amb Euler
    float fov = glm::radians(angleVisioGeneral);
    float halfFov = fov * 0.5f;
    // En un aspecte estret, el FOV horitzontal és el que limita l'escena visible
    float halfHorizontalFov = atanf(tanf(halfFov) * ra);
    float limitingHalfFov = std::min(halfFov, halfHorizontalFov);
    float limitingS = sinf(limitingHalfFov);
    float dist = (limitingS > 1e-6f) ? (radiEscena / limitingS) : (std::max(MySceneWidget::N, MySceneWidget::M) * 1.5f);

    // Apliquem una escala multiplicativa a la distància de la càmera per fer zoom
    dist *= cameraDistScale;

    glm::vec3 offset(cosf(cameraTheta) * sinf(cameraPsi),sinf(cameraTheta), cosf(cameraTheta) * cosf(cameraPsi));
    glm::vec3 OBS = sceneCenter + dist * offset;
    glm::vec3 VRP = sceneCenter;
    glm::vec3 UP(0.0f, 1.0f, 0.0f);

    glm::mat4 VM = glm::lookAt(OBS, VRP, UP);
    float nearP = std::max(0.1f, dist - radiEscena - 1.0f);
    float farP = dist + radiEscena + 10.0f;
    glm::mat4 PM = glm::perspective(fov, ra, nearP, farP);
    currentVM = VM;
    glUniformMatrix4fv(VMLoc, 1, GL_FALSE, &VM[0][0]);
    glUniformMatrix4fv(PMLoc, 1, GL_FALSE, &PM[0][0]);

    } else if (cameraMode == CameraFirstPerson) {
        // Càmera en primera persona que segueix Morty
        const float alcadaYull = 0.82f;
        const float desplaçEndavant = 0.22f;
        const glm::vec3 ancora(float(mortyFila) + 0.5f, alcadaYull, float(mortyCol) + 0.5f);

        glm::vec3 davant(sinf(cameraMorty), 0.0f, cosf(cameraMorty));
        davant = glm::normalize(davant);

        glm::vec3 posCam = ancora + davant * desplaçEndavant;
        glm::vec3 target = posCam + davant;
        glm::vec3 UP(0.0f, 1.0f, 0.0f);

        glm::mat4 VM = glm::lookAt(posCam, target, UP);
        glm::mat4 PM = glm::perspective(glm::radians(90.0f), ra, 0.1f, 120.0f);
          currentVM = VM;
        glUniformMatrix4fv(VMLoc, 1, GL_FALSE, &VM[0][0]);
        glUniformMatrix4fv(PMLoc, 1, GL_FALSE, &PM[0][0]);
  }
}

// Funció per carregar els shaders
void MySceneWidget:: carregaShaders() {
    BaseGLWidget :: carregaShaders();

    const GLuint prog = program->programId();
    llumPosLoc = glGetUniformLocation(prog, "posFocus");
    llumColorLoc = glGetUniformLocation(prog, "colorFocus");
    solActiuLoc = glGetUniformLocation(prog, "solActiu");
    llumAmbientGlobalLoc = glGetUniformLocation(prog, "llumAmbientGlobal");
    llanternaActivaLoc = glGetUniformLocation(prog, "llanternaActiva");
    posLlanternaLoc = glGetUniformLocation(prog, "posLlanterna");
    colorLlanternaLoc = glGetUniformLocation(prog, "colorLlanterna");
    llumFantasmaActivaLoc = glGetUniformLocation(prog, "llumFantasmaActiva");
    posLlumFantasmaLoc = glGetUniformLocation(prog, "posLlumFantasma");
    colorLlumFantasmaLoc = glGetUniformLocation(prog, "colorLlumFantasma");
    numMonedesLlumLoc = glGetUniformLocation(prog, "numMonedesLlum");
    posMonedaLlumLoc = glGetUniformLocation(prog, "posMonedaLlum");
    dirMonedaLlumLoc = glGetUniformLocation(prog, "dirMonedaLlum");
    colorMonedaLlumLoc = glGetUniformLocation(prog, "colorMonedaLlum");
}

void MySceneWidget::resizeGL(int w, int h) {
  // Deixem que la classe base actualitzi la finestra de vista i les dimensions guardades
  BaseGLWidget::resizeGL(w, h);
  update();
}

void MySceneWidget::carregaVAOModelIluminat(GLuint &vao, Model &model) {
  const int nVerts = int(model.faces().size()) * 3;
  const size_t bufSizeVec = sizeof(GLfloat) * nVerts * 3;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vb[6];
  glGenBuffers(6, vb);

  glBindBuffer(GL_ARRAY_BUFFER, vb[0]);
  glBufferData(GL_ARRAY_BUFFER, bufSizeVec, model.VBO_vertices(), GL_STATIC_DRAW);
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glBindBuffer(GL_ARRAY_BUFFER, vb[1]);
  glBufferData(GL_ARRAY_BUFFER, bufSizeVec, model.VBO_normals(), GL_STATIC_DRAW);
  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  glBindBuffer(GL_ARRAY_BUFFER, vb[2]);
  glBufferData(GL_ARRAY_BUFFER, bufSizeVec, model.VBO_matamb(), GL_STATIC_DRAW);
  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  glBindBuffer(GL_ARRAY_BUFFER, vb[3]);
  glBufferData(GL_ARRAY_BUFFER, bufSizeVec, model.VBO_matdiff(), GL_STATIC_DRAW);
  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  glBindBuffer(GL_ARRAY_BUFFER, vb[4]);
  glBufferData(GL_ARRAY_BUFFER, bufSizeVec, model.VBO_matspec(), GL_STATIC_DRAW);
  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  glBindBuffer(GL_ARRAY_BUFFER, vb[5]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nVerts, model.VBO_matshin(), GL_STATIC_DRAW);
  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}

void MySceneWidget::creaBufferCub(){
    // Carreguem el cub que hem carregat a BaseGLWidget,
    // que ens servirà per dibuixar el terra i les parets del laberint
    BaseGLWidget::creaBufferCub();

    // Carreguem els VAO dels models que tenim carregats a initializeGL()
    carregaVAOModelIluminat(VAO_Morty, mortyModel);
    carregaVAOModelIluminat(VAO_Fantasma, fantasmaModel);
    carregaVAOModelIluminat(VAO_Moneda, monedaModel);
    carregaVAOModelIluminat(VAO_Torre, torreModel);
  }

// Funció per configurar la transformació de model de Morty
  void MySceneWidget :: modelTransformMorty(int fila, int col){
    glm::mat4 TG(1.0f);
    float scaleMorty = mortyScale;
    glm::vec3 centreBaseMorty = mortyCentreBase;

    // Cada casella del laberint és 1x1; col·loquem Morty al centre de la casella, amb el mateix gir que la càmera en primera persona.
    TG = glm::translate(TG, glm::vec3(float(fila) + 0.5f, 0.0f, float(col) + 0.5f));
    TG = glm::rotate(TG, cameraMorty, glm::vec3(0.0f, 1.0f, 0.0f));
    TG = glm::scale(TG, glm::vec3(scaleMorty));
    TG = glm::translate(TG, -centreBaseMorty);

    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
    const glm::mat3 NM = glm::mat3(glm::transpose(glm::inverse(currentVM * TG)));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &NM[0][0]);
  }

// Funció per configurar la transformació de model del fantasma
  void MySceneWidget :: modelTransformFantasma(int fila, int col){
    glm:: mat4 TG(1.0f);
    float scaleFantasma = fantasmaScale;
    glm::vec3 centreBaseFantasma = fantasmaCentreBase;
    float rotY = 0.0f;

    if (fantasmaDirFila != 0 || fantasmaDirCol != 0) {
      rotY = atan2f(float(fantasmaDirFila), float(fantasmaDirCol));
    }

    TG = glm:: translate(TG, glm:: vec3 (float(fila) + 0.5f, 0.0f, float(col) + 0.5f));
    TG = glm::rotate(TG, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    TG = glm::scale(TG, glm :: vec3 (scaleFantasma));
    TG = glm::translate(TG, -centreBaseFantasma);

    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
    const glm::mat3 NM = glm::mat3(glm::transpose(glm::inverse(currentVM * TG)));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &NM[0][0]);

  }

// Funció per configurar la transformació de model de la moneda
void MySceneWidget::modelTransformMoneda(int fila, int col){
  glm:: mat4 TG(1.0f);
  float scaleMoneda = monedaScale;
  glm::vec3 centreBaseMoneda = monedaCentreBase;

    // El terra té una alçada de 0.1, per situar la moneda damunt del terra
    const float superficieTerraY = 0.1f;
    
    TG = glm::translate(TG, glm::vec3(float(fila) + 0.5f, superficieTerraY, float(col) + 0.5f));
    TG = glm::rotate(TG, angleGirMonedes, glm::vec3(0.0f, 1.0f, 0.0f));
    TG = glm::scale(TG, glm :: vec3 (scaleMoneda));
    TG = glm::translate(TG, -centreBaseMoneda);
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
    const glm::mat3 NM = glm::mat3(glm::transpose(glm::inverse(currentVM * TG)));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &NM[0][0]);
  }

// Funció per configurar la transformació de model de la torre
  void MySceneWidget::modelTransformTorre(int fila, int col){
    glm:: mat4 TG(1.0f);
    float scaleTorre = torreScale;
    float offsetExterior = 2.0f;  
    glm::vec3 centreBaseTorre = torreCentreBase;

    float x = float(fila) + 0.5f;
    float z = float(col) + 0.5f;
    float rotY = 0.0f;  // Rotació per orientar la porta cap a la sortida

    // Depenent de la posició de la torre (si és en una paret exterior),
    // l'orientem cap a fora del laberint
    if (fila == 0) {
      x -= offsetExterior;         
      rotY = glm::half_pi<float>(); 
    } else if (fila == N - 1) {
      x += offsetExterior;      
      rotY = -glm::half_pi<float>();  
    } else if (col == 0) {
      z -= offsetExterior;      
      rotY = 0.0f;                   
    } else if (col == M - 1) {
      z += offsetExterior;      
      rotY = glm::pi<float>();      
    }

    TG = glm::translate(TG, glm:: vec3 (x, 0.0f, z));
    TG = glm::rotate(TG, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    TG = glm::scale(TG, glm::vec3 (scaleTorre));
    TG = glm::translate(TG, -centreBaseTorre);

    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
    const glm::mat3 NM = glm::mat3(glm::transpose(glm::inverse(currentVM * TG)));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &NM[0][0]);
  }

// Funció per configurar la transformació de model del terra
  void MySceneWidget::modelTransformTerra(int fila, int col){
  // Les files corresponen a l'eix X positiu i les columnes a l'eix Z positiu
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, glm::vec3(float(fila), 0.0f, float(col)));
  TG = glm::scale(TG, glm::vec3(1.0f, 0.1f, 1.0f));  
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
  const glm::mat3 NM = glm::mat3(glm::transpose(glm::inverse(currentVM * TG)));
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &NM[0][0]);
}

// Funció per configurar la transformació de model de la paret
void MySceneWidget::modelTransformWall(int fila, int col){
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, glm::vec3(float(fila) + 0.5f, 0.1f, float(col) + 0.5f));
  TG = glm::scale(TG, wallScale);
  TG = glm::translate(TG, -wallCentreBase);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
  const glm::mat3 NM = glm::mat3(glm::transpose(glm::inverse(currentVM * TG)));
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &NM[0][0]);
}


// Funció per configurar el color del cel de laViewport
glm::vec3 MySceneWidget::colorCelViewport() const {
  if (modeNocturn) {
    return glm::vec3(0.02f, 0.02f, 0.06f);
  }

  const float elevacioSol = glm::clamp(sinf(angleSol), 0.0f, 1.0f);
  static const glm::vec3 cieloBaixBase(0.06f, 0.08f, 0.14f);
  static const glm::vec3 cieloAltBase(0.5f, 0.7f, 1.0f);
  const glm::vec3 cieloBaix = glm::clamp(
      cieloBaixBase * colorFocusPrincipal, glm::vec3(0.02f), glm::vec3(1.0f));
  const glm::vec3 cieloAlt = glm::clamp(
      cieloAltBase * colorFocusPrincipal, glm::vec3(0.0f), glm::vec3(1.0f));
  return glm::mix(cieloBaix, cieloAlt, elevacioSol);
}

// Funció per enviar els uniforms de la llum
void MySceneWidget::enviarUniformsLlum() {
  const glm::vec3 ambientDia(0.22f);
  const glm::vec3 ambientNit(0.04f);

  float elevacioSol;
  if (modeNocturn) {
    elevacioSol = 0.0f;
  } else {
    elevacioSol = glm::clamp(sinf(angleSol), 0.0f, 1.0f);
  }

  glm::vec3 ambientActual;
  if (modeNocturn) {
    ambientActual = ambientNit;
  } else {
    ambientActual = ambientDia * (0.08f + 0.92f * elevacioSol) * colorFocusPrincipal;
  }
  if (llumAmbientGlobalLoc >= 0)
    glUniform3fv(llumAmbientGlobalLoc, 1, &ambientActual[0]);

  glm::vec3 dirSol(cosf(angleSol), sinf(angleSol), 0.12f);
  dirSol = glm::normalize(dirSol);
  const glm::vec3 colorSol = colorFocusPrincipal * elevacioSol;

  int solActiu;
  if (modeNocturn) {
    solActiu = 0;
  } else {
    solActiu = 1;
  }
  if (solActiuLoc >= 0)
    glUniform1i(solActiuLoc, solActiu);
  if (llumColorLoc >= 0)
    glUniform3fv(llumColorLoc, 1, &colorSol[0]);
  if (llumPosLoc >= 0)
    glUniform3fv(llumPosLoc, 1, &dirSol[0]);

    // Gestió de la llanterna per al Morty i el fantasma
  int llanternaOn;
  if (modeNocturn && mortyFila >= 0) {
    llanternaOn = 1;
  } else {
    llanternaOn = 0;
  }

  int fantasmaLlumOn;
  if (modeNocturn && fantasmaFila >= 0) {
    fantasmaLlumOn = 1;
     }else {
    fantasmaLlumOn = 0;
  }

  glm::vec3 posLlanterna(0.0f);
  if (llanternaOn) {
    posLlanterna = glm::vec3(float(mortyFila) + 0.5f, alcadaLlanterna, float(mortyCol) + 0.5f);
  }

  glm::vec3 posLlumFantasma(0.0f);
  if (fantasmaLlumOn) {
    posLlumFantasma = glm::vec3(float(fantasmaFila) + 0.5f, alcadaLlumFantasma, float(fantasmaCol) + 0.5f);
  }

  // Gestió de la llanterna per al Morty i el fantasma
  if (llanternaActivaLoc >= 0)
    glUniform1i(llanternaActivaLoc, llanternaOn);
  if (llanternaOn && colorLlanternaLoc >= 0)
    glUniform3fv(colorLlanternaLoc, 1, &colorLlanterna[0]);
  if (llanternaOn && posLlanternaLoc >= 0)
    glUniform3fv(posLlanternaLoc, 1, &posLlanterna[0]);

  if (llumFantasmaActivaLoc >= 0)
    glUniform1i(llumFantasmaActivaLoc, fantasmaLlumOn);
  if (fantasmaLlumOn && colorLlumFantasmaLoc >= 0)
    glUniform3fv(colorLlumFantasmaLoc, 1, &colorLlumFantasma[0]);
  if (fantasmaLlumOn && posLlumFantasmaLoc >= 0)
    glUniform3fv(posLlumFantasmaLoc, 1, &posLlumFantasma[0]);


    // Gestió de les monedes
  const int nMonedes = std::min(int(posicionsMonedes.size()), maxMonedesLlum);
  glm::vec3 posMonedes[maxMonedesLlum];
  glm::vec3 dirMonedes[maxMonedesLlum];
  const float superficieTerraY = 0.11f;
  const glm::mat4 rotMoneda = glm::rotate(glm::mat4(1.0f), angleGirMonedes, glm::vec3(0.0f, 1.0f, 0.0f));

  // Gestió de la llum de les monedes (calcula la posició i la direcció de cada moneda)
  for (int i = 0; i < nMonedes; ++i) {
    const int fila = posicionsMonedes[i].first;
    const int col = posicionsMonedes[i].second;
    posMonedes[i] = glm::vec3(
        float(fila) + 0.5f,
        superficieTerraY + 0.5f * monedaScale * monedaModelHeight,
        float(col) + 0.5f);
    dirMonedes[i] = glm::normalize(glm::vec3(rotMoneda * glm::vec4(monedaNormalCaraLocal, 0.0f)));
  }

  // Envia els uniforms de les monedes
  if (numMonedesLlumLoc >= 0)
    glUniform1i(numMonedesLlumLoc, nMonedes);
  if (nMonedes > 0 && posMonedaLlumLoc >= 0)
    glUniform3fv(posMonedaLlumLoc, nMonedes, &posMonedes[0][0]);
  if (nMonedes > 0 && dirMonedaLlumLoc >= 0)
    glUniform3fv(dirMonedaLlumLoc, nMonedes, &dirMonedes[0][0]);
  if (colorMonedaLlumLoc >= 0)
    glUniform3fv(colorMonedaLlumLoc, 1, &colorLlumMoneda[0]);
}

// Funció per establir el color principal del focus
void MySceneWidget::setFocusColorMix(float blueAmt, float redAmt, float yellowAmt) {
  static const glm::vec3 kFocusBlau(0.45f, 0.65f, 1.0f);
  static const glm::vec3 kFocusVermell(1.0f, 0.35f, 0.30f);
  static const glm::vec3 kFocusGroc(1.0f, 0.92f, 0.40f);
  static const glm::vec3 kFocusBlanc(1.0f);

  const float b = glm::clamp(blueAmt, 0.0f, 1.0f);
  const float r = glm::clamp(redAmt, 0.0f, 1.0f);
  const float y = glm::clamp(yellowAmt, 0.0f, 1.0f);

  const float mixAmt = glm::clamp(std::max({b, r, y}), 0.0f, 1.0f);
  if (mixAmt <= 0.0f) {
    colorFocusPrincipal = kFocusBlanc;
  } else {
    const glm::vec3 tint = glm::clamp(
        b * kFocusBlau + r * kFocusVermell + y * kFocusGroc,
        glm::vec3(0.0f), glm::vec3(1.0f));
    colorFocusPrincipal = glm::mix(kFocusBlanc, tint, mixAmt);
  }

  makeCurrent();
  update();
}

// El paintGL és el mètode on es dibuixa cada fotograma. Aquí s'ha de dibuixar tot el que es vol veure a la finestra.
// Primer actualitzem la càmera segons la posició de Morty, després dibuixem el laberint (terra i parets)
// i finalment els models (Morty, fantasma, monedes i torres).
void MySceneWidget:: paintGL(){
  // Establim el color del cel de laViewport
  const glm::vec3 cel = colorCelViewport();
  glClearColor(cel.r, cel.g, cel.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Establim el viewport
  glViewport(0, 0, ample, alt);

  // Si la posició de Morty no està definida, la busquem al laberint
  if (mortyFila < 0) {
    for (int i = 0; i < N; ++i) for (int j = 0; j < M; ++j) if (maze[i][j] == 2) { mortyFila = i; mortyCol = j; }
  }
  // Si la posició del fantasma no està definida, la busquem al laberint una sola vegada
  if (fantasmaFila < 0) {
    for (int i = 0; i < N; ++i) for (int j = 0; j < M; ++j) if (maze[i][j] == 3) { fantasmaFila = i; fantasmaCol = j; }
  }

  // Actualitzem la càmera abans de dibuixar
  if (mortyFila >= 0)
    updateCamera(mortyFila, mortyCol);
  else
    updateCamera(1, 1);

  enviarUniformsLlum();

  auto dibuixaEscena = [&](bool mostraMorty) {
    glUniform1i(texActiveLoc, 0);
    // Dibuix del laberint amb el cub base (VAO_Cub de BaseGLWidget)
    glBindVertexArray(VAO_Cub);

    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < M; ++j) {
        // No dibuixem res si és una sortida
        if (maze[i][j] == 4) continue;

        if (maze[i][j] != 1) {
          glUniform1i(texActiveLoc, 0);
          modelTransformTerra(i, j);
          glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        if (maze[i][j] == 1) {
          modelTransformWall(i, j);
          glUniform1i(texLoc, 0);
          glUniform1i(texActiveLoc, 1);
          wallMesh->Render();
          // Tornem al cub per continuar dibuixant el terra
          glUniform1i(texActiveLoc, 0);
          glBindVertexArray(VAO_Cub); 
        } 
      }
    }

    glBindVertexArray(0);

    // Dibuixem Morty
    if (mostraMorty) {
      glUniform1i(texActiveLoc, 0);
      modelTransformMorty(mortyFila, mortyCol);
      glBindVertexArray(VAO_Morty);
      glDrawArrays(GL_TRIANGLES, 0, int(mortyModel.faces().size() * 3));
      glBindVertexArray(0);
    }

    // Dibuixem el fantasma
    glUniform1i(texActiveLoc, 0);
    modelTransformFantasma(fantasmaFila, fantasmaCol);
    glBindVertexArray (VAO_Fantasma);
    glDrawArrays(GL_TRIANGLES, 0, fantasmaModel.faces().size() * 3);
    glBindVertexArray(0);

    // Dibuixem les monedes
    for (const auto &posicioMoneda : posicionsMonedes) {
      glUniform1i(texActiveLoc, 0);
      modelTransformMoneda(posicioMoneda.first, posicioMoneda.second);
      glBindVertexArray (VAO_Moneda);
      glDrawArrays(GL_TRIANGLES, 0, monedaModel.faces().size() * 3);
    }

    // Dibuixem les torres a les sortides (valor 4 al laberint)
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < M; ++j) {
        if (maze[i][j] == 4) {
         modelTransformTorre(i, j);
          glUniform1i(texActiveLoc, 1);
          glUniform1i(texLoc, 0);
          towerMesh->Render();
          glUniform1i(texActiveLoc, 0);
        }
      }
    }

    glBindVertexArray(0);
  };

  // Dibuixem l'escena segons la càmera
  dibuixaEscena(cameraMode != CameraFirstPerson);

  // Establim la finestra de vista secundària
  const int width2Viewport = std::max(60, ample / 3);
  const int height2Viewport = std::max(60, alt / 4);
  const int x2Viewport = std::max(0, ample - width2Viewport - 30);
  const int y2Viewport = 0;

  glViewport(x2Viewport, y2Viewport, width2Viewport, height2Viewport);

  GLfloat clearColor[4];
  glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
  glEnable(GL_SCISSOR_TEST);
  glScissor(x2Viewport, y2Viewport, width2Viewport, height2Viewport);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
  glDisable(GL_SCISSOR_TEST);

float aspect2Viewport;

if (height2Viewport == 0) {
    aspect2Viewport = 1.0f;
      } else {
          aspect2Viewport = float(width2Viewport) / float(height2Viewport);
      }

  const float halfWidth2Viewport = radiEscena;
  const float halfHeight2Viewport = halfWidth2Viewport / aspect2Viewport;
  glm::vec3 OBS = sceneCenter + glm::vec3(0.0f, radiEscena * 1.0f, 0.0f);
  glm::vec3 VRP = sceneCenter;
  glm::vec3 UP(-1.0f, 0.0f, 0.0f);

  glm::mat4 VM_aeria = glm::lookAt(OBS, VRP, UP);
  float l2Viewport = -halfWidth2Viewport;
  float r2Viewport = halfWidth2Viewport;
  float b2Viewport = -halfHeight2Viewport;
  float t2Viewport = halfHeight2Viewport;
  float zNear2Viewport = -radiEscena * 8.0f;
  float zFar2Viewport = radiEscena * 8.0f;

  glm::mat4 PM_aeria = glm::ortho(l2Viewport, r2Viewport, b2Viewport, t2Viewport, zNear2Viewport, zFar2Viewport);

  currentVM = VM_aeria;
  glUniformMatrix4fv(VMLoc, 1, GL_FALSE, &VM_aeria[0][0]);
  glUniformMatrix4fv(PMLoc, 1, GL_FALSE, &PM_aeria[0][0]);
  enviarUniformsLlum();

  dibuixaEscena(true);
}
