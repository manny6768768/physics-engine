#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>

typedef struct Ball {
    double x;
    double y;
    double radius;
    double y_velocity;
    double x_velocity;
    double mass;
    bool is_dragging;
    double prev_x;
    double prev_y;
    *Ball;
} Ball;

const int screen_x = 1920;
const int screen_y = 1080;

int main(void)
{
    Ball ball1 = { .x = 960, .y = 540, .radius = 2, .y_velocity = 1, .x_velocity = 10, .mass = 10, .is_dragging = false };

    InitWindow(screen_x, screen_y, "physics engine");
    ToggleFullscreen();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {   
        BallMovement(&ball1);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawBall(ball1.x, ball1.y, ball1.radius, RED);
        printf("x: %f, y: %f\n", ball1.x_velocity, ball1.y_velocity);
        EndDrawing();
    }
}

void BallMovement(Ball *ball) {
    ball->x += ball->x_velocity;
    ball->y += ball->y_velocity;

    if (ball->y + ball->radius > screen_y) {
        ball->y = screen_y - ball->radius;
        ball->y_velocity *= -1;
    }

    if (ball->y - ball->radius < 0) {
        ball->y = ball->radius;
        ball->y_velocity *= -1;
    }

    if (ball->x - ball->radius < 0) {
        ball->x = ball->radius;
        ball->x_velocity *= -1;
    }
    if (ball->x + ball->radius > screen_x) {
        ball->x = screen_x - ball->radius;
        ball->x_velocity *= -1;
        }
}
