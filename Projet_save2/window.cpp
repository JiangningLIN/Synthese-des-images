/*!\file window.c
 *
 * \brief Utilisation de la SDL2/GL4Dummies et d'OpenGL 3+
 *
 * \author Fares BELHADJ, amsi@ai.univ-paris8.fr
 * \date April 24 2014
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include <GL4D/gl4du.h>
#include <GL4D/gl4duw_SDL2.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <objdetect.hpp>


using namespace cv;
using namespace std;
/*
 * Prototypes des fonctions statiques contenues dans ce fichier C
 */
static void init(void);
static void resize(int w, int h);
static void keydown(int keycode);
static void draw(void);
static void quit(void);

//!\brief identifiant de la sphere et du quad
static GLuint _sphere = 0, _quad = 0;
//!\brief translation de la sphere
//static GLfloat _t[2] = {1000, 1000};
//---------------------------------------------------------------------- end sphere


/*!\brief dimensions de la fenêtre */
static GLfloat _dim[] = {800, 600};
/*!\brief identifiants des (futurs) GLSL programs */
static GLuint _pId = 0;
/*!\brief identifiant de la texture chargee */
static GLuint _tId = 0;
/*!\brief device de capture video */
static VideoCapture * _cap = NULL;
//---------------lumière-------------------------------------------
/*!\brief position de la lumière relativement à la sphère éclairée */
static GLfloat _lumPos0[4] = {0, -1, 0, 1.0};

//-------------------------end lumière-----------------------------
CascadeClassifier * face_cc;
CascadeClassifier * eye_cc;

/*!\brief La fonction principale initialise la bibliotheque SDL2, demande la creation de la fenetre SDL et du contexte OpenGL par
 * l'appel a \ref initWindow, initialise OpenGL avec \ref initGL et lance la boucle (infinie) principale.
 */
int main(int argc, char ** argv) {

  face_cc = new CascadeClassifier("haarcascade_frontalface_default.xml");
  eye_cc  = new CascadeClassifier("haarcascade_eye.xml");
  if(face_cc == NULL || eye_cc == NULL) {
      return 1;
  }
  if(!gl4duwCreateWindow(argc, argv, "GL4Dummies", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			 (int)_dim[0], (int)_dim[1], SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN)) {
    return 1;
  }
  atexit(quit);
  init();
  gl4duwResizeFunc(resize);
  gl4duwKeyDownFunc(keydown);
  gl4duwDisplayFunc(draw);
  gl4duwMainLoop();
  return 0;
}

//------------------------------------------------------------------------ functions sphere
//!\brief Initialise les parametres OpenGL et géométrie
static void init(void) {
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
  _pId  = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
  gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  resize((int)_dim[0], (int)_dim[1]);
  _sphere = gl4dgGenSpheref(30, 30);    // 30 : triangles
  _quad   = gl4dgGenQuadf();
  glGenTextures(1, &_tId);
  glBindTexture(GL_TEXTURE_2D, _tId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  _cap = new VideoCapture(0);
  if(!_cap || !_cap->isOpened()) {
    delete _cap;
    _cap = new VideoCapture(CV_CAP_ANY);
  }
  _cap->set(CV_CAP_PROP_FRAME_WIDTH,  (int)_dim[0]);
  _cap->set(CV_CAP_PROP_FRAME_HEIGHT, (int)_dim[1]);
}

//!\brief Cette fonction parametre la vue (viewport) OpenGL en fonction des dimensions de la fenetre.
static void resize(int w, int h) {
  _dim[0]  = w; _dim[1] = h;
  glViewport(0, 0, (int)_dim[0], (int)_dim[1]);
  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-1.0f, 1.0f, -h / (GLfloat)w, h / (GLfloat)w, 2.0f, 1000.0f);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
}

static void draw(void) {
  const GLfloat blanc[] = {1.0f, 1.0f, 1.0f, 1.0f};

  Mat ci;
  *_cap >> ci;

  // --------------------------------------------- recognizeFace
  Mat gsi;
  cvtColor( ci, gsi, CV_BGR2GRAY );
  //rectangle(ci, Point(10,19), Point(100,100), Scalar(0, 255, 0), 2, CV_AA);

  int NB_CHAPEAU = 10;
  vector<Rect> faces;
  face_cc->detectMultiScale(gsi, faces, 1.3, 5);
  vector<Rect>::iterator fc = faces.begin();
  int rw[NB_CHAPEAU];   // reverse taille X
 // int rh[NB_CHAPEAU];   // reverse taille Y
  int lx[NB_CHAPEAU];   // reverse Position Y
  int ly[NB_CHAPEAU];   // reverse Position Y
  int nb_person = 0;
  for (int i = 0; i < NB_CHAPEAU; i++) {
      rw[i] = -1;
   //   rh[i] = -1;
      lx[i] = -1;
      ly[i] = -1;
  }
  
  for (; fc != faces.end(); fc++) {
      if( rw[nb_person] < 0 ) {
          rw[nb_person] = (*fc).br().x - (*fc).tl().x;
         // rh[nb_person] = (*fc).br().y - (*fc).tl().y;
          lx[nb_person] = ((*fc).br()).x;
          ly[nb_person] = ((*fc).br()).y;
          nb_person++;
          printf("NB PERSON %d\n",nb_person );
      }

      rectangle(ci, (*fc).tl(), (*fc).br(), Scalar(0, 255, 0), 2, CV_AA);
      circle(ci, (*fc).tl()+Point(90, 90), 110, Scalar(0, 255, 0), 2, CV_AA);

      /* sub-image of GSI according to the rest *fc */
      Mat gsi_roi = gsi( *fc );
      vector<Rect> eyes;

      eye_cc->detectMultiScale(gsi_roi, eyes, 1.3, 10);
      for (vector<Rect>::iterator ec = eyes.begin(); ec != eyes.end(); ec++) {
          rectangle(ci, (*fc).tl() + (*ec).tl(), (*fc).tl() + (*ec).br(), Scalar(0, 0, 255), 2, CV_AA);
      }
  }
  // --------------------------------------------- end of recognizeFace

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ci.cols, ci.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, ci.data);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(_pId);
  glEnable(GL_DEPTH_TEST);
  glUniform1i(glGetUniformLocation(_pId, "myTexture"), 0);
  glUniform1i(glGetUniformLocation(_pId, "fond"), 1);
  glUniform1f(glGetUniformLocation(_pId, "width"), _dim[0]);
  glUniform1f(glGetUniformLocation(_pId, "height"), _dim[1]);

  // streaming au fond
  gl4duBindMatrix("modelViewMatrix");
  gl4duPushMatrix();                                              // sauver modelview
  gl4duLoadIdentityf();
  gl4duTranslatef(0, 0, 0.9999f);
  gl4duScalef(1, -1, 1);
  gl4duBindMatrix("projectionMatrix");
  gl4duPushMatrix();                                              // Sauver projection
  gl4duLoadIdentityf();
  gl4duSendMatrices();                                            // Envoyer les matrices
  gl4duPopMatrix();                                               // Restaurer projection
  gl4duBindMatrix("modelViewMatrix");                             // Restaurer modelview
  gl4duPopMatrix();                                               // Restaurer modelview
  glUniform4fv(glGetUniformLocation(_pId, "couleur"), 1, blanc);  // Envoyer une couleur
  gl4dgDraw(_quad); // Dessin du fond
  glUniform1i(glGetUniformLocation(_pId, "fond"), 0);


  for (int i = 0; i < nb_person; i++) {
    // Sphère
    // -----------------------------------------
    
    /*--------------rotation--------------------*/
    static GLfloat a0 = 0.0;
    static Uint32 t0=0;
    GLfloat dt = 0.0;
    GLfloat lumPos[4];
    Uint32 t;
    dt = ((t = SDL_GetTicks()) - t0) / 1000.0; //1000.0
    t0 = t;
    /*****************************/
    
    void *m;
    GLfloat fm = rw[i] / (2 * _dim[0]) ; //0.5;                     // Taille de la sphere , fm : formule magique
    GLfloat Pp[16];                                                 // P prime
    GLfloat xe = lx[i];                                             // Position en 2D
    GLfloat ye = ly[i];
    GLfloat xg = 2 * (xe / (_dim[0] - 1.0)) - 1;                    // Position en 3D
    GLfloat yg = 2 * ((_dim[1] - 1.0 - ye) / (_dim[1] - 1.0)) - 1;  // Position en 3D
    GLfloat zg = 0.0;                                               // Position en 3D
    GLfloat v[4] = { xg, yg, zg, 1.0 };                             // point une fois projeté
    GLfloat w[4] = { fm, fm, fm, 1.0 };                             //  fm : formule magique
    GLfloat rt[4];                                                  // reverse-translation  , Projection inverse
    GLfloat rs[4];                                                  // reverse-scale
    gl4duBindMatrix("projectionMatrix");                            // ???
    m = gl4duGetMatrixData();                                       // ???
    memcpy(Pp, m, sizeof Pp);                                       // ???
    MMAT4INVERSE(Pp);                                               // Calcul de la Matrice vecteur 4 inverse
   
    glUseProgram(_pId);
   
    MMAT4XVEC4( rt, Pp, v);                                         // Calcul de la Matrice vecteur 4 en X
    MVEC4WEIGHT(rt);                                                // Calcul de la Matrice vecteur 4 weight
    MMAT4XVEC4( rs, Pp, w);                                         // Calcul de la Matrice vecteur 4
    MVEC4WEIGHT(rs);                                                // Calcul de la Matrice vecteur 4 weight
  //********lumière*********** 
    //MMAT4XVEC4(lumPos,Pp, _lumPos0);
    //MVEC4WEIGHT(lumPos);
    //*******end lumière*****
    glUniform4fv(glGetUniformLocation(_pId, "lumPos"), 1, _lumPos0);
    gl4duBindMatrix("modelViewMatrix");                             // Sauver modelview
    gl4duPushMatrix();  
    gl4duLoadIdentityf();                                           // Met la matrice dans idenité
    gl4duTranslatef(rt[0], rt[1], rt[2]);                           // reverse Translation
    gl4duRotatef(a0, 0, 1, 0);
    gl4duScalef(rs[0], rs[1], MIN(rs[0], rs[1]));                   // Reverse scale
    //gl4duScalef(1, 0.1f, 0.1f);                   // Reverse scale
    
   // gl4duTranslatef(lumPos[0],lumPos[1],lumPos[3]);
    
    gl4duSendMatrices();                                            // Envoyer les matrices
    gl4duPopMatrix();                                               // Restaurer modelview
    gl4dgDraw(_sphere);                                             // Dessin de la sphere
    // ----------------------------------------- end Sphere 1
    a0 += 360.0 * dt / (1.0 /* * 60.0 )*/);
     
  }
}

static void keydown(int keycode) {
  GLint v[2];
  switch(keycode) {
  case 'w':
    glGetIntegerv(GL_POLYGON_MODE, v);
    if(v[0] == GL_FILL)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  case SDLK_ESCAPE:
  case 'q':
    exit(0);
  default:
    break;
  }
}


/*!\brief Cette fonction est appelee au moment de sortir du programme
 *  (atexit), elle libere la fenetre SDL \ref _win et le contexte OpenGL \ref _oglContext.
 */
static void quit(void) {
  if(_cap) {
    delete _cap;
    _cap = NULL;
  }
  if(_tId)
    glDeleteTextures(1, &_tId);
  gl4duClean(GL4DU_ALL);
}
