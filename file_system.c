#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

struct info{
	char fs_mounted_name[256];
	int fsid;
	int num_inode_blocks;
	int num_data_blocks;
} mounted_fs_info;

int i,j;

int search_filesystems(){
	DIR *dir;
	struct dirent *ent;
	int const_length = strlen("filesystem");
	int count = 0;
	if ((dir = opendir (".")) != NULL) {
	while ((ent = readdir (dir)) != NULL) {
		if(strncmp(ent->d_name, "filesystem", const_length) == 0) {
			printf("  %d.) %s", count+1, ent->d_name + const_length + 1 );
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

int get_fsid(){
	DIR *dir;
	struct dirent *ent;
	int const_length = strlen("filesystem");
	int count = 0;
	if ((dir = opendir (".")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if(strncmp(ent->d_name, "filesystem", const_length) == 0) {
				count++;
			}
		}
		closedir (dir);
	}
	return count + 1;	
}

int check_file_existence(char * filename){
	char fs_filename[50]="filesystem_";
	strcat(fs_filename, mounted_fs_info.fs_mounted_name);
	//read inode bitmap
	FILE * fp;
	fp = fopen(fs_filename, "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	getline(&line, &len, fp);
	getline(&line, &len, fp);
	getline(&line, &len, fp);
	for(i=0 ; i < mounted_fs_info.num_inode_blocks * 16 ; i++){
		getline(&line, &len, fp);
		if(strcmp(line,"\n") != 0){
			const char s[2] = ",";
			char * token;
			token = strtok(line, s);
			if(strcmp(token,filename) == 0)
				return 1;
		}
	}
	return 0;
}

int create_sfs(char * name, int nbytes){
	// 1 line per block, each line 4 KB ==> each block 4 KB
	// Line may or may not have comma seperated values
	// each value 256 Bytes
	// first block(first line) : super block
	// second block : inode bitmap
	// third block : data bitmap
	char fs_name[100] = "filesystem_";
	strcat(fs_name, name);

	int fsid = get_fsid();

	if (check_fs_validaity(fs_name)){
		printf("Filesystem with name : %s already exists!\n", name);
		return -1;
	}
	else{
		
		printf("Enter number of bytes for data\n");
		int num_bytes;
		scanf("%d", &num_bytes);

		FILE * fp;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;
		int data_blocks = (num_bytes/1024)%4 == 0 ? (num_bytes/1024)/4 : ((num_bytes/1024)/4) + 1;
		int num_inodes = (data_blocks)%16 == 0 ? (data_blocks)/16 : (data_blocks/16) + 1;
		

		char inode_bitmap[num_inodes+1], data_bitmap[data_blocks+1];

		for(i = 0 ; i < num_inodes*16 ; i++){
			inode_bitmap[i]='0';
		}
		inode_bitmap[i]='\0';
		for(i = 0 ; i < data_blocks ; i++){
			data_bitmap[i]='0';
		}
		data_bitmap[i]='\0';

		fp = fopen(fs_name, "w");
		fprintf(fp,"%d,%s,%d,%d\n%s\n%s", fsid, name, num_inodes, data_blocks, inode_bitmap, data_bitmap);
		int i, NUMBER_OF_LINES = num_inodes*16 + data_blocks;
		for(i=1 ; i <= (NUMBER_OF_LINES) ; i++){
			fprintf(fp, "\n");
		}

		printf("Filesystem %s with %d data block(s), with %d block(s) for inodes, that is, a capacity for %d file(s), successfully created!\nFile System ID : %d\n", name, data_blocks, num_inodes, 16 * num_inodes, fsid);

		fclose(fp);
		return fsid;
	}
}

int read_data(int disk, int blocknum, void * block){
	char * block_ptr=(char *)block;
	char * token_2;
	const char delim[2] = ";";
	block_ptr[strlen(block_ptr) - 1]='\0';
	token_2 = strtok(block_ptr, delim);
	while(token_2 != NULL){
		printf("%s\n", token_2);
		token_2=strtok(NULL, delim);
	}
}

int write_data(int disk, int blocknum, void * block){
	//write to disk block
	//update inode and data bitmaps
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char lines_of_file[100][100];
	char fs_filename[50]="filesystem_";
	strcat(fs_filename, mounted_fs_info.fs_mounted_name);
	fp = fopen(fs_filename, "r");
	int count=0;
	while ((read = getline(&line, &len, fp)) != -1){
		strcpy(lines_of_file[count], line);
		count++;
	}
	fclose(fp);
	lines_of_file[1][blocknum - 1]='1';
	lines_of_file[2][disk - 1]='1';
	fp = fopen(fs_filename, "w");
	for(i=0;i<count;i++){
		fprintf(fp, "%s", lines_of_file[i]);
	}
	fclose(fp);
	return 1;
}

int write_file(int disk, char* filename, void* block){
	//read filesystem name
	char fs_filename[50]="filesystem_";
	strcat(fs_filename, mounted_fs_info.fs_mounted_name);
	//read inode bitmap
	FILE * fp;
	fp = fopen(fs_filename, "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	getline(&line, &len, fp);
	getline(&line, &len, fp);
	line[strlen(line)-1] = '\0';
	for(i=0;i<strlen(line);i++){
		if(line[i]=='0')
			break;
	}
	int line_number = 3 + i + 1;

	getline(&line, &len, fp);
	line[strlen(line)-1] = '\0';
	for(i=0;i<strlen(line);i++){
		if(line[i]=='0')
			break;
	}
	int block_number = i + 1;
	int blocknum = 3 + mounted_fs_info.num_inode_blocks*16 + block_number;
	fclose(fp);
	//make inode entry
	char lines_of_file[100][100];
	fp = fopen(fs_filename, "r");
	int count=0;
	while ((read = getline(&line, &len, fp)) != -1){
		strcpy(lines_of_file[count], line);
		count++;
	}
	fclose(fp);
	char temp_str[50];
	sprintf(temp_str ,"%s,%d\n", filename, block_number);
	strcpy(lines_of_file[line_number - 1], temp_str);
	char temp_str_2[50];
	sprintf(temp_str_2 ,"%s\n", (char *) block);
	strcpy(lines_of_file[blocknum - 1], temp_str_2);
	
	fp = fopen(fs_filename, "w");
	for(i=0;i<count;i++){
		fprintf(fp, "%s", lines_of_file[i]);
	}
	fclose(fp);
	//commit the write
	return write_data(block_number, line_number - 3, block);
}

int read_file(int disk, char* filename, void* block){
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char lines_of_file[100][100];
	char fs_filename[50]="filesystem_";
	strcat(fs_filename, mounted_fs_info.fs_mounted_name);
	fp = fopen(fs_filename, "r");
	int count=0;
	while ((read = getline(&line, &len, fp)) != -1){
		strcpy(lines_of_file[count], line);
		count++;
	}
	fclose(fp);
	for(i=3 ; i < 3 + mounted_fs_info.num_inode_blocks * 16 ; i++){
		if (strcmp(lines_of_file[i],"\n")==0)
			continue;
		lines_of_file[i][strlen(lines_of_file[i]) - 1] = '\0';
		const char s[2] = ",";
		char * token;
		token = strtok(lines_of_file[i], s);
		if(strcmp(token, filename)==0){
			token = strtok(NULL, s);
			int size=0,block_arr[100];
			while(token != NULL){
				block_arr[size]=atoi(token);
				size++;
				token = strtok(NULL, s); 
			}
			for(j=0;j<size;j++){
				int blocknum = block_arr[j];
				blocknum = 3 + mounted_fs_info.num_inode_blocks*16 + blocknum;
				read_data(block_arr[j], blocknum, lines_of_file[blocknum - 1]);
			}
			return 1;
		}
	}
	printf("File does not exist!\n");
}

void print_inodebitmaps(int fsid){
	char fs_filename[50]="filesystem_";
	strcat(fs_filename, mounted_fs_info.fs_mounted_name);
	//read inode bitmap
	FILE * fp;
	fp = fopen(fs_filename, "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	getline(&line, &len, fp);
	getline(&line, &len, fp);
	for(i=0 ; i<strlen(line)-1 ; i++){
		if(line[i]=='1')
			printf("Inode number %d : Occupied\n", i);
		else
			printf("Inode number %d : Free\n", i);
	}
}

void print_databitmaps(int fsid){
	char fs_filename[50]="filesystem_";
	strcat(fs_filename, mounted_fs_info.fs_mounted_name);
	//read inode bitmap
	FILE * fp;
	fp = fopen(fs_filename, "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	getline(&line, &len, fp);
	getline(&line, &len, fp);
	getline(&line, &len, fp);
	for(i=0 ; i<strlen(line)-1 ; i++){
		if(line[i]=='1')
			printf("Data block number %d : Occupied\n", i);
		else
			printf("Data block number %d : Free\n", i);
	}
}

void print_filelist(int fsid){
	char fs_filename[50]="filesystem_";
	strcat(fs_filename, mounted_fs_info.fs_mounted_name);
	//read inode bitmap
	FILE * fp;
	fp = fopen(fs_filename, "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	getline(&line, &len, fp);
	getline(&line, &len, fp);
	getline(&line, &len, fp);
	for(i=0 ; i < mounted_fs_info.num_inode_blocks * 16 ; i++){
		getline(&line, &len, fp);
		if(strcmp(line,"\n") != 0){
			const char s[2] = ",";
			char * token;
			token = strtok(line, s);
			printf("%s\n", token);
		}
	}
}

int main(){
	int mounted=0;

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
			printf("Enter name of filesystem\n");
			char name[20];
			scanf("%s", name);
			
			create_sfs(name, 0);
		}
		if(strncmp(command, "mount", 5) == 0){
			if(mounted){
				char garbage[50];
				scanf("%s",garbage);
				printf("Filesystem %s already mounted\n", mounted_fs_info.fs_mounted_name);
			}
			else{
				mounted_fs_info.fs_mounted_name[0] = '\0';
				scanf("%s",mounted_fs_info.fs_mounted_name);
				char filename[200]="filesystem_";
				strcat(filename, mounted_fs_info.fs_mounted_name);
				if (check_fs_validaity(filename)){
					FILE * fp;
					fp = fopen(filename, "r");
					char * line = NULL;
					size_t len = 0;
					ssize_t read;
					getline(&line, &len, fp);
					const char s[2] = ",";
					char * token;
					token = strtok(line, s);
					mounted_fs_info.fsid = atoi(token);
					token = strtok(NULL, s);
					token = strtok(NULL, s);
					mounted_fs_info.num_inode_blocks = atoi(token);
					token = strtok(NULL, s);
					mounted_fs_info.num_data_blocks = atoi(token);
					mounted = 1;
					printf("%s(id : %d) mounted successfully!\n", mounted_fs_info.fs_mounted_name, mounted_fs_info.fsid);	
				}
				else
					printf("Filesystem does not exist!\n");
			}
		}
		if(strcmp(command,"write_file") == 0){
			if(mounted){
				//implement write
				char file_name[20];
				char block_data[4096]="";
				printf("Enter filename : ");
				scanf("%s", file_name);
				if(check_file_existence(file_name)){
					printf("File already exists!\n");
				}
				else{
					printf("Number of lines in the file : ");
					int num_lines;
					scanf("%d", &num_lines);
					printf("Enter the lines below \n");
					getchar();
					while(num_lines -- ){
						char temp[100];
						temp[0]='\0';
						gets(temp);
						// printf("%s\n",temp);
						strcat(block_data,temp);
						if(num_lines)
							strcat(block_data,";");
					}
					// printf("%s\n", block_data);
					write_file(mounted_fs_info.fsid, file_name, block_data);
				}
			}
			else{
				printf("Mount a filesystem first!\n");
			}
		}
		if (strcmp(command,"read_file") == 0){
			if(mounted){
				char name[50];
				printf("Enter filename: ");
				scanf("%s",name);
				read_file(mounted_fs_info.fsid, name, NULL);
			}
			else{
				printf("Mount a filesystem first!\n");
			}
		}
		if (strcmp(command,"list_files") == 0){
			if(mounted){
				print_filelist(mounted_fs_info.fsid);
			}
			else{
				printf("Mount a filesystem first!\n");
			}
		}
		if (strcmp(command,"data_bitmap") == 0){
			if(mounted){
				print_databitmaps(mounted_fs_info.fsid);
			}
			else{
				printf("Mount a filesystem first!\n");
			}
		}
		if (strcmp(command,"inode_bitmap") == 0){
			if(mounted){
				print_inodebitmaps(mounted_fs_info.fsid);
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