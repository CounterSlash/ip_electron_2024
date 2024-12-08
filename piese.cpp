#include <string.h>
#include <winbgim.h>

//Defines
#define MAX_NAME_LENGTH 30
#define MAX_CONNECTION_POINTS 6
#define MAX_DRAWING_INSTRUCTIONS 15

#define JOINT_RADIUS 4

struct collisionBox
{
    float x_TL, y_TL;
    float x_BR, y_BR;
};

struct instruction
{
    char type;
    float x1, x2;
    float y1, y2;
};

struct joint
{
    float x, y;
};

struct connection{
    int first_ID, second_ID;
    int first_Joint, second_Joint;
};

struct component
{
    int ID;
    float x;                                                  //Coordonata x
    float y;                                                  //Coordonata y
    char name[MAX_NAME_LENGTH];                             //Numele, definit in initializeComponentIndex
    float zoom;                                             //Zoom-ul fiecarei componente

    unsigned int instructionCount;                        //Numarul de instructiuni de desen
    instruction drawingGuide[MAX_DRAWING_INSTRUCTIONS];     //Vector de instructiuni

    unsigned int jointCount;                              //Numarul de legaturi
    joint solderJoints[MAX_CONNECTION_POINTS];              //Vector de leagturi

    collisionBox boundary;
};

void drawRectangle(int x1, int y1, int x2, int y2)
{
    rectangle(x1, y1, x2, y2);
}

void drawLine(int x1, int y1, int x2, int y2)
{
    line(x1, y1, x2, y2);
}

void drawComponent(component &comp)
{
    // drawRectangle(comp.x + comp.boundary.x_TL, comp.y + comp.boundary.y_TL, comp.x + comp.boundary.x_BR, comp.y + comp.boundary.y_BR);
    for(int i=0; i < comp.instructionCount; i++)
    {
        switch (comp.drawingGuide[i].type)
        {
        case 'L':
            line(comp.x + comp.drawingGuide[i].x1*comp.zoom, comp.y + comp.drawingGuide[i].y1*comp.zoom, comp.x + comp.drawingGuide[i].x2*comp.zoom, comp.y + comp.drawingGuide[i].y2*comp.zoom);
            //printf("Drawing line at %f, %f\n", comp.x, comp.y);
            break;
        case 'R':
            break;
        case 'E':
            break;
        case 'C':
            break;
        default:
            printf("Took case %c which is unimplemented", comp.drawingGuide[i].type);
            break;
        }
    }
    for(int i = 0; i < comp.jointCount; i++){
        circle(comp.x + comp.solderJoints[i].x*comp.zoom, comp.y + comp.solderJoints[i].y*comp.zoom, JOINT_RADIUS*comp.zoom);
    }
}

void drawConnection(connection &conn, component compVector[], int compCount){
    int startID = -1, stopID = -1;
    for(int i = 0; i <= compCount; i++){
        if(conn.first_ID == compVector[i].ID && startID == -1)
            startID = i;
        if(conn.second_ID == compVector[i].ID && stopID == -1)
            stopID = i;
    }
    //printf("Drawing line from %d to %d!]n", startID, stopID);
    line(compVector[startID].x + compVector[startID].solderJoints[conn.first_Joint].x*compVector[startID].zoom, 
         compVector[startID].y + compVector[startID].solderJoints[conn.first_Joint].y*compVector[startID].zoom,
         compVector[stopID].x + compVector[stopID].solderJoints[conn.second_Joint].x*compVector[stopID].zoom,
         compVector[stopID].y + compVector[stopID].solderJoints[conn.second_Joint].y*compVector[stopID].zoom);

}
