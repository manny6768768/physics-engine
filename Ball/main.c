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
    bool is_dragging;
    double prev_x;
    double prev_y;
    double x_friction;
} Ball;


void UpdateBall(Ball *ball, double gravity, double *screen);
void BallBounce(Ball *ball1, Ball *ball2);
void DrawVectorBall(Ball ball);
bool MouseBall(Ball *ball, Vector2 mouse_pos, float dt);

Vector2 original_ball_pos;


int main(void)
{
    const int screen_x = 1920;
    const int screen_y = 1080;
    const int num_balls = 4;

    double gravity = 0.4;

    Ball ball1 = {800.0, 200.0, 30.0, 0.80, -15.0, -5.0, 2.0, false, 0, 0}; // missing friction is 0
    Ball ball2 = {400.0, 200.0, 25.0, 0.85, -10.0, -2.5, 1.0, false, 0, 0}; // missing friction is 0
    Ball ball3 = {200.0, 200.0, 35.0, 0.75, -5.0, 1.0, 2.5, false, 0, 0}; // missing friction is 0
    Ball ball4 = {1000.0, 200.0, 15.0, 0.25, -20.0, -10.0, 1000.0, false, 0, 0}; // missing friction is 0

    Ball balls[] = {ball1, ball2, ball3, ball4};

    balls[0].x_friction = gravity * balls[0].mass * 0.50;
    balls[1].x_friction = gravity * balls[1].mass * 0.05;
    balls[2].x_friction = gravity * balls[2].mass * 0.01;
    balls[3].x_friction = gravity * balls[3].mass * 0.9999;

    InitWindow(screen_x, screen_y, "physics engine");
    ToggleFullscreen();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {   // Update ball velocity
        for (int i = 0; i < num_balls; i++) {
            float dt = GetFrameTime();
            if (!MouseBall(&balls[i], GetMousePosition(), dt)) {
            UpdateBall(&balls[i], gravity, (double[]){screen_y, screen_x});
            }
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
        DrawCircle(balls[3].x, balls[3].y, balls[3].radius, BLACK);
        DrawVectorBall(balls[0]);
        DrawVectorBall(balls[1]);
        DrawVectorBall(balls[2]);
        DrawVectorBall(balls[3]);
        EndDrawing();
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

void DrawVectorBall(Ball ball) {
    // Implementation for drawing the ball vector
    DrawText(TextFormat("vx: %.2f", ball.x_velocity), ball.x + ball.radius*1.5, ball.y - 20, 10, DARKGRAY);
    DrawText(TextFormat("vy: %.2f", ball.y_velocity), ball.x + ball.radius*1.5, ball.y - 10, 10, DARKGRAY);
    DrawText(TextFormat("Velocity: %.2f", sqrt(ball.x_velocity * ball.x_velocity + ball.y_velocity * ball.y_velocity)), ball.x + ball.radius*1.5, ball.y, 10, DARKGRAY);
    if (ball.x_velocity == 0 && ball.y_velocity == 0) return; 
    if (ball.is_dragging) return; // Don't draw velocity vector while dragging
    Vector2 velocity_vector = {ball.x_velocity, ball.y_velocity};
    DrawLine(ball.x, ball.y, ball.x + velocity_vector.x * 5, ball.y + velocity_vector.y * 5, BLACK);
}