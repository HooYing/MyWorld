#include "World.h"
#include "ui_World.h"
#include <time.h>
#include "LoadingMap.h"
#include "RoomClient.h"

World::World(QWidget *parent, bool room_game, bool enter_room_game) : QOpenGLWidget(parent)
    ,timeId(0)
    ,deltaTime(0.0f),lastFrame(0.0f)
    ,x(0.0f),y(0.0f),z(0.0f)
    ,dy(0.0f),rx(0.0f),ry(0.0f)
    ,sz(0),sx(0)
    ,chunk_count(0)
    ,player_count(0),observe1(0),observe2(0),exclusive(1),left_click(0)
    ,right_click(0),flying(0),block_type(1),typing(0)
    ,is_room_game(room_game)
    ,is_enter_room_game(enter_room_game)
    ,ui(new Ui::World)
{
    ui->setupUi(this);
    setWindowTitle("wlecome");
    setWindowIcon(QIcon(":/resource/favicon.ico"));

    setMouseTracking(true);  //开启鼠标追踪：Qt默认不会实时监控鼠标移动
    setCursor(Qt::BlankCursor);
    //setMinimumSize(800, 600);
    //setMaximumSize(800, 600);
    //showMinimized();

    //setWindowFlags(Qt::CustomizeWindowHint);
    //setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint);
    //setWindowFlags (Qt::Window | Qt::FramelessWindowHint);
    //setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);


    menu = new GameMenu(this);
    menu->move(0,0);
    menu->resize(this->width(), this->height());
    menu->hide();

    ui->lineEdit->close();
    ui->lineEdit->setFocusPolicy(Qt::NoFocus);

    if(!is_room_game){
        ui->label->close();
    }

    void (GameMenu::*return_start_signal)() = &GameMenu::gamemenu_return_start_clicked_signal;
    void (GameMenu::*continue_game_signal)() = &GameMenu::gamemenu_continue_game_clicked_signal;
    connect(menu, return_start_signal, this, &World::send_world_return_start_signal);
    connect(menu, continue_game_signal, this, &World::on_continue_game_clicked);

    showFullScreen();
    //showMaximized();
}

World::~World()
{
    //db.db_save_state(x,y,z,rx,ry);
    //db.db_close();
    //cln.client_stop();
    delete menu;
    delete BlockShader;
    delete LineShader;
    delete TextShader;
    delete me;

    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
        hashMap.map_free(&chunk->map);
        del_buffer(chunk->buffer);
    }
}

void World::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);  //启用深度测试，根据坐标的远近自动隐藏被遮住的图形（材料）
    glEnable(GL_LINE_SMOOTH); //执行后，过虑线段的锯齿
    glLogicOp(GL_INVERT);     //原颜色与目标颜色转换
    glClearColor(0.53, 0.81, 0.92, 1);

    BlockShader = new QOpenGLShaderProgram(this);
    if(!BlockShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/block.vert")){
        qDebug()<<"error block.vert";
        return;
    }
    if(!BlockShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/block.frag")){
        qDebug()<<"error block.frag";
        return;
    }
    if(!BlockShader->link()){
        qDebug()<<"error BlockProgram link()";
        return;
    }

    LineShader = new QOpenGLShaderProgram(this);
    if(!LineShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/line.vert")){
        qDebug()<<"error line.vert";
        return;
    }
    if(!LineShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/line.frag")){
        qDebug()<<"error line.frag";
        return;
    }
    if(!LineShader->link()){
        qDebug()<<"error LineProgram link()";
        return;
    }

    TextShader = new QOpenGLShaderProgram(this);
    if(!TextShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/text.vert")){
        qDebug()<<"error text.vert";
        return;
    }
    if(!TextShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/text.frag")){
        qDebug()<<"error text.frag";
        return;
    }
    if(!TextShader->link()){
        qDebug()<<"error TextProgram link()";
        return;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    QImage img1 = QImage(":/texture/texture.png").convertToFormat(QImage::Format_RGBA8888);
    if (!img1.isNull()) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img1.width(), img1.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img1.bits());
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glGenTextures(1, &font);
    glBindTexture(GL_TEXTURE_2D, font);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    QImage img2 = QImage(":/texture/font.png").convertToFormat(QImage::Format_RGBA8888);
    if (!img2.isNull()) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img2.width(), img2.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img2.bits());
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    time.start();

    fps = {0,0,0};
    message_index = 0;
    last_commit = (GLfloat)time.elapsed()/1000;
    last_update = (GLfloat)time.elapsed()/1000;
    last_send_position = (GLfloat)time.elapsed()/1000;

    me = players;
    me->id = 0;
    me->buffer = 0;
    strncpy(me->name, "me", MAX_NAME_LENGTH);
    player_count = 1;

    x = pos_x, y = pos_y, z = pos_z, rx = pos_rx, ry = pos_ry;
    ensure_chunks(x, y, z, 1);
    qDebug()<<"pos"<<x<<y<<z<<rx<<ry;
    if(!is_loadingmap){
        y = highest_block(x, z) + 2;
    }
    //Client::get_instance()->client_position(x,y,z,rx,ry);

//    timer.setInterval(1);
//    connect(&timer,&QTimer::timeout,this,static_cast<void (World::*)()>(&World::update));
//    timer.start();
}

void World::paintGL()
{
    update_fps(&fps);

    int width = this->width();
    int height = this->height();

    GLfloat currentFrame = (GLfloat)time.elapsed()/1000;
    deltaTime = MIN(0.2f, currentFrame - lastFrame);
    lastFrame = currentFrame;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font);

    /*
    if(currentFrame - last_commit > COMMIT_INTERVAL){
        last_commit = currentFrame;
        db.db_commit();
    }
    */

    float vx, vy, vz;
    get_motion_vector(flying,sz,sx,rx,ry,&vx,&vy,&vz);
    sz=sx=0;
    float speed = flying?20:5;
    int step = 8;
    float ut = deltaTime/step;
    vx = vx*ut*speed;
    vy = vy*ut*speed;
    vz = vz*ut*speed;

    for (int i = 0; i < step; i++) {
        if (flying) {
            dy = 0;
        }
        else {
            dy -= ut * 25;
            dy = MAX(dy, -250);
        }
        x += vx;
        y += vy + dy * ut;
        z += vz;
        if (collide(2, &x, &y, &z)) {
            dy = 0;
        }
    }

    if (y < 0) {
        y = highest_block(x, z) + 2;
    }

    if (left_click) {
        left_click = 0;
        int hx, hy, hz;
        int hw = hit_test(0, x, y, z, rx, ry,
            &hx, &hy, &hz);
        if (hy > 0 && hy < 256 && is_destructable(hw)) {
            set_block(hx, hy, hz, 0);
            int above = get_block(hx, hy + 1, hz);
            if (is_plant(above)) {
                set_block(hx, hy + 1, hz, 0);
            }
        }
    }

    if (right_click) {
        right_click = 0;
        int hx, hy, hz;
        int hw = hit_test(1, x, y, z, rx, ry,
            &hx, &hy, &hz);
        if (hy > 0 && hy < 256 && is_obstacle(hw)) {
            if (!player_intersects_block(2, x, y, z, hx, hy, hz)) {
                set_block(hx, hy, hz, block_type);
            }
        }
    }

    if(is_room_game){
        room_recv();
    }

    if(!is_enter_room_game){
        if(currentFrame - last_update > 1){
            last_update = currentFrame;
            Client::get_instance()->client_position(x,y,z,rx,ry);
        }
    }


    if(is_room_game){
        if(currentFrame - last_send_position > 0.1){
            last_send_position = currentFrame;
            RoomClient::get_instance()->roomclient_position(me->id,x,y,z,rx,ry);
        }
    }


    observe1 = observe1 % player_count;
    observe2 = observe2 % player_count;
    delete_chunks();
    update_player(me, x, y, z, rx, ry, 0);
    for (int i = 1; i < player_count; i++) {
        interpolate_player(players + i);
    }

    Player *player = players + observe1;

    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    render_chunks(width, height, player);
    render_players(width, height, player);
    render_wireframe(width, height, player);

    glClear(GL_DEPTH_BUFFER_BIT);
    render_crosshairs(width, height);
    render_item(width, height);

    char text_buffer[1024];
    float ts = 12;
    float tx = ts / 2;
    float ty = height - ts;
    snprintf(
        text_buffer, 1024, "(%d, %d) (%.2f, %.2f, %.2f) [%d, %d] %d",
        chunked(x), chunked(z), x, y, z,
        player_count, chunk_count, fps.fps);
    render_text(width, height,
        LEFT, tx, ty, ts, text_buffer);
    for (int i = 0; i < MAX_MESSAGES; i++) {
        int index = (message_index + i) % MAX_MESSAGES;
        if (strlen(messages[index])) {
            ty -= ts * 2;
            render_text(width, height,
                LEFT, tx, ty, ts, messages[index]);
        }
    }
    if (typing) {
        ty -= ts * 2;
        snprintf(text_buffer, 1024, "> %s", typing_buffer);
        render_text(width, height,
            LEFT, tx, ty, ts, text_buffer);
    }

    if (player != me) {
        render_text(width, height,
            CENTER, width / 2, ts, ts, player->name);
    }

    Player *other = player_crosshair(player);
    if (other) {
        render_text(width, height, CENTER,
            width / 2, height / 2 - ts - 24, ts, other->name);
    }

    // 画中画
    if (observe2) {
        player = players + observe2;

        int pw = 256;
        int ph = 256;
        int pad = 3;
        int sw = pw + pad * 2;
        int sh = ph + pad * 2;

        glEnable(GL_SCISSOR_TEST);
        glScissor(width - sw - 32 + pad, 32 - pad, sw, sh);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glScissor(width - pw - 32, 32, pw, ph);
        glClearColor(0.53, 0.81, 0.92, 1.00);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(width - pw - 32, 32, pw, ph);

        render_chunks(pw, ph, player);
        render_players(pw, ph, player);

        glClear (GL_DEPTH_BUFFER_BIT);
        render_text (pw, ph,
            CENTER, pw / 2 , ts, ts, player-> name );
    }

    update();
}


void World::room_recv()
{
    char buffer[RECV_BUFFER_SIZE];
    int count = 0;
    while (count < 1024 && RoomClient::get_instance()->roomclient_recv(buffer, RECV_BUFFER_SIZE)) {
        count++;
        int pid;
        char name[MAX_NAME_LENGTH];
        if (sscanf(buffer, "you %d %s", &pid) == 1)
        {
            qDebug()<<pid;
            me->id = pid;
        }
        int bp, bq, bx, by, bz, bw;
        if (sscanf(buffer, "block %d %d %d %d %d %d",
            &bp, &bq, &bx, &by, &bz, &bw) == 6)
        {
            _set_block(bp, bq, bx, by, bz, bw);
            if (player_intersects_block(2, x, y, z, bx, by, bz)) {
                y = highest_block(x, z) + 2;
            }
        }
        float px, py, pz, prx, pry;
        if (sscanf(buffer, "position %d %f %f %f %f %f",
            &pid, &px, &py, &pz, &prx, &pry) == 6)
        {
            Player *player = find_player(pid);
            if (!player && player_count < MAX_PLAYERS) {
                qDebug()<<pid;
                player = players + player_count;
                player_count++;
                player->id = pid;
                player->buffer = 0;
                snprintf(player->name, MAX_NAME_LENGTH, "player%d", pid);
                update_player(player, px, py, pz, prx, pry, 1); // twice
            }
            if (player) {
                update_player(player, px, py, pz, prx, pry, 1);
            }
        }
        if (sscanf(buffer, "delete %d", &pid) == 1) {
            delete_player(pid);
        }
        if (sscanf(buffer, "player_name %d %s", &pid, name) == 2){
            Player *player = find_player(pid);
            if (player) {
                strncpy(player->name, name, MAX_NAME_LENGTH);
            }
        }
        if (sscanf(buffer, "enter player_name %d %s", &pid, name) == 2){
            QString s = (QString)"<font color=blue>"+name+"加入了房间"+"</font><br>";
            chat_vec.push_back(s);
            show_chat();
        }
        char text[100];
        if (sscanf(buffer, "talk %d %s", &pid, text) == 2){
            Player *player = find_player(pid);
            if (player) {
                QString s = (QString)"<font color=black>"+player->name+": "+text +"</font><br>";
                chat_vec.push_back(s);
                show_chat();
            }
        }
    }
}

void World::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    menu->resize(this->width(), this->height());
}

void World::keyPressEvent(QKeyEvent *event)
{
    //isAutoRepeat用于判断此按键的来源是否是长按
    keys.insert(event->key());                              //添加按键
    if(!event->isAutoRepeat()&&timeId==0){                  //如果定时器未启动，则启动定时器
        timeId=startTimer(1);
    }

    if(event->key() == Qt::Key_P){
        observe2 = (observe2 + 1) % player_count;
    }
    if(event->key() == Qt::Key_F){
        flying = !flying;
    }
    if(event->key() == Qt::Key_O){
        observe1 = (observe1 + 1) % player_count;
    }

    if(event->key() == Qt::Key_Escape){
        //close();
        if(typing){
            typing = 0;
        }
        else if(exclusive){
            setMouseTracking(false);
            setCursor(Qt::ArrowCursor);
            exclusive = 0;
            menu->show();
        }
        else if(!exclusive){
            exclusive = 1;
            setMouseTracking(true);  //开启鼠标追踪：Qt默认不会实时监控鼠标移动
            setCursor(Qt::BlankCursor);
            int btnPosX = mapToGlobal(QPoint(0, 0)).x() + width()/2;
            int btnPosY = mapToGlobal(QPoint(0, 0)).y() + height()/2;
            QCursor::setPos(btnPosX,btnPosY);
            menu->hide();
        }
    }

    if(is_room_game){
        if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return){
            ui->lineEdit->close();
            ui->lineEdit->clearFocus();
            if(ui->lineEdit->text() == "")return;
            if(is_room_game){
                string text = ui->lineEdit->text().toStdString();
                RoomClient::get_instance()->roomclient_talk(me->id, const_cast<char*>(text.c_str()));
            }
            QString s = (QString)"<font color=green>"+me->name + ": " + ui->lineEdit->text() +"</font><br>";
            chat_vec.push_back(s);
            ui->lineEdit->clear();
            show_chat();
        }
        if(event->key() == Qt::Key_Tab){
            if(ui->lineEdit->isVisible()){
                ui->lineEdit->close();
                ui->lineEdit->clearFocus();
            }
            else{
                ui->lineEdit->show();
                ui->lineEdit->setFocus();
            }
        }
    }
}

void World::keyReleaseEvent(QKeyEvent *event)
{
    keys.remove(event->key());
    if(!event->isAutoRepeat()&&timeId!=0&&keys.empty()){    //当没有按键按下且定时器正在运行，才关闭定时器
         killTimer(timeId);
         timeId=0;                                          //重置定时器id
    }
}


void World::timerEvent(QTimerEvent *event)
{
    if(keys.contains(Qt::Key_Enter)){
        if(typing){
            typing = 0;
            //cln.client_talk(typing_buffer);
        }
    }
    if(keys.contains(Qt::Key_Backspace)){
        if(typing){
            int n = strlen(typing_buffer);
            if(n>0){
                typing_buffer[n-1] = '\0';
            }
        }
    }
    if(!typing){
        if(keys.contains(Qt::Key_W)){
            sz--;
        }
        if(keys.contains(Qt::Key_S)){
            sz++;
        }
        if(keys.contains(Qt::Key_A)){
            sx--;
        }
        if(keys.contains(Qt::Key_D)){
            sx++;
        }
        if(keys.contains(Qt::Key_Space)){
            if(dy==0)dy=8;
        }
    }

}

void World::mouseMoveEvent(QMouseEvent *event)
{
    if(!exclusive)return;
    //682 383
    int btnPosX = mapToGlobal(QPoint(0, 0)).x() + width()/2;
    int btnPosY = mapToGlobal(QPoint(0, 0)).y() + height()/2;

    //qDebug()<<rect().center().x()<<rect().center().y();

    GLfloat xoffset = event->x() - rect().center().x();
    GLfloat yoffset = rect().center().y() - event->y();

    GLfloat sensitivity = 0.0025f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    rx += xoffset;
    ry += yoffset;

    if(rx<0)rx+=M_PI*360.0f/180;
    if(rx>=M_PI*360.0f/180)rx-=M_PI*360.0f/180;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (ry > M_PI*89.0f/180)
        ry = M_PI*89.0f/180;
    if (ry < -M_PI*89.0f/180)
        ry = -M_PI*89.0f/180;

    QCursor::setPos(btnPosX,btnPosY);

}

void World::mousePressEvent(QMouseEvent *event)
{
    if(event->button() & Qt::LeftButton){
        if(exclusive){
            left_click = 1;
        }
    }

    if(event->button() & Qt::RightButton){
        if(exclusive){
            right_click = 1;
        }
    }
}

void World::wheelEvent(QWheelEvent *event)
{
    static double ypos = 0;
    ypos += event->delta();
    if (ypos < -SCROLL_THRESHOLD) {
        block_type++;
        if (block_type > 14) {
            block_type = 1;
        }
        ypos = 0;
    }
    if (ypos > SCROLL_THRESHOLD) {
        block_type--;
        if (block_type < 1) {
            block_type = 14;
        }
        ypos = 0;
    }
}

void World::show_chat()
{
    if(chat_vec.size() > SHOW_SIZE){
        chat_vec.pop_front();
    }
    QString all_chat;
    for(auto chat : chat_vec){
        all_chat += chat;
    }
    ui->label->clear();
    ui->label->setText(all_chat);
}

int World::rand_int(int n) {
    int result;
    while (n <= (result = rand() / (RAND_MAX / n)));
    return result;
}

double World::rand_double() {
    return (double)rand() / (double)RAND_MAX;
}

void World::update_fps(FPS *fps) {
    fps->frames++;
    double now = (double)time.elapsed()/1000;
    double elapsed = now - fps->since;
    if (elapsed >= 1) {
        fps->fps = fps->frames / elapsed;
        fps->frames = 0;
        fps->since = now;
    }
}

//是否是植物
int World::is_plant(int w) {
    return w > 16;
}

//是否是石块
int World::is_obstacle(int w) {
    w = ABS(w);
    return w > 0 && w < 16;
}

//是否可以通过
int World::is_transparent(int w) {
    w = ABS(w);
    return w == 0 || w == 10 || w == 15 || is_plant(w);
}

//是否可以摧毁
int World::is_destructable(int w) {
    return w > 0 && w != 16;
}

int World::is_selectable(int w) {
    return w > 0 && w <= 14;
}

int World::chunked(float x) {
    return floorf(roundf(x) / CHUNK_SIZE);
}


void World::get_sight_vector(float rx, float ry, float *vx, float *vy, float *vz)
{
    float m = cosf(ry);
    *vx = cosf(rx - RADIANS(90)) * m;
    *vy = sinf(ry);
    *vz = sinf(rx - RADIANS(90)) * m;
}

void World::get_motion_vector(int flying, int sz, int sx, float rx, float ry,
    float* vx, float* vy, float* vz)
{
    *vx = 0; *vy = 0; *vz = 0;
    if (!sz && !sx) {
        return;
    }
    float strafe = atan2f(sz, sx);
    if (flying) {
        float m = cosf(ry);
        float y = sinf(ry);
        if (sx) {
            y = 0;
            m = 1;
        }
        if (sz > 0) {
            y = -y;
        }
        *vx = cosf(rx + strafe) * m;
        *vy = y;
        *vz = sinf(rx + strafe) * m;
    }
    else {
        *vx = cosf(rx + strafe);
        *vy = 0;
        *vz = sinf(rx + strafe);
    }
}


GLuint World::gen_buffer(GLsizei size, GLfloat *data) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return buffer;
}

void World::del_buffer(GLuint buffer) {
    glDeleteBuffers(1, &buffer);
}

GLfloat* World::malloc_faces(int components, int faces) {
    return (GLfloat*)malloc(sizeof(GLfloat) * 6 * components * faces);
}

GLuint World::gen_faces(int components, int faces, GLfloat *data) {
    GLuint buffer = gen_buffer(
        sizeof(GLfloat) * 6 * components * faces, data);
    free(data);
    return buffer;
}


GLuint World::gen_crosshair_buffer(int width, int height)
{
    int x = width / 2;
    int y = height / 2;
    int p = 10;
    float data[] = {
        x, y - p, x, y + p,
        x - p, y, x + p, y
    };
    return gen_buffer(sizeof(data), data);
}

GLuint World::gen_wireframe_buffer(float x, float y, float z, float n) {
    float data[144];
    cube.make_cube_wireframe(data, x, y, z, n);
    return gen_buffer(sizeof(data), data);
}

GLuint World::gen_cube_buffer(float x, float y, float z, float n, int w) {
    GLfloat *data = malloc_faces(8, 6);
    cube.make_cube(data, 1, 1, 1, 1, 1, 1, x, y, z, n, w);
    return gen_faces(8, 6, data);
}

GLuint World::gen_plant_buffer(float x, float y, float z, float n, int w) {
    GLfloat *data = malloc_faces(8, 4);
    float rotation = noise.simplex3(x, y, z, 4, 0.5, 2) * 360;
    cube.make_plant(data, x, y, z, n, w, rotation);
    return gen_faces(8, 4, data);
}

GLuint World::gen_player_buffer(float x, float y, float z, float rx, float ry) {
    GLfloat *data = malloc_faces(8, 6);
    cube.make_player(data, x, y, z, rx, ry);
    return gen_faces(8, 6, data);
}

GLuint World::gen_text_buffer(float x, float y, float n, char *text) {
    int length = strlen(text);
    GLfloat *data = malloc_faces(4, length);
    for (int i = 0; i < length; i++) {
        cube.make_character(data + i * 24, x, y, n / 2, n, text[i]);
        x += n;
    }
    return gen_faces(4, length, data);
}


void World::create_world(Map *map, int p, int q) {
    //pad便于处理边界问题，方便碰撞检测
    int pad = 1;
    for (int dx = -pad; dx < CHUNK_SIZE + pad; dx++) {
        for (int dz = -pad; dz < CHUNK_SIZE + pad; dz++) {
            int x = p * CHUNK_SIZE + dx;
            int z = q * CHUNK_SIZE + dz;
            float f = noise.simplex2(x * 0.01, z * 0.01, 4, 0.5, 2);
            float g = noise.simplex2(-x * 0.01, -z * 0.01, 2, 0.9, 2);
            int mh = g * 32 + 16;
            int h = f * mh;
            int w = 1;
            int t = 12;
            if (h <= t) {
                h = t;
                w = 2;
            }
            if (dx < 0 || dz < 0 || dx >= CHUNK_SIZE || dz >= CHUNK_SIZE) {
                w = -1;
            }
            // sand and grass terrain
            for (int y = 0; y < h; y++) {
                hashMap.map_set(map, x, y, z, w);
            }
            // TODO: w = -1 if outside of chunk
            if (w == 1) {
                // grass
                if (noise.simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.6) {
                    hashMap.map_set(map, x, h, z, 17);
                }
                // flowers
                if (noise.simplex2(x * 0.05, -z * 0.05, 4, 0.8, 2) > 0.7) {
                    int w = 18 + noise.simplex2(x * 0.1, z * 0.1, 4, 0.8, 2) * 7;
                    hashMap.map_set(map, x, h, z, w);
                }
                // trees
                int ok = 1;
                if (dx - 4 < 0 || dz - 4 < 0 ||
                    dx + 4 >= CHUNK_SIZE || dz + 4 >= CHUNK_SIZE)
                {
                    ok = 0;
                }
                if (ok && noise.simplex2(x, z, 6, 0.5, 2) > 0.84) {
                    for (int y = h + 3; y < h + 8; y++) {
                        for (int ox = -3; ox <= 3; ox++) {
                            for (int oz = -3; oz <= 3; oz++) {
                                int d = (ox * ox) + (oz * oz) +
                                    (y - (h + 4)) * (y - (h + 4));
                                if (d < 11) {
                                    hashMap.map_set(map, x + ox, y, z + oz, 15);
                                }
                            }
                        }
                    }
                    for (int y = h; y < h + 7; y++) {
                        hashMap.map_set(map, x, y, z, 5);
                    }
                }
            }
            // clouds
            for (int y = 64; y < 72; y++) {
                if (noise.simplex3(x * 0.01, y * 0.1, z * 0.01, 8, 0.5, 2) > 0.75) {
                    hashMap.map_set(map, x, y, z, 16);
                }
            }
        }
    }
}

void World::draw_triangles_3d(GLuint buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 6));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void World::draw_triangles_2d(GLuint buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, (GLvoid *)(sizeof(GLfloat) * 2));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void World::draw_lines(GLuint buffer, int components, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, components, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_LINES, 0, count);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void World::draw_chunk(Chunk *chunk) {
    draw_triangles_3d(chunk->buffer, chunk->faces * 6);
}

void World::draw_item(GLuint buffer, int count) {
    draw_triangles_3d(buffer, count);
}

void World::draw_text(GLuint buffer, int length) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_triangles_2d(buffer, length * 6);
    glDisable(GL_BLEND);
}

void World::draw_cube(GLuint buffer) {
    draw_item(buffer, 36);
}

void World::draw_plant(GLuint buffer) {
    draw_item(buffer, 24);
}

void World::draw_player(Player *player) {
    draw_cube(player->buffer);
}


Player* World::find_player(int id) {
    for (int i = 0; i < player_count; i++) {
        Player *player = players + i;
        if (player->id == id) {
            return player;
        }
    }
    return 0;
}

void World::update_player(Player *player,
    float x, float y, float z, float rx, float ry, int interpolate)
{
    if (interpolate) {
        State *s1 = &player->state1;
        State *s2 = &player->state2;
        memcpy(s1, s2, sizeof(State));
        s2->x = x; s2->y = y; s2->z = z; s2->rx = rx; s2->ry = ry;
        s2->t = (GLfloat)time.elapsed()/1000;
        if (s2->rx - s1->rx > PI) {
            s1->rx += 2 * PI;
        }
        if (s1->rx - s2->rx > PI) {
            s1->rx -= 2 * PI;
        }
    }
    else {
        State *s = &player->state;
        s->x = x; s->y = y; s->z = z; s->rx = rx; s->ry = ry;
        del_buffer(player->buffer);
        player->buffer = gen_player_buffer(s->x, s->y, s->z, s->rx, s->ry);
    }
}

void World::interpolate_player(Player *player) {
    State *s1 = &player->state1;
    State *s2 = &player->state2;
    float t1 = s2->t - s1->t;
    float t2 = (float)time.elapsed()/1000 - s2->t;
    t1 = MIN(t1, 1);
    t1 = MAX(t1, 0.1);
    float p = MIN(t2 / t1, 1);
    update_player(
        player,
        s1->x + (s2->x - s1->x) * p,
        s1->y + (s2->y - s1->y) * p,
        s1->z + (s2->z - s1->z) * p,
        s1->rx + (s2->rx - s1->rx) * p,
        s1->ry + (s2->ry - s1->ry) * p,
        0);
}

void World::delete_player(int id) {
    Player *player = find_player(id);
    if (!player) {
        return;
    }
    int count = player_count;
    del_buffer(player->buffer);
    Player *other = players + (--count);
    memcpy(player, other, sizeof(Player));
    player_count = count;
}

float World::player_player_distance(Player *p1, Player *p2) {
    State *s1 = &p1->state;
    State *s2 = &p2->state;
    float x = s2->x - s1->x;
    float y = s2->y - s1->y;
    float z = s2->z - s1->z;
    return sqrtf(x * x + y * y + z * z);
}

float World::player_crosshair_distance(Player *p1, Player *p2) {
    State *s1 = &p1->state;
    State *s2 = &p2->state;
    float d = player_player_distance(p1, p2);
    float vx, vy, vz;
    get_sight_vector(s1->rx, s1->ry, &vx, &vy, &vz);
    vx *= d; vy *= d; vz *= d;
    float px, py, pz;
    px = s1->x + vx; py = s1->y + vy; pz = s1->z + vz;
    float x = s2->x - px;
    float y = s2->y - py;
    float z = s2->z - pz;
    return sqrtf(x * x + y * y + z * z);
}

Player* World::player_crosshair(Player *player) {
    Player *result = 0;
    float threshold = RADIANS(5);
    float best = 0;
    for (int i = 0; i < player_count; i++) {
        Player *other = players + i;
        if (other == player) {
            continue;
        }
        float p = player_crosshair_distance(player, other);
        float d = player_player_distance(player, other);
        if (p / d < threshold) {
            if (best == 0 || d < best) {
                best = d;
                result = other;
            }
        }
    }
    return result;
}

Chunk* World::find_chunk(int p, int q) {
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk->p == p && chunk->q == q) {
            return chunk;
        }
    }
    return 0;
}

int World::chunk_distance(Chunk *chunk, int p, int q) {
    int dp = ABS(chunk->p - p);
    int dq = ABS(chunk->q - q);
    return MAX(dp, dq);
}

int World::chunk_visible(Chunk *chunk, float *matrix) {
    for (int dp = 0; dp <= 1; dp++) {
        for (int dq = 0; dq <= 1; dq++) {
            for (int y = 0; y < 128; y += 16) {
                float vec[4] = {
                    (chunk->p + dp) * CHUNK_SIZE - dp,
                    y,
                    (chunk->q + dq) * CHUNK_SIZE - dq,
                    1};
                mat.mat_vec_multiply(vec, matrix, vec);
                if (vec[3] >= 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int World::highest_block(float x, float z) {
    int result = -1;
    int nx = roundf(x);
    int nz = roundf(z);
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        Map *map = &chunk->map;
        for (unsigned int i = 0; i <= map->mask; i++) {
            Entry* e = map->data + i;
            if ((!(e)->x && !(e)->y && !(e)->z && !(e)->w)) {
                continue;
            }
            if (is_obstacle(e->w) && e->x == nx && e->z == nz) {
                result = MAX(result, e->y);
            }
        }
    }
    return result;
}

int World::_hit_test(
    Map *map, float max_distance, int previous,
    float x, float y, float z,
    float vx, float vy, float vz,
    int *hx, int *hy, int *hz)
{
    int m = 8;
    int px = 0;
    int py = 0;
    int pz = 0;
    for (int i = 0; i < max_distance * m; i++) {
        int nx = roundf(x);
        int ny = roundf(y);
        int nz = roundf(z);
        if (nx != px || ny != py || nz != pz) {
            int hw = hashMap.map_get(map, nx, ny, nz);
            if (hw > 0) {
                if (previous) {
                    *hx = px; *hy = py; *hz = pz;
                }
                else {
                    *hx = nx; *hy = ny; *hz = nz;
                }
                return hw;
            }
            px = nx; py = ny; pz = nz;
        }
        x += vx / m; y += vy / m; z += vz / m;
    }
    return 0;
}

int World::hit_test(
    int previous, float x, float y, float z, float rx, float ry,
    int *bx, int *by, int *bz)
{
    int result = 0;
    float best = 0;
    int p = chunked(x);
    int q = chunked(z);
    float vx, vy, vz;
    get_sight_vector(rx, ry, &vx, &vy, &vz);
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk_distance(chunk, p, q) > 1) {
            continue;
        }
        int hx, hy, hz;
        int hw = _hit_test(&chunk->map, 8, previous,
            x, y, z, vx, vy, vz, &hx, &hy, &hz);
        if (hw > 0) {
            float d = sqrtf(
                powf(hx - x, 2) + powf(hy - y, 2) + powf(hz - z, 2));
            if (best == 0 || d < best) {
                best = d;
                *bx = hx; *by = hy; *bz = hz;
                result = hw;
            }
        }
    }
    return result;
}

int World::collide(int height, float *x, float *y, float *z) {
    int result = 0;
    int p = chunked(*x);
    int q = chunked(*z);
    Chunk *chunk = find_chunk(p, q);
    if (!chunk) {
        return result;
    }
    Map *map = &chunk->map;
    int nx = roundf(*x);
    int ny = roundf(*y);
    int nz = roundf(*z);
    float px = *x - nx;
    float py = *y - ny;
    float pz = *z - nz;
    float pad = 0.25;
    for (int dy = 0; dy < height; dy++) {
        if (px < -pad && is_obstacle(hashMap.map_get(map, nx - 1, ny - dy, nz))) {
            *x = nx - pad;
        }
        if (px > pad && is_obstacle(hashMap.map_get(map, nx + 1, ny - dy, nz))) {
            *x = nx + pad;
        }
        if (py < -pad && is_obstacle(hashMap.map_get(map, nx, ny - dy - 1, nz))) {
            *y = ny - pad;
            result = 1;
        }
        if (py > pad && is_obstacle(hashMap.map_get(map, nx, ny - dy + 1, nz))) {
            *y = ny + pad;
            result = 1;
        }
        if (pz < -pad && is_obstacle(hashMap.map_get(map, nx, ny - dy, nz - 1))) {
            *z = nz - pad;
        }
        if (pz > pad && is_obstacle(hashMap.map_get(map, nx, ny - dy, nz + 1))) {
            *z = nz + pad;
        }
    }
    return result;
}

int World::player_intersects_block(
    int height,
    float x, float y, float z,
    int hx, int hy, int hz)
{
    int nx = roundf(x);
    int ny = roundf(y);
    int nz = roundf(z);
    for (int i = 0; i < height; i++) {
        if (nx == hx && ny - i == hy && nz == hz) {
            return 1;
        }
    }
    return 0;
}

void World::exposed_faces(
    Map *map, int x, int y, int z,
    int *f1, int *f2, int *f3, int *f4, int *f5, int *f6)
{
    *f1 = is_transparent(hashMap.map_get(map, x - 1, y, z));
    *f2 = is_transparent(hashMap.map_get(map, x + 1, y, z));
    *f3 = is_transparent(hashMap.map_get(map, x, y + 1, z));
    *f4 = is_transparent(hashMap.map_get(map, x, y - 1, z)) && (y > 0);
    *f5 = is_transparent(hashMap.map_get(map, x, y, z - 1));
    *f6 = is_transparent(hashMap.map_get(map, x, y, z + 1));
}


void World::gen_chunk_buffer(Chunk *chunk) {
    Map *map = &chunk->map;

    int faces = 0;
    for (unsigned int i = 0; i <= map->mask; i++) {
        Entry* e = map->data + i;
        if ((!(e)->x && !(e)->y && !(e)->z && !(e)->w)) {
            continue;
        }
        if (e->w <= 0) {
            continue;
        }
        int f1, f2, f3, f4, f5, f6;
        exposed_faces(map, e->x, e->y, e->z, &f1, &f2, &f3, &f4, &f5, &f6);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (is_plant(e->w)) {
            total = total ? 4 : 0;
        }
        faces += total;
    }

    GLfloat *data = malloc_faces(8, faces);
    int offset = 0;
    for (unsigned int i = 0; i <= map->mask; i++) {
        Entry* e = map->data + i;
        if ((!(e)->x && !(e)->y && !(e)->z && !(e)->w)) {
            continue;
        }
        if (e->w <= 0) {
            continue;
        }
        int f1, f2, f3, f4, f5, f6;
        exposed_faces(map, e->x, e->y, e->z, &f1, &f2, &f3, &f4, &f5, &f6);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (is_plant(e->w)) {
            total = total ? 4 : 0;
        }
        if (total == 0) {
            continue;
        }
        if (is_plant(e->w)) {
            float rotation = noise.simplex3(e->x, e->y, e->z, 4, 0.5, 2) * 360;
            cube.make_plant(
                data + offset,
                e->x, e->y, e->z, 0.5, e->w, rotation);
        }
        else {
            cube.make_cube(
                data + offset,
                f1, f2, f3, f4, f5, f6,
                e->x, e->y, e->z, 0.5, e->w);
        }
        offset += total * 48;
    }

    del_buffer(chunk->buffer);
    chunk->buffer = gen_faces(8, faces, data);
    chunk->faces = faces;
    chunk->dirty = 0;
}

void World::load_map(Map *new_map, int p, int q){
    map< pair<int,int>, Map >::iterator it = loadingmap.find(make_pair(p, q));
    if(it == loadingmap.end()){
        return;
    }
    Map *old_map = &(it->second);
    for (unsigned int i = 0; i <= old_map->mask; i++) {
        Entry* e = old_map->data + i;
        if ((!(e)->x && !(e)->y && !(e)->z && !(e)->w)) {
            continue;
        }
        hashMap.map_set(new_map, e->x, e->y, e->z, e->w);
    }
}

void World::create_chunk(Chunk *chunk, int p, int q) {
    chunk->p = p;
    chunk->q = q;
    chunk->faces = 0;
    chunk->dirty = 1;
    chunk->buffer = 0;
    Map *map = &chunk->map;
    hashMap.map_alloc(map);
    create_world(map, p, q);
    load_map(map, p, q);
    //db.db_load_map(map, p, q);
    gen_chunk_buffer(chunk);
    //int key = db.db_get_key(p, q);
    //cln.client_chunk(p, q, key);
}

void World::delete_chunks() {
    int count = chunk_count;
    State *s1 = &players->state;
    State *s2 = &(players + observe1)->state;
    State *s3 = &(players + observe2)->state;
    State *states[3] = {s1, s2, s3};
    for (int i = 0; i < count; i++) {
        Chunk *chunk = chunks + i;
        int del = 1;
        for (int j = 0; j < 3; j++) {
            State *s = states[j];
            int p = chunked(s->x);
            int q = chunked(s->z);
            if (chunk_distance(chunk, p, q) < DELETE_CHUNK_RADIUS) {
                del = 0;
                break;
            }
        }
        if (del) {
            hashMap.map_free(&chunk->map);
            del_buffer(chunk->buffer);
            Chunk *other = chunks + (--count);
            memcpy(chunk, other, sizeof(Chunk));
        }
    }
    chunk_count = count;
}

void World::ensure_chunks(float x, float y, float z, int force) {
    int count = chunk_count;
    int p = chunked(x);
    int q = chunked(z);
    int generated = 0;
    int rings = force ? 1 : CREATE_CHUNK_RADIUS;
    for (int ring = 0; ring <= rings; ring++) {
        for (int dp = -ring; dp <= ring; dp++) {
            for (int dq = -ring; dq <= ring; dq++) {
                if (ring != MAX(ABS(dp), ABS(dq))) {
                    continue;
                }
                if (!force && generated && ring > 1) {
                    continue;
                }
                int a = p + dp;
                int b = q + dq;
                Chunk *chunk = find_chunk(a, b);
                if (chunk) {
                    if (chunk->dirty) {
                        gen_chunk_buffer(chunk);
                        generated++;
                    }
                }
                else {
                    if (count < MAX_CHUNKS) {
                        create_chunk(chunks + count, a, b);
                        generated++;
                        count++;
                    }
                }
            }
        }
    }
    chunk_count = count;
}

void World::_set_block(int p, int q, int x, int y, int z, int w) {
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        Map *map = &chunk->map;
        if (hashMap.map_get(map, x, y, z) != w) {
            hashMap.map_set(map, x, y, z, w);
            chunk->dirty = 1;
        }
    }

    if(!is_enter_room_game){
        Client::get_instance()->client_save_block(p, q, x, y, z, w);
    }

    map< pair<int,int>, Map >::iterator it = loadingmap.find(make_pair(p, q));
    if(it!=loadingmap.end()){
        hashMap.map_set(&(it->second), x, y, z, w);
    }
    //db.db_insert_block(p, q, x, y, z, w);
}

void World::set_block(int x, int y, int z, int w) {
    int p = chunked(x);
    int q = chunked(z);
    qDebug()<<p<<q<<x<<y<<z<<w;
    _set_block(p, q, x, y, z, w);
    if(is_room_game){
        RoomClient::get_instance()->roomclient_block(p, q, x, y, z, w);
    }
    if (chunked(x - 1) != p) {
        _set_block(p - 1, q, x, y, z, -w);
        if(is_room_game){
            RoomClient::get_instance()->roomclient_block(p - 1, q, x, y, z, -w);
        }

    }
    if (chunked(x + 1) != p) {
        _set_block(p + 1, q, x, y, z, -w);
        if(is_room_game){
            RoomClient::get_instance()->roomclient_block(p + 1, q, x, y, z, -w);
        }

    }
    if (chunked(z - 1) != q) {
        _set_block(p, q - 1, x, y, z, -w);
        if(is_room_game){
            RoomClient::get_instance()->roomclient_block(p, q - 1, x, y, z, -w);
        }

    }
    if (chunked(z + 1) != q) {
        _set_block(p, q + 1, x, y, z, -w);
        if(is_room_game){
            RoomClient::get_instance()->roomclient_block(p, q + 1, x, y, z, -w);
        }

    }
    //cln.client_block(x, y, z, w);
}

int World::get_block(int x, int y, int z) {
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        Map *map = &chunk->map;
        return hashMap.map_get(map, x, y, z);
    }
    return 0;
}

void World::render_chunks(int width, int height, Player *player) {
    State *s = &player->state;
    ensure_chunks(s->x, s->y, s->z, 0);
    int p = chunked(s->x);
    int q = chunked(s->z);
    float matrix[16];
    mat.set_matrix_3d(
        matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, 65.0, 0);
    BlockShader->bind();
    glUniformMatrix4fv(BlockShader->uniformLocation("matrix"), 1, GL_FALSE, matrix);
    BlockShader->setUniformValue("camera",s->x,s->y,s->z);
    BlockShader->setUniformValue("sampler", 0);
    BlockShader->setUniformValue("fog_distance", (GLfloat)32*6);
    BlockShader->setUniformValue("timer",(GLfloat)time.elapsed()/1000);
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk_distance(chunk, p, q) > RENDER_CHUNK_RADIUS) {
            continue;
        }
        if (s->y < 100 && !chunk_visible(chunk, matrix)) {
            continue;
        }
        draw_chunk(chunk);
    }
}

void World::render_players(int width, int height, Player *player) {
    State *s = &player->state;
    float matrix[16];
    mat.set_matrix_3d(
        matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, 65.0, 0);
    BlockShader->bind();
    glUniformMatrix4fv(BlockShader->uniformLocation("matrix"), 1, GL_FALSE, matrix);
    BlockShader->setUniformValue("camera",s->x,s->y,s->z);
    BlockShader->setUniformValue("sampler", 0);
    BlockShader->setUniformValue("timer",(GLfloat)time.elapsed()/1000);
    for (int i = 0; i < player_count; i++) {
        Player *other = players + i;
        if (other != player) {
            draw_player(other);
        }
    }
}

void World::render_wireframe(int width, int height, Player *player) {
    State *s = &player->state;
    float matrix[16];
    mat.set_matrix_3d(
        matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, 65.0, 0);
    int hx, hy, hz;
    int hw = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    if (is_obstacle(hw)) {
        LineShader->bind();
        glLineWidth(1);
        glEnable(GL_COLOR_LOGIC_OP);
        glUniformMatrix4fv(LineShader->uniformLocation("matrix"), 1, GL_FALSE, matrix);
        GLuint wireframe_buffer = gen_wireframe_buffer(hx, hy, hz, 0.53);
        draw_lines(wireframe_buffer, 3, 48);
        del_buffer(wireframe_buffer);
        glDisable(GL_COLOR_LOGIC_OP);
    }
}

void World::render_crosshairs(int width, int height) {
    float matrix[16];
    mat.set_matrix_2d(matrix, width, height);
    LineShader->bind();
    glLineWidth(4);
    glEnable(GL_COLOR_LOGIC_OP);
    glUniformMatrix4fv(LineShader->uniformLocation("matrix"), 1, GL_FALSE, matrix);
    GLuint crosshair_buffer = gen_crosshair_buffer(width, height);
    draw_lines(crosshair_buffer, 2, 4);
    del_buffer(crosshair_buffer);
    glDisable(GL_COLOR_LOGIC_OP);
}

void World::render_item(int width, int height) {
    float matrix[16];
    mat.set_matrix_item(matrix, width, height);
    BlockShader->bind();
    glUniformMatrix4fv(BlockShader->uniformLocation("matrix"), 1, GL_FALSE, matrix);
    BlockShader->setUniformValue("camera", 0, 0, 5);
    BlockShader->setUniformValue("sampler", 0);
    BlockShader->setUniformValue("timer", (GLfloat)time.elapsed()/1000);
    if (is_plant(block_type)) {
        GLuint buffer = gen_plant_buffer(0, 0, 0, 0.5, block_type);
        draw_plant(buffer);
        del_buffer(buffer);
    }
    else {
        GLuint buffer = gen_cube_buffer(0, 0, 0, 0.5, block_type);
        draw_cube(buffer);
        del_buffer(buffer);
    }
}

void World::render_text(
    int width, int height,
    int justify, float x, float y, float n, char *text)
{
    float matrix[16];
    mat.set_matrix_2d(matrix, width, height);
    TextShader->bind();
    glUniformMatrix4fv(TextShader->uniformLocation("matrix"), 1, GL_FALSE, matrix);
    TextShader->setUniformValue("sampler", 1);
    int length = strlen(text);
    x -= n * justify * (length - 1) / 2;
    GLuint buffer = gen_text_buffer(x, y, n, text);
    draw_text(buffer, length);
    del_buffer(buffer);
}

void World::render_hand()
{
    float matrix[16];

}

void World::on_continue_game_clicked()
{
    exclusive = 1;
    setMouseTracking(true);  //开启鼠标追踪：Qt默认不会实时监控鼠标移动
    setCursor(Qt::BlankCursor);
    int btnPosX = mapToGlobal(QPoint(0, 0)).x() + width()/2;
    int btnPosY = mapToGlobal(QPoint(0, 0)).y() + height()/2;
    QCursor::setPos(btnPosX,btnPosY);
    menu->hide();
}


void World::send_world_return_start_signal()
{
    if(is_room_game && !is_enter_room_game){
        Client::get_instance()->client_delete_room();
    }
    if(is_room_game){
        RoomClient::get_instance()->roomclient_stop();
    }
    emit world_return_start_clicked_signal();
}
