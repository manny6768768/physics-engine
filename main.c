#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>

typedef struct Ball {
    double x;
    double y;
    double radius;
    double elasticity;
    double y_velocity;
    double x_velocity;
    double mass;
    double x_friction;
} Ball;

void UpdateBall(Ball *ball, double gravity, double *screen);
bool BallBounce(Ball *ball1, Ball *ball2);

int main(void)
{
    const int screen_x = 1200;
    const int screen_y = 850;

    double gravity = 0.4;

    Ball ball1 = {800.0, 200.0, 25.0, 0.80, -50.0, 10.0, 1.0}; // missing friction
    Ball ball2 = {400.0, 200.0, 25.0, 0.80, 0.0, 5.0, 1.0}; // missing friction

    ball1.x_friction = gravity * ball1.mass * 0.50;
    ball2.x_friction = gravity * ball2.mass * 0.05;


    InitWindow(screen_x, screen_y, "physics engine");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BallBounce(&ball1, &ball2);
        UpdateBall(&ball1, gravity, (double[]){screen_y, screen_x});
        UpdateBall(&ball2, gravity, (double[]){screen_y, screen_x});
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircle(ball1.x, ball1.y, ball1.radius, RED);
        DrawCircle(ball2.x, ball2.y, ball2.radius, BLACK);
        EndDrawing();
    }
}

void UpdateBall(Ball *ball, double gravity, double *screen) {
    ball->y_velocity += gravity;
    ball->y += ball->y_velocity;

    // Floor/Ceiling collision
    if (ball->y >= screen[0] - ball->radius) 
    {
        ball->y = screen[0] - ball->radius;
        ball->y_velocity = -ball->y_velocity * ball->elasticity;
        if (fabs(ball->y_velocity) < 0.321) ball->y_velocity = 0.0;

        double friction_acc = ball->x_friction / ball->mass;
 
        if (ball->x_velocity > 0.0) {
            ball->x_velocity -= friction_acc;
            if (ball->x_velocity < 0.0) ball->x_velocity = 0.0;
        }
        else if (ball->x_velocity < 0.0) {
            ball->x_velocity += friction_acc;
            if (ball->x_velocity > 0.0) ball->x_velocity = 0.0;
        }
    }
    if (ball->y <= ball->radius) {
        ball->y = ball->radius;
        ball->y_velocity = -ball->y_velocity * ball->elasticity;
    }

    ball->x += ball->x_velocity;

    // Wall collision
    if (ball->x >= screen[1] - ball->radius) {
        ball->x = 2 * (screen[1] - ball->radius) - ball->x;
        ball->x_velocity = -ball->x_velocity;
    }
    if (ball->x <= ball->radius) {
        ball->x = 2 * ball->radius - ball->x;
        ball->x_velocity = -ball->x_velocity;
    }
    printf("x: %f, y: %f, x_velocity: %f, y_velocity: %f\n, x_friction: %f", ball->x, ball->y, ball->x_velocity, ball->y_velocity, ball->x_friction);
}

bool BallBounce(Ball *ball1, Ball *ball2) {
    double dx = ball2->x - ball1->x;
    double dy = ball2->y - ball1->y;
    double distance = sqrt(dx * dx + dy * dy);
    double min_distance = ball1->radius + ball2->radius;

    if (distance < min_distance)
    {
        //pass
    }
}

