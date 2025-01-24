#include<cstdlib>
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"


int main()
{
    InitWindow(1280, 720, "Main Menu");
    SetTargetFPS(60);

    bool showMessageBoxPlay = false;
    bool showMessageBoxExit = false;
    bool showMessageBoxAbout = false;

    while (!WindowShouldClose())
    {
        /// Draw
        BeginDrawing();

            ClearBackground(RED);
            GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
            /// Title (ARC RIVALS)
            DrawText("ARC RIVALS", 1280/2-MeasureText("ARC RIVALS", 60)/2, 50, 60, BLACK);

            /// Play Button
            if (GuiButton((Rectangle){50,170,1180,100}, "PLAY GAME")) showMessageBoxPlay = true;
            /// About Us Button
            if (GuiButton((Rectangle){50,320,1180,100}, "ABOUT US")) showMessageBoxAbout = true;
            /// Exit Button
            if (GuiButton((Rectangle){50,470,1180,100}, "EXIT")) showMessageBoxExit = true;

            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            /// Play Button Interaction
            if (showMessageBoxPlay)
            {
                int result = GuiMessageBox((Rectangle){ 170, 80, 940, 560 },
                    "GAME START", "DO YOU WISH TO PLAY?", "YES; NO");

                if(result==1) 
                {
                        CloseWindow();
                        system("./main");
                        showMessageBoxPlay = false;
                }                
                else if (result == 0 || result==2) showMessageBoxPlay = false;
            }

            /// Exit Button Interaction
            if (showMessageBoxExit)
            {
                int exit = GuiMessageBox((Rectangle){170, 80, 940, 560}, "EXIT", "DO YOU REALLY WISH TO EXIT?", "Yes;Cancel");

                if (exit==1)
                {
                        break;
                }
                else if (exit==0 || exit==2) showMessageBoxExit = false;
            }
            /// AboutUS Button Interaction
            if (showMessageBoxAbout)
            {
                int about = GuiMessageBox((Rectangle){170, 80, 940, 560}, "About Us", 
            TextFormat("We are three CSE students exploring \n\n\
game development through a 2D game project using \n\n\
RAYLIB and C and a bit of C++. While the core code was sourced \n\n\
externally, we enhanced the gameplay and crafted \n\n\
the UI ourselves. This project helped us gain \n\n\
hands-on experience in programming, UI/UX design, \n\n\
and teamwork, laying a solid foundation for \n\n\
future endeavors in software development.", 1), 
                            "OK");

                if (about>=0)
                {
                    showMessageBoxAbout = false;
                }
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}