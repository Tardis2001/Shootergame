#include<iostream>
#include<vector>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include"objects.h"
#include"Constants.h"
using namespace std;


ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_EVENT ev;
ALLEGRO_KEYBOARD_STATE keyState;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_FONT *font = NULL;
ALLEGRO_BITMAP *player_idle = NULL;
ALLEGRO_BITMAP *player_left = NULL;
ALLEGRO_BITMAP *player_right = NULL;
ALLEGRO_BITMAP *player_up = NULL;
ALLEGRO_BITMAP *Bullet_image = NULL;
ALLEGRO_BITMAP *mapa = NULL;
ALLEGRO_BITMAP *enemy = NULL;

bool isGameOver = false;

int x = 332,y = 232;
bool active = false,playing = true ,jump = false,redraw = true;
int vel_x,vel_y;
enum MYKEYS{
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

int dir = KEY_DOWN;

int main(int argc, char const *argv[]);

// prototipos
int init();
void dispose();
void collision_check();
void initPlayer(Player &Player);
void drawIdlePlayer(Player &Player);

void initBullet(Bullet bullet[], int size);
void DrawBullet(Bullet bullet[], int size);
void FireBullet(Bullet bullet[], int size,Player &player);
void UpdateBullet(Bullet bullet[], int size);

void CollideBullet(Bullet bullet[], int bsize,Enemy enemy[],int size_enemy,Player &player);

void initEnemy(Enemy enemy[], int size);
void DrawEnemy(Enemy enemy[], int size);
void startEnemy(Enemy enemy[], int size);
void updateEnemy(Enemy enemy[], int size);
void CollideEnemy(Enemy enemy[], int size,Player &player);

int main(int argc, char const *argv[])
{ 
    
    Player Player;
    Bullet bullet[NUM_BULLET];
    Enemy Enemy[NUM_ENEMIES];
    if(init()){
        dispose();
        return -1;
    }
    srand(time(NULL));
    initPlayer(Player);
    initBullet(bullet,NUM_BULLET);
    initEnemy(Enemy,NUM_ENEMIES);
    //registra na fila os eventos de tela(ex:clickar no X na janela);
    al_register_event_source(event_queue,al_get_display_event_source(display));
    //registra na fila os eventos de teclado
    al_register_event_source(event_queue,al_get_keyboard_event_source());
    //registra na fila os eventos de timer
    al_register_event_source(event_queue,al_get_timer_event_source(timer));
    
    al_start_timer(timer);
    
    font = al_load_font("assets/fonts/IsWasted.ttf",18,0);

        
    while (playing)
    {

        al_wait_for_event(event_queue,&ev);
        al_get_keyboard_state(&keyState);
        if(ev.type == ALLEGRO_EVENT_TIMER){
            al_clear_to_color(al_map_rgba_f(0,0,0,0));
            al_draw_bitmap(mapa,0,0,0);
            redraw = true;

            Player.x = x;
            Player.y = y;
            active = true;
            if(al_key_down(&keyState,ALLEGRO_KEY_A)){
                vel_x = - move_speed;
                dir = KEY_LEFT;
                

            }
            // if(al_key_down(&keyState,ALLEGRO_KEY_S)){
            //       vel_x =+ move_speed;
            // }
            else if(al_key_down(&keyState,ALLEGRO_KEY_D)){

                vel_x = move_speed;
                dir = KEY_RIGHT;

            }         
            else {
                
                vel_x = 0;
                active = false;

            }   
            if(al_key_down(&keyState,ALLEGRO_KEY_W) && jump){
                vel_y =- jumpSpeed;
                jump = false;
            }

            
            if(!jump)
                vel_y += gravity;
            else
                 vel_y = 0;
            
            x += vel_x;
            y += vel_y;

            jump =(y + 50 >= 600);

            if(jump)
                y = 600 - 50;
            if(!isGameOver){
                UpdateBullet(bullet,NUM_BULLET);
                startEnemy(Enemy,NUM_ENEMIES);
                updateEnemy(Enemy,NUM_ENEMIES);
                CollideBullet(bullet,NUM_BULLET,Enemy,NUM_ENEMIES,Player);
                CollideEnemy(Enemy,NUM_ENEMIES,Player);
                if(Player.vidas <= 0){
                    isGameOver = true;
                }
            }
        }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
            playing = false;
        }
        else if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
            if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE){
                playing = false;
            }
            else if(ev.keyboard.keycode == ALLEGRO_KEY_J){
                FireBullet(bullet,NUM_BULLET,Player);
            }
        }
        if(redraw && al_is_event_queue_empty(event_queue)){
            redraw = false;
            if(!isGameOver){
                if(dir == KEY_LEFT){
                    al_draw_bitmap(player_left,x,y,0);
                }
                else if(dir == KEY_RIGHT){
                    al_draw_bitmap(player_right,x,y,0);

                }
                else{
                al_draw_bitmap(player_idle,Player.x,Player.y,0);

                }
                dir = 0;
                DrawBullet(bullet,NUM_BULLET);
                DrawEnemy(Enemy,NUM_ENEMIES);
                al_draw_textf(font,al_map_rgb(0,255,255),5, 5, ALLEGRO_ALIGN_LEFT,"Player tem %i vidas, e destruiu %i bigornas", Player.vidas, Player.score);
            }
            else
            {
                al_draw_textf(font,al_map_rgb(0,255,255),SCREEN_W / 2, SCREEN_H / 2, ALLEGRO_ALIGN_CENTER,"GAME OVER. Final Score %i", Player.score);
            }
            al_flip_display();
            al_clear_to_color(al_map_rgb(0,0,0));
        }
    }
    
    dispose();
}
int init(){

    al_set_window_title(display,"Plataform");

    //Inicializa o allegro
    if(!al_init()){
        fprintf(stderr,"Falhou em inicializar o allegro\n");
        return -1;
    }

    // Cria uma tela com dimensões de SCREEN_W, SCREEN_H
    display = al_create_display(SCREEN_W,SCREEN_H);
    if(!display){
        fprintf(stderr,"failed to create display\n");
        return -1;
    }
    
    timer = al_create_timer(1.0 / FPS);
    if(!timer){
        fprintf(stderr,"failed to create timer\n");
        al_destroy_timer(timer);
        return -1;
    }
    // cria fila de eventos   
    event_queue = al_create_event_queue();

    if(!event_queue){
        fprintf(stderr,"failed to create event_queue\n");
        al_destroy_display(display);
        return -1;
    }
        
    //instalar o teclado
    if(!al_install_keyboard()){
        fprintf(stderr,"failed to install keyboard\n");
        return -1;
    }

    if(!al_init_image_addon())
    {
        cout <<"Falha ao iniciar al_init_image_addon!" << endl;
        return -1;
    }
    mapa = al_load_bitmap("assets/images/map.bmp");
    if(!mapa)
    {
        cout << "Falha ao carregar o mapa!" << endl;
        al_destroy_display(display);
        return -1;
    }
    player_idle = al_load_bitmap("assets/images/player.bmp");

    if(!player_idle)
    {
        cout << "Falha ao carregar o mapa!" << endl;
        al_destroy_display(display);
        return -1;
    }
    player_right = al_load_bitmap("assets/images/player_right.bmp");
    if(!player_right)
    {
        cout << "Falha ao carregar o mapa!" << endl;
        al_destroy_display(display);
        return -1;
    }

    player_left = al_load_bitmap("assets/images/player_left.bmp");
    if(!player_left)
    {
        cout << "Falha ao carregar o player!" << endl;
        al_destroy_display(display);
        return -1;
    }
    Bullet_image = al_load_bitmap("assets/images/bullet.bmp");
    if(!Bullet_image)
    {
        cout << "Falha ao carregar a bala!" << endl;
        al_destroy_display(display);
        return -1;
    }
    enemy = al_load_bitmap("assets/images/bigorna.bmp");
    if (!enemy)
    {
        cout << "Falha ao carregar a bigorna" << endl;
        al_destroy_display(display);
    }

    if(!al_init_font_addon()){
        cout << "falha ao carregar a fonte" << endl;
        al_destroy_display(display);
        
    }

    if(!al_init_ttf_addon()){
        cout << "falha ao carregar a fonte" << endl;
        al_destroy_display(display);
        
    }
    //atualiza a tela ( quando houver algo para mostrar)
    al_flip_display();
    return 0;
}
void dispose(){

    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_bitmap(mapa);
    al_destroy_bitmap(player_idle);
    al_destroy_bitmap(player_left);
    al_destroy_bitmap(player_right);
    al_destroy_bitmap(player_up);
    al_destroy_font(font);
    al_destroy_event_queue(event_queue);
    al_uninstall_keyboard();

}
void initPlayer(Player &player){
    player.x = 20;
    player.y = SCREEN_H  / 2;
    player.id = PLAYER;
    player.vidas = 3;
    player.velocidade = move_speed;
    player.jump = jumpSpeed;
    player.borda_x = 32;
    player.borda_y = 32;
    player.score = 0;
}
void drawIdlePlayer(Player &player){
}
void initBullet(Bullet bullet[], int size){
    for (int i = 0; i < size; i++)
    {
        bullet[i].id = BULLET;
        bullet[i].speed = 5;
        bullet[i].vida = false;
    }
    
}
void DrawBullet(Bullet bullet[], int size)
{
    for (int i = 0; i < size; i++)
    {
        if(bullet[i].vida){
            al_draw_bitmap(Bullet_image,bullet[i].x,bullet[i].y,0);
        }
    }
    
}
void FireBullet(Bullet bullet[],int size,Player &player){
    for (int i = 0; i < size; i++)
    {
        if(!bullet[i].vida)
        {
            bullet[i].x = player.x+32;
            bullet[i].y = player.y - 5;
            bullet[i].vida = true;
            break;
        }
    }
    
}
void UpdateBullet(Bullet bullet[], int size)

{

    for (int i = 0; i < size; i++)
    {
        if(bullet[i].vida){
            
                bullet[i].y -= bullet[i].speed;
                if(bullet[i].y > SCREEN_H){
                    bullet[i].vida = false;
                    break;
                }
        }
    }
    
}

void CollideBullet(Bullet bullet[], int bsize,Enemy enemy[],int esize,Player &player){
    for (int i = 0; i < bsize; i++){
        if(bullet[i].vida){
            for (int j = 0; j < esize; j++)
            {
                if(enemy[j].vida){
                    if(bullet[i].x >(enemy[j].x - enemy[j].borda_x)&&
                    bullet[i].x < (enemy[j].x + enemy[j].borda_x)&& 
                    bullet[i].y > (enemy[j].y - enemy[j].borda_y) && 
                    bullet[i].y < (enemy[j].y + enemy[j].borda_y))
                    {   

                        bullet[i].vida = false;
                        enemy[j].vida = false;
                        player.score++;
                    }
                }
            }
            
        }
    }
    
}

void initEnemy(Enemy Enemy[], int size){
    for (int i = 0; i < size; i++)
    {
        Enemy[i].id = ENEMY;
        Enemy[i].vida = false;
        Enemy[i].velocidade = 3;
        Enemy[i].borda_x = 18;
        Enemy[i].borda_y = 18;
    }
    
}
void DrawEnemy(Enemy Enemy[], int size){

    for (int i = 0; i < size; i++)
    {
        if(Enemy[i].vida){
            al_draw_bitmap(enemy,Enemy[i].x,Enemy[i].y,0);
        }
    }
    
}
void startEnemy(Enemy Enemy[],int size){
    for(int i = 0; i < size; i ++){
        if(!Enemy[i].vida){
            if(rand()%500 == 0){
                Enemy[i].vida = true;
                Enemy[i].x = 30 + rand() % (SCREEN_W - 60);
                Enemy[i].y = 0;
            }
        }
    }
}
void updateEnemy(Enemy Enemy[], int size){
    for (int i = 0; i < size; i++)
    {
        if(Enemy[i].vida){
            Enemy[i].y += Enemy[i].velocidade;
            if(Enemy[i].y > SCREEN_H)
                Enemy[i].vida = false;
        }
    }
    
}
void CollideEnemy(Enemy enemy[], int size,Player &player){
    for(int i = 0; i < size; i ++){
        if(enemy[i].vida){
            if(enemy[i].x - enemy[i].borda_x < player.x + player.borda_x &&
                enemy[i].x + enemy[i].borda_x > player.x - player.borda_x &&
                enemy[i].y - enemy[i].borda_y < player.y + player.borda_y &&
                enemy[i].y + enemy[i].borda_y > player.y  - player.borda_y){
                    player.vidas--;
                    enemy[i].vida = false;
                }
                else if(enemy[i].x < 0){
                    enemy[i].vida = false;
                    player.vidas--;
                }
            
        }
    }
}
