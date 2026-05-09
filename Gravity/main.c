#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>
#include <raymath.h>

typedef struct Ball
{
    // make position and velocity a vector
    Vector2 position;
    double radius;
    double temperature;
    Vector2 velocity;
    double mass;
    bool is_dragging;
    double prev_x;
    double prev_y;
    double elasticity;
    struct Ball *next;
} Ball;

// function prototypes
void BallMovement(Ball *ball);
bool MouseBall(Ball *ball, Vector2 mouse_pos, float dt);
void add_ball( Ball *new_ball);
void clickadd ( Vector2 mouse_pos);
int Ballcollide (Ball *b1, Ball *b2);
void gravity (Ball *b1, Ball *b2);
void temperature_change (Ball *b1, Ball *b2);
void remove_ball(Ball *to_remove);

const double MAX_TEMP = 100000.0;
Ball *HEAD = NULL;
bool PAUSED = false;
float timescale = 1.0;

float dt;

int main(void)
{
    Ball *ball1 = malloc(sizeof(Ball));
    *ball1 = (Ball){.position = {960.0, 540.0}, .radius = 10.0, .velocity = {0, 0}, .mass = 5.0, .temperature = 0, .is_dragging = false, .elasticity = 0.8, .next = NULL};
    HEAD = ball1;

    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "physics engine");

    Camera2D camera = {0};
    Vector2 prevMouse = {0};

    camera.target = (Vector2){GetMonitorWidth(0)/2.0, GetMonitorHeight(0)/2.0};
    camera.offset = (Vector2){GetMonitorWidth(0) / 2.0f, GetMonitorHeight(0) / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    SetTargetFPS(240);

    while (!WindowShouldClose())
    {

        Vector2 mouse_world_pos = GetScreenToWorld2D(GetMousePosition(), camera);

        if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON))
        {
            prevMouse = GetMousePosition();
        }

        if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
        {
            Vector2 mouse = GetMousePosition();

            Vector2 delta = Vector2Subtract(prevMouse, mouse);

            delta = Vector2Scale(delta, 1.0f / camera.zoom);

            camera.target = Vector2Add(camera.target, delta);

            prevMouse = mouse;
        }

        float wheel = GetMouseWheelMove();

        if (wheel != 0)
        {
            Vector2 mouseWorldBefore =
                GetScreenToWorld2D(GetMousePosition(), camera);

            // Zoom
            camera.zoom += wheel * 0.1f;

            // Clamp zoom
            if (camera.zoom < 0.07f)
                camera.zoom = 0.07f;

            if (camera.zoom > 50.0f)
                camera.zoom = 50.0f;

            // World position after zoom
            Vector2 mouseWorldAfter =
                GetScreenToWorld2D(GetMousePosition(), camera);

            // Move camera so cursor stays on same world point
            camera.target = Vector2Add(
                camera.target,
                Vector2Subtract(mouseWorldBefore, mouseWorldAfter)
            );
        }

        dt = GetFrameTime() * timescale;

        if (dt > 0.1) {
            dt = 0.1; 
        }

        if (IsKeyDown(KEY_KP_ADD))
        {
            timescale += 0.5;

            if (timescale > 200.0)
                timescale = 200.0;
        }

        if (IsKeyDown(KEY_KP_SUBTRACT))
        {
            timescale -= 0.5;

            if (timescale < 1)
                timescale = 1;
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            clickadd(mouse_world_pos);
        }

        if (IsKeyPressed(KEY_SPACE)) {
            PAUSED = !PAUSED;
        }
        // Gravity loop
        if (!PAUSED) {
            Ball *ballA = HEAD;
            while (ballA != NULL) {
                Ball *nextA = ballA->next;  
                Ball *ballB = ballA->next;
                while (ballB != NULL) {
                    Ball *nextB = ballB->next;
                    if (Ballcollide(ballA, ballB) == 0) {
                        gravity(ballA, ballB);
                        temperature_change(ballA, ballB);
                    }
                    ballB = nextB;  
                }
                ballA = nextA;
            }
}
        // Physics loop
        Ball *ball = HEAD;
        while (ball != NULL) {
            Ball *next = ball->next;
            MouseBall(ball, mouse_world_pos, dt);
            if (!ball->is_dragging && !PAUSED) {
                BallMovement(ball);
            }
            ball = next;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
        ball = HEAD;
        while (ball != NULL) {
            unsigned char t = (unsigned char)((ball->temperature / MAX_TEMP) * 255);
            DrawCircle(ball->position.x, ball->position.y, ball->radius, (Color){t, 0, 255 - t, 255});
            DrawText(TextFormat("Velocity: %.2f", Vector2Length(ball->velocity)), ball->position.x - 20, ball->position.y + ball->radius + 50, 10, WHITE);
            DrawText(TextFormat("Temperature: %.2fX", ball->temperature), ball->position.x - 20, ball->position.y + ball->radius + 35, 10, WHITE);
            ball = ball->next;
        }
        EndMode2D();
        DrawText(TextFormat("Time warp: %.2f", timescale), 10, 20, 20, WHITE);
        EndDrawing();
    }
    return 0;
}

void BallMovement(Ball *ball)
{

    ball->temperature -= 0.00001 * ball->temperature * dt;

    ball->position = (Vector2){
        ball->position.x + ball->velocity.x * dt,
        ball->position.y + ball->velocity.y * dt
    };

    if (ball->position.y - ball->radius > GetMonitorHeight(0) + 10000000)
    {
        remove_ball(ball);
    }

    if (ball->position.y + ball->radius < 0 - 10000000)
    {
        remove_ball(ball);
    }

    if (ball->position.x + ball->radius < 0 - 10000000)
    {
        remove_ball(ball);
    }
    if (ball->position.x - ball->radius > GetMonitorWidth(0) + 10000000)
    {
        remove_ball(ball);
    }
}

bool MouseBall(Ball *ball, Vector2 mouse_pos, float dt) {

    double dx = mouse_pos.x - ball->position.x;
    double dy = mouse_pos.y - ball->position.y;
    double dist = sqrt(dx*dx + dy*dy);

    // Start dragging
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && dist <= ball->radius) {
        ball->is_dragging = true;
        ball->prev_x = mouse_pos.x;  
        ball->prev_y = mouse_pos.y;  
        ball->velocity = (Vector2){0, 0};
    }

    // While dragging
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && ball->is_dragging) {
        // velocity in pixels/frame, not pixels/second
        ball->velocity.x = (mouse_pos.x - ball->prev_x )/ dt;
        ball->velocity.y = (mouse_pos.y - ball->prev_y)/ dt;

        ball->prev_x = mouse_pos.x;
        ball->prev_y = mouse_pos.y;

        ball->position = mouse_pos;

        return true;
    }

    // Release
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && ball->is_dragging) {
        ball->is_dragging = false;
    }

    return ball->is_dragging;
}

void add_ball(Ball *new_ball) {
    Ball *temp = HEAD;
    HEAD = new_ball;
    HEAD->next = temp;
}

void clickadd (Vector2 mouse_pos) {
    Ball *new_ball = malloc(sizeof(Ball));

    int K;

    new_ball->position.x = mouse_pos.x;
    new_ball->position.y = mouse_pos.y;

    new_ball->radius = rand() % 5 + 5.0;
    double density;
    if ((rand() % 100) < 10) {  //10%  of being a sun
        density = 1000 + rand() % 9000;
        new_ball->radius = 20 + rand() % 50;
        K = 1;
    } 
    else if ((rand() % 100) < 90) {
        density = 500 + rand() % 1000; 
        K = 0; 
    }
    else {
        density = 100 + rand() % 400;
        K = 0;     
    }

    new_ball->mass = density * (4.0/3.0) * PI * pow(new_ball->radius, 3); // calculate mass based on volume and density (realism)

    new_ball->velocity = (Vector2){0, 0};
    new_ball->temperature = MAX_TEMP * K * 0.99;
    new_ball->is_dragging = false;
    new_ball->elasticity = 0.8;
    new_ball->next = NULL;

    add_ball(new_ball);
}

int Ballcollide (Ball *b1, Ball *b2) {

    double epsilon = 0.01;

    double dx = b2->position.x - b1->position.x;
    double dy = b2->position.y - b1->position.y;
    double distance = sqrt(dx*dx + dy*dy + epsilon*epsilon);

    if (distance < b1->radius + b2->radius) {
    
        Ball *better = (b1->mass >= b2->mass) ? b1 : b2;
        Ball *worse = (better == b1) ? b2 : b1;

        double m1 = better->mass;
        double m2 = worse->mass;
        double total_mass = better->mass + worse->mass;

         //Temperature change based on kinetic energy lost in the collision
        Vector2 relative = Vector2Subtract(better->velocity, worse->velocity);

        double avg_temp = (m1 * better->temperature + m2 * worse->temperature) / total_mass;

        double rel_speed = Vector2Length(relative);
        double constant = 0.0001; 
        double delta_t = (constant* m1 * m2 * rel_speed * rel_speed) / (total_mass * total_mass);


        // Conservation of momentum for inelastic collision
        better->velocity = (Vector2){
            (
                better->velocity.x * m1 +
                worse->velocity.x  * m2
            ) / total_mass,

            (
                better->velocity.y * m1 +
                worse->velocity.y  * m2
            ) / total_mass
        };

        better->temperature = avg_temp + delta_t;
        better->radius = cbrt((better->radius * better->radius * better->radius) + (worse->radius * worse->radius * worse->radius));
        better->mass = total_mass;

        if (better->temperature > MAX_TEMP) {
            better->temperature = MAX_TEMP;
        }

        remove_ball(worse);
        return 1;
    }
    return 0;
}

void gravity (Ball *b1, Ball *b2)
{
    double G = 0.1; // Gravitational constant, adjusted for simulation scale

    double dx = b2->position.x - b1->position.x;
    double dy = b2->position.y - b1->position.y;

    // adjusted formula for non-0 dist
    double epsilon = 0.1;
    double r2 = dx*dx + dy*dy + epsilon*epsilon; 
    double r = sqrt(r2);
    double inv_r3 = 1.0 / (r2 * r);

    // Gravitational acceleration
    double ax1 = G * b2->mass * dx * inv_r3;
    double ay1 = G * b2->mass * dy * inv_r3;

    double ax2 = -G * b1->mass * dx * inv_r3;
    double ay2 = -G * b1->mass * dy * inv_r3;

    b1->velocity = (Vector2){
        b1->velocity.x + ax1 * dt,
        b1->velocity.y + ay1 * dt
    };

    b2->velocity = (Vector2){
        b2->velocity.x + ax2 * dt,
        b2->velocity.y + ay2 * dt
    };
}

void remove_ball(Ball *to_remove) {
    if (HEAD == to_remove) {
        Ball *temp = HEAD;
        HEAD = HEAD->next;
        free(temp);
        return;
    }

    Ball *current = HEAD;
    while (current->next != NULL && current->next != to_remove) {
        current = current->next;
    }

    if (current->next == to_remove) {
        current->next = to_remove->next;
        free(to_remove);
        return;
    }

    printf("Error: Ball not found in the list.\n");
}

void temperature_change(Ball *b1, Ball *b2)
{
    double dx = b2->position.x - b1->position.x;
    double dy = b2->position.y - b1->position.y;

    double r2 = dx*dx + dy*dy + 1.0;

    double temp_diff =
        b1->temperature - b2->temperature;

    double transfer =
        temp_diff * 2* dt / r2;

    b1->temperature -= transfer;
    b2->temperature += transfer;
}

void target_cam_planet(Ball *ball, Camera2D *camera, Vector2 mouse_pos){
    //pass
}
