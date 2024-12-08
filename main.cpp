#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/glext.h>
#endif
#include <stdlib.h>
#include <math.h>
#include "tiny_obj_loader.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

// Structure pour repr�senter un obstacle
struct Obstacle {
    float x, y, z;      // Position
    float width, height, depth; // Dimensions
    int type;           // Type d'obstacle (0: cube, 1: escalier, 2: plateforme, 3: sol)
};

// Variables globales
GLuint textureID;
float playerX = 48.0f, playerY = 0.5f, playerZ = -48.0f;
float playerAngle = 0.0f;
float ballRotation = 0.0f;
const float ballRadius = 0.5f;

// Nouveaux param�tres pour la gestion des collisions et de la gravit�
bool isGrounded = false;
float verticalVelocity = 0.0f;
const float GRAVITY = -100.8f;

// Mouvements
bool moveForward = false, moveBackward = false, moveLeft = false, moveRight = false;
bool isJumping = false;
float jumpHeight = 3.5f;
float jumpProgress = 0.0f;

// Vecteur d'obstacles
std::vector<Obstacle> obstacles;

// Fonction pour initialiser les obstacles
void initObstacles() {
    // Sol
    Obstacle ground = {0.0f, 0.0f, 0.0f, 100.0f, 0.1f, 100.0f, 3};
    obstacles.push_back(ground);



    // Plateformes flottantes
    Obstacle platform1 = {47.0f, 2.5f, -43.0f, 3.0f, 0.5f, 3.0f, 2};
    Obstacle platform2 = {47.0f, 5.0f, -37.0f, 3.0f, 0.5f, 3.0f,2};
    Obstacle platform3 = {43.0f, 7.0f, -30.0f, 3.0f, 0.5f, 3.0f, 2};

    obstacles.push_back(platform1);
    obstacles.push_back(platform2);
    obstacles.push_back(platform3);
}

bool checkCollision(float nextX, float nextY, float nextZ) {
    const float playerSize = ballRadius;

    // V�rifier les limites de la sc�ne
    if (nextX - playerSize < -50.0f || nextX + playerSize > 50.0f ||
        nextZ - playerSize < -50.0f || nextZ + playerSize > 50.0f) {
        return true;
    }

    for (const auto& obstacle : obstacles) {
        // Calculer les limites de l'obstacle
        float obstacleMinX = obstacle.x - obstacle.width/2;
        float obstacleMaxX = obstacle.x + obstacle.width/2;
        float obstacleMinY = obstacle.y;
        float obstacleMaxY = obstacle.y + obstacle.height;
        float obstacleMinZ = obstacle.z - obstacle.depth/2;
        float obstacleMaxZ = obstacle.z + obstacle.depth/2;

        // Calcul des limites de la balle
        float ballMinX = nextX - playerSize;
        float ballMaxX = nextX + playerSize;
        float ballMinY = nextY - playerSize;
        float ballMaxY = nextY + playerSize;
        float ballMinZ = nextZ - playerSize;
        float ballMaxZ = nextZ + playerSize;

        // V�rifier la collision sur tous les axes
        bool collideX = (ballMaxX > obstacleMinX) && (ballMinX < obstacleMaxX);
        bool collideY = (ballMaxY > obstacleMinY) && (ballMinY < obstacleMaxY);
        bool collideZ = (ballMaxZ > obstacleMinZ) && (ballMinZ < obstacleMaxZ);

        // Si collision sur tous les axes
        if (collideX && collideY && collideZ) {
            // Gestion diff�rente selon le type d'obstacle
            switch(obstacle.type) {
                case 0: // Cube - collision totale
                    if (nextY - playerSize <= obstacleMaxY) {
                        return true;
                    }
                    break;

                case 1: // Escalier - collision complexe
                    // Autoriser le passage si au-dessus de l'escalier
                    if (nextY > obstacleMaxY) {
                        return false;
                    }
                    return true;

                case 2: // Plateforme
                    if (nextY - playerSize <= obstacleMaxY) {
                        return true;
                    }
                    break;

                case 3: // Sol - collision horizontale uniquement
                    if (nextY - playerSize <= obstacleMaxY) {
                        return true;
                    }
                    break;
            }
        }
    }

    // Pas de collision
    return false;
}


// Charger la texture
bool loadTexture(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier " << filename << std::endl;
        return false;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);
    if (header[0] != 'B' || header[1] != 'M') {
        std::cerr << "Erreur : Ce fichier n'est pas un BMP valide." << std::endl;
        fclose(file);
        return false;
    }

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int imageSize = 3 * width * height;

    unsigned char* data = new unsigned char[imageSize];
    fread(data, sizeof(unsigned char), imageSize, file);
    fclose(file);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] data;
    return true;
}

// Fonction pour dessiner le personnage
void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    glRotatef(ballRotation/2, 1.0f, 0.0f, 0.0f);
    glRotatef(playerAngle, 0.0f, 1.0f, 0.0f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glColor3f(1.0f, 0.5f, 0.0f);
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere(quad, 0.5f, 32, 32);
    gluDeleteQuadric(quad);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

// Fonction pour dessiner les obstacles
void drawObstacles() {
    for (const auto& obstacle : obstacles) {
        glPushMatrix();

        // Couleur diff�rente selon le type d'obstacle
        switch(obstacle.type) {
            case 0: glColor3f(1.0f, 0.0f, 0.0f); break;   // Cubes en rouge
            case 1: glColor3f(0.0f, 1.0f, 0.0f); break;   // Escaliers en vert
            case 2: glColor3f(1.0f, 0.9f, 1.0f); break;   // Plateformes en blanc
            case 3: glColor3f(0.0f, 0.0f, 0.0f); break;   // Sol en vert clair
        }

        glTranslatef(obstacle.x, obstacle.y, obstacle.z);

        glScalef(obstacle.width, obstacle.height, obstacle.depth);
        glutSolidCube(1.0f);

        glPopMatrix();
    }
}

// Fonction d'affichage
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Position de la cam�ra � la troisi�me personne
    float camX = playerX - 5.0f * sin(playerAngle * M_PI / 180.0f);
    float camY = playerY + 3.0f;
    float camZ = playerZ - 5.0f * cos(playerAngle * M_PI / 180.0f);
    gluLookAt(camX, camY, camZ, // Position de la cam�ra
              playerX, playerY, playerZ, // Point regard� (le joueur)
              0.0f, 1.0f, 0.0f); // Vecteur "haut"

    // Dessiner le personnage et les obstacles
    drawPlayer();
    drawObstacles();

    glutSwapBuffers();
}

// Fonction de redimensionnement de la fen�tre
void reshape(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, aspect, 1.0f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
}

// Fonction de gestion des touches clavier
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // Touche ESC
            exit(0);
            break;
        case 'w':
        case 'W':
            moveForward = true;
            break;
        case 's':
        case 'S':
            moveBackward = true;
            break;
        case 'a':
        case 'A':
            moveLeft = true;
            break;
        case 'd':
        case 'D':
            moveRight = true;
            break;
        case ' ':
            if (isGrounded) {
                isJumping = true;
            }
            break;
    }
}

// Fonction de gestion des touches clavier rel�ch�es
void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
        case 'W':
            moveForward = false;
            break;
        case 's':
        case 'S':
            moveBackward = false;
            break;
        case 'a':
        case 'A':
            moveLeft = false;
            break;
        case 'd':
        case 'D':
            moveRight = false;
            break;
    }
}
void update(int value) {
    float speed = 0.2f;
    float distance = 0.0f;

    // Positions potentielles du joueur
    float nextX = playerX;
    float nextY = playerY;
    float nextZ = playerZ;

    // Gestion des mouvements horizontaux
    if (moveForward) {
        nextX += speed * sin(playerAngle * M_PI / 180.0f);
        nextZ += speed * cos(playerAngle * M_PI / 180.0f);
        distance = speed;
    }
    if (moveBackward) {
        nextX -= speed * sin(playerAngle * M_PI / 180.0f);
        nextZ -= speed * cos(playerAngle * M_PI / 180.0f);
        distance = -speed;
    }

    // Rotation de la balle
    ballRotation += (distance / (2.0f * M_PI * ballRadius)) * 360.0f;
    if (ballRotation > 360.0f) ballRotation -= 360.0f;
    if (ballRotation < 0.0f) ballRotation += 360.0f;

    // Gestion de la gravit�
    verticalVelocity += GRAVITY * 0.016f;
    nextY += verticalVelocity * 0.016f;

    // V�rification des collisions
    bool horizontalCollision = checkCollision(nextX, playerY, nextZ);
    bool verticalCollision = checkCollision(playerX, nextY, playerZ);

    // Mise � jour des positions
    if (!horizontalCollision) {
        playerX = nextX;
        playerZ = nextZ;
    }

    if (!verticalCollision) {
        playerY = nextY;
    } else {
        // En cas de collision verticale, r�initialiser la v�locit� verticale
        verticalVelocity = 0.0f;

        // Trouver la position correcte
        for (const auto& obstacle : obstacles) {
            float obstacleMinY = obstacle.y;
            float obstacleMaxY = obstacle.y + obstacle.height;

            if (nextY - ballRadius <= obstacleMaxY) {
                playerY = obstacleMaxY + ballRadius;
                break;
            }
        }
    }

    // Gestion du saut
    if (isJumping && isGrounded) {
        verticalVelocity = sqrt(2.0f * std::abs(GRAVITY) * jumpHeight);
        isJumping = false;
        isGrounded = false;
    }

    // V�rification si le joueur est au sol
    isGrounded = checkCollision(playerX, playerY - ballRadius, playerZ);

    // Rotation du joueur
    if (moveLeft) {
        playerAngle += 2.0f;
    }
    if (moveRight) {
        playerAngle -= 2.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // Environ 60 FPS
}


// Fonction d'initialisation OpenGL
void init() {
    initObstacles();
    glEnable(GL_DEPTH_TEST); // Activer le test de profondeur
    glEnable(GL_LIGHTING);   // Activer l'�clairage
    glEnable(GL_LIGHT0);     // Activer la premi�re source de lumi�re
    glEnable(GL_COLOR_MATERIAL); // Permettre aux couleurs des objets de r�agir � la lumi�re
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Couleur d'arri�re-plan (ciel)
    glClearColor(0.8f, 0.9f, 1.0f, 1.0f);

    // Configurer la source de lumi�re
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat lightPos[] = {0.0f, 10.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    if (!loadTexture("ball.bmp")) {
        std::cout << "Erreur : Impossible de charger le mod�le ball.bmp" << std::endl;
        exit(EXIT_FAILURE);
    }
}


// Fonction principale
int main(int argc, char** argv) {
    if (!std::ifstream("ball.bmp")) {
    std::cout << "Erreur : Fichier ball.bmp introuvable" << std::endl;
    exit(EXIT_FAILURE);
}
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Roll-in");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
