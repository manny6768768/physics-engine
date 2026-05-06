#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>

typedef struct Ball
{
    double x;
    double y;
    double radius;
    double temperature;
    double y_velocity;
    double x_velocity;
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
const double MAX_TEMP = 5000.0;
Ball *HEAD = NULL;
bool PAUSED = false;

int main(void)
{
    Ball *ball1 = malloc(sizeof(Ball));
    *ball1 = (Ball){.x = 960.0, .y = 540.0, .radius = 10.0, .y_velocity = 0, .x_velocity = 0, .mass = 5.0, .temperature = 3000.0, .is_dragging = false, .elasticity = 0.8, .next = NULL};
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
                Ball *b2 = b1->next;
                while (b2 != NULL) {
                    if (b1->is_dragging || b2->is_dragging) {
                        b2 = b2->next;
                        continue;
                    }
                    gravity(b1, b2);
                    b2 = b2->next;
                }
                b1 = b1->next;
            }
        }
        // Physics loop
        Ball *ball = HEAD;
        while (ball != NULL) {
            MouseBall(ball, GetMousePosition(), GetFrameTime());
            if (!ball->is_dragging && !PAUSED) {
                BallMovement(ball);
            }
            ball = ball->next;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        ball = HEAD;
        while (ball != NULL) {
            unsigned char t = (unsigned char)((ball->temperature / MAX_TEMP) * 255);
            DrawCircle(ball->x, ball->y, ball->radius, (Color){t, 0, 255 - t, 255});
            ball = ball->next;
        }
        EndDrawing();
    }
    return 0;
}

void BallMovement(Ball *ball)
{
    ball->x += ball->x_velocity;
    ball->y += ball->y_velocity;

    if (ball->y - ball->radius > screen_y + 100)
    {
        remove_ball(ball);
    }

    if (ball->y + ball->radius < 0 - 100)
    {
        remove_ball(ball);
    }

    if (ball->x + ball->radius < 0 - 100)
    {
        remove_ball(ball);
    }
    if (ball->x - ball->radius > screen_x + 100)
    {
        remove_ball(ball);
    }
}

bool MouseBall(Ball *ball, Vector2 mouse_pos, float dt) {

    double dx = mouse_pos.x - ball->x;
    double dy = mouse_pos.y - ball->y;
    double dist = sqrt(dx*dx + dy*dy);

    // Start dragging
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && dist <= ball->radius) {
        ball->is_dragging = true;
        ball->prev_x = mouse_pos.x;  
        ball->prev_y = mouse_pos.y;  
        ball->x_velocity = 0;
        ball->y_velocity = 0;
    }

    // While dragging
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && ball->is_dragging) {
        // velocity in pixels/frame, not pixels/second
        ball->x_velocity = mouse_pos.x - ball->prev_x;
        ball->y_velocity = mouse_pos.y - ball->prev_y;

        ball->prev_x = mouse_pos.x;
        ball->prev_y = mouse_pos.y;

        ball->x = mouse_pos.x;
        ball->y = mouse_pos.y;

        return true;
    }

    // Release
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && ball->is_dragging) {
        ball->is_dragging = false;
    }

    return ball->is_dragging;
}

void add_ball(Ball *new_ball) {
    if (HEAD == NULL) {
        HEAD = new_ball;
        return;
    }
    Ball *current = HEAD;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_ball;
    new_ball->next = NULL;
}

void clickadd (Vector2 mouse_pos) {
    Ball *new_ball = malloc(sizeof(Ball));

    new_ball->x = mouse_pos.x;
    new_ball->y = mouse_pos.y;

    new_ball->radius = rand() % 25 + 3.0;
    double density;
    if ((rand() % 100) < 10) {  //10% sun
        density = 1000 + rand() % 9000;
        new_ball->radius = 50 + rand() % 50;
    } 
    else if ((rand() % 100) < 90) {
        density = 500 + rand() % 1000;    
    }
    else {
        density = 100 + rand() % 400;     
    }

    new_ball->mass = density * (4.0/3.0) * PI * pow(new_ball->radius, 3);

    new_ball->y_velocity = 0.0;
    new_ball->x_velocity = 0.0;
    new_ball->temperature = rand() % (int)MAX_TEMP;
    new_ball->is_dragging = false;
    new_ball->elasticity = 0.8;
    new_ball->next = NULL;

    add_ball(new_ball);
}

bool Ballcollide (Ball *b1, Ball *b2) {

    double epsilon = 0.01;

    double dx = b2->x - b1->x;
    double dy = b2->y - b1->y;
    double distance = sqrt(dx*dx + dy*dy + epsilon*epsilon);
    return distance < (b1->radius + b2->radius);
}

void gravity (Ball *b1, Ball *b2)
{
    double G = 0.0001;

    double dx = b2->x - b1->x;
    double dy = b2->y - b1->y;

    double epsilon = 0.1;
    double r2 = dx*dx + dy*dy + epsilon*epsilon;
    double r = sqrt(r2);
    double inv_r3 = 1.0 / (r2 * r);

    double ax1 = G * b2->mass * dx * inv_r3;
    double ay1 = G * b2->mass * dy * inv_r3;

    double ax2 = -G * b1->mass * dx * inv_r3;
    double ay2 = -G * b1->mass * dy * inv_r3;

    b1->x_velocity += ax1;
    b1->y_velocity += ay1;

    b2->x_velocity += ax2;
    b2->y_velocity += ay2;
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