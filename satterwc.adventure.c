/** Author: Chelsea Satterwhite
 ** Date: 7/2/2020
 ** Assignment: Project #1 Adventure Program
 ** Description: Adventure program which reads from most recently created rooms directory to 
 		traverse the dungeon. The user can move to different rooms by typing the name of the 
 		room. The game is done and will exit once the user reaches the end room. 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include <sys/stat.h>		//struct stat
#include <sys/types.h>		//ssize_t
#include <dirent.h>			//dir			
#include <pthread.h>		//pthread_mutex_t
#include <unistd.h>			//read

//defintions
#define DIR_PREFIX "satterwc.rooms."

//global variables
int MAX_DUNEGON_ROOMS = 7;
char *NAMES[10] = {"slime", "treasure", "trap", "ogre", "dragon",
		"gold", "health", "mana", "bat", "boss"};

struct Room {
	char *roomName;
	char *roomType;
	int numConnectingRooms;
	char *connections[6];
};

//prototype function declarations
char *findLatestDirectory(char *path, char *prefix);	//from lecture
void initiateRooms(struct Room *rooms[], int size);
void deleteRooms(struct Room *rooms[], int size);
int checkConnection(struct Room *room, char roomName[]);
int getRoomByName(struct Room *room[], char roomName[]);
int fillRooms(const char *dirName, struct Room *rooms[], int start_room);
void printRooms(struct Room *rooms[]);

int main(){
	char *latestDir = findLatestDirectory(".", DIR_PREFIX);
	struct Room *rooms[MAX_DUNEGON_ROOMS];
	int currentRoom = 0;
	int stepCount = 0;
	
	/************************/
	//uncomment before submission
	int endGame = 0;
	/************************/
	
	char roomName[100];
	char *dungeonPath[100];

	//set up rooms
	initiateRooms(rooms, MAX_DUNEGON_ROOMS);

	//fill rooms with info and get the index for the START_ROOM
	int entrance = fillRooms(latestDir, rooms, currentRoom);
	printRooms(rooms);
	currentRoom = entrance;

	//do while endGame != 1 (gameover)
	do {
		printf("CURRENT LOCATION: %s\n", rooms[currentRoom]->roomName);
		
		printf("POSSIBLE CONNECTIONS: ");
		//printed like this to keep , , . punctuation
		printf("%s", rooms[currentRoom]->connections[0]);
		for (int i = 1; i < rooms[currentRoom]->numConnectingRooms; i++){
			printf(", %s", rooms[currentRoom]->connections[i]);
		}
		printf(".\n");

		printf("\nWHERE TO? >");

		/************************/
		//uncomment before submission
		scanf("%s", roomName);
		/************************/

		//if the inputed name doesnt match send error message

		if (checkConnection(rooms[currentRoom], roomName) == 0){
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		} 
		//if it does match then it is a valid path 
		else {
			printf("\n");
			dungeonPath[stepCount] = rooms[currentRoom]->roomName;
			stepCount++;
			currentRoom = getRoomByName(rooms, roomName);
			printf("%d\n", currentRoom);

			if (strcmp(rooms[currentRoom]->roomType, "END_ROOM" ) == 0){
				endGame = 1;
			}
		}

	} while (endGame != 1);

	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICORY WAS: \n", stepCount);
	for (int i = 1; i < stepCount; i++){
		printf("%s\n", dungeonPath[i]);
	}
	printf("%s\n", rooms[currentRoom]->roomName);

	//free memory
	free(latestDir);
	deleteRooms(rooms, MAX_DUNEGON_ROOMS);
	return 0;
}

/*Function was based on lecture notes
 *Takes a pointer to a directory path and is passed a predefined prefix
 *Returns the most recently updated directory from buildrooms*/
char *findLatestDirectory(char *path, char *prefix){
	struct stat dirStat;
	struct dirent *aDir;
	char dirName[100];
	char *latestDirName;
	time_t lastModifTime;
	int i = 0;

	//open the direcotry
	DIR *currentDir = opendir(path);

	//while you can read the directory
	while ((aDir = readdir(currentDir)) != NULL){
		
		//if the passed in prefix matches the directory name perform a stat call to get its info
		if (strncmp(prefix, aDir->d_name, strlen(prefix)) == 0){
			stat(aDir->d_name, &dirStat);
			
			//check to see if this direcotry has the latest modification time 
			if (i == 0 || difftime(dirStat.st_mtime, lastModifTime) > 0){
				lastModifTime = dirStat.st_mtime;
				memset(dirName, '\0', sizeof(dirName));
				strcpy(dirName, aDir->d_name);
			}

			//move along
			i++;
		}
	}

	latestDirName = malloc(sizeof(char) * (strlen(dirName) + 1));
	strcpy(latestDirName, dirName);

	//close direcotry
	closedir(currentDir);

	//return most recent modified direcotry 
	return latestDirName;
}

/*Function to create and set the rooms to their starting values
 *Takes a pointer to the rooms and the size of the dungeon
 *Returns N/A*/
void initiateRooms(struct Room *rooms[], int size){
	for (int i = 0; i < size; i++){
		rooms[i] = malloc (sizeof(struct Room));
		rooms[i]->roomName = NULL;
		rooms[i]->roomType = NULL;
		rooms[i]->numConnectingRooms = 0;
		for (int j = 0; j < 6; j++){
			rooms[i]->connections[j] = NULL;
		}
	}
}

/*Function to free the rooms memory
 *Takes a pointer to the rooms and the size of the dungeon
 *Returns N/A*/
void deleteRooms(struct Room *rooms[], int size){
	for (int i = 0; i < size; i++){
		free(rooms[i]);
	}
}

/*Function to fill the room structs with their data from the files
 *Takes a pointer to most recent direcotry, pointer to rooms and a pointer to the current room (will be set as the first room  "START_ROOM")
 *Returns an int for successful completion*/
int fillRooms(const char *dirName, struct Room *rooms[], int firstRoom){
	struct dirent *aDir;
	DIR *dir;
	FILE *file;
	char buffer[100];
	char filePath[100];
	int currentRoom = 0;
	int itemCount = 0;
	int conCount = 0;

	//open directory
	dir = opendir(dirName);
	//while you still have file to read from directory 
	while ((aDir = readdir(dir))){
		//ignore the parent directories
		if (strlen(aDir->d_name) > 2){
			sprintf(filePath, "./%s/%s", dirName, aDir->d_name);
			
			//open file as read only
			file = fopen(filePath, "r");

			//reset item count
			itemCount = 0;
			conCount = 0;

			while (fgets(buffer, 100, file)!= NULL){
				//put room names in struct
				rooms[currentRoom]->roomName = aDir->d_name;

				//breaks the line off into smaller sections 
				//gets rid of spaces (and :) and reads to \n which leaves the name of the room 
				char *data = strtok(buffer, " ");
					//look for the work connection
					if(strcmp(data, "CONNECTION") == 0){
						data = strtok(NULL, " ");
						data = strtok(NULL, "\n");

						//This section of code made me chage the structure of the struct from 
						//having an array of structs to just having an arry of ints to keep track 
						//of the connections 
						for (int j = 0; j < 10; j++){
							if(strcmp(data, NAMES[j]) == 0){ 
								rooms[currentRoom]->connections[conCount++] = NAMES[j];
							}	
						}
						//update how many connecting rooms there are 
						rooms[currentRoom]->numConnectingRooms++;
					} else {
						data = strtok(NULL, " ");
						data = strtok(NULL, "\n");
						//else put room types in struct
						for (int i = 0; i < MAX_DUNEGON_ROOMS; i++){
							if (strcmp(data, "START_ROOM") == 0){
								rooms[currentRoom]->roomType = "START_ROOM";
								firstRoom = currentRoom;
							} else if (strcmp(data, "END_ROOM") == 0){
								rooms[currentRoom]->roomType = "END_ROOM";
							} else{
								rooms[currentRoom]->roomType = "MID_ROOM";
							}
						} 
					}
					itemCount++;
			}
			currentRoom++;
		}
	}
	return firstRoom;
}

/*Function to check if there is a connection between two room names 
 *Takes a pointer to rooms and a pointer to a room name to check 
 *Returns an int for successful completion*/
int checkConnection(struct Room *room, char roomName[]){
	for (int i = 0; i < room->numConnectingRooms; i++){
		if (roomName == NULL) return 0;
        if (room->connections[i] == NULL) return 0;
		if (strcmp(roomName, room->connections[i]) == 0){
			//match found
			return 1;
		}
	}
	return 0;
}

/*Function get a room index
 *Takes a pointer to rooms and a pointer to a room name to check 
 *Returns an index on successful completion*/
int getRoomByName(struct Room *rooms[], char roomName[]){
	for (int i = 0; i < MAX_DUNEGON_ROOMS; i++){
		if (strcmp(roomName, rooms[i]->roomName) == 0){
			return i;
		}
	}
	return -1;
}

/*Function to print the rooms
 *Takes a pointer to rooms
 *Returns N/A*/
void printRooms(struct Room *rooms[]){
	for (int i = 0; i < 7; i++){
		printf("ROOM %d:\n", i);
		printf("Name: %s\n",rooms[i]->roomName);
		printf("Connections: ");
		for (int j = 0; j < rooms[i]->numConnectingRooms; j++){ 	//rooms[i]->numConnectingRooms
			printf("%s, ", rooms[i]->connections[j]);
		}
		printf("\n");
		printf("Type: %s\n\n", rooms[i]->roomType);
	}
}
