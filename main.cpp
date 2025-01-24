#include"raylib.h"

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define screenWidth 1280
#define screenHeight 720


#define MAX_BUILDINGS                    15
#define MAX_EXPLOSIONS                  200
#define MAX_PLAYERS                       2

#define BUILDING_RELATIVE_ERROR          30        // Building size random range %
#define BUILDING_MIN_RELATIVE_HEIGHT     20        // Minimum height in % of the screenHeight
#define BUILDING_MAX_RELATIVE_HEIGHT     60        // Maximum height in % of the screenHeight
#define BUILDING_MIN_GRAYSCALE_COLOR    120        // Minimum gray color for the buildings
#define BUILDING_MAX_GRAYSCALE_COLOR    200        // Maximum gray color for the buildings

#define MIN_PLAYER_POSITION               5        // Minimum x position %
#define MAX_PLAYER_POSITION              20        // Maximum x position %

#define GRAVITY                       9.81f
#define DELTA_FPS                        60

typedef struct Player {
    Vector2 position;
    Vector2 size;

    Vector2 previousPoint;
    int previousAngle, previousPower;

    Vector2 aimingPoint;
    int aimingAngle;
    int aimingPower;

    Vector2 impactPoint;

    bool isLeftTeam;                
    bool isPlayer;                 
    bool isAlive;
} Player;

typedef struct Building {
    Rectangle rectangle;
    Color color;
} Building;

typedef struct Explosion {
    Vector2 position;
    int radius;
    bool active;
} Explosion;

typedef struct Ball {
    Vector2 position;
    Vector2 speed;
    int radius;
    bool active;
} Ball;

static bool gameOver = false;
static bool pause = false;
static Player player[MAX_PLAYERS]; 
static Building building[MAX_BUILDINGS];    
static Explosion explosion[MAX_EXPLOSIONS]; 
static Ball ball; 

static int playerTurn = 0;
static bool ballOnAir = false;

static void InitGame();         // Initialize game
static void UpdateGame();       // Update game (one frame)
static void DrawGame();         // Draw game (one frame)
static void UpdateDrawFrame();  // Update and Draw (one frame)
static void LeftScore();
static void RightScore();

static void InitBuildings();
static void InitPlayers();
static bool UpdatePlayer(int playerTurn);
static bool UpdateBall(int playerTurn);

/// Loading Screen
int loading =1;
int frames = 0;
float loadWidth = 0;


int main()
{
    InitWindow(screenWidth, screenHeight, "Arc Rivals");
    InitAudioDevice();

    Sound fight = LoadSound("assets/fight.wav");
       
    InitGame();

    SetTargetFPS(DELTA_FPS);   /// frames per second
    while(1)
    {
        BeginDrawing();
        ClearBackground(BLACK);
        if(loading)
        {
            loadWidth += 10.67;
            frames++;
            DrawText("ARC RIVALS", 1280/2-MeasureText("ARC RIVALS", 50)/2, 100, 50, WHITE);
            DrawText("Loading...", screenWidth/2-MeasureText("Loading...", 30)/2, 250, 30, RED);
            DrawRectangle(0,335,loadWidth, 50, RED);
            if(frames>=120) {PlaySound(fight);break;}
        }
        EndDrawing();
    }
    
    while (!WindowShouldClose())
    {
        
        UpdateDrawFrame();
    }
        
    UnloadSound(fight);
    CloseAudioDevice();  
    CloseWindow();   
    system("./gui");   
    return 0;
}

void InitGame()
{
    ball.radius = 10;
    ballOnAir = false;
    ball.active = false;

    InitBuildings();
    InitPlayers();

    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        explosion[i].radius = 30;
        explosion[i].active = false;
    }
}

void UpdateGame()
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            if (!ballOnAir) ballOnAir = UpdatePlayer(playerTurn); // If we are aiming
            else
            {
                if (UpdateBall(playerTurn))                       // If collision
                {
                    bool leftTeamAlive = false;
                    bool rightTeamAlive = false;

                    for (int i = 0; i < MAX_PLAYERS; i++)
                    {
                        if (player[i].isAlive)
                        {
                            if (player[i].isLeftTeam) leftTeamAlive = true;
                            if (!player[i].isLeftTeam) rightTeamAlive = true;
                        }
                    }

                    if (leftTeamAlive && rightTeamAlive)
                    {
                        ballOnAir = false;
                        ball.active = false;

                        playerTurn++;

                        if (playerTurn == MAX_PLAYERS) playerTurn = 0;
                    }
                    else
                    {
                        gameOver = true;
                    }
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

void DrawGame()
{

    BeginDrawing();

        ClearBackground(CLITERAL(Color){72, 179, 255, 255});
        if (!gameOver)
        {
            // Draw buildings
            for (int i = 0; i < MAX_BUILDINGS; i++) DrawRectangleRec(building[i].rectangle, building[i].color);

            // Draw explosions
            for (int i = 0; i < MAX_EXPLOSIONS; i++)
            {
                if (explosion[i].active) DrawCircle(explosion[i].position.x, explosion[i].position.y, explosion[i].radius, CLITERAL(Color){72, 179, 255, 255});
            }

            // Draw players
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (player[i].isLeftTeam) DrawRectangle(player[i].position.x - player[i].size.x/2, player[i].position.y - player[i].size.y/2, player[i].size.x, player[i].size.y, BLUE);
                else DrawRectangle(player[i].position.x - player[i].size.x/2, player[i].position.y - player[i].size.y/2, player[i].size.x, player[i].size.y, RED);
            }

            // Draw ball
            if (ball.active) DrawCircle(ball.position.x, ball.position.y, ball.radius, MAROON);

            // Draw the angle and the power of the aim, and the previous ones
            if (!ballOnAir)
            {
                // Draw aim
                if (player[playerTurn].isLeftTeam)
                {
                    DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
                                 (Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
                                 player[playerTurn].aimingPoint, DARKBLUE);
                }
                else
                {
                    DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
                                 (Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
                                 player[playerTurn].aimingPoint, MAROON);
                }
            }

            if (pause) DrawText("GAME PAUSED. Press P to continue", screenWidth/2 - MeasureText("GAME PAUSED. Press P to continue", 40)/2, 100, 40, WHITE);
        }
        else 
        {
            if(player[0].isAlive) LeftScore();
            else RightScore();
            DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, WHITE);
            DrawText("PRESS [ESC] TO EXIT", GetScreenWidth()/2 - MeasureText("PRESS [ESC] TO EXIT", 20)/2, GetScreenHeight()/2 + 20, 20, WHITE);
        }
    
    EndDrawing();
}

void UpdateDrawFrame()
{
    UpdateGame();
    DrawGame();
}

static void InitBuildings()
{
    int currentWidth = 0;

    float relativeWidth = 100/(100 - BUILDING_RELATIVE_ERROR); 
    float buildingWidthMean = (screenWidth*relativeWidth/MAX_BUILDINGS) + 1;       // We add one to make sure we will cover the whole screen.

    int currentHeighth = 0;
    int buildingColor;

    for (int i = 0; i < MAX_BUILDINGS; i++)
    {
        building[i].rectangle.x = currentWidth;
        building[i].rectangle.width = GetRandomValue(buildingWidthMean*(100 - BUILDING_RELATIVE_ERROR/2)/100 + 1, buildingWidthMean*(100 + BUILDING_RELATIVE_ERROR)/100);



        currentWidth += building[i].rectangle.width;

        currentHeighth = GetRandomValue(BUILDING_MIN_RELATIVE_HEIGHT, BUILDING_MAX_RELATIVE_HEIGHT);
        building[i].rectangle.y = screenHeight - (screenHeight*currentHeighth/100);
        building[i].rectangle.height = screenHeight*currentHeighth/100 + 1;

        buildingColor = GetRandomValue(BUILDING_MIN_GRAYSCALE_COLOR, BUILDING_MAX_GRAYSCALE_COLOR);
        building[i].color = (Color){ buildingColor, buildingColor, buildingColor, 255 };
    }
}

static void InitPlayers()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        player[i].isAlive = true;

        // Decide the team of this player
        if (i % 2 == 0) player[i].isLeftTeam = true;
        else player[i].isLeftTeam = false;

        // Now there is no AI
        player[i].isPlayer = true;

        // Set size, by default by now
        player[i].size = (Vector2){ 40, 40 };

        // Set position
        if (player[i].isLeftTeam) player[i].position.x = GetRandomValue(screenWidth*MIN_PLAYER_POSITION/100, screenWidth*MAX_PLAYER_POSITION/100);
        else player[i].position.x = screenWidth - GetRandomValue(screenWidth*MIN_PLAYER_POSITION/100, screenWidth*MAX_PLAYER_POSITION/100);

        for (int j = 0; j < MAX_BUILDINGS; j++)
        {
            if (building[j].rectangle.x > player[i].position.x)
            {   
                // Set the player in the center of the building
                player[i].position.x = building[j-1].rectangle.x + building[j-1].rectangle.width/2;
                // Set the player at the top of the building 
                player[i].position.y = building[j-1].rectangle.y - player[i].size.y/2;
                break;
            }
        }

        // Initialize aimPoint and previousPoint   
        player[i].aimingPoint = player[i].position;
        player[i].previousPoint = player[i].position;
    }
}

static bool UpdatePlayer(int playerTurn)
{
    // If we are aiming at the firing quadrant, we calculate the angle
    if (GetMousePosition().y <= player[playerTurn].position.y)
    {
        // Left team
        if (player[playerTurn].isLeftTeam && (GetMousePosition().x >= player[playerTurn].position.x))
        {
            // Distance (calculating the fire power)
            player[playerTurn].aimingPower = sqrt(pow(player[playerTurn].position.x - GetMousePosition().x, 2) + pow(player[playerTurn].position.y - GetMousePosition().y, 2));
            // Calculates the angle via arcsin
            player[playerTurn].aimingAngle = asin((player[playerTurn].position.y - GetMousePosition().y)/player[playerTurn].aimingPower)*RAD2DEG;
            // Point of the screen we are aiming at
            player[playerTurn].aimingPoint = GetMousePosition();

            // Ball fired
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                player[playerTurn].previousPoint = player[playerTurn].aimingPoint;
                player[playerTurn].previousPower = player[playerTurn].aimingPower;
                player[playerTurn].previousAngle = player[playerTurn].aimingAngle;
                ball.position = player[playerTurn].position;

                return true;
            }
        }
        // Right team
        else if (!player[playerTurn].isLeftTeam && GetMousePosition().x <= player[playerTurn].position.x)
        {
            /// Calculating Power
            player[playerTurn].aimingPower = sqrt(pow(player[playerTurn].position.x - GetMousePosition().x, 2) + pow(player[playerTurn].position.y - GetMousePosition().y, 2));
            player[playerTurn].aimingAngle = asin((player[playerTurn].position.y - GetMousePosition().y)/player[playerTurn].aimingPower)*RAD2DEG;   /// Calculating angle
            player[playerTurn].aimingPoint = GetMousePosition();    /// Aimpoint: Mouse Position

            // Ball fired
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                player[playerTurn].previousPoint = player[playerTurn].aimingPoint;
                player[playerTurn].previousPower = player[playerTurn].aimingPower;
                player[playerTurn].previousAngle = player[playerTurn].aimingAngle;
                ball.position = player[playerTurn].position;

                return true;
            }
        }
        else
        {
            player[playerTurn].aimingPoint = player[playerTurn].position;
            player[playerTurn].aimingPower = 0;
            player[playerTurn].aimingAngle = 0;
        }
    }
    else
    {
        player[playerTurn].aimingPoint = player[playerTurn].position;
        player[playerTurn].aimingPower = 0;
        player[playerTurn].aimingAngle = 0;
    }

    return false;
}

static bool UpdateBall(int playerTurn)
{
    static int explosionNumber = 0;

    // Activate ball
    if (!ball.active)
    {
        if (player[playerTurn].isLeftTeam)
        {
            ball.speed.x = cos(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
            ball.speed.y = -sin(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
            ball.active = true;
        }
        else
        {
            ball.speed.x = -cos(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
            ball.speed.y = -sin(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
            ball.active = true;
        }
    }

    ball.speed.y += GRAVITY/DELTA_FPS;
    ball.position.x += ball.speed.x;
    ball.position.y += ball.speed.y;

    // Collision
    if (ball.position.x + ball.radius < 0) return true;
    else if (ball.position.x - ball.radius > screenWidth) return true;
    else
    {
        // Player collision
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (CheckCollisionCircleRec(ball.position, ball.radius,  (Rectangle){ player[i].position.x - player[i].size.x/2, player[i].position.y - player[i].size.y/2,
                                                                                  player[i].size.x, player[i].size.y }))
            {
                // We can't hit ourselves
                if (i == playerTurn) return false;
                else
                {
                    // We set the impact point
                    player[playerTurn].impactPoint.x = ball.position.x;
                    player[playerTurn].impactPoint.y = ball.position.y;

                    // We destroy the player
                    player[i].isAlive = false;
                    return true;
                }
            }
        }

        // Building collision
        for (int i = 0; i < MAX_EXPLOSIONS; i++)
        {
            if (CheckCollisionCircles(ball.position, ball.radius, explosion[i].position, explosion[i].radius - ball.radius))
            {
                return false;
            }
        }

        for (int i = 0; i < MAX_BUILDINGS; i++)
        {
            if (CheckCollisionCircleRec(ball.position, ball.radius, building[i].rectangle))
            {
                // We set the impact point
                player[playerTurn].impactPoint.x = ball.position.x;
                player[playerTurn].impactPoint.y = ball.position.y;

                // We create an explosion
                explosion[explosionNumber].position = player[playerTurn].impactPoint;
                explosion[explosionNumber].active = true;
                explosionNumber++;

                return true;
            }
        }
    }

    return false;
}
static void LeftScore()
{
    DrawText("Player 1 Wins!", screenWidth/2-MeasureText("Player 1 Wins!", 40)/2, 100, 40, WHITE);
}
static void RightScore()
{
    DrawText("Player 2 Wins!", screenWidth/2-MeasureText("Player 1 Wins!", 40)/2, 100, 40, WHITE);
}