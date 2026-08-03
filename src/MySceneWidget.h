#include "BaseGLWidget.h"
#include "../Model/model.h"
#include <QTimer>
#include <QMouseEvent>
#include <QString>
#include <vector>
#include <utility>
#include "../assimp/Mesh.h"

class MySceneWidget : public BaseGLWidget {
  Q_OBJECT

  public:
    explicit MySceneWidget(QWidget *parent = nullptr);
    ~MySceneWidget() override;

    // Dimensions del laberint
    static const int N = 15;
    static const int M = 20;

  protected:
    // Per la OpenGL: inicialitzar, pintar i preparar recursos
    void initializeGL() override;
    void paintGL() override;
    void creaBufferCub() override;
    void carregaShaders() override;
    void resizeGL(int w, int h) override;

    // Per els esdeveniments de teclat i ratolí
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // Per les matrius de transformació per cada cosa que es dibuixa
    void modelTransformMorty(int fila, int col);
    void modelTransformFantasma(int fila, int col);
    void modelTransformMoneda(int fila, int col);
    void modelTransformTorre(int fila, int col);
    void modelTransformWall(int fila, int col);
    void modelTransformTerra(int fila, int col);

    // Per la lògica del joc (moviment, monedes, fantasma, càmera)
    void inicialitzaMonedes();
    void recullMonedaSiCal(int fila, int col);
    void mouFantasma();
    void inicialitzaDireccioFantasma();
    void comprovaColisioIVictoria(bool mortyHaCanviatCasella);
    void updateCamera(int mortyFila, int mortyCol);

  public slots:
    // Funció per iniciar la partida
    void startGame();

    // Per la càmera (des de la UI o el ratolí)
    void setCameraPsi(float valor);
    void setCameraTheta(float valor);
    void setCameraZoomScale(float valor);
    void zoomCameraIn();
    void zoomCameraOut();
    void setCameraFirstPerson(bool activat);

    // Per les opcions de l'escena
    void setCoinsRotationEnabled(bool activat);
    void setFocusColorMix(float blueAmt, float redAmt, float yellowAmt);
    void setNightMode(bool activat);

  // En el apartat de signals posem les funcions que emiten senyals per a que el MyForm les escolti i actualitzi la UI
  signals:
    // Per l'estat de la partida (ho escolto des de MyForm per actualitzar la UI)
    void coinsUpdated(int collected, int total);
    void statusMessageChanged(const QString &message);
    void gameOver();

    // Per la càmera (per sincronitzar spinbox i checkbox amb el que passa a l'escena)
    void cameraStateChanged(float psi, float theta, float zoomScale, int cameraMode);

    // Per les opcions de l'escena (per mantenir el panell al dia)
    void coinsRotationStateChanged(bool activat);
    void nightModeChanged(bool activat);

  private slots:
    // Per girar les monedes
    void rotateCoins();

    // Per el càlculs d'escena (bounding box i centre)
    void boundingBox(Model &m, glm::vec3 &minP, glm::vec3 &maxP, float &height) const;
    void calculaCentreEscenaIRadi();

    // Per el cel i les llums del shader
    glm::vec3 colorCelViewport() const;
    void enviarUniformsLlum();

  private:
    void carregaVAOModelIluminat(GLuint &vao, Model &model);

    // Límits i constants internes 
    static constexpr int maxMonedesLlum = 10;
    static constexpr float alcadaLlanterna = 0.85f;
    static constexpr float alcadaLlumFantasma = 0.65f;

    // Enumeració per triar la càmera
    enum CameraMode {
      CameraGeneral = 0,
      CameraFirstPerson = 1
    };
    CameraMode cameraMode = CameraGeneral;

    // Angles de la càmera
    float cameraPsi = 0.862f;
    float cameraTheta = 0.895f;
    float cameraMorty = 0.0f;
    // Distància de la càmera
    float cameraDistScale = 1.0f;
    float minCameraDistScale = 0.25f;
    float maxCameraDistScale = 4.0f;

    // Botons del ratolí
    // Posició del darrer clic del ratolí
    QPoint lastMousePos;
    bool mousePressed = false;
    bool rightMousePressed = false;

    // Estat de la partida 
    bool gameStarted = false;
    bool gameWon = false;

    // Models carregats i els seus VAO 
    Model mortyModel, fantasmaModel, monedaModel, torreModel;
    GLuint VAO_Morty, VAO_Fantasma, VAO_Moneda, VAO_Torre;

    // Per mirar quantes monedes hi ha i on són
    std::vector<std::pair<int, int>> posicionsMonedes;
    int monedesRecollides = 0;
    int totalMonedes = 0;
    QTimer timerMonedes;
    float angleGirMonedes = 0.0f;

    // Escala i centre de cada model (ho calculo amb el bounding box)
    glm::vec3 mortyCentreBase = glm::vec3(0.0f);
    float mortyScale = 1.0f;
    float mortyModelHeight = 1.0f;

    glm::vec3 fantasmaCentreBase = glm::vec3(0.0f);
    float fantasmaScale = 1.0f;
    float fantasmaModelHeight = 1.0f;

    glm::vec3 monedaCentreBase = glm::vec3(0.0f);
    float monedaScale = 1.0f;
    float monedaModelHeight = 1.0f;

    glm::vec3 torreCentreBase = glm::vec3(0.0f);
    float torreScale = 1.0f;
    float torreModelHeight = 1.0f;

    glm::vec3 wallCentreBase = glm::vec3(0.0f);
    glm::vec3 wallScale = glm::vec3(1.0f);
    float wallModelHeight = 1.0f;

    // Vista general de l'escena (per encuadrar tot el laberint)
    glm::vec3 sceneCenter = glm::vec3(0.0f);
    float radiEscena = 1.0f;
    float angleVisioGeneral = 70.0f;

    // Posicions al laberint (files i columnes)
    int mortyFila = -1;
    int mortyCol = -1;
    int fantasmaFila = -1;
    int fantasmaCol = -1;
    int fantasmaDirFila = 0;
    int fantasmaDirCol = 0;

    // Per la textura de les parets i les torres
    Mesh *wallMesh = nullptr;
    Mesh *towerMesh = nullptr;

    // Per la il·luminació: sol, mode nit i colors
    float angleSol = glm::half_pi<float>(); // 90° per defecte
    bool modeNocturn = false;
    glm::vec3 colorFocusPrincipal = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 colorLlumMoneda = glm::vec3(1.0f, 0.88f, 0.4f);
    glm::vec3 monedaNormalCaraLocal = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 colorLlanterna = glm::vec3(1.0f, 0.92f, 0.35f);
    glm::vec3 colorLlumFantasma = glm::vec3(0.85f, 0.88f, 1.0f);

    // Locations dels uniforms al shader (per no buscar-los cada frame)
    GLuint llumPosLoc;
    GLuint llumColorLoc;
    GLuint solActiuLoc;
    GLuint llumAmbientGlobalLoc;
    GLuint llanternaActivaLoc;
    GLuint posLlanternaLoc;
    GLuint colorLlanternaLoc;
    GLuint llumFantasmaActivaLoc;
    GLuint posLlumFantasmaLoc;
    GLuint colorLlumFantasmaLoc;
    GLuint numMonedesLlumLoc;
    GLuint posMonedaLlumLoc;
    GLuint dirMonedaLlumLoc;
    GLuint colorMonedaLlumLoc;

    // Mapa del laberint
    // 0 = buit, 1 = paret, 2 = Morty, 3 = fantasma, 4 = torre/sortida
    const int maze[MySceneWidget::N][MySceneWidget::M] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
        {1,1,0,0,0,0,1,0,0,1,0,1,0,0,0,1,0,0,1,1},
        {1,1,0,1,1,0,1,1,0,0,0,1,1,1,0,1,1,0,1,1},
        {1,1,3,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1},
        {1,0,0,1,1,1,0,1,1,0,1,1,1,1,0,1,0,1,1,1},
        {1,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,4},
        {1,2,0,1,1,0,1,1,0,1,0,1,1,1,0,1,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,1,0,1,0,1,0,1,0,0,1,1,1,0,1,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1},
        {4,0,0,0,0,0,1,1,0,1,0,0,0,0,0,1,1,0,0,1},
        {1,1,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,4,1,1,1,1,1,1,1,1,1,1}
    };
};