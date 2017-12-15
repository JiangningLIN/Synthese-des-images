/*!\file window.c
 * \brief g�om�trie/suivi de souris GL4Dummies
 * \author Far�s BELHADJ, amsi@ai.univ-paris8.fr
 * \date October 17 2017 */
#include <GL4D/gl4du.h>
#include <GL4D/gl4duw_SDL2.h>

/* Prototypes des fonctions statiques contenues dans ce fichier C */
static void init(void);
//static void resize(int w, int h);
static void draw(void);
static void quit(void);
/*!\brief dimensions de la fen�tre */
static int _windowWidth = 800, _windowHeight = 600;
/*!\brief identifiant du programme GLSL */
static GLuint _pId = 0;
/*!\brief identifiant de la sph�re */
static GLuint _sphere = 0;
/*!\brief translation de la sph�re */
static GLfloat _t[2] = {0, 0};

/*!\brief La fonction principale cr�� la fen�tre d'affichage,
 * initialise GL et les donn�es, affecte les fonctions d'�v�nements et
 * lance la boucle principale d'affichage.*/
int main(int argc, char ** argv) {
  if(!gl4duwCreateWindow(argc, argv, "GL4Dummies", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			 _windowWidth, _windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN)) {
    return 1;
  }

  init();
  atexit(quit);
  //gl4duwResizeFunc(resize);
  gl4duwDisplayFunc(draw);
  gl4duwMouseFunc(mouse);
  gl4duwMainLoop();
  return 0;
}

/*!\brief initialise les param�tres OpenGL et g�om�trie */
static void init(void) {
  glEnable(GL_DEPTH_TEST);
  //glClearColor(1.0f, 0.7f, 0.7f, 0.0f);
  _pId  = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
  gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  resize(_windowWidth, _windowHeight);
  _sphere = gl4dgGenSpheref(30, 30);
}

/*!\brief Cette fonction param�tre la vue (viewport) OpenGL en
 * fonction des dimensions de la fen�tre.*/
static void resize(int w, int h) {
  _windowWidth  = w; _windowHeight = h;
  glViewport(0, 0, _windowWidth, _windowHeight);
  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  gl4duOrthof(-5, 5, -5, 5, 0.0, 1000.0);
  gl4duBindMatrix("modelViewMatrix");
}

/*!\brief dessine dans le contexte OpenGL actif. */
static void draw(void) {
  GLfloat rouge[] = {1, 0, 0, 1};

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  glUseProgram(_pId);
  gl4duTranslatef(_t[0], _t[1], -10.0);
  gl4duSendMatrices();
  glUniform4fv(glGetUniformLocation(_pId, "couleur"), 1, rouge);
  gl4dgDraw(_sphere);
}

/*!\brief appel�e au moment de sortir du programme (atexit), lib�re les �l�ments utilis�s */
static void quit(void) {
  gl4duClean(GL4DU_ALL);
}
