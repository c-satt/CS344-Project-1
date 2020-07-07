/** Author: Chelsea Satterwhite
 ** Date: 6/27/2020
 **	Assignment: Project #1 Rooms Program
 ** Description: Rooms Program code that creates a directory called satterwc.rooms.random_number,
 **		then generates 7 different room files, which contains one room per file. Rooms should have
 **		a unique name, room type, and rooms connected. The program should have 10 room names 
 **		hardcoded for random file creation. 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>				
#include <time.h>			//srand for random function
#include <sys/stat.h>		//mkdir
#include <sys/types.h>		//mkdir
#include <unistd.h>			//chdir


//global variables
int MAX_DUNEGON_ROOMS = 7;
int MAX_DUNGEON_CONNECTIONS = 6;

//needs to be above the prototype declarations 
struct Room {
	char *roomName;
	char *roomType;
	int numConnectingRooms;
	struct Room *connections[6];
};

//prototype function declarations
void randomize(int arr[], int n);
void swap(int *a, int *b);
void makeRooms(struct Room *rooms[], char *names[], char *types[]);
void makeRoomFiles(struct Room *rooms[], char *directory);

//professor recommended functions
int isGraphFull(struct Room *rooms[]);
struct Room* getRandomRoom(struct Room *rooms[]);
int canAddConnectionFrom(struct Room *room);
int connectionAlreadyExists(struct Room *roomX, struct Room *roomY);
void connectRoom(struct Room *roomX, struct Room *roomY);
int isSameRoom(struct Room *roomX, struct Room *roomY);
void addRandomConnection(struct Room* rooms[]);

int main(){

	//seed time for random function
	srand(time(NULL));

	//hard code the room types and the room names
	char *roomTypes[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};
	char *roomNames[10] = {"slime", "treasure", "trap", "ogre", "dragon",
		"gold", "health", "mana", "bat", "boss"};
	
	//array of pointers to hold the rooms
	struct Room *rooms[MAX_DUNEGON_ROOMS];

	//generates a random number between 0 and 100,000
	int random = rand() % 100001;
	//printf("%d\n", random);

	//create file note: file name = satterwc.rooms.random_number
	char dirName[100];
	sprintf(dirName, "satterwc.rooms.%d", random);
	//printf("%s\n", fileName ); 
	//permissions set to create, read and write for the creator, no one else can change it
	mkdir(dirName, 0755); 

	//initalize the room values
	makeRooms(rooms, roomNames, roomTypes);
	
	//create the dungron by connecting rooms (using the functions advised by the professor)
	while (isGraphFull(rooms) == 0){
		addRandomConnection(rooms);
	}
	
	//create the room files and place it into the directory 
	makeRoomFiles(rooms, dirName);

	return 0;
}

/* 
Credit - https://bits.mdminhazulhaque.io/c/shuffle-items-of-array-in-c.html
*/
void randomize(int arr[], int n){
	for (int i = n-1; i > 0; i--){
		int j = rand() % (i+1);
		swap(&arr[i], &arr[j]);
	}
}

void swap(int *a, int *b){
	int temp = *a;
	*a = *b;
	*b = temp;
}

void makeRooms(struct Room *rooms[], char *names[], char *types[]){
	//used to shuffle the rooms so it is different everytime the program is run 
	int tempShuffle[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int shuffleSize = sizeof(tempShuffle)/sizeof(tempShuffle[0]);
	randomize(tempShuffle, shuffleSize);

	//create and place the proper info into the Rooms  
	for (int i = 0; i < MAX_DUNEGON_ROOMS; i++){
		int index = tempShuffle[i];

		//allocate memory for the rooms
		rooms[i] = malloc (sizeof(struct Room));

		//name the rooms
		rooms[i]->roomName = names[index];

		//determine room type
		if (i == 0){
			rooms[i]->roomType = types[0];
		} else if (i == 6){
			rooms[i]->roomType = types[2];
		} else{
			rooms[i]->roomType = types[1];
		}

		//connections start at 0
		rooms[i]->numConnectingRooms = 0;

		//initialized the connecting pointers to NULL
		for (int j = 0; j < MAX_DUNGEON_CONNECTIONS; j++){
			rooms[i]->connections[j] = NULL;
		}
		
	}
}

void makeRoomFiles(struct Room *rooms[], char *directory){
	chdir(directory);

	for (int i = 0; i < MAX_DUNEGON_ROOMS; i++){
		//opens the file with append mode
		FILE *roomFile = fopen(rooms[i]->roomName, "a");
		fprintf(roomFile, "ROOM NAME: %s\n", rooms[i]->roomName);

		for (int j = 0; j < rooms[i]->numConnectingRooms; j++){
			fprintf(roomFile, "CONNECTION %d: %s\n", j+1, rooms[i]->connections[j]->roomName);
		}

		fprintf(roomFile, "ROOM TYPE: %s\n", rooms[i]->roomType);

		fclose(roomFile);
	}
}

int isGraphFull(struct Room *rooms[]){
	for (int i = 0; i < 7; ++i){
		if (rooms[i]->numConnectingRooms < 3){
			return 0;
		}
	}
	return 1;
}

struct Room* getRandomRoom(struct Room *rooms[]){
	int random = rand() % 7;

	return rooms[random];
}

int canAddConnectionFrom(struct Room *room){
	//check if connecting rooms are less than 6
	if (room->numConnectingRooms < 6){
		//true: more connections can be made
		return 1;
	} else{
		//false
		return 0;
	}
}

int connectionAlreadyExists(struct Room *roomX, struct Room *roomY){
	int i = 0;
	while (i < 6){
		if (roomX->connections[i] == NULL){
			//if you hit a NULL value it is end of written list and no connection exists 
			return 0;
		} else if (strcmp(roomY->roomName, roomX->connections[i]->roomName) == 0){
			//true: connection already exists 
			return 1;
		} else{
			//increment and check the next one
			i++;
		}
	}
	return 0;
}

void connectRoom(struct Room *roomX, struct Room *roomY){
	int i = 0;
	while (roomX->connections[i] != NULL && i < 6){
		i++;
	}
	roomX->connections[i] = roomY;
	roomX->numConnectingRooms++;
}

int isSameRoom(struct Room *roomX, struct Room *roomY){
	if (strcmp(roomX->roomName, roomY->roomName) == 0)
		return 1;
	else 
		return 0;
}

void addRandomConnection(struct Room* rooms[]){
	struct Room *roomX;
	struct Room *roomY;

	int i = 1;

	do{
		roomX = getRandomRoom(rooms);

	} while (canAddConnectionFrom(roomX) == 0);

	do{
		roomY = getRandomRoom(rooms);
	} while (canAddConnectionFrom(roomY) == 0 || isSameRoom(roomX, roomY) == 1 || connectionAlreadyExists(roomX, roomY));

	connectRoom(roomX, roomY);
	connectRoom(roomY, roomX);
}



