#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/glext.h>
#endif
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <SFML/Audio.hpp>
#include <SFML/Audio.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdlib>

//variables globales:
bool gameStarted = false;
time_t gameStartTime;
time_t gameEndTime;
sf::Music backgroundMusic;
sf::SoundBuffer jumpSoundBuffer;
sf::Sound jumpSound;
sf::Music victoryMusic;
bool isVictoryReached = false;
GLuint textureID;
float playerX = 48.0f, playerY = 3.5f, playerZ = -48.0f;
float playerAngle = 0.0f;
float ballRotation = 0.0f;
const float ballRadius = 0.5f;
bool isGrounded = false;
float verticalVelocity = 0.0f;
const float GRAVITY = -100.8f;
bool moveForward = false, moveBackward = false, moveLeft = false, moveRight = false;
bool isJumping = false;
float jumpHeight = 3.5f;
float jumpProgress = 0.0f, M_PI=3.141592653589793 ;

//structures réutilisables:
struct Obstacle {
    float x, y, z;
    float width, height, depth;
    int type;
};
struct Tree {
    float x, z;
};
std::vector<Obstacle> obstacles;
std::vector<Tree> trees;

//fonctions types "initialisations:"
void initAudio() {
    if (!backgroundMusic.openFromFile("music.wav")) {
        std::cerr << "Erreur : Impossible de charger music.mp3" << std::endl;
        //exit(EXIT_FAILURE);
    }
    backgroundMusic.setLoop(true);
    backgroundMusic.play();

    if (!jumpSoundBuffer.loadFromFile("jump.wav")) {
        std::cerr << "Erreur : Impossible de charger jump.mp3" << std::endl;
        //exit(EXIT_FAILURE);
    }
    jumpSound.setBuffer(jumpSoundBuffer);

    if (!victoryMusic.openFromFile("victory.wav")) {
        std::cerr << "Erreur : Impossible de charger victory.mp3" << std::endl;
        exit(EXIT_FAILURE);
    }
}
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
void initObstacles() {
    // sol
    Obstacle ground = {0.0f, 0.0f, 0.0f, 100.0f, 0.1f, 100.0f, 3};
    obstacles.push_back(ground);

    // plateformes flottantes
    Obstacle platform1 = {47.0f, 2.5f, -43.0f, 4.0f, 0.5f, 4.0f, 2};
    Obstacle platform2 = {47.0f, 5.0f, -37.0f, 4.0f, 0.5f, 4.0f, 2};
    Obstacle platform3 = {43.0f, 7.0f, -30.0f, 4.0f, 0.5f, 4.0f, 2};
    Obstacle platform4 = {39.0f, 9.0f, -25.0f,4.0f, 0.5f, 4.0f, 2};
    Obstacle platform5 = {35.0f, 12.0f, -20.0f, 4.0f, 0.5f, 4.0f, 2};
    Obstacle platform6 = {39.0f, 15.0f, -15.0f, 4.0f, 0.5f, 4.0f, 2};
    Obstacle platform7 = {43.0f, 17.0f, -10.0f, 4.0f, 0.5f, 4.0f, 2};
    Obstacle platform8 = {47.0f, 20.0f, -7.0f, 4.0f, 0.5f, 4.0f, 2};
    Obstacle platform9 = {43.0f, 23.0f, -5.0f, 3.5f, 0.5f, 3.5f, 2};
    Obstacle platform10 = {40.0f, 26.0f, 0.0f, 2.0f, 0.5f, 2.0f, 2};
    Obstacle platform11 = {37.0f, 28.0f, 5.0f, 3.5f, 0.5f, 3.5f, 2};
    Obstacle platform12 = {32.0f, 28.0f, 12.0f, 3.0f, 0.5f, 3.0f, 2};
    Obstacle platform13 = {30.0f, 15.0f, 20.0f, 3.0f, 0.5f, 3.0f, 2};
    Obstacle platform14 = {35.0f, 15.0f, 25.0f, 3.0f, 0.5f, 3.0f, 2};
    Obstacle platform15 = {40.0f, 15.0f, 30.0f, 2.0f, 0.5f, 2.0f, 2};
    Obstacle platform16 = {45.0f, 15.0f, 35.0f, 1.5f, 0.5f, 1.5f, 2};
    Obstacle platform17 = {45.0f, 12.0f, 42.0f, 3.0f, 0.5f, 3.0f, 2};
    Obstacle platform19 = {47.5f, 12.0f, 48.0f, 3.0f, 0.5f, 1.5f, 2};
    Obstacle platform20 = {44.0f, 15.0f, 47.5f, 2.0f, 0.5f, 2.0f, 2};
    Obstacle platform22 = {40.0f, 15.0f, 48.0f, 1.8f, 0.5f, 1.8f, 2};
    Obstacle platform23 = {35.0f, 15.0f, 45.5f, 1.7f, 0.5f, 1.7f, 2};
    Obstacle platform24 = {30.0f, 15.0f, 47.5f, 1.6f, 0.5f, 1.6f, 2};
    Obstacle platform25 = {25.0f, 15.0f, 45.5f, 1.5f, 0.5f, 1.5f, 2};
    Obstacle platform26 = {20.0f, 15.0f, 45.0f, 1.4f, 0.5f, 1.4f, 2};
    Obstacle platform27 = {15.0f, 15.0f, 48.0f, 1.3f, 0.5f, 1.3f, 2};
    Obstacle platform28 = {10.0f, 15.0f, 45.0f, 1.2f, 0.5f, 1.2f, 2};
    Obstacle platform29 = {5.0f, 15.0f, 48.0f, 1.1f, 0.5f, 1.1f, 2};
    Obstacle platform30 = {0.0f, 15.0f, 45.0f, 1.0f, 0.5f, 1.0f, 2};
    Obstacle platform31 = {-7.0f, 15.0f, 42.0f, 3.0f, 0.5f, 3.0f, 2};
    Obstacle platform32 = {-10.0f, 10.0f, 30.1f, 6.0f, 0.5f, 4.0f, 4};

    obstacles.push_back(platform1);
    obstacles.push_back(platform2);
    obstacles.push_back(platform3);
    obstacles.push_back(platform4);
    obstacles.push_back(platform5);
    obstacles.push_back(platform6);
    obstacles.push_back(platform7);
    obstacles.push_back(platform8);
    obstacles.push_back(platform9);
    obstacles.push_back(platform10);
    obstacles.push_back(platform11);
    obstacles.push_back(platform12);
    obstacles.push_back(platform13);
    obstacles.push_back(platform14);
    obstacles.push_back(platform15);
    obstacles.push_back(platform16);
    obstacles.push_back(platform17);
    obstacles.push_back(platform19);
    obstacles.push_back(platform20);
    obstacles.push_back(platform22);
    obstacles.push_back(platform23);
    obstacles.push_back(platform24);
    obstacles.push_back(platform25);
    obstacles.push_back(platform26);
    obstacles.push_back(platform27);
    obstacles.push_back(platform28);
    obstacles.push_back(platform29);
    obstacles.push_back(platform30);
    obstacles.push_back(platform31);
    obstacles.push_back(platform32);
}
void initTrees() {
    srand(static_cast<unsigned int>(time(nullptr)));

    for (int i = 0; i < 10; ++i) {
        Tree tree;
        tree.x = static_cast<float>(rand() % 101 - 50);
        tree.z = static_cast<float>(rand() % 101 - 50);
        trees.push_back(tree);
    }
}
void init() {
    initObstacles();
    initAudio();
    initTrees();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glClearColor(0.8f, 0.9f, 1.0f, 1.0f);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat lightPos[] = {0.0f, 10.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    if (!loadTexture("ball.bmp")) {
        std::cout << "Erreur : Impossible de charger le modèle ball.bmp" << std::endl;
        exit(EXIT_FAILURE);
    }
}


//fonctions type "drawing":
void drawTree(const Tree& tree) {
    glPushMatrix();
    glTranslatef(tree.x, 0.0f, tree.z);

    glColor3f(0.55f, 0.27f, 0.07f);
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);
    glRotatef(90.0f,1.0f,0.0f,0.0f);
    gluCylinder(quad, 0.3f, 0.3f, 4.0f, 20, 20);
    glPopMatrix();

    glColor3f(0.0f, 0.8f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);
    glutSolidSphere(1.0f, 20, 20);
    glTranslatef(-0.7f, 0.5f, -0.7f);
    glutSolidSphere(0.8f, 20, 20);
    glTranslatef(1.4f, 0.0f, 0.0f);
    glutSolidSphere(0.8f, 20, 20);
    glTranslatef(-0.7f, 0.0f, 1.4f);
    glutSolidSphere(0.8f, 20, 20);
    glPopMatrix();

    gluDeleteQuadric(quad);
    glPopMatrix();
}
void drawTrees() {
    for (const auto& tree : trees) {
        drawTree(tree);
    }
}
void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    glRotatef(ballRotation/2, 1.0f, 0.0f, 0.0f);
    glRotatef(playerAngle, 0.0f, 1.0f, 0.0f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glColor3f(1.0f, 1.0f, 1.0f);
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere(quad, 0.5f, 32, 32);
    gluDeleteQuadric(quad);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}
void drawObstacles() {
    for (const auto& obstacle : obstacles) {
        glPushMatrix();
        switch(obstacle.type) {
            case 0: glColor3f(1.0f, 0.0f, 0.0f); break;
            case 1: glColor3f(0.0f, 1.0f, 0.0f); break;
            case 2: glColor3f(1.0f, 0.9f, 1.0f); break;
            case 3: glColor3f(0.0f, 0.1f, 0.0f); break;
            case 4: glColor3f(1.0f, 0.0f, 0.0f); break;
        }

        glTranslatef(obstacle.x, obstacle.y, obstacle.z);

        glScalef(obstacle.width, obstacle.height, obstacle.depth);
        glutSolidCube(1.0f);

        glPopMatrix();
    }
}
void renderText(float x, float y, const std::string& text) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    gluOrtho2D(0, w, h, 0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);

    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

//fonctions + techniques relatives au déplacements/collisions/sauts mise a jour
bool checkCollision(float nextX, float nextY, float nextZ) {
    const float playerSize = ballRadius;
    if(!isVictoryReached){
        if (nextX - playerSize < -50.0f || nextX + playerSize > 50.0f ||
            nextZ - playerSize < -50.0f || nextZ + playerSize > 50.0f) {
            return true;
        }

        for (const auto& obstacle : obstacles) {
            float obstacleMinX = obstacle.x - obstacle.width/2;
            float obstacleMaxX = obstacle.x + obstacle.width/2;
            float obstacleMinY = obstacle.y;
            float obstacleMaxY = obstacle.y + obstacle.height;
            float obstacleMinZ = obstacle.z - obstacle.depth/2;
            float obstacleMaxZ = obstacle.z + obstacle.depth/2;

            float ballMinX = nextX - playerSize;
            float ballMaxX = nextX + playerSize;
            float ballMinY = nextY - playerSize;
            float ballMaxY = nextY + playerSize;
            float ballMinZ = nextZ - playerSize;
            float ballMaxZ = nextZ + playerSize;

            bool collideX = (ballMaxX > obstacleMinX) && (ballMinX < obstacleMaxX);
            bool collideY = (ballMaxY > obstacleMinY) && (ballMinY < obstacleMaxY);
            bool collideZ = (ballMaxZ > obstacleMinZ) && (ballMinZ < obstacleMaxZ);

            if (collideX && collideY && collideZ) {
                switch(obstacle.type) {
                    case 0:
                        if (nextY - playerSize <= obstacleMaxY) {
                            return true;
                        }
                        break;

                    case 1:
                        if (nextY > obstacleMaxY) {
                            return false;
                        }
                        return true;

                    case 2:
                        if (nextY - playerSize <= obstacleMaxY) {
                            return true;
                        }
                        break;

                    case 3:
                        if (nextY - playerSize <= obstacleMaxY) {
                            return true;
                        }
                        break;
                    case 4:
                        if (nextY - playerSize <= obstacleMaxY) {
                            return true;
                        }
                        break;
                }
            }
        }
}
        // pas de collision
    return false;
}

void keyboard(unsigned char key, int x, int y) {

    switch (key) {
        case 27:
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
                jumpSound.play();
            }
            break;
    }
}
void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
            exit(0);
            break;
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

    float nextX = playerX;
    float nextY = playerY;
    float nextZ = playerZ;

    //  verification de la cond de victoire
    Obstacle finalPlatform = {-10.0f, 10.0f, 30.1f, 6.0f, 0.5f, 4.0f, 4};
    if (playerX >= finalPlatform.x - finalPlatform.width/2-0.45 &&
        playerX <= finalPlatform.x + finalPlatform.width/2+0.45 &&
        playerZ >= finalPlatform.z - finalPlatform.depth/2-0.45 &&
        playerZ <= finalPlatform.z + finalPlatform.depth/2+0.45 &&
        playerY >= finalPlatform.y &&
        playerY <= finalPlatform.y + finalPlatform.height+2.0) {
        if (!isVictoryReached) {
            gameEndTime = time(nullptr);
            backgroundMusic.stop();
            victoryMusic.play();
            isVictoryReached = true;
            moveForward = false;
            moveBackward = false;
            moveLeft = false;
            moveRight = false;
            isJumping = false;
        }
    }
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

    ballRotation += (distance / (2.0f * M_PI * ballRadius)) * 360.0f;
    if (ballRotation > 360.0f) ballRotation -= 360.0f;
    if (ballRotation < 0.0f) ballRotation += 360.0f;

    verticalVelocity += GRAVITY * 0.016f;
    nextY += verticalVelocity * 0.016f;

    bool horizontalCollision = checkCollision(nextX, playerY, nextZ);
    bool verticalCollision = checkCollision(playerX, nextY, playerZ);

    if (!horizontalCollision) {
        playerX = nextX;
        playerZ = nextZ;
    }

    if (!verticalCollision) {
        playerY = nextY;
    } else {
        verticalVelocity = 0.0f;

        for (const auto& obstacle : obstacles) {
            float obstacleMinY = obstacle.y;
            float obstacleMaxY = obstacle.y + obstacle.height;

            if (nextY - ballRadius <= obstacleMaxY) {
                playerY = obstacleMaxY + ballRadius;
                break;
            }
        }
    }

    if (isJumping && isGrounded) {
        verticalVelocity = sqrt(2.0f * std::abs(GRAVITY) * jumpHeight);
        isJumping = false;
        isGrounded = false;
    }

    isGrounded = checkCollision(playerX, playerY - ballRadius, playerZ);

    if (moveLeft) {
        playerAngle += 2.0f;
    }
    if (moveRight) {
        playerAngle -= 2.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}


//relatifs a l'affichage du jeu:
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    float camX = playerX - 5.0f * sin(playerAngle * M_PI / 180.0f);
    float camY = playerY + 3.0f;
    float camZ = playerZ - 5.0f * cos(playerAngle * M_PI / 180.0f);
    gluLookAt(camX, camY, camZ,
              playerX, playerY, playerZ,
              0.0f, 1.0f, 0.0f);

    drawPlayer();
    drawObstacles();
    drawTrees();

    if (isVictoryReached) {
        double totalTime = difftime(gameEndTime, gameStartTime);
        int minutes = static_cast<int>(totalTime) / 60;
        int seconds = static_cast<int>(totalTime) % 60;

        std::ostringstream oss;
        oss << "Bravo! Temps: " << minutes << " min " << seconds << " sec";

        renderText(glutGet(GLUT_WINDOW_WIDTH) / 2 - 150,
                   glutGet(GLUT_WINDOW_HEIGHT) / 2,
                   oss.str());
    }

    glutSwapBuffers();
}
void reshape(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, aspect, 1.0f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
}

//main
int main(int argc, char** argv) {
    gameStartTime = time(nullptr);
    gameStarted = true;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1400, 800);
    glutCreateWindow("Roll-In!");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
