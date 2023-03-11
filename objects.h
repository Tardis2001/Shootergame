enum {PLAYER,ENEMY,BULLET};
class Player
{
public:
    int id;
    int x,y;
    int vidas;
    int velocidade;
    int jump;
    int score;
    int borda_x,borda_y;
};
struct Bullet
{
     int id;
     int x;
     int y;
     bool vida;
     int speed;
};
struct Enemy{
    int id;
    int x,y;
    bool vida;
    int velocidade;
    int borda_x,borda_y;
};