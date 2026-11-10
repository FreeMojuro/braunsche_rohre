#include <cstdlib>
#include <raylib.h>
#include <deque>

constexpr int window_width{1800};
constexpr int window_height{900};
float dt;
Color customblue{11,19,53,255};


class Kondensator_w{
    public:
    Vector2 pos;
    bool ispositive_charged;
    int width;
    int height;

    explicit Kondensator_w(const Vector2& v, bool c, int w, int h): pos{v},ispositive_charged{c}, width{w}, height{h}{}

    virtual void Draw()const{
        DrawRectangle(pos.x, pos.y, width, height, DARKGRAY);
        for(int i{height/10}; i < height;i+=(height/10)){
            DrawText(ispositive_charged ? "+" : "-", pos.x + width / 3.0f, pos.y + i, 25, ispositive_charged ? BLUE : RED);
            }
    }


};

class Kondensator_h : public Kondensator_w{
    public:
    Vector2 pos;
    bool ispositive_charged;
    int width;
    int height;

    explicit Kondensator_h(const Vector2& v, bool c, int w, int h): Kondensator_w(v, c, w, h), pos{v}, ispositive_charged{c}, width{w}, height{h} {}

    void Draw() const override {
      DrawRectangle(pos.x, pos.y, width, height, DARKGRAY);
      for(int i{width / 15}; i < width; i += (width / 15)){
        DrawText(ispositive_charged ? "+" : "-", pos.x + i,pos.y + height / 3.0f, 25, ispositive_charged ? BLUE : RED);
      }
    }



};

struct Particle{
    Vector2 pos;
    Vector2 velocity;
    Vector2 acceleration;
    float mass;
    float radius;
    bool ispositive_charged;
    float q;
    bool moving{true};
    bool sym_ison{false};
    
    Particle(const Vector2& p, const Vector2& v, const Vector2& a, float m, float r, bool ch, float Q): pos{p}, velocity{v}, acceleration{a}, mass{m}, radius{r}, ispositive_charged{ch}, q{Q} {}

    void Draw(const Kondensator_h& k3)const{
        Color c = ispositive_charged ? BLUE : RED;
        Color d = (c.r == BLUE.r && c.g == BLUE.g && c.b == BLUE.b && c.a == BLUE.a) ? RED : BLUE;
        DrawCircle(pos.x, pos.y, radius, c);
        if(sym_ison){
            float ypos;
            if(!k3.ispositive_charged)ypos = 450 + std::abs(pos.y-450);
            else ypos = 450 - std::abs(pos.y-450);
            DrawCircle(pos.x, ypos, radius, d);
        }
    }

    float calculate_accel(float U, float d){
        float a = (U * q) / (d * mass);
        return a;
    }

    void integrate_position(const Kondensator_h& k3){
        pos.x += velocity.x * dt + 0.5 * acceleration.x * dt *dt;
        velocity.x += 0.5 * acceleration.x * dt;

        if(k3.ispositive_charged){
        pos.y += velocity.y * dt + 0.5 * acceleration.y * dt *dt;
        velocity.y += 0.5 * acceleration.y * dt;
    }
        else{
            pos.y -= velocity.y * dt + 0.5 * acceleration.y * dt *dt;
            velocity.y += 0.5 * acceleration.y * dt;
        }
    }

    void integrate_velocity(const Vector2& new_acc){
        velocity.x += 0.5 * new_acc.x * dt;
        velocity.y += 0.5 * new_acc.y * dt;
        acceleration = new_acc;
    }
};


int main(){
    InitWindow(window_width, window_height, "br");
    SetTargetFPS(60);
    float d = 300;
    float s = 650;
    float Ub = 1000; // U von startbeschleunigung
    float Ua = 700; // U von ablenkung

    Kondensator_w k1{{10,100},true,50,700};
    Kondensator_w k2{{k1.pos.x+k1.width+d,100},false,50,700};
    Kondensator_h k3{{k2.pos.x+k2.width+10,100},false,900,50};
    Kondensator_h k4{{k2.pos.x+k2.width+10,k3.pos.y + s},true,900,50};
    Particle particle{{80,450},{0,0},{0,0},10,20,true,200};

    Rectangle gap{k2.pos.x, k2.pos.y+k2.height/2.0f-40, 80, 80,};

    std::deque<Vector2> trail;
    int deque_max_size{800};

    while(!WindowShouldClose()){
        dt = GetFrameTime();

        if(IsKeyPressed(KEY_C) && particle.pos.x < gap.x){k3.ispositive_charged= !k3.ispositive_charged; k4.ispositive_charged = !k4.ispositive_charged;}
        if(IsKeyPressed(KEY_S))particle.sym_ison = !particle.sym_ison;

        if(particle.moving){
            particle.integrate_position(k3);
            trail.push_back(particle.pos);
            if(particle.pos.y - particle.radius/2.0f <= k3.pos.y + k3.height || particle.pos.y + particle.radius/2.0f >= k4.pos.y){
                particle.moving = false;
            }
        }
        if(trail.size() > deque_max_size)trail.pop_front();

        Vector2 new_acc{};
        if(particle.pos.x <= gap.x + gap.width){
            float ax = particle.calculate_accel(Ub, d);
            new_acc = {ax,0};
        }
        else{
            float ay = particle.calculate_accel(Ua, s);
            new_acc = {0,ay};
        }

        if(particle.moving)particle.integrate_velocity(new_acc);

        BeginDrawing();

        ClearBackground(customblue);

        k1.Draw();
        k2.Draw();
        k3.Draw();
        k4.Draw();
        DrawRectangleRec(gap, customblue);
        particle.Draw(k3);
        for(int i{1}; i < trail.size(); ++i){
            DrawLine(trail[i].x, trail[i].y,trail[i-1].x,trail[i-1].y, WHITE);
            if(particle.sym_ison){
                if(!k3.ispositive_charged) 
                    DrawLine(trail[i].x, 450 + std::abs(trail[i].y-450), trail[i-1].x, 450 + std::abs(trail[i-1].y-450), WHITE);
                else DrawLine(trail[i].x, 450 - std::abs(trail[i].y-450), trail[i-1].x, 450 - std::abs(trail[i-1].y-450), WHITE);
            }
        }
        //DrawLine(80, 450, window_width, 450, RED); //symmetry achsis

        DrawText("Key s: enable symmetry", 40, window_height-50, 25, RAYWHITE);
        DrawText("Key c: switch charges", 40, window_height-80, 25, RAYWHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
