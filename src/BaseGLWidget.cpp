#include "BaseGLWidget.h"
// namespace {
// QString assetPath(const QString &relativePath) {
//   return QDir(QCoreApplication::applicationDirPath()).filePath(relativePath);
// }
// }

#include <iostream>

BaseGLWidget::BaseGLWidget (QWidget* parent) : QOpenGLWidget(parent), program(NULL){
  setFocusPolicy(Qt::StrongFocus);  // per rebre events de teclat
}

BaseGLWidget::~BaseGLWidget (){
  if (program != NULL)
    delete program;
}

void BaseGLWidget::initializeGL (){
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();  

  // Habilitar depth testing
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  carregaShaders();
  creaBufferCub();
  
  // Inicializar matrices
  glm::mat4 PM(1.0f);
  PM = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 20.0f);
  glUniformMatrix4fv(PMLoc, 1, GL_FALSE, &PM[0][0]);
  
  glm::vec3 OBS = glm::vec3(2.2f, 2.0f, 2.2f);
  glm::vec3 VRP = glm::vec3(0.5f, 0.5f, 0.5f);
  glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::mat4 VM = glm::lookAt(OBS, VRP, UP);
  currentVM = VM;
  glUniformMatrix4fv(VMLoc, 1, GL_FALSE, &VM[0][0]);
}

void BaseGLWidget::paintGL () {
  // Esborrem el frame-buffer i depth buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // Carreguem la transformació de model
  modelTransform();
  
  // Dibuixem el cub
  glBindVertexArray(VAO_Cub);
  glDrawArrays(GL_TRIANGLES, 0, 36);  // 36 vértices
  glBindVertexArray(0);
}

void BaseGLWidget::modelTransform () {
  // Centrar el cubo [0,1] en el origen visual
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, glm::vec3(-0.5f, -0.5f, -0.5f));
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
  const glm::mat3 NM = glm::mat3(glm::transpose(glm::inverse(currentVM * TG)));
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &NM[0][0]);
}

void BaseGLWidget::resizeGL (int w, int h) {
// Aquest codi és necessari únicament per a MACs amb pantalla retina.
#ifdef __APPLE__
  GLint vp[4];
  glGetIntegerv (GL_VIEWPORT, vp);
  ample = vp[2];
  alt = vp[3];
#else
  ample = w;
  alt = h;
#endif

  float ra = (alt == 0) ? 1.0f : float(ample) / float(alt);
  glm::mat4 PM = glm::perspective(glm::radians(60.0f), ra, 0.1f, 20.0f);
  glUniformMatrix4fv(PMLoc, 1, GL_FALSE, &PM[0][0]);
}

void BaseGLWidget::keyPressEvent(QKeyEvent* event) {
  makeCurrent();
  switch (event->key()) {
    // Aqui anyadireis els events de teclat que necesiteu
    default: event->ignore(); break;
  }
  update();
}

void BaseGLWidget::creaBufferCub (){
   // Dades del cub
  // Vèrtexs del cub
  glm::vec3 vertexs[8] = {
       /* 0*/ glm::vec3( 0.0, 0.0, 0.0),  /* 1*/ glm::vec3( 1.0, 0.0, 0.0),
       /* 2*/ glm::vec3( 0.0, 1.0, 0.0),  /* 3*/ glm::vec3( 1.0, 1.0, 0.0),
       /* 4*/ glm::vec3( 0.0, 0.0, 1.0),  /* 5*/ glm::vec3( 1.0, 0.0, 1.0),
       /* 6*/ glm::vec3( 0.0, 1.0, 1.0),  /* 7*/ glm::vec3( 1.0, 1.0, 1.0)
  };

  // VBO amb la posició dels vèrtexs
  glm::vec3 poscub[36] = {  // 12 triangles
       vertexs[0], vertexs[2], vertexs[1],
       vertexs[1], vertexs[2], vertexs[3],
       vertexs[5], vertexs[1], vertexs[7],
       vertexs[1], vertexs[3], vertexs[7],
       vertexs[2], vertexs[6], vertexs[3],
       vertexs[3], vertexs[6], vertexs[7],
       vertexs[0], vertexs[4], vertexs[6],
       vertexs[0], vertexs[6], vertexs[2],
       vertexs[0], vertexs[1], vertexs[4],
       vertexs[1], vertexs[5], vertexs[4],
       vertexs[4], vertexs[5], vertexs[6],
       vertexs[5], vertexs[7], vertexs[6]
  };

  // VBO amb la normal de cada vèrtex
  glm::vec3 normals[6] = {
       /* 0*/ glm::vec3( 1.0, 0.0,  0.0),  /* 1*/ glm::vec3( -1.0, 0.0, 0.0),
       /* 2*/ glm::vec3( 0.0, 1.0,  0.0),  /* 3*/ glm::vec3( 0.0, -1.0, 0.0),
       /* 4*/ glm::vec3( 0.0, 0.0,  1.0),  /* 5*/ glm::vec3( 0.0, 0.0, -1.0)
  };
  glm::vec3 normcub[36] = {
       normals[5], normals[5], normals[5],
       normals[5], normals[5], normals[5],
       normals[0], normals[0], normals[0],
       normals[0], normals[0], normals[0],
       normals[2], normals[2], normals[2],
       normals[2], normals[2], normals[2],
       normals[1], normals[1], normals[1],
       normals[1], normals[1], normals[1],
       normals[3], normals[3], normals[3],
       normals[3], normals[3], normals[3],
       normals[4], normals[4], normals[4],
       normals[4], normals[4], normals[4]
  };

  // inicialitzem el material del cub
  glm::vec3 amb, diff, spec;
  float shin;
  amb = glm::vec3(0.1,0.0,0.0);
  diff = glm::vec3(0.6,0.5,0.5);
  spec = glm::vec3(0.6,0.6,0.6);
  shin = 100;

  // Fem que aquest material afecti a tots els vèrtexs per igual
  glm::vec3 matambcub[36] = {
	amb, amb, amb, amb, amb, amb,
	amb, amb, amb, amb, amb, amb,
	amb, amb, amb, amb, amb, amb,
	amb, amb, amb, amb, amb, amb,
	amb, amb, amb, amb, amb, amb,
	amb, amb, amb, amb, amb, amb
  };
  float a=0.5;
  glm::vec3 diff1 = glm::vec3(0.6,0.0,0.0);
  glm::vec3 matdiffcub[36] = {
	diff, diff, diff1, diff, diff, diff,
	diff, diff, diff1, diff, diff, diff,
	diff*a, diff*a, diff1*a, diff*a, diff*a, diff1*a,
	diff, diff, diff1, diff, diff, diff,
	diff, diff, diff1, diff, diff, diff,
	diff, diff, diff1, diff, diff, diff
  };
  glm::vec3 matspeccub[36] = {
	spec, spec, spec, spec, spec, spec,
	spec, spec, spec, spec, spec, spec,
	spec, spec, spec, spec, spec, spec,
	spec, spec, spec, spec, spec, spec,
	spec, spec, spec, spec, spec, spec,
	spec, spec, spec, spec, spec, spec
  };
  float matshincub[36] = {
	shin, shin, shin, shin, shin, shin,
	shin, shin, shin, shin, shin, shin,
	shin, shin, shin, shin, shin, shin,
	shin, shin, shin, shin, shin, shin,
	shin, shin, shin, shin, shin, shin,
	shin, shin, shin, shin, shin, shin
  };

// Creació del Vertex Array Object del cub
  glGenVertexArrays(1, &VAO_Cub);
  glBindVertexArray(VAO_Cub);

  GLuint VBO_Cub[6];
  glGenBuffers(6, VBO_Cub);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Cub[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(poscub), poscub, GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_Cub[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normcub), normcub, GL_STATIC_DRAW);

  // Activem l'atribut normalLoc
  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Cub[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matambcub), matambcub, GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Cub[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matdiffcub), matdiffcub, GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Cub[4]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matspeccub), matspeccub, GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Cub[5]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matshincub), matshincub, GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}


void BaseGLWidget::carregaShaders(){
  // Creem els shaders per al fragment shader i el vertex shader
  QOpenGLShader fs (QOpenGLShader::Fragment, this);
  QOpenGLShader vs (QOpenGLShader::Vertex, this);
  // Carreguem el codi dels fitxers i els compilem
  fs.compileSourceFile("../shaders/basicShader.frag");
  vs.compileSourceFile("../shaders/basicShader.vert");
  // Creem el program
  program = new QOpenGLShaderProgram(this);
  // Li afegim els shaders corresponents
  program->addShader(&fs);
  program->addShader(&vs);
  // Linkem el program
  program->link();
  // Indiquem que aquest és el program que volem usar
  program->bind();

  // Obtenim identificador per a l'atribut “vertex” del vertex shader
  vertexLoc = glGetAttribLocation (program->programId(), "vertex");
  // Obtenim identificador per a l'atribut “color” del vertex shader
  colorLoc = glGetAttribLocation (program->programId(), "color");
  normalLoc = glGetAttribLocation (program->programId(), "normal");
  matambLoc = glGetAttribLocation (program->programId(), "matamb");
  matdiffLoc = glGetAttribLocation (program->programId(), "matdiff");
  matspecLoc = glGetAttribLocation (program->programId(), "matspec");
  matshinLoc = glGetAttribLocation (program->programId(), "matshin");
  texUVLoc = glGetAttribLocation(program->programId(), "texUV");
  // Uniform locations
  transLoc = glGetUniformLocation(program->programId(), "TG");  VMLoc = glGetUniformLocation(program->programId(), "VM");
  PMLoc = glGetUniformLocation(program->programId(), "PM");
  normalMatrixLoc = glGetUniformLocation(program->programId(), "NM");
  texLoc = glGetUniformLocation(program->programId(), "text");
  texActiveLoc = glGetUniformLocation(program->programId(), "textActive");
  // (no global color multiplier; use per-vertex colors)
}


int BaseGLWidget::printOglError(const char file[], int line, const char func[]){
    GLenum glErr;
    int retCode = 0;

    glErr = glGetError();
    const char * error = 0;
    switch (glErr)
    {
        case 0x0500:
            error = "GL_INVALID_ENUM";
            break;
        case 0x501:
            error = "GL_INVALID_VALUE";
            break;
        case 0x502:
            error = "GL_INVALID_OPERATION";
            break;
        case 0x503:
            error = "GL_STACK_OVERFLOW";
            break;
        case 0x504:
            error = "GL_STACK_UNDERFLOW";
            break;
        case 0x505:
            error = "GL_OUT_OF_MEMORY";
            break;
        default:
            error = "unknown error!";
    }
    if (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %s function: %s\n",
                             file, line, error, func);
        std::cout << std::endl;
        retCode = 1;
    }
    return retCode;
}
