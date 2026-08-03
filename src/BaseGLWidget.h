#define GLM_FORCE_RADIANS
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class BaseGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
  Q_OBJECT

  public:
    BaseGLWidget (QWidget *parent=0);
    ~BaseGLWidget ();

  protected:
    // initializeGL - Aqui incluim les inicialitzacions del contexte grafic.
    virtual void initializeGL ( );
    // paintGL - Mètode cridat cada cop que cal refrescar la finestra.
    // Tot el que es dibuixa es dibuixa aqui.
    virtual void paintGL ( );
    // resizeGL - És cridat quan canvia la mida del widget
    virtual void resizeGL (int width, int height);
    // keyPressEvent - Es cridat quan es prem una tecla
    virtual void keyPressEvent (QKeyEvent *event);

  
    virtual void creaBufferCub ();
    virtual void carregaShaders ();
    virtual void modelTransform ();

    int printOglError(const char file[], int line, const char func[]);


    // attribute locations
    GLuint vertexLoc, colorLoc;
    GLuint normalLoc, matambLoc, matdiffLoc, matspecLoc, matshinLoc;
    GLuint texUVLoc, texLoc, texActiveLoc;
    // uniform locations
    GLuint transLoc, VMLoc, PMLoc, normalMatrixLoc;
    // Program
    QOpenGLShaderProgram *program;
    // Viewport
    GLint ample, alt;
    glm::mat4 currentVM;
    
    // Cub
    GLuint VAO_Cub;
};
