#include <cmath>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <digitalv.h>
#pragma comment(lib, "winmm.lib")
#endif
#include <GL/glut.h>
#include <GL/glu.h>

constexpr float PI = 3.14159265358979323846f;

struct Vec3 { float x,y,z; Vec3(float X=0,float Y=0,float Z=0):x(X),y(Y),z(Z){} };

struct Fish {
    float slotAngle;
    float radius;
    float mouthPhase;
    float mouthSpeed;
    bool  caught;
    float color[3];
    GLuint texId;
    Vec3  worldPos;
};

int   winW=1280, winH=800;
float camAngle=35.0f;
float camDist =22.0f;
float camHeight=13.0f;

float boardRot=0.0f;

const int NUM_FISH=20;
std::vector<Fish> fishes;

int   caughtCount=0;
int   score=0;
bool  gameOver=false;
int   startMs=0;
int   finishMs=0;

float rodX=0.0f, rodZ=6.0f, hookY=5.0f;

GLuint texBoardBase=0, texBoardTop=0;
GLuint texFish[6] = {0};

std::string g_musicPath = "audio/bg.wav";
bool g_musicMuted=false;
#ifdef _WIN32
bool g_musicUsingMCI=false;
#endif

void readMusicPath() {
    std::ifstream in("audio/music_path.txt");
    if (in) {
        std::getline(in, g_musicPath);
        if (g_musicPath.size()==0) g_musicPath = "audio/bg.wav";
    }
}

#ifdef _WIN32
void stopMusic() {
    if (g_musicUsingMCI) {
        mciSendString(TEXT("stop mymp3"), NULL, 0, NULL);
        mciSendString(TEXT("close mymp3"), NULL, 0, NULL);
        g_musicUsingMCI=false;
    } else {
        PlaySound(NULL,0,0);
    }
}
void playMusic(const std::string& path) {
    stopMusic();
    std::string ext;
    if (path.size() >= 4) ext = path.substr(path.size()-4);
    for(char &c: ext) c = tolower(c);
    if (ext == ".mp3") {
        // Use MCI for mp3
        std::string cmd1 = "open \"" + path + "\" type mpegvideo alias mymp3";
        mciSendString(cmd1.c_str(), NULL, 0, NULL);
        mciSendString("play mymp3 repeat", NULL, 0, NULL);
        g_musicUsingMCI=true;
    } else {
        // Default WAV via PlaySound
        PlaySound(path.c_str(), NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
        g_musicUsingMCI=false;
    }
}
#endif

#pragma pack(push,1)
struct BMPFileHeader {
    unsigned short bfType;
    unsigned int   bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffBits;
};
struct BMPInfoHeader {
    unsigned int   biSize;
    int            biWidth;
    int            biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    int            biXPelsPerMeter;
    int            biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
};
#pragma pack(pop)

GLuint loadBMP(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if(!f) { return 0; }
    BMPFileHeader fh; BMPInfoHeader ih;
    fread(&fh, sizeof(fh), 1, f);
    if (fh.bfType != 0x4D42) { fclose(f); return 0; }
    fread(&ih, sizeof(ih), 1, f);
    if (ih.biBitCount != 24 || ih.biCompression != 0) { fclose(f); return 0; }
    fseek(f, fh.bfOffBits, SEEK_SET);
    int w = ih.biWidth, h = ih.biHeight;
    int row_padded = (w*3 + 3) & (~3);
    std::vector<unsigned char> data(w*h*3);
    std::vector<unsigned char> row(row_padded);
    for (int y=0; y<h; ++y) {
        fread(row.data(), 1, row_padded, f);
        for (int x=0; x<w; ++x) {
            unsigned char b = row[x*3+0];
            unsigned char g = row[x*3+1];
            unsigned char r = row[x*3+2];
            int dst = (h-1-y)*w*3 + x*3;
            data[dst+0]=r; data[dst+1]=g; data[dst+2]=b;
        }
    }
    fclose(f);

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, w, h, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    return texId;
}

void setupLighting(){
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat pos0[]={10.0f,16.0f,12.0f,1.0f};
    GLfloat diff0[]={0.95f,0.95f,0.95f,1.0f};
    GLfloat amb0[] ={0.20f,0.22f,0.25f,1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diff0);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  amb0);

    GLfloat pos1[]={-12.0f, 10.0f, -8.0f,1.0f};
    GLfloat diff1[]={0.25f,0.35f,0.55f,1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  diff1);

    GLfloat spec[]={0.9f,0.9f,0.9f,1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
}

void drawBackground(){
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();

    glBegin(GL_QUADS);
      glColor3f(0.10f, 0.46f, 0.99f);
      glVertex2f(0, winH);
      glVertex2f(winW, winH);
      glColor3f(0.05f, 0.55f, 0.85f);
      glVertex2f(winW, 0);
      glVertex2f(0, 0);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
      glColor4f(0.30f, 0.55f, 0.95f, 0.35f);
      glVertex2f(winW*0.55f, winH*0.55f);
      glColor4f(0.30f, 0.55f, 0.95f, 0.0f);
      for(int i=0;i<=64;i++){
          float a = i*2*PI/64.0f;
          float r = winH*0.5f;
          glVertex2f(winW*0.55f + cosf(a)*r, winH*0.55f + sinf(a)*r);
      }
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void drawCylinder(float radius,float height,int slices=48){
    GLUquadric*q=gluNewQuadric();
    gluQuadricTexture(q, GL_TRUE);
    gluCylinder(q,radius,radius,height,slices,1);
    gluDeleteQuadric(q);
}
void drawDiskTextured(float radius, float thickness, GLuint tex){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPushMatrix();
        glTranslatef(0.0f, thickness, 0.0f);
        glRotatef(-90,1,0,0);
        GLUquadric*q = gluNewQuadric();
        gluQuadricTexture(q, GL_TRUE);
        gluDisk(q, 0.0, radius, 64, 1);
        gluDeleteQuadric(q);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}
void drawDiscH(float radius,float thickness=0.3f, GLuint tex=0, bool textured=false){
    glPushMatrix();
    glRotatef(-90,1,0,0);
    if (textured && tex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
    }
    GLUquadric*q=gluNewQuadric();
    gluQuadricTexture(q, textured?GL_TRUE:GL_FALSE);
    gluCylinder(q, radius, radius, thickness, 64, 1);
    gluDisk(q, 0.0, radius, 64, 1);
    glTranslatef(0,0,thickness);
    gluDisk(q, 0.0, radius, 64, 1);
    gluDeleteQuadric(q);
    if (textured && tex) glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawBoard(){
    glPushMatrix();
        glTranslatef(0.0f, -0.5f, 0.0f);
        drawDiscH(8.0f, 1.1f, texBoardBase, true);
    glPopMatrix();
    glPushMatrix();
        glColor3f(0.164f,0.494f,0.098f);
        drawDiscH(7.8f, 0.55f, 0, false);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0.0f, 0.1f, 0.0f);
        drawDiskTextured(7.0f, 0.0f, texBoardTop);
    glPopMatrix();

    for(int i=0;i<NUM_FISH;i++){
        float a = (360.0f/NUM_FISH)*i * PI/180.0f;
        float r = 5.6f;
        float x = cosf(a)*r;
        float z = sinf(a)*r;
        glPushMatrix();
            glTranslatef(x, 0.35f, z);
            glRotatef(90,1,0,0);
            glColor3f(0.98f,0.98f,0.98f);
            glutSolidTorus(0.18, 0.62, 24, 36);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(x, 0.2f, z);
            glColor3f(0.07f,0.07f,0.12f);
            glRotatef(-90,1,0,0);
            GLUquadric*q=gluNewQuadric();
            gluDisk(q, 0.0, 0.55f, 24, 1);
            gluDeleteQuadric(q);
        glPopMatrix();
    }
}

void drawSphereTextured(float radius, GLuint tex){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLUquadric*q=gluNewQuadric();
    gluQuadricTexture(q, GL_TRUE);
    gluSphere(q, radius, 28, 28);
    gluDeleteQuadric(q);
    glDisable(GL_TEXTURE_2D);
}

void drawFishModel(float mouthOpen, GLuint tex){
    glPushMatrix();
        glScalef(1.0f,0.7f,0.5f);
        drawSphereTextured(0.48f, tex);
    glPopMatrix();
    glPushMatrix();
      glTranslatef(0.42f,0.0f,0.0f);
      drawSphereTextured(0.32f, tex);
    glPopMatrix();
    glPushMatrix();
      glTranslatef(-0.6f,0.0f,0.0f);
      glRotatef(90,0,1,0);
      GLUquadric*q=gluNewQuadric();
      gluCylinder(q, 0.22f, 0.0f, 0.3f, 16, 1);
      gluDeleteQuadric(q);
    glPopMatrix();
    glPushMatrix();
      glTranslatef(0.65f,0.0f,0.0f);
      glRotatef(-35.0f + mouthOpen*55.0f, 0,0,1);
      glScalef(0.28f,0.11f,0.22f);
      glutSolidCube(1.0);
    glPopMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(0,0,0);
    glPushMatrix();
      glTranslatef(0.5f,0.16f,0.16f);
      glutSolidSphere(0.05,12,12);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawFishesAndUpdate(){
    for (size_t i = 0; i < fishes.size(); ++i) {
        Fish& f = fishes[i];
        if (f.caught) continue;

        // Position: ring fish use radius/angle; center fish uses radius=0 → x=z=0
        float a = (f.slotAngle + boardRot) * PI / 180.0f;
        float r = f.radius;
        float x = cosf(a) * r;
        float z = sinf(a) * r;

        float pop = 0.12f + 0.25f * f.mouthPhase;
        float y = 0.3f + pop;

        f.worldPos = Vec3(x,y,z);

        glPushMatrix();
          glTranslatef(x,y,z);
          glRotatef(-90.0f, 0,0,1);
          glColor3f(f.color[0], f.color[1], f.color[2]);
          drawFishModel(f.mouthPhase, f.texId);
        glPopMatrix();
    }
}


void drawRod(){
    glPushMatrix();
      glColor3f(0.1f,0.5f,0.1f);
      glTranslatef(rodX, 9.2f, rodZ);
      glRotatef(-90,1,0,0);
      GLUquadric*q=gluNewQuadric();
      gluCylinder(q,0.4,0.4,4.0,18,1);
      gluDeleteQuadric(q);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glColor3f(0.2f,0.2f,0.2f);
    glBegin(GL_LINES);
      glVertex3f(rodX, 5.2f, rodZ);
      glVertex3f(rodX, hookY, rodZ);
    glEnd();
    glPushMatrix();
      glTranslatef(rodX, hookY, rodZ);
      glRotatef(90,1,0,0);
      glutSolidTorus(0.05,0.16,16,20);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawHUD(){
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glColor3f(1,1,1);
    int nowMs = glutGet(GLUT_ELAPSED_TIME);
    float elapsed = (nowMs - startMs) / 1000.0f;
    char buf[256];

    if (!gameOver) {
        std::snprintf(buf, sizeof(buf),
    "Caught: %d/%d   Time: %.1fs   (Space=catch, Arrows=move, W/S=up/down, A/D=cam, M=mute, L=reload music, R=reset)",
    caughtCount, (int)fishes.size(), elapsed);
    } else {
        float timeSec = (finishMs - startMs)/1000.0f;
        std::snprintf(buf, sizeof(buf),
            "ALL FISH CAUGHT!   Time: %.2fs   Final Score: %d   (R to play again, M mute, L reload music)",
            timeSec, score);
    }
    glColor3f(0,0,0); glRasterPos2f(22, winH-30); for(char c: std::string(buf)) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,c);
    glColor3f(1,1,1); glRasterPos2f(20, winH-28); for(char c: std::string(buf)) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,c);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}

void computeFinalScore() {
    float timeSec = (finishMs - startMs)/1000.0f;
    int timeBonus = std::max(0, 1000 - (int)(timeSec * 20.0f));
    score = timeBonus + caughtCount * 50;
}

void checkGameOver() {
    if (!gameOver && caughtCount == (int)fishes.size()) {
        gameOver = true;
        finishMs = glutGet(GLUT_ELAPSED_TIME);
        computeFinalScore();
    }
}

void resetGame(){
    fishes.clear(); fishes.reserve(NUM_FISH + 1);

    // 20 ring fish (unchanged)
    for(int i=0;i<NUM_FISH;i++){
        Fish f;
        f.slotAngle=(360.0f/NUM_FISH)*i;
        f.radius=5.6f;
        f.mouthPhase= (float) (rand()%100)/100.0f;
        f.mouthSpeed= 0.5f + 0.6f*((float)(rand()%100)/100.0f);
        f.caught=false;
        float palette[6][3]={{1.00f,0.20f,0.25f},{0.20f,0.90f,0.35f},{0.20f,0.65f,1.00f},{1.00f,0.55f,0.15f},{0.75f,0.35f,1.00f},{1.00f,0.85f,0.20f}};
        int k=i%6; f.color[0]=palette[k][0]; f.color[1]=palette[k][1]; f.color[2]=palette[k][2];
        f.texId = texFish[k];
        f.worldPos = Vec3(); // will be updated when drawn
        fishes.push_back(f);
    }

    // +1 center fish (catchable)
    {
        Fish center;
        center.slotAngle = 0.0f;              // not used for center
        center.radius    = 0.0f;              // center → x=z=0
        center.mouthPhase = (float)(rand()%100)/100.0f;
        center.mouthSpeed = 0.5f + 0.6f*((float)(rand()%100)/100.0f);
        center.caught = false;
        center.color[0] = 1.0f; center.color[1] = 0.7f; center.color[2] = 0.2f; // golden
        center.texId = texFish[0];
        center.worldPos = Vec3();             // filled during draw
        fishes.push_back(center);
    }

    boardRot=0.0f;
    rodX=0.0f; rodZ=6.0f; hookY=5.0f;
    caughtCount=0; score=0; gameOver=false;
    startMs = glutGet(GLUT_ELAPSED_TIME);

#ifdef _WIN32
    readMusicPath();
    if(!g_musicMuted) playMusic(g_musicPath);
#endif
}


void tryCatch(){
    if (gameOver) return;
    for(auto& f: fishes){
        if(f.caught) continue;
        if(f.mouthPhase < 0.55f) continue;
        float dx=rodX - f.worldPos.x;
        float dy=hookY - f.worldPos.y;
        float dz=rodZ - f.worldPos.z;
        if((dx*dx + dy*dy + dz*dz) < (0.7f*0.7f)){
            f.caught=true; caughtCount++; break;
        }
    }
    checkGameOver();
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float rad = camAngle*PI/180.0f;
    float eyeX = cosf(rad)*camDist;
    float eyeZ = sinf(rad)*camDist;
    gluLookAt(eyeX, camHeight, eyeZ,  0,1.0,0,  0,1,0);

    setupLighting();

    boardRot += gameOver ? 0.04f : 0.08f;
    if(boardRot>=360.0f) boardRot-=360.0f;

    glPushMatrix();
      glRotatef(boardRot, 0,1,0);
      drawBoard();
    glPopMatrix();

    for(auto& f: fishes){
        float speed = gameOver ? (f.mouthSpeed*0.004f) : (f.mouthSpeed*0.018f);
        f.mouthPhase += speed;
        if(f.mouthPhase>1.0f) f.mouthPhase -= 1.0f;
    }
    drawFishesAndUpdate();
    drawRod();
    drawHUD();

    glutSwapBuffers();
}
void reshape(int w,int h){
    winW=(w<1)?1:w; winH=(h<1)?1:h;
    glViewport(0,0,winW,winH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0,(double)winW/(double)winH,0.1,200.0);
    glMatrixMode(GL_MODELVIEW);
}
void timer(int){ glutPostRedisplay(); glutTimerFunc(16,timer,0); }
void keyboard(unsigned char key,int,int){
    switch(key){
        case 27:
#ifdef _WIN32
            stopMusic();
#endif
            std::exit(0); break;
        case ' ': tryCatch(); break;
        case 'w': case 'W': hookY+=0.3f; if(hookY>9.0f) hookY=9.0f; break;
        case 's': case 'S': hookY-=0.3f; if(hookY<0.7f) hookY=0.7f; break;
        case 'a': case 'A': camAngle-=4.0f; break;
        case 'd': case 'D': camAngle+=4.0f; break;
        case 'r': case 'R': resetGame(); break;
        case 'm': case 'M':
#ifdef _WIN32
            g_musicMuted = !g_musicMuted;
            if (g_musicMuted) stopMusic(); else playMusic(g_musicPath);
#endif
        break;
        case 'l': case 'L':
#ifdef _WIN32
            readMusicPath();
            if (!g_musicMuted) playMusic(g_musicPath);
#endif
        break;
    }
    glutPostRedisplay();
}
void special(int key,int,int){
    const float step=0.4f;
    if(key==GLUT_KEY_LEFT) rodX-=step;
    if(key==GLUT_KEY_RIGHT) rodX+=step;
    if(key==GLUT_KEY_UP) rodZ-=step;
    if(key==GLUT_KEY_DOWN) rodZ+=step;
    if(rodX>8.0f) rodX=8.0f;
    if(rodX<-8.0f) rodX=-8.0f;
    if(rodZ>10.0f) rodZ=10.0f;
    if(rodZ<-10.0f) rodZ=-10.0f;
    glutPostRedisplay();
}

void initGL(){
    glClearColor(0.06f,0.10f,0.18f,1.0f);
    glEnable(GL_TEXTURE_2D);
    texBoardBase = loadBMP("textures/board_base.bmp");
    texBoardTop  = loadBMP("textures/board_top.bmp");
    texFish[0] = loadBMP("textures/fish1.bmp");
    texFish[1] = loadBMP("textures/fish2.bmp");
    texFish[2] = loadBMP("textures/fish3.bmp");
    texFish[3] = loadBMP("textures/fish4.bmp");
    texFish[4] = loadBMP("textures/fish5.bmp");
    texFish[5] = loadBMP("textures/fish6.bmp");
}

int main(int argc,char**argv){
    srand(1234);
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW,winH);
    glutCreateWindow("3D Fishing Game");
    initGL();
    resetGame();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutTimerFunc(16,timer,0);
    glutMainLoop();
    return 0;
}
