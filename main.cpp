#include "piese.cpp"
#include <stdio.h>
#include <string.h>
#include <winbgim.h>
#include <cmath>

#define WIDTH 1280
#define HEIGHT 720
#define IMPLEMENTED_COMPONENTS 3

component componentVector[50];
int componentCount = -1;
component componentMenu[IMPLEMENTED_COMPONENTS];

enum selState
{
    NONE = 0,
    SEL_MENU_ITEM = 1
//    SEL_COMPONENT = 2
};

struct mouseState
{
    float x;
    float y;
    selState state = NONE;
    int selection;
};

mouseState mouseTracker = {600, 600, NONE, -1};

bool doesCollideWithBoundary(float x, float y, component &comp)
{
    if(x > comp.x + comp.boundary.x_TL && y > comp.y + comp.boundary.y_TL && x < comp.x + comp.boundary.x_BR && y < comp.y + comp.boundary.y_BR)
        return true;
    return false;
}

bool doesCollideWithComponent(component &desired, component &existing){
    if(desired.x + desired.boundary.x_TL > existing.x + existing.boundary.x_BR || existing.x + existing.boundary.x_TL > desired.x + desired.boundary.x_BR)
        return false;
    if(existing.y + existing.boundary.y_TL > desired.y + desired.boundary.y_BR || desired.y + desired.boundary.y_TL > existing.y + existing.boundary.y_BR)
        return false;
    return true;
}

void initializeComponent(component &comp, float x, float y)
{
    char temp;
    char filename[20] = "";
    char s[MAX_NAME_LENGTH] = "";
    strcpy(filename, comp.name);
    strcat(filename, ".dat");    //Initializare nume fisier
    FILE *input = fopen(filename, "rt");
    if(input == NULL)
    {
        printf("Fisier inexistent!: %s", filename);
        exit(1);
    }
    comp.x = x;
    comp.y = y;
    fscanf(input, "%d", &comp.instructionCount);     //Numar de instructiuni de desenare
    for( int i = 0; i < comp.instructionCount; i++) //Instructiunile
    {
        fscanf(input, "%c", &temp);
        fscanf(input, "%c/%f/%f/%f/%f/", &comp.drawingGuide[i].type, &comp.drawingGuide[i].x1, &comp.drawingGuide[i].y1, &comp.drawingGuide[i].x2, &comp.drawingGuide[i].y2);
        //printf("Read %c/%f/%f/%f/%f\n", comp.drawingGuide[i].type, comp.drawingGuide[i].x1, comp.drawingGuide[i].y1, comp.drawingGuide[i].x2, comp.drawingGuide[i].y2);
    }
    fscanf(input, "%s", s);                         //Citim o linie, aia cu "Joints"
    fscanf(input, "%d", &comp.jointCount);           //Numarul de conexiuni
    //printf("Conexiuni: %d\n", comp.jointCount);
    for(int i = 0; i < comp.jointCount; i++)        //Conexiunile
    {
        fscanf(input, "%c", &temp);
        fscanf(input, "/%f/%f/", &comp.solderJoints[i].x, &comp.solderJoints[i].y);
    }
    fscanf(input, "%s", s);                         //Linia cu "Boundary"
    fscanf(input, "%c", &temp);
    fscanf(input, "/%f/%f/%f/%f/", &comp.boundary.x_TL, &comp.boundary.y_TL, &comp.boundary.x_BR, &comp.boundary.y_BR);   //Citim punctul din stanga-sus si dreapta-jos (extremele)
    printf("Read boundary for %s, is %f/%f/%f/%f\n", comp.name, comp.boundary.x_TL, comp.boundary.y_TL, comp.boundary.x_BR, comp.boundary.y_BR);
    fclose(input);
}

void initializeComponentIndex(component componentIndex[IMPLEMENTED_COMPONENTS])
{
    //---------------------------------------
    //Nu uita sa schimbi IMPLEMENTED_COMPONENTS dupa ce adaugi un component!
    strcpy(componentIndex[0].name, "diode");
    strcpy(componentIndex[1].name, "capacitor");
    strcpy(componentIndex[2].name, "amplificator_operational");
    //---------------------------------------

    float yPos = HEIGHT/IMPLEMENTED_COMPONENTS/2.0;

    for(int i = 0; i<IMPLEMENTED_COMPONENTS; i++)
    {
        initializeComponent(componentIndex[i], 35, yPos);
        yPos += HEIGHT/IMPLEMENTED_COMPONENTS;
    }
}

int placeComponent(component menu[IMPLEMENTED_COMPONENTS], component storage[], float x, float y, int index)  //Takes: menu, where you want to store the component, x coord, y coord, index in menu
{
    if(componentCount >= 50)
    {
        return -1;  //No more space
    }
    component temp;
    temp.x = x;
    temp.y = y;
    temp.boundary = menu[index].boundary;
    for(int i = 0; i <= componentCount; i++)
    {
        //if(doesCollideWithBoundary(x, y, storage[i]))
        if(doesCollideWithComponent(temp, storage[i]))
        {
            return -2; //Overlap
        }
    }

    storage[++componentCount] = menu[index];
    storage[componentCount].x = x;
    storage[componentCount].y = y;
    drawComponent(storage[componentCount]);
    return 0;   //Success
}

void drawMenu(component componentMenu[IMPLEMENTED_COMPONENTS])
{
    drawRectangle(0, 0, 70, 720);
    for( int i = 0; i < IMPLEMENTED_COMPONENTS; i++)
    {
        line(0, HEIGHT/IMPLEMENTED_COMPONENTS * i, 70, HEIGHT/IMPLEMENTED_COMPONENTS * i);
    }

    for(int i = 0; i < IMPLEMENTED_COMPONENTS; i++)
    {
        printf("Drawing %s with %d instructions\n", componentMenu[i].name, componentMenu[i].instructionCount);
        drawComponent(componentMenu[i]);
    }
}

collisionBox exitButton = {1200, 1, 1279, 80};
void drawExitButton()
{
    drawRectangle(exitButton.x_TL, exitButton.y_TL, exitButton.x_BR, exitButton.y_BR);
    line(1200, 1, 1279, 80);
    line(1200, 80, 1279, 1);
}



bool iWantToLeave = false;
void handleClick()
{
    if(ismouseclick(WM_LBUTTONDOWN))
    {
        mouseTracker.x = mousex();
        mouseTracker.y = mousey();
        clearmouseclick(WM_LBUTTONDOWN);

        //The exit button (TM)(all rights reserved)(the most used one)
        if(mouseTracker.x >= exitButton.x_TL && mouseTracker.y >= exitButton.y_TL && mouseTracker.x <= exitButton.x_BR && mouseTracker.y <= exitButton.y_BR)
            iWantToLeave = true;
        //The menu
        else if(mouseTracker.x > 0 && mouseTracker.x < 69)
        {
            mouseTracker.state = SEL_MENU_ITEM;
            mouseTracker.selection = (mouseTracker.y / ((float)HEIGHT/IMPLEMENTED_COMPONENTS));
            printf("Mouse selection is %d, at %f/%f\n", mouseTracker.selection, mouseTracker.x, mouseTracker.y);
            //printf("Doing %f/%d gives %f", y, HEIGHT/IMPLEMENTED_COMPONENTS, y/((float)HEIGHT/IMPLEMENTED_COMPONENTS));
        }
        //Plasare piese (for now)
        else if(mouseTracker.x > 70)
        {
            if(mouseTracker.state == SEL_MENU_ITEM)      //mouseTracker stie pozitia si indexul piesei dorite (indexul in meniu)
            {
                int ret = placeComponent(componentMenu, componentVector, mouseTracker.x, mouseTracker.y, mouseTracker.selection);
                if(ret == -1)
                {
                    printf("No more space for components!\n");
                }
                else if(ret == -2)
                {
                    printf("Overlap with another piece!");
                }
                mouseTracker.state = NONE;
            }

        }
    }
}



int main()
{
    initwindow(WIDTH, HEIGHT);

    drawExitButton();

    initializeComponentIndex(componentMenu);
    drawMenu(componentMenu);

    do
    {
        handleClick();

    }
    while(!iWantToLeave);


    closegraph();
    return 0;
}
