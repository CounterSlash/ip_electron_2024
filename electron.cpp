#include "piese.cpp"
#include <stdio.h>
#include <cmath>

#define WIDTH 1280
#define HEIGHT 720
#define IMPLEMENTED_COMPONENTS 7

component componentVector[50];
int componentCount = -1;
component componentMenu[IMPLEMENTED_COMPONENTS];
int transientComponentID = 0;
connection connectionVector[50];
int connectionCount = -1;

enum selState
{
    NONE = 0,
    SEL_MENU_ITEM = 1,
    //    SEL_COMPONENT = 2
    DO_DELETE = 3,
    DO_RESIZE = 4,
    DO_ROTATE = 5,
    DO_MOVE = 6,
    DO_CREATE_CONNECTION = 7
};

struct mouseState
{
    float x;
    float y;
    selState state = NONE;
    int selection;
    int joint;
};

mouseState mouseTracker = {600, 600, NONE, -1};

bool doesCollideWithBoundary(float x, float y, collisionBox bound)
{
    if (x >= bound.x_TL && y >= bound.y_TL && x <= bound.x_BR && y <= bound.y_BR)
        return true;
    return false;
}

bool doesCollideWithComponent(component &desired, component &existing)
{
    if (desired.x + desired.boundary.x_TL > existing.x + existing.boundary.x_BR || existing.x + existing.boundary.x_TL > desired.x + desired.boundary.x_BR)
        return false;
    if (existing.y + existing.boundary.y_TL > desired.y + desired.boundary.y_BR || desired.y + desired.boundary.y_TL > existing.y + existing.boundary.y_BR)
        return false;
    return true;
}

int doesCollideWithJoint(float x, float y, component &comp)
{
    for (int i = 0; i < comp.jointCount; i++)
    {
        if (pow(x - (comp.x + comp.solderJoints[i].x*comp.zoom), 2) +
                pow(y - (comp.y + comp.solderJoints[i].y*comp.zoom), 2) <=
            pow(JOINT_RADIUS*comp.zoom, 2))
        {
            // printf("Collided with joint!\n");
            return i;
        }
    }
    return -1;
}

void initializeComponent(component &comp, float x, float y)
{
    char temp;
    char filename[MAX_NAME_LENGTH] = "";
    char s[MAX_NAME_LENGTH] = "";
    strcpy(filename, comp.name);
    strcat(filename, ".dat"); // Initializare nume fisier
    FILE *input = fopen(filename, "rt");
    if (input == NULL)
    {
        printf("Fisier inexistent!: %s", filename);
        exit(1);
    }
    comp.x = x;
    comp.y = y;
    comp.zoom = 1;
    fscanf(input, "%d", &comp.instructionCount);    // Numar de instructiuni de desenare
    for (int i = 0; i < comp.instructionCount; i++) // Instructiunile
    {
        fscanf(input, "%c", &temp);
        fscanf(input, "%c/%f/%f/%f/%f/", &comp.drawingGuide[i].type, &comp.drawingGuide[i].x1, &comp.drawingGuide[i].y1, &comp.drawingGuide[i].x2, &comp.drawingGuide[i].y2);
        // printf("Read %c/%f/%f/%f/%f\n", comp.drawingGuide[i].type, comp.drawingGuide[i].x1, comp.drawingGuide[i].y1, comp.drawingGuide[i].x2, comp.drawingGuide[i].y2);
    }
    fscanf(input, "%s", s);                // Citim o linie, aia cu "Joints"
    fscanf(input, "%d", &comp.jointCount); // Numarul de conexiuni
    // printf("Conexiuni: %d\n", comp.jointCount);
    for (int i = 0; i < comp.jointCount; i++) // Conexiunile
    {
        fscanf(input, "%c", &temp);
        fscanf(input, "/%f/%f/", &comp.solderJoints[i].x, &comp.solderJoints[i].y);
    }
    fscanf(input, "%s", s); // Linia cu "Boundary"
    fscanf(input, "%c", &temp);
    fscanf(input, "/%f/%f/%f/%f/", &comp.boundary.x_TL, &comp.boundary.y_TL, &comp.boundary.x_BR, &comp.boundary.y_BR); // Citim punctul din stanga-sus si dreapta-jos (extremele)
    printf("Read boundary for %s, is %f/%f/%f/%f\n", comp.name, comp.boundary.x_TL, comp.boundary.y_TL, comp.boundary.x_BR, comp.boundary.y_BR);
    fclose(input);
}

void initializeComponentIndex(component componentIndex[IMPLEMENTED_COMPONENTS])
{
    //---------------------------------------
    // Nu uita sa schimbi IMPLEMENTED_COMPONENTS dupa ce adaugi un component!
    strcpy(componentIndex[0].name, "diode");
    strcpy(componentIndex[1].name, "capacitor");
    strcpy(componentIndex[2].name, "amplificator_operational");
    strcpy(componentIndex[3].name, "baterie");
    strcpy(componentIndex[4].name, "polarizator");
    strcpy(componentIndex[5].name, "TranzistorNPN");
    strcpy(componentIndex[6].name, "dioda_zenner");
    //---------------------------------------

    float yPos = HEIGHT / IMPLEMENTED_COMPONENTS / 2.0;

    for (int i = 0; i < IMPLEMENTED_COMPONENTS; i++)
    {
        initializeComponent(componentIndex[i], 35, yPos);
        yPos += HEIGHT / IMPLEMENTED_COMPONENTS;
    }
}

int placeComponent(component menu[IMPLEMENTED_COMPONENTS], component storage[], float x, float y, int index) // Takes: menu, where you want to store the component, x coord, y coord, index in menu
{
    if (componentCount >= 50)
        return -1; // No more space

    component temp;
    temp.x = x;
    temp.y = y;
    temp.boundary = menu[index].boundary;
    for (int i = 0; i <= componentCount; i++)
    {
        // if(doesCollideWithBoundary(x, y, storage[i]))
        if (doesCollideWithComponent(temp, storage[i]))
            return -2; // Overlap
    }
    storage[++componentCount] = menu[index];
    storage[componentCount].x = x;
    storage[componentCount].y = y;
    storage[componentCount].ID = transientComponentID;
    transientComponentID++; // Un ID unic. Va fi nevoie de el la corelatia dintre legaturi si noduri
    drawComponent(storage[componentCount]);
    return 0; // Success
}

void drawMenu(component componentMenu[IMPLEMENTED_COMPONENTS])
{
    drawRectangle(0, 0, 70, 720);
    for (int i = 0; i < IMPLEMENTED_COMPONENTS; i++)
    {
        line(0, HEIGHT / IMPLEMENTED_COMPONENTS * i, 70, HEIGHT / IMPLEMENTED_COMPONENTS * i);
    }

    for (int i = 0; i < IMPLEMENTED_COMPONENTS; i++)
    {
        // printf("Drawing %s with %d instructions\n", componentMenu[i].name, componentMenu[i].instructionCount);
        drawComponent(componentMenu[i]);
    }
}

collisionBox exitButton = {1220, 1, 1279, 60};
void drawExitButton()
{
    drawRectangle(exitButton.x_TL, exitButton.y_TL, exitButton.x_BR, exitButton.y_BR);
    line(exitButton.x_TL, exitButton.y_TL, exitButton.x_BR, exitButton.y_BR);
    line(exitButton.x_TL, exitButton.y_BR, exitButton.x_BR, exitButton.y_TL);
}

collisionBox rightMenu = {1220, 270, 1280, 510};
void drawRightMenu()
{
    // Order from top to bottom is: Delete, Rotate, Move
    rectangle(1220, 270, 1280, 510);
    line(1220, 330, 1280, 330);
    line(1220, 390, 1280, 390);
    line(1220, 450, 1280, 450);
}

void doMenuSelection()
{
    mouseTracker.state = SEL_MENU_ITEM;
    mouseTracker.selection = (mouseTracker.y / ((float)HEIGHT / IMPLEMENTED_COMPONENTS));
    printf("Mouse selection is %d, at %f/%f\n", mouseTracker.selection, mouseTracker.x, mouseTracker.y);
    // printf("Doing %f/%d gives %f", y, HEIGHT/IMPLEMENTED_COMPONENTS, y/((float)HEIGHT/IMPLEMENTED_COMPONENTS));
}

void doPlaceComponent()
{
    // mouseTracker stie pozitia si indexul piesei dorite (indexul in meniu)
    int ret = placeComponent(componentMenu, componentVector, mouseTracker.x, mouseTracker.y, mouseTracker.selection);
    if (ret == -1)
        printf("No more space for components!\n");
    else if (ret == -2)
        printf("Overlap with another piece!\n");
    mouseTracker.state = NONE;
}

void doRightMenuSelection()
{
    if (mouseTracker.y >= 270 && mouseTracker.y < 330)
        mouseTracker.state = DO_DELETE;
    if (mouseTracker.y >= 330 && mouseTracker.y < 390)
        mouseTracker.state = DO_RESIZE;
    if (mouseTracker.y >= 390 && mouseTracker.y < 450)
        mouseTracker.state = DO_ROTATE;
    if (mouseTracker.y >= 450 && mouseTracker.y < 510)
        mouseTracker.state = DO_MOVE;
}

void doDeleteComponent()
{
    if (componentCount <= -1)
    {
        printf("No components to delete, what are you doing?\n");
        mouseTracker.state = NONE;
        return;
    }

    for (int i = 0; i <= componentCount; i++)
    {
        collisionBox tempBoundary;
        tempBoundary.x_TL = componentVector[i].x + componentVector[i].boundary.x_TL;
        tempBoundary.y_TL = componentVector[i].y + componentVector[i].boundary.y_TL;
        tempBoundary.x_BR = componentVector[i].x + componentVector[i].boundary.x_BR;
        tempBoundary.y_BR = componentVector[i].y + componentVector[i].boundary.y_BR;
        if (doesCollideWithBoundary(mouseTracker.x, mouseTracker.y, tempBoundary))
        {
            for (int j = i; j < componentCount; j++) // starting from i, move all other nodes
                componentVector[j] = componentVector[j + 1];
            componentCount--;
        }
    }
    mouseTracker.state = NONE;
}

void doResizeComponent(char click_select)
{
    if (componentCount <= -1)
    {
        printf("No components to resize, what are you doing?\n");
        mouseTracker.state = NONE;
        return;
    }
    mouseTracker.state = NONE;
    for (int i = 0; i <= componentCount; i++)
    {
        collisionBox tempBoundary;
        tempBoundary.x_TL = componentVector[i].x + componentVector[i].boundary.x_TL;
        tempBoundary.y_TL = componentVector[i].y + componentVector[i].boundary.y_TL;
        tempBoundary.x_BR = componentVector[i].x + componentVector[i].boundary.x_BR;
        tempBoundary.y_BR = componentVector[i].y + componentVector[i].boundary.y_BR;
        if (doesCollideWithBoundary(mouseTracker.x, mouseTracker.y, tempBoundary))
        {
            mouseTracker.state = DO_RESIZE;
            if (click_select == 'L')
            {
                componentVector[i].boundary.x_TL /= componentVector[i].zoom;
                componentVector[i].boundary.y_TL /= componentVector[i].zoom;
                componentVector[i].boundary.x_BR /= componentVector[i].zoom;
                componentVector[i].boundary.y_BR /= componentVector[i].zoom;
                componentVector[i].zoom += 0.1;
                componentVector[i].boundary.x_TL *= componentVector[i].zoom;
                componentVector[i].boundary.y_TL *= componentVector[i].zoom;
                componentVector[i].boundary.x_BR *= componentVector[i].zoom;
                componentVector[i].boundary.y_BR *= componentVector[i].zoom;
            }
            else if (click_select == 'R')
            {
                componentVector[i].boundary.x_TL /= componentVector[i].zoom;
                componentVector[i].boundary.y_TL /= componentVector[i].zoom;
                componentVector[i].boundary.x_BR /= componentVector[i].zoom;
                componentVector[i].boundary.y_BR /= componentVector[i].zoom;
                componentVector[i].zoom -= 0.1;
                componentVector[i].boundary.x_TL *= componentVector[i].zoom;
                componentVector[i].boundary.y_TL *= componentVector[i].zoom;
                componentVector[i].boundary.x_BR *= componentVector[i].zoom;
                componentVector[i].boundary.y_BR *= componentVector[i].zoom;
            }
        }
    }
}
#include <cmath>

void rotateComponent(component &comp, float angle)
{
    // Convertim unghiul din grade în radiani
    float radian = angle * M_PI / 180.0;

    if (componentCount <= -1)
    {
        printf("No components to rotate, what are you doing?\n");
        mouseTracker.state = NONE;
        return;
    }
    mouseTracker.state = NONE;
    // Calculăm centrul componentei (centrul ar putea fi calculat în funcție de limitele acesteia)
    float centerX = comp.x + (comp.boundary.x_TL + comp.boundary.x_BR) / 2.0;
    float centerY = comp.y + (comp.boundary.y_TL + comp.boundary.y_BR) / 2.0;
    // Rotește fiecare punct de desenare
    for (int i = 0; i < comp.instructionCount; i++)
    {
        float x1 = comp.drawingGuide[i].x1;
        float y1 = comp.drawingGuide[i].y1;
        float x2 = comp.drawingGuide[i].x2;
        float y2 = comp.drawingGuide[i].y2;

        // Aplica rotația pentru fiecare punct
        comp.drawingGuide[i].x1 = centerX + (x1 - centerX) * cos(radian) - (y1 - centerY) * sin(radian);
        comp.drawingGuide[i].y1 = centerY + (x1 - centerX) * sin(radian) + (y1 - centerY) * cos(radian);
        comp.drawingGuide[i].x2 = centerX + (x2 - centerX) * cos(radian) - (y2 - centerY) * sin(radian);
        comp.drawingGuide[i].y2 = centerY + (x2 - centerX) * sin(radian) + (y2 - centerY) * cos(radian);
    }

    // Rotește fiecare punct de conexiune
    for (int i = 0; i < comp.jointCount; i++)
    {
        float x = comp.solderJoints[i].x;
        float y = comp.solderJoints[i].y;

        // Aplica rotația pentru fiecare punct de conexiune
        comp.solderJoints[i].x = (x - centerX) * cos(radian) - (y - centerY) * sin(radian) + centerX;
        comp.solderJoints[i].y = (x - centerX) * sin(radian) + (y - centerY) * cos(radian) + centerY;
    }

    // Rotește limitele (boundary) componentei
    float x_TL = comp.boundary.x_TL;
    float y_TL = comp.boundary.y_TL;
    float x_BR = comp.boundary.x_BR;
    float y_BR = comp.boundary.y_BR;

    comp.boundary.x_TL = (x_TL - centerX) * cos(radian) - (y_TL - centerY) * sin(radian) + centerX;
    comp.boundary.y_TL = (x_TL - centerX) * sin(radian) + (y_TL - centerY) * cos(radian) + centerY;
    comp.boundary.x_BR = (x_BR - centerX) * cos(radian) - (y_BR - centerY) * sin(radian) + centerX;
    comp.boundary.y_BR = (x_BR - centerX) * sin(radian) + (y_BR - centerY) * cos(radian) + centerY;
}


void doSelectJoint(int index, int chosenJoint)
{
    mouseTracker.state = DO_CREATE_CONNECTION;
    mouseTracker.selection = componentVector[index].ID;
    mouseTracker.joint = chosenJoint;
}

void doCreateConnection()
{
    for (int i = 0; i <= componentCount; i++)
    {
        if (doesCollideWithJoint(mouseTracker.x, mouseTracker.y, componentVector[i]) != -1)
        {
            if (componentVector[i].ID == mouseTracker.selection)
            {
                printf("Connections between terminals of the same component are not allowed!\n");
                mouseTracker.state = NONE;
                return;
            }
            // creating new connection
            int secondComponentJoint = doesCollideWithJoint(mouseTracker.x, mouseTracker.y, componentVector[i]);
            connectionVector[++connectionCount] = {mouseTracker.selection, componentVector[i].ID, mouseTracker.joint, secondComponentJoint};
            mouseTracker.state = NONE;
            break;
        }
    }
    mouseTracker.state = NONE;
}

bool iWantToLeave = false;
void handleClick()
{
    char click_select;
    if (ismouseclick(WM_LBUTTONDOWN))
    {
        printf("Mouse state is: %d\n", mouseTracker.state);
        mouseTracker.x = mousex();
        mouseTracker.y = mousey();
        click_select = 'L';
        clearmouseclick(WM_LBUTTONDOWN);

        // The exit button (TM)(all rights reserved)(the most used one)
        if (mouseTracker.x >= exitButton.x_TL && mouseTracker.y >= exitButton.y_TL && mouseTracker.x <= exitButton.x_BR && mouseTracker.y <= exitButton.y_BR)
            iWantToLeave = true;
        // The component menu
        else if (mouseTracker.x > 0 && mouseTracker.x < 69)
        {
            doMenuSelection();
        }
        // Meniu Dreapta
        else if (doesCollideWithBoundary(mouseTracker.x, mouseTracker.y, rightMenu))
        {
            doRightMenuSelection();
        }
        else if (mouseTracker.x > 70)
        {
            if (mouseTracker.state == SEL_MENU_ITEM)
                doPlaceComponent();
            else if (mouseTracker.state == DO_DELETE)
                doDeleteComponent();
            else if (mouseTracker.state == DO_RESIZE)
                doResizeComponent(click_select);
            else if (mouseTracker.state = DO_ROTATE) {
                    // Aplica rotația unui component selectat
                rotateComponent(componentVector[mouseTracker.selection], 90);} // Rotează cu 90 de grade
            else if (mouseTracker.state == NONE)
            {
                for (int i = 0; i <= componentCount; i++)
                {
                    int selectedJoint = doesCollideWithJoint(mouseTracker.x, mouseTracker.y, componentVector[i]);
                    if (selectedJoint != -1)
                    {
                        doSelectJoint(i, selectedJoint);
                        break;
                    }
                }
            }
            else if (mouseTracker.state == DO_CREATE_CONNECTION)
            {
                doCreateConnection();
            }

        }
    }
    else if (ismouseclick(WM_RBUTTONDOWN))
    {
        printf("Mouse state is: %d\n", mouseTracker.state);
        mouseTracker.x = mousex();
        mouseTracker.y = mousey();
        click_select = 'R';
        clearmouseclick(WM_RBUTTONDOWN);
        if (mouseTracker.x > 70)
        {
            if (mouseTracker.state == DO_RESIZE)
                doResizeComponent(click_select);
        }
    }
}

void drawFrame()
{
    cleardevice();
    drawExitButton();
    drawMenu(componentMenu);
    drawRightMenu();
    if (componentCount + 1 > 0)
    {
        for (int i = 0; i <= componentCount; i++)
            drawComponent(componentVector[i]);
    }
    if (connectionCount + 1 > 0)
    {
        for (int i = 0; i <= connectionCount; i++)
            drawConnection(connectionVector[i], componentVector, componentCount);
    }
    swapbuffers();
}

int main()
{
    initwindow(WIDTH, HEIGHT);
    initializeComponentIndex(componentMenu);

    do
    {
        drawFrame();
        handleClick();
        delay(100);
    } while (!iWantToLeave);

    closegraph();
    return 0;
}
