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
void BallBounce(Ball *ball1, Ball *ball2);

int main(void)
{
    const int screen_x = 1200;
    const int screen_y = 850;
    const int num_balls = 3;

    double gravity = 0.4;

    Ball ball1 = {800.0, 200.0, 30.0, 0.80, -15.0, -5.0, 2.0}; // missing friction
    Ball ball2 = {400.0, 200.0, 25.0, 0.85, -10.0, -2.5, 1.0}; // missing friction
    Ball ball3 = {200.0, 200.0, 35.0, 0.75, -5.0, 1.0, 2.5}; // missing friction

    Ball balls[] = {ball1, ball2, ball3};

    balls[0].x_friction = gravity * balls[0].mass * 0.50;
    balls[1].x_friction = gravity * balls[1].mass * 0.05;
    balls[2].x_friction = gravity * balls[2].mass * 0.01;


    InitWindow(screen_x, screen_y, "physics engine");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {   // Update ball velocity
        for (int i = 0; i < num_balls; i++) {
            UpdateBall(&balls[i], gravity, (double[]){screen_y, screen_x});
        }
        // Handle ball collisions
        for (int i = 0; i < num_balls; i++) {
            for (int j = i + 1; j < num_balls; j++) {
                BallBounce(&balls[i], &balls[j]);
            }
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Update ball position
        DrawCircle(balls[0].x, balls[0].y, balls[0].radius, RED);
        DrawCircle(balls[1].x, balls[1].y, balls[1].radius, GREEN);
        DrawCircle(balls[2].x, balls[2].y, balls[2].radius, BLUE);
        EndDrawing();
    }
}

// Update ball position and velocity, handle collisions with floor, ceiling, and walls
void UpdateBall(Ball *ball, double gravity, double *screen) {
    ball->y_velocity += gravity;
    ball->y += ball->y_velocity;

    // Floor/Ceiling collision
    if (ball->y >= screen[0] - ball->radius) 
    {
        ball->y = screen[0] - ball->radius;
        ball->y_velocity = -ball->y_velocity * ball->elasticity;
        if (fabs(ball->y_velocity) < 0.341) ball->y_velocity = 0.0;

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

// Handle collision between two balls
void BallBounce(Ball *ball1, Ball *ball2) {
    double dx = ball2->x - ball1->x;
    double dy = ball2->y - ball1->y;
    double distance = sqrt(dx * dx + dy * dy);

    if (distance < 1e-6) return;

    double min_distance = ball1->radius + ball2->radius;

    if (distance < min_distance)
    {
        Vector2 n = {dx / distance, dy / distance};
        Vector2 rel_vel = {
            ball2->x_velocity - ball1->x_velocity,
            ball2->y_velocity - ball1->y_velocity
        };

        double rel_vel_n = rel_vel.x * n.x + rel_vel.y * n.y;

        double overlap = min_distance - distance; // how much the balls are overlapping

        double total_mass = ball1->mass + ball2->mass;

        //overlap correction
        ball1->x -= n.x * overlap * (ball2->mass / total_mass);
        ball1->y -= n.y * overlap * (ball2->mass / total_mass);

        ball2->x += n.x * overlap * (ball1->mass / total_mass);
        ball2->y += n.y * overlap * (ball1->mass / total_mass);

        if (rel_vel_n > 0) return;

        double e = fmin(ball1->elasticity, ball2->elasticity);

        double j = -(1 + e) * rel_vel_n / (1 / ball1->mass + 1 / ball2->mass);

        ball1->x_velocity -= j * n.x / ball1->mass;
        ball1->y_velocity -= j * n.y / ball1->mass;

        ball2->x_velocity += j * n.x / ball2->mass;
        ball2->y_velocity += j * n.y / ball2->mass;
    }
}

