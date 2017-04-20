#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

int search_filesystems(){
	DIR *dir;
	struct dirent *ent;
	int const_length = strlen("filesystem");
	int count = 0;
	if ((dir = opendir (".")) != NULL) {
	while ((ent = readdir (dir)) != NULL) {
		if(strncmp(ent->d_name, "filesystem", const_length) == 0) {
			printf("  %d.) %s", count+1, ent->d_name + const_length + 1 );
			// fwrite(ent->d_name, const_length + 1, strlen(ent -> d_name) - 1, stdout);
			count++;
		}
	}
		closedir (dir);
	}
	if (count){
		printf("\n");
		return 1;
	}
	return 0;
}

int check_fs_validaity(char * fs_name){
	DIR *dir;
	struct dirent *ent;
	int const_length = strlen("filesystem");
	int exists = 0;
	if ((dir = opendir (".")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if(strcmp(ent->d_name, fs_name) == 0) {
				exists = 1;
				break;
			}
		}
		closedir (dir);
	}
	return exists;
}

void create_filesystem(){
	// 4 lines per block, each line 1 KB ==> each block 4 KB
	// each line has 4 comma seperated values
	// each value 256 Bytes
	// first block(first 4 lines) : super block
	// second block : inode bitmap
	// third block : data bitmap
	printf("Enter name of filesystem\n");
	char name[20];
	scanf("%s", name);
	char fs_name[100] = "filesystem_";
	strcat(fs_name, name);

	if (check_fs_validaity(fs_name))
		printf("Filesystem with name : %s already exists!\n", name);
	else{
		
		printf("Number of blocks(each 4KB in size) for Inodes\n");
		int num_inodes;
		scanf("%d", &num_inodes);

		FILE * fp;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;

		fp = fopen(fs_name, "w");

		fprintf(fp,"%s", fs_name);
		fprintf(fp,",%d", num_inodes);
		
		printf("Filesystem %s , with %d blocks for inodes, that is, a capacity for %d files, successfully created!\n", name, num_inodes, 16 * num_inodes);

		fclose(fp);
	}
}

int main(){
	int mounted=0;
	char fs_mounted_name[256];

	printf("existing file systems:\n");
	int found = search_filesystems();
	if(found){
		printf("\n	use command 'mount <file_system_name>' to mount the filesystem\n");
	}
	else{
		printf("	None found!\n");
	}
	printf("	use command 'create_filesystem' to create a new filesystem\n" );
	printf("	use command 'write_file' to create and write to a new file\n" );
	printf("	use command 'read_file' to read an existing file\n" );
	printf("	use command 'list_files' to list all files\n" );
	printf("	use command 'inode_bitmap' to list the status of inode blocks\n" );
	printf("	use command 'data_bitmap' to list the status of data blocks\n\n" );

	while(1){
		printf("prompt -> ");
		char command[256];
		scanf("%s",command);
		if(strcmp(command, "create_filesystem") == 0 ){
			create_filesystem();
		}
		if(strncmp(command, "mount", 5) == 0){
			if(mounted){
				char garbage[50];
				scanf("%s",garbage);
				printf("Filesystem %s already mounted\n", fs_mounted_name);
			}
			else{
				fs_mounted_name[0] = '\0';
				scanf("%s",fs_mounted_name);
				char filename[200]="filesystem_";
				strcat(filename, fs_mounted_name);
				if (check_fs_validaity(filename)){
					mounted = 1;
					printf("%s mounted successfully!\n", fs_mounted_name);	
				}
				else
					printf("Filesystem does not exist!\n");
			}
		}
		if(strcmp(command,"write_file") == 0){
			if(mounted){
				//implement write
			}
			else{
				printf("Mount a filesystem first!\n");
			}
		}
		if (strcmp(command,"read_file") == 0){
			if(mounted){
				//implement read
			}
			else{
				printf("Mount a filesystem first!\n");
			}
		}
		if (strcmp(command,"list_files") == 0){
			if(mounted){
				//implement list_files
			}
			else{
				printf("Mount a filesystem first!\n");
			}
		}
		if (strcmp(command,"data_bitmap") == 0){
			if(mounted){
				//implement data_bitmap
			}
			else{
				printf("Mount a filesystem first!\n");
			}
		}
		if (strcmp(command,"inode_bitmap") == 0){
			if(mounted){
				//implement inode_bitmap
			}
			else{
				printf("Mount a filesystem first!\n");
			}
		}
		if(strcmp(command,"exit") == 0)
			break;
	}
	return 0;
}