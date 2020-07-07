/** Author: Chelsea Satterwhite
 ** Date: 7/2/2020
 **	Assignment: Project #1 Adventure Program
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

//defintions
#define _GNU_SOURCE
#define DIR_PREFIX "satterwc.rooms."

//global variables
int MAX_DUNEGON_ROOMS = 7;
//pthread_mutex_t mutex;

struct Room {
	char *roomName;
	char *roomType;
	int numConnectingRooms;
	char *connections[6];
};

//prototype function declarations
char *findLatestDirectory(char *path, char *prefix);	//from lecture
void getRoomFiles(char *recentDir, struct Room *rooms[]);
void initiateRooms(struct Room *rooms[], int size);
void deleteRooms(struct Room *rooms[], int size);
void parseRoomFile(char *roomFileName, char *dirName, struct Room *rooms[]);
int getStartingRoom(struct Room *rooms[]);
int checkConnection(struct Room *room, char roomName[]);
int getRoomByName(struct Room *room, char roomName[]);
//int createRooms(char *recentDir, struct Room *rooms, int *start_room);
void printRooms(struct Room *rooms[]);

int main(){
	char *latestDir = findLatestDirectory(".", DIR_PREFIX);
	//printf("Latest direcotry is %s\n", latestDir);
	struct Room *rooms[MAX_DUNEGON_ROOMS];
	int currentRoom = 0;
	int stepCount;
	int endGame = 1;
	char roomName[100];
	char *dungeonPath[100];

	initiateRooms(rooms, MAX_DUNEGON_ROOMS);
	getRoomFiles(latestDir, rooms);

	//seg fault
	//currentRoom = getStartingRoom(rooms);

	do {
		printf("CURRENT LOCATION: %s\n", rooms[currentRoom]->roomName);
		
		printf("POSSIBLE CONNECTIONS: ");
		//printed like this to keep , , . punctuation
		printf("%s ", rooms[currentRoom]->connections[0]);
		for (int i = 1; i < rooms[currentRoom]->numConnectingRooms; i++){
			printf(", %s ", rooms[currentRoom]->connections[i]);
		}
		printf(".\n");

		printf("WHERE TO? >");
		//scanf("%s", roomName);

		//if the inputed name doesnt match send error message
		if (checkConnection(rooms[currentRoom], roomName) == 0){
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
		} 
		//if it does match then it is a valid path 
		else {
			dungeonPath[stepCount] = rooms[currentRoom]->roomName;
			stepCount++;
			currentRoom = getRoomByName(rooms[currentRoom], roomName);
		}

		//seg fault
		/*if (strcmp(rooms[currentRoom]->roomType, "END_ROOM" ) == 0){
			endGame = 1;
		}*/

	} while (endGame != 1);

	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICORY WAS: \n", stepCount);
	for (int i = 0; i < stepCount; i++){
		printf("%s\n", dungeonPath[i]);
	}

	deleteRooms(rooms, MAX_DUNEGON_ROOMS);
	return 0;
}

char *findLatestDirectory(char *path, char *prefix){
	struct stat dirStat;
	struct dirent *aDir;
	char dirName[100];
	char *latestDirName;
	time_t lastModifTime;
	int i = 0;

	//open the direcotry
	DIR *currentDir = opendir(path);

	while ((aDir = readdir(currentDir)) != NULL){
		
		if (strncmp(prefix, aDir->d_name, strlen(prefix)) == 0){
			stat(aDir->d_name, &dirStat);
			
			//check to see if this direcotry has the latest modification time 
			if (i == 0 || difftime(dirStat.st_mtime, lastModifTime) > 0){
				lastModifTime = dirStat.st_mtime;
				memset(dirName, '\0', sizeof(dirName));
				strcpy(dirName, aDir->d_name);
			}

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

void getRoomFiles(char* recentDir, struct Room *rooms[]){

	char targetFileSuffix[50] = "room";
	char roomFileName[20];
	memset(roomFileName, '\0', sizeof(roomFileName));

	DIR *dirToCheck;
	struct dirent *fileInDir;

	int currentRoomNum = 0;

	dirToCheck = opendir(recentDir);
	if (dirToCheck > 0){
		while ((fileInDir = readdir(dirToCheck)) != NULL){
			//6
			 //if (strstr(fileInDir->d_name, targetFileSuffix) != NULL){
				//0
				memset(roomFileName, '\0', sizeof(roomFileName));
				strcpy(roomFileName, fileInDir->d_name);
				//printf("Room File Name: %s\n", roomFileName);

				parseRoomFile(roomFileName, recentDir, rooms);
				currentRoomNum++;
			//}
		}
	}
	closedir(dirToCheck);
}

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

void deleteRooms(struct Room *rooms[], int size){
	for (int i = 0; i < size; i++){
		free(rooms[i]);
	}
}

void parseRoomFile(char *roomFileName, char *dirName, struct Room *rooms[]){
	char roomPathName[50];
	memset(roomPathName, '\0', sizeof(roomPathName));
	sprintf(roomPathName, "./%s/%s", dirName, roomFileName);
	//printf("Room Path Name: %s\n", roomPathName);

	char field1[20];
	char field2[20];
	char value[20];

	int connectionsNum = 0;

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	//open room file to read
	fp = fopen(roomPathName, "r");

	while ((read = getline(&line, &len, fp)) != EOF){
		//uncomment me to see room info untouched
		//printf("%s", line);

		memset(field1, '\0', sizeof(field1));
		memset(field2, '\0', sizeof(field2));
		memset(value, '\0', sizeof(value));

		sscanf(line, "%s %s %s\n", field1, field2, value);
		//uncomment me to see room info parsed into 3 sections
		//printf("field1: '%s' field2: '%s' value: '%s'\n", field1, field2, value);
		

		//this doesnt work and is a mess.  But I am trying to go with a similar logic 
/*		for (int i = 0; i < MAX_DUNEGON_ROOMS; i++){
			//printf("%d - field1: '%s'\nfield2: '%s'\nvalue: '%s'\n", i, field1, field2, value);
			if (strcmp(field2, "NAME:") == 0){
				//printf("*%s -", value);
				//rooms[i]->roomName = value;
				//printf("ROOM %d: %s\n", i, rooms[i]->roomName);
			} else if (strcmp(field2, "TYPE:") == 0){
				if (strstr(value, "START") != NULL){
					//printf("*%s -\n", value);
					rooms[i]->roomType = "START_ROOM";
					//printf("TYPE: %s\n", rooms[i]->roomType);
				}
				
			} else {
				//printf("*%s", value);
			}
		}*/

	} 
	//extra space to help visualize rooms
	//printf("\n");
}

int getStartingRoom(struct Room *rooms[]){
	for (int i = 0; i < MAX_DUNEGON_ROOMS; i++){
		if (strcmp(rooms[i]->roomType, "START_ROOM") == 0)
			return i;
			
	}
	return -1;
}


int checkConnection(struct Room *room, char roomName[]){
	for (int i = 0; i < room->numConnectingRooms; i++){
		if (strcmp(roomName, room->connections[i]) == 0){
			return 1;
		}
	}
	return 0;
}

int getRoomByName(struct Room *room, char roomName[]){
	for (int i = 0; i < room->numConnectingRooms; i++){
		if (strcmp(roomName, room->roomName) == 0){
			return 1;
		}
	}
	return 0;
}

void printRooms(struct Room *rooms[]){
	for (int i = 0; i < MAX_DUNEGON_ROOMS; i++){
		printf("Name: %s",rooms[i]->roomName);
		printf(" - Type: %s",rooms[i]->roomType);
		printf("- Num of Connections: %d",rooms[i]->numConnectingRooms);
		printf("- Connections: ");
		for (int j = 0; j < MAX_DUNEGON_ROOMS - 1; j++){
			printf("%s, ", rooms[i]->connections[j]);
		}
		printf("\n");
	}
}




