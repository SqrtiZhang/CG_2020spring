// defines
#define MAX_GAME_OBJECTS 100
#define MAX_FACES 100
#define MAX_VERTICES 1000

// data structures
typedef struct
{
	char filename[2][50];

}FILERECORD;

typedef struct
{
	float x,y,z;
}VERTEX;

typedef struct
{
	int numsides;
	int vert[4];
	
}FACE;

typedef struct
{   
	int numfaces;
	int numverts;
	FACE *faces[MAX_FACES];
	VERTEX *vertices[MAX_VERTICES];

}OBJECT;


// includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

// protos
void LoadObj(char *setname);
void CountObjects(void);
int ObjFileQue(int whichobject,char whatdata);
void LoadFaces(int theobject);
void LoadVertices(int theobject);