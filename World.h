#ifndef WORLD_H
#define WORLD_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QTime>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QLabel>
#include <QLineEdit>
#include <QVector>

#include "common.h"
#include "HashMap.h"
#include "Noise.h"
#include "Client.h"
#include "RoomClient.h"
#include "Cube.h"
#include "Matrix.h"

#include "GameMenu.h"

#define MAX_CHUNKS 1024
#define MAX_MESSAGES 4
#define MAX_PLAYERS 128
#define MAX_NAME_LENGTH 32
#define CREATE_CHUNK_RADIUS 6
#define RENDER_CHUNK_RADIUS 6
#define DELETE_CHUNK_RADIUS 6
#define SHADOW_TEXTURES 2
#define SHADOW_TEXTURE_SIZE 2048
#define SCROLL_THRESHOLD 0.1
#define ITEM_COUNT 10
#define RECV_BUFFER_SIZE 1024
#define TEXT_BUFFER_SIZE 256
#define COMMIT_INTERVAL 5
#define LEFT 0
#define CENTER 1
#define RIGHT 2
#define SHOW_SIZE 5

typedef struct{
    Map map;
    int p;
    int q;
    int faces;
    int dirty;
    GLuint buffer;
}Chunk;

typedef struct {
    float x;
    float y;
    float z;
    float rx;
    float ry;
    float t;
} State;

typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    State state;
    State state1;
    State state2;
    GLuint buffer;
} Player;

typedef struct{
    unsigned int fps;
    unsigned int frames;
    double since;
}FPS;


namespace Ui {
class World;
}

class World : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    World(QWidget *parent = 0, bool room_game = false, bool enter_room_game = false);
    ~World();
    
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void timerEvent(QTimerEvent *event);

    void show_chat();
    void room_recv();

    int rand_int(int n);
    double rand_double();
    void update_fps(FPS *fps);

    int is_plant(int w);
    int is_obstacle(int w);
    int is_transparent(int w);
    int is_destructable(int w);
    int is_selectable(int w);
    int chunked(float x);

    void get_sight_vector(float rx, float ry, float* vx, float* vy, float* vz);
    void get_motion_vector(int flying, int sz, int sx, float rx, float ry,
        float* vx, float* vy, float* vz);

    GLuint gen_buffer(GLsizei size, GLfloat *data);
    void del_buffer(GLuint buffer);
    GLfloat *malloc_faces(int components, int faces);
    GLuint gen_faces(int components, int faces, GLfloat *data);
    GLuint gen_crosshair_buffer(int width, int height);
    GLuint gen_wireframe_buffer(float x, float y, float z, float n);
    GLuint gen_cube_buffer(float x, float y, float z, float n, int w);
    GLuint gen_plant_buffer(float x, float y, float z, float n, int w);
    GLuint gen_player_buffer(float x, float y, float z, float rx, float ry);
    GLuint gen_text_buffer(float x, float y, float n, char *text);

    void create_world(Map *map, int p, int q);

    void draw_triangles_3d(GLuint buffer, int count);
    void draw_triangles_2d(GLuint buffer, int count);
    void draw_lines(GLuint buffer, int components, int count);
    void draw_chunk(Chunk *chunk);
    void draw_item(GLuint buffer, int count);
    void draw_text(GLuint buffer, int length);
    void draw_cube(GLuint buffer);
    void draw_plant(GLuint buffer);
    void draw_player(Player *player);

    Player *find_player(int id);
    void update_player(Player *player,
        float x, float y, float z, float rx, float ry, int interpolate);
    void interpolate_player(Player *player);
    void delete_player(int id);

    float player_player_distance(Player *p1, Player *p2);
    float player_crosshair_distance(Player *p1, Player *p2);
    Player *player_crosshair(Player *player);

    Chunk *find_chunk(int p, int q);
    int chunk_distance(Chunk *chunk, int p, int q);
    int chunk_visible(Chunk *chunk, float *matrix);

    int highest_block(float x, float z);

    int _hit_test(
        Map *map, float max_distance, int previous,
        float x, float y, float z,
        float vx, float vy, float vz,
        int *hx, int *hy, int *hz);
    int hit_test(
        int previous, float x, float y, float z, float rx, float ry,
        int *bx, int *by, int *bz);

    int collide(int height, float *x, float *y, float *z);

    int player_intersects_block(
        int height,
        float x, float y, float z,
        int hx, int hy, int hz);

    void exposed_faces(
        Map *map, int x, int y, int z,
        int *f1, int *f2, int *f3, int *f4, int *f5, int *f6);

    void load_map(Map* map, int p, int q);

    void gen_chunk_buffer(Chunk *chunk);
    void create_chunk(Chunk *chunk, int p, int q);
    void delete_chunks();
    void ensure_chunks(float x, float y, float z, int force);

    void _set_block(int p, int q, int x, int y, int z, int w);
    void set_block(int x, int y, int z, int w);

    int get_block(int x, int y, int z);

    void render_chunks(int width, int height, Player *player);
    void render_players(int width, int height, Player *player);
    void render_wireframe(int width, int height, Player *player);
    void render_crosshairs(int width, int height);
    void render_item(int width, int height);
    void render_text(
        int width, int height,
        int justify, float x, float y, float n, char *text);

    void render_hand();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

private:
    Ui::World *ui;

private:
    QOpenGLShaderProgram *BlockShader;  //方块作色器
    QOpenGLShaderProgram *LineShader;   //画线着色器
    QOpenGLShaderProgram *TextShader;
    
    GLuint texture;
    GLuint font;
    
    QTimer timer;
    QTime time;
    QSet<int>keys;
    int timeId;

    float deltaTime;    //增量时间
    float lastFrame;    //上一帧更新时间
    FPS fps;            //帧率

    float x, y, z;      //玩家所在位置
    float dy;
    float rx, ry;       //俯仰角和偏航角
    float matrix[16];
    int sz, sx;         //前后和左右移动的值

    Chunk chunks[MAX_CHUNKS];   //单元区域块
    int chunk_count;            //要绘画单元区域块数量

    Player players[MAX_PLAYERS];
    int player_count;
    Player *me;

    int observe1;
    int observe2;
    int exclusive;
    int left_click, right_click;
    int flying;
    int block_type;
    int typing;
    char typing_buffer[TEXT_BUFFER_SIZE];

    char message_index;
    char messages[MAX_MESSAGES][TEXT_BUFFER_SIZE];
    double last_commit;
    double last_update;
    double last_send_position;

    bool is_room_game;
    bool is_enter_room_game;

    Noise noise;
    HashMap hashMap;
    Cube cube;
    Matrix mat;

    GameMenu *menu;

    QVector<QString>chat_vec;

public:
    void send_world_return_start_signal();
    void on_continue_game_clicked();

signals:
    void world_return_start_clicked_signal();
};

#endif // WORLD_H
