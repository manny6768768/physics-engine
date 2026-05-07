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
bool Ballcollide (Ball *b1, Ball *b2);
void gravity (Ball *b1, Ball *b2);
void remove_ball(Ball *to_remove);

const int screen_x = 1920;
const int screen_y = 1080;
const double MAX_TEMP = 10000.0;
Ball *HEAD = NULL;
bool PAUSED = false;

int main(void)
{
    Ball *ball1 = malloc(sizeof(Ball));
    *ball1 = (Ball){.position = {960.0, 540.0}, .radius = 10.0, .velocity = {0, 0}, .mass = 5.0, .temperature = 0, .is_dragging = false, .elasticity = 0.8, .next = NULL};
    HEAD = ball1;

    InitWindow(screen_x, screen_y, "physics engine");
    ToggleFullscreen();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            clickadd(GetMousePosition());
        }

        if (IsKeyPressed(KEY_SPACE)) {
            PAUSED = !PAUSED;
        }
        // Gravity loop
        if (!PAUSED) {
            Ball *b1 = HEAD;
            while (b1 != NULL) {
                Ball *next_b1 = b1->next;
                Ball *b2 = b1->next;
                while (b2 != NULL) {
                    Ball *next_b2 = b2->next;
                    if (Ballcollide(b1, b2)) {
                        break;
                    }
                    if (!b1->is_dragging && !b2->is_dragging) {
                        gravity(b1, b2);  
                    }
                    b2 = next_b2;
                }
                b1 = next_b1;
            }
        }
        // Physics loop
        Ball *ball = HEAD;
        while (ball != NULL) {
            Ball *next = ball->next;
            MouseBall(ball, GetMousePosition(), GetFrameTime());
            if (!ball->is_dragging && !PAUSED) {
                BallMovement(ball);
            }
            ball = next;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        ball = HEAD;
        while (ball != NULL) {
            unsigned char t = (unsigned char)((ball->temperature / MAX_TEMP) * 255);
            DrawCircle(ball->position.x, ball->position.y, ball->radius, (Color){t, 0, 255 - t, 255});
            ball = ball->next;
        }
        EndDrawing();
    }
    return 0;
}

void BallMovement(Ball *ball)
{
    ball->position = Vector2Add(ball->position, ball->velocity);

    if (ball->position.y - ball->radius > screen_y + 100)
    {
        remove_ball(ball);
    }

    if (ball->position.y + ball->radius < 0 - 100)
    {
        remove_ball(ball);
    }

    if (ball->position.x + ball->radius < 0 - 100)
    {
        remove_ball(ball);
    }
    if (ball->position.x - ball->radius > screen_x + 100)
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
        ball->velocity.x = mouse_pos.x - ball->prev_x;
        ball->velocity.y = mouse_pos.y - ball->prev_y;

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
    new_ball->temperature = MAX_TEMP * K * 0.75;
    new_ball->is_dragging = false;
    new_ball->elasticity = 0.8;
    new_ball->next = NULL;

    add_ball(new_ball);
}

bool Ballcollide (Ball *b1, Ball *b2) {

    double epsilon = 0.01;

    double dx = b2->position.x - b1->position.x;
    double dy = b2->position.y - b1->position.y;
    double distance = sqrt(dx*dx + dy*dy + epsilon*epsilon);

    if (distance < b1->radius + b2->radius) {
    
        Ball *better = (b1->mass >= b2->mass) ? b1 : b2;
        Ball *worse = (better == b1) ? b2 : b1;

        double total_mass = better->mass + worse->mass;

        better->velocity = (Vector2){
            (
                better->velocity.x * better->mass +
                worse->velocity.x  * worse->mass
            ) / total_mass,

            (
                better->velocity.y * better->mass +
                worse->velocity.y  * worse->mass
            ) / total_mass
        };

        better->radius = better->radius = cbrt(pow(better->radius, 3) + pow(worse->radius, 3));
        better->mass += worse->mass;
        remove_ball(worse);
        return true;
    }
    return false;
}

void gravity (Ball *b1, Ball *b2)
{
    double G = 0.000075; // Gravitational constant, adjusted for simulation scale

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

    b1->velocity = Vector2Add(b1->velocity, (Vector2){ax1, ay1});
    b2->velocity = Vector2Add(b2->velocity, (Vector2){ax2, ay2});
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

void temperature_change(Ball *b1, Ball *b2) {
    // TODO
}

