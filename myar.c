/* Rudy Gonzalez
   gonzalru@onid.orst.edu
   CS311-400
   Homework 4
*/

/*
	A regular file is distinguished from special files. Special files include directories, symbolic links, named pipes, sockets, devices and doors. Regular files contain data of some sort including text, binary, program data etc.
		http://en.wikipedia.org/wiki/Unix_file_types#Regular_file
		http://www.livefirelabs.com/unix_tip_trick_shell_script/unix_operating_system_fundamentals/file-types-in-unix.htm
*/

#define _BSD_SOURCE
#define _XOPEN_SOURCE 500

#include <ar.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
 
#define BLOCKSIZE 1

void filecheck(int);

int main(int argc, char **argv)
{
	
	off_t hdr_filename_offset = 0; //filename length = 16
	off_t hdr_modtime_offset = 16; //file modtime length = 12
	off_t hdr_userid_offset = 28; //ownerID length = 6
	off_t hdr_groupid_offset = 34; //groupID length = 6
	off_t hdr_mode_offset = 40; //file mode length = 8
	off_t hdr_filesize_offset = 48; //filesize length = 10
	off_t hdr_filemagic_offset = 58; //file magic length = 2
	
	off_t file_curbyte = 0;
	off_t file_endbyte;
	
	int c;
	int fd;
	int fd_data;
	int fd_newarchive;
	int fd_memextract;
	int num_read;
	int num_written;
	int filesize_int;
	int i;
	int j;
	int memflag;
	int fill_spaces;

	static int b;
	static int p;
	char *filename;
	char mod_time[12];
	char userid[6];
	char groupid[6];
	char mode[8];
	char *modestring;
	long octlong;
	char filesize[10];
	char buf_rw[BLOCKSIZE];
	
	char permstring[] = "---------";
	
	char *temptimestring;
	char *timestring;
	char *spaces;
	char *archiveFile = argv[2];
	
	struct ar_hdr h;
	
	time_t *timeconv;
	long tmsecs;
	size_t date_maxsize = 25;
	struct tm *local;
	
	struct stat st;
	struct dirent *direntry;
	DIR *dirp;
	
	
	while ((c = getopt(argc, argv, "tvxqdAw")) != -1)
	{
		switch(c)
		{	
			case 't': //print concise table of archive contents
			{
				fd = open(archiveFile, O_RDONLY);
				if (fd == -1) //the file didn't open correctly
				{   
					printf("There was an error opening the file. Check the path and file name.\n");
					exit(1);
				}
				filecheck(fd);
				file_endbyte = lseek(fd, 0, SEEK_END);
				
				file_curbyte += SARMAG;
				
				while (file_curbyte < file_endbyte) //loops until no more data in file
				{				
					filename = malloc(sizeof(h.ar_name) * sizeof(char));
					pread(fd, filename, sizeof(h.ar_name), file_curbyte); //get filename
					j=0;
					for (i=0; i<sizeof(filename); i++)
					{
						if (filename[i] == '/')
						{
							filename[i] = '\0';
							filename = realloc(filename, j+1);
							break;
						}
						j++;
					}
					printf("%s\n", filename);
					
					file_curbyte += (hdr_filesize_offset);
					
					pread(fd, &filesize, sizeof(h.ar_size), file_curbyte);
					
					if (atoi(filesize) % 2 == 1) //if filesize is odd number; if so, increment for proper offset position
					{
						file_curbyte += (atoi(filesize) + sizeof(h.ar_size) + sizeof(h.ar_fmag) + 1);
					}
					else
					{
						file_curbyte += (atoi(filesize) + sizeof(h.ar_size) + sizeof(h.ar_fmag));
					}
					
				}
				break;	
			}
		
			case 'v': //print a verbose table of contents
			{
				fd = open(archiveFile, O_RDONLY);
				if (fd == -1) //the file didn't open correctly
				{   
					printf("There was an error opening the file. Check the path and file name.\n");
					exit(1);
				}
				filecheck(fd);
				file_endbyte = lseek(fd, 0, SEEK_END);

				file_curbyte += SARMAG; //current byte = 8
				
				while (file_curbyte < file_endbyte) //loops until no more data in file
				{
				/*=======================GET FILENAME==========================*/	
					filename = malloc(sizeof(h.ar_name) * sizeof(char));
					pread(fd, filename, sizeof(h.ar_name), file_curbyte); //get filename
					j=0;
					for (i=0; i<sizeof(filename); i++)
					{
						if (filename[i] == '/')
						{
							filename[i] = '\0';
							filename = realloc(filename, j+1);
							break;
						}
						j++;
					}
					
				/*=======================GET TIMESTAMP==========================*/

					file_curbyte += (hdr_modtime_offset - hdr_filename_offset); //current byte = 24				
					pread(fd, &mod_time, sizeof(h.ar_date), file_curbyte);
					
					//the following section converts Epoch time to the following format: Apr 19 21:08 2014
					
					tmsecs = atol(mod_time);
					timeconv = (time_t*)&tmsecs;
					temptimestring = ctime(timeconv);
					
					//Sat Apr 19 21:08:01 2014
					local = localtime(timeconv);
					strptime(temptimestring, "%b %d %T %Y", local);
					
					timestring = malloc(date_maxsize * sizeof(char));
					strftime(timestring, date_maxsize, "%b %d %H:%M %Y", local);
															
					
				/*=======================GET USERID==========================*/

					file_curbyte += (hdr_userid_offset - hdr_modtime_offset);  //current byte = 52			
					pread(fd, &userid, sizeof(h.ar_uid), file_curbyte);
					
					
				/*=======================GET GROUPID==========================*/

					file_curbyte += (hdr_groupid_offset - hdr_userid_offset);				
					pread(fd, &groupid, sizeof(h.ar_gid), file_curbyte);
					
					
				/*=======================GET MODE PERMISSIONS ==========================*/
					
					file_curbyte += (hdr_mode_offset - hdr_groupid_offset);				
					pread(fd, &mode, sizeof(h.ar_mode), file_curbyte);			
					
					while (b <= 6)
					{
						
						for (p = 3; p <= 5; p++)
						{
							if (mode[p] == '7')
							{
								permstring[b] = 'r';
								permstring[b+1] = 'w';
								permstring[b+2] = 'x';
							}
							else if (mode[p] == '6')
							{
								permstring[b] = 'r';
								permstring[b+1] = 'w';
								permstring[b+2] = '-';
							}

							else if (mode[p] == '5')
							{
								permstring[b] = 'r';
								permstring[b+1] = '-';
								permstring[b+2] = 'x';
							}

							else if (mode[p] == '4')
							{
								permstring[b] = 'r';
								permstring[b+1] = '-';
								permstring[b+2] = '-';
							}

							else if (mode[p] == '3')
							{
								permstring[b] = '-';
								permstring[b+1] = 'w';
								permstring[b+2] = 'x';
							}

							else if (mode[p] == '2')
							{
								permstring[b] = '-';
								permstring[b+1] = 'w';
								permstring[b+2] = '-';
							}

							else if (mode[p] == '1')
							{
								permstring[b] = '-';
								permstring[b+1] = '-';
								permstring[b+2] = 'x';
							}

							else
							{
								permstring[b] = '-';
								permstring[b+1] = '-';
								permstring[b+2] = '-';
							}
							
							b += 3;
						}
					}
					
					

				/*=======================GET FILESIZE==========================*/
				
					file_curbyte += (hdr_filesize_offset - hdr_mode_offset);
					pread(fd, &filesize, sizeof(h.ar_size), file_curbyte);
					
					if (atoi(filesize) % 2 == 1) //if filesize is odd number; if so, increment for proper offset position
					{
						file_curbyte += (atoi(filesize) + sizeof(h.ar_size) + sizeof(h.ar_fmag) + 1);
					}
					else
					{
						file_curbyte += (atoi(filesize) + sizeof(h.ar_size) + sizeof(h.ar_fmag));
					}
					
				/*=======================PRINT FORMATTED MEMBER FILE STRING==========================*/	

					printf("%s %d/%d%7d %s %s\n", permstring, atoi(userid), atoi(groupid),
							atoi(filesize), timestring, filename);	
				}
				break;
			}
			
			case 'x': //extract archive member files; if named, extract those, otherwise extract all
			{

				fd = open(archiveFile, O_RDONLY);
				if (fd == -1) //the file didn't open correctly
				{   
					printf("There was an error opening the file. Check the path and file name.\n");
					exit(1);
				}
				filecheck(fd);
				file_endbyte = lseek(fd, 0, SEEK_END);
				
				file_curbyte += SARMAG;
				
				while (file_curbyte < file_endbyte) //loops until no more data in file
				{				
					filename = malloc(sizeof(h.ar_name) * sizeof(char));
					pread(fd, filename, sizeof(h.ar_name), file_curbyte); //get filename
					j=0;
					for (i=0; i<sizeof(filename); i++)
					{
						if (filename[i] == '/')
						{
							filename[i] = '\0';
							filename = realloc(filename, j+1);
							break;
						}
						j++;
					}
					
					//read octal permission for later use		
					pread(fd, &mode, sizeof(h.ar_mode), (file_curbyte + hdr_mode_offset));
					
					//the following method to convert the octal value into something that could be used in the open() function was suggested in the following post: https://piazza.com/class/hsqoyehzu0l4ro?cid=136
					
					octlong = strtol(mode, &modestring, 8);
					
					file_curbyte += (hdr_filesize_offset);
					
					pread(fd, &filesize, sizeof(h.ar_size), file_curbyte); //get filesize string
					filesize_int = atoi(filesize);
					
				/*=======CHECK IF MEMBER ARGS SUPPLIED ========*/	
					memflag = 0;
					if (argc > 3) //check member arguments are supplied; if yes, set memflag
					{
						for (i=3; i < argc; i++)
						{
							//if (strcmp(filename, argv[i]) == 0) //if match found, extract member
							if (strcmp(filename, argv[i]) == 0) //if match found, extract member
							{
								memflag = 1;
							}
						}
						
						if (memflag)
						{
							
							//open fd_memextract, create if doesn't already exist
							fd_memextract = open(filename, O_CREAT | O_RDWR, octlong);
							
							if (fd_memextract == -1) //extracted member file couldn't be created correctly
							{   
								perror("There was an error opening/creating member file.\n");
								exit(-1);
							}	
							
							file_curbyte += (sizeof(h.ar_size) + sizeof(h.ar_fmag)); //moves to start of data
							
							num_read = 0;
							num_written = 0;
							lseek(fd, file_curbyte, SEEK_SET);
							while (num_written < filesize_int)
							{
								read(fd, buf_rw, BLOCKSIZE);
								write(fd_memextract, buf_rw, BLOCKSIZE);
							
								if (num_written != num_read)
								{
									perror("Error writing file");
									unlink(filename);
									exit(-1);
								}
								num_read++;
								num_written++;
							}	
							
							if (filesize_int % 2 == 1) //if filesize is odd number; if so, increment for proper offset position
							{
								file_curbyte += filesize_int + 1;
							}
							else
							{
								file_curbyte += filesize_int;
							}
							
							close(fd_memextract);
						}
						else
						{

							if (filesize_int % 2 == 1) //if filesize is odd number; if so, increment for proper offset position
							{
								file_curbyte += filesize_int + sizeof(h.ar_size) + sizeof(h.ar_fmag) +  1;
							}
							else
							{
								file_curbyte += filesize_int + sizeof(h.ar_size) + sizeof(h.ar_fmag);
							}
						}
							
					}
					else
					{
						//open fd_memextract, create if doesn't already exist
						fd_memextract = open(filename, O_CREAT | O_RDWR, octlong);
						
						if (fd_memextract == -1) //extracted member file couldn't be created correctly
						{   
							printf("There was an error opening/creating member file.\n");
							exit(1);
						}	
						
						file_curbyte += (sizeof(h.ar_size) + sizeof(h.ar_fmag)); //moves to start of data
						
						num_read = 0;
						num_written = 0;
						lseek(fd, file_curbyte, SEEK_SET);
						while (num_written < filesize_int)
						{
							read(fd, buf_rw, BLOCKSIZE);
							write(fd_memextract, buf_rw, BLOCKSIZE);
						
							if (num_written != num_read)
							{
								perror("Error writing file");
								unlink(filename);
								exit(-1);
							}
							num_read++;
							num_written++;
						}	
						close(fd_memextract);
						
						if (filesize_int % 2 == 1) //if filesize is odd number; if so, increment for proper offset position
						{
							file_curbyte += filesize_int + 1;
						}
						else
						{
							file_curbyte += filesize_int;
						}
					}
										
				}
				break;
			}
			
			case 'q': //QUICK APPEND
			{
				int k;
				char *orig_filename;
				
				
				if (stat(argv[2], &st) == 0) //file exists
				{
					fd = open(archiveFile, O_RDWR | O_APPEND);
					if (fd == -1) //the file didn't open correctly
					{   
						printf("There was an error opening the existing file. Check the path and file name.\n");
						exit(1);
					}
					filecheck(fd);
					//checks for armag header, if missing, its a bad archive
								
					
				}
				else //archive file does not exist, create it
				{
					fd = open(argv[2], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
					if (fd == -1) //the file didn't open correctly
					{  
						perror("Error creating the file you specified");
						unlink(argv[2]);
						exit(-1);
					}

					//add armag header
					if (write(fd, ARMAG, sizeof(ARMAG)) == -1)
					{
							perror("Error writing file");
							unlink(argv[2]);
							exit(-1);
					}				
				}
				
				if (argc <= 3) //no members selected, empty archive, just exit
				{
					close(fd);
					exit(1);
				}
				else //add members to archive
				{
					lseek(fd, SARMAG, SEEK_SET);
					//file_curbyte += SARMAG;
					
					for (i=3; i < argc; i++)
					{
				/*=======================ADD FILENAME==========================*/				
						if (stat(argv[i], &st) == -1) //file exists?
						{
							printf("The file argument %s doesn't exist. Not added.\n", argv[i]);
						}
							
						orig_filename = malloc(sizeof(argv[i]) * sizeof(char));
						strcpy(orig_filename, argv[i]);
						
						filename = malloc(sizeof(h.ar_name) * sizeof(char));
						strcpy(filename, argv[i]);
						
						strcat(filename, "/");
						
						write(fd, filename, strlen(filename));
						
						spaces = malloc(30 * sizeof(char));
						fill_spaces = sizeof(h.ar_name) - strlen(filename);
						for (k=0; k<fill_spaces; k++)
						{
							strcat(spaces, " ");
						}
						write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to time offset
						
						
				/*=======================ADD TIME STAMP==========================*/
				
						temptimestring = malloc(sizeof(argv[i]) * sizeof(char));
						
						snprintf(temptimestring, sizeof(h.ar_date), "%d", (int)(st.st_mtime));
						
						write(fd, temptimestring, strlen(temptimestring));								
						spaces = malloc(30 * sizeof(char));
						fill_spaces = sizeof(h.ar_date) - strlen(temptimestring);
						for (k=0; k<fill_spaces; k++)
						{
							strcat(spaces, " ");
						}
						write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to userid offset
				/*=======================ADD USERID==========================*/
  			
						snprintf(userid, sizeof(h.ar_uid), "%d", (int)(st.st_uid));
						
						write(fd, userid, strlen(userid));
												
						spaces = malloc(30 * sizeof(char));
						fill_spaces = sizeof(h.ar_uid) - strlen(userid);
						for (k=0; k<fill_spaces; k++)
						{
							strcat(spaces, " ");
						}
						write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to groupid offset
					
					
				/*=======================ADD GROUPID==========================*/

						snprintf(groupid, sizeof(h.ar_gid), "%d", (int)(st.st_gid));
						
						write(fd, groupid, strlen(groupid));
						
						spaces = malloc(30 * sizeof(char));
						fill_spaces = sizeof(h.ar_gid) - strlen(groupid);
						for (k=0; k<fill_spaces; k++)
						{
							strcat(spaces, " ");
						}
						write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to mode offset			
					
				/*=======================ADD MODE PERMISSIONS ==========================*/
					
						snprintf(mode, sizeof(h.ar_mode), "%o", (int)(st.st_mode));
						
						write(fd, mode, strlen(mode));
						
						spaces = malloc(30 * sizeof(char));
						fill_spaces = sizeof(h.ar_mode) - strlen(mode);
						for (k=0; k<fill_spaces; k++)
						{
							strcat(spaces, " ");
						}
						write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to size offset	

				/*=======================ADD FILESIZE ==========================*/
						snprintf(filesize, sizeof(h.ar_size), "%d", (int)(st.st_size));
						
						//lseek(fd, file_curbyte, SEEK_SET);
						write(fd, filesize, strlen(filesize));
						
						spaces = malloc(30 * sizeof(char));
						fill_spaces = sizeof(h.ar_size) - strlen(filesize);
						for (k=0; k<fill_spaces; k++)
						{
							strcat(spaces, " ");
						}
						write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to arfmag offset						


				/*=======================ADD MAGIC FILE ==========================*/
						write(fd, ARFMAG, strlen(ARFMAG));
				
				/*=======================ADD FILE DATA ==========================*/
				
						//open member file specified in argv[i] to get data from
						fd_data = open(orig_filename, O_RDWR);
						if (fd_data == -1) //the file to get data from didn't open correctly
						{   
							printf("There was an error opening the file to get data from.\n");
							exit(1);
						}
						
						num_read = 0;
						num_written = 0;
						//lseek(fd_data, 0, SEEK_SET);
						while (num_written < st.st_size)
						{
							read(fd_data, buf_rw, BLOCKSIZE);
							write(fd, buf_rw, BLOCKSIZE);
						
							if (num_written != num_read)
							{
								perror("Error writing file");
								unlink(filename);
								exit(-1);
							}
							num_read++;
							num_written++;
						}

						//set position of current byte ready to add more elements if available
						filesize_int = st.st_size;
						if (filesize_int % 2 == 1) //if filesize is odd number; if so, increment for proper offset position
						{
							write(fd, "\n", BLOCKSIZE);
						}
						close(fd_data);
												
					}
					
					lseek(fd, -1, SEEK_END);

					close(fd);
				}
				break;
			}
			
			
			case 'A': //APPEND ALL REGULAR FILES IN DIRECTORY (except archive)
			{
				int k;
				char *orig_filename;
				
				
				if (stat(argv[2], &st) == 0) //file exists
				{
					fd = open(archiveFile, O_WRONLY | O_APPEND);
					if (fd == -1) //the file didn't open correctly
					{   
						printf("There was an error opening the existing file. Check the path and file name.\n");
						exit(1);
					}
					filecheck(fd);								
				}
				else //archive file does not exist, create it
				{
					fd = open(argv[2], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
					if (fd == -1) //the file didn't open correctly
					{  
						perror("Error creating the file you specified");
						unlink(argv[2]);
						exit(-1);
					}

					//add armag header
					if (write(fd, ARMAG, sizeof(ARMAG)) == -1)
					{
							perror("Error writing file");
							unlink(argv[2]);
							exit(-1);
					}				
				}
				
				if (argc > 3) //member argument(s) specified, ignore it
				{
					printf("Member arguments are ignored for this option.\n");
				}
				else //add members to archive
				{

				lseek(fd, SARMAG, SEEK_SET);

				/*=============ITERATE THROUGH DIR, ADD REG FILES==================*/
					
					//referenced: http://manpages.courier-mta.org/htmlman3/scandir.3.html
					direntry = malloc(sizeof(direntry)); 
					
					
					if ((dirp = opendir(".")) == NULL)
					{
						perror("Error opening directory");
						exit(-1);
					}
					else
					{
						//evaluate if file is regular
						while ((direntry = readdir(dirp)) != NULL)
						{
							stat(direntry->d_name, &st);
							if ((direntry->d_type == DT_REG) && (strcmp(direntry->d_name, archiveFile) !=0)) //regular file and not archive file? 
							{
					
						/*=======================ADD FILENAME==========================*/				
									
								orig_filename = malloc(sizeof(direntry->d_name) * sizeof(char));
								strcpy(orig_filename, direntry->d_name);
								
								filename = malloc(sizeof(h.ar_name) * sizeof(char));
								strcpy(filename, direntry->d_name);
								
								strcat(filename, "/");
								
								write(fd, filename, strlen(filename));
								
								spaces = malloc(30 * sizeof(char));
								fill_spaces = sizeof(h.ar_name) - strlen(filename);
								for (k=0; k<fill_spaces; k++)
								{
									strcat(spaces, " ");
								}
								write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to time offset
								
								
						/*=======================ADD TIME STAMP==========================*/
				
								temptimestring = malloc(sizeof(argv[i]) * sizeof(char));
								
								snprintf(temptimestring, sizeof(h.ar_date), "%d", (int)(st.st_mtime));
								
								write(fd, temptimestring, strlen(temptimestring));								
								spaces = malloc(30 * sizeof(char));
								fill_spaces = sizeof(h.ar_date) - strlen(temptimestring);
								for (k=0; k<fill_spaces; k++)
								{
									strcat(spaces, " ");
								}
								write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to userid offset
						/*=======================ADD USERID==========================*/

								//file_curbyte += (hdr_userid_offset - hdr_modtime_offset);  			
								snprintf(userid, sizeof(h.ar_uid), "%d", (int)(st.st_uid));
								
								//lseek(fd, file_curbyte, SEEK_SET);
								write(fd, userid, strlen(userid));
														
								spaces = malloc(30 * sizeof(char));
								fill_spaces = sizeof(h.ar_uid) - strlen(userid);
								for (k=0; k<fill_spaces; k++)
								{
									strcat(spaces, " ");
								}
								write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to groupid offset
							
							
						/*=======================ADD GROUPID==========================*/

								snprintf(groupid, sizeof(h.ar_gid), "%d", (int)(st.st_gid));
								
								//lseek(fd, file_curbyte, SEEK_SET);
								write(fd, groupid, strlen(groupid));
								
								spaces = malloc(30 * sizeof(char));
								fill_spaces = sizeof(h.ar_gid) - strlen(groupid);
								for (k=0; k<fill_spaces; k++)
								{
									strcat(spaces, " ");
								}
								write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to mode offset			
							
						/*=======================ADD MODE PERMISSIONS ==========================*/
							
								snprintf(mode, sizeof(h.ar_mode), "%o", (int)(st.st_mode));
								
								//lseek(fd, file_curbyte, SEEK_SET);
								write(fd, mode, strlen(mode));
								
								spaces = malloc(30 * sizeof(char));
								fill_spaces = sizeof(h.ar_mode) - strlen(mode);
								for (k=0; k<fill_spaces; k++)
								{
									strcat(spaces, " ");
								}
								write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to size offset	

						/*=======================ADD FILESIZE ==========================*/
								snprintf(filesize, sizeof(h.ar_size), "%d", (int)(st.st_size));
								
								//lseek(fd, file_curbyte, SEEK_SET);
								write(fd, filesize, strlen(filesize));
								
								spaces = malloc(30 * sizeof(char));
								fill_spaces = sizeof(h.ar_size) - strlen(filesize);
								for (k=0; k<fill_spaces; k++)
								{
									strcat(spaces, " ");
								}
								write(fd, spaces, strlen(spaces)); //adds appropriate num spaces to get to arfmag offset						


						/*=======================ADD MAGIC FILE ==========================*/
								write(fd, ARFMAG, strlen(ARFMAG));
						
						/*=======================ADD FILE DATA ==========================*/
						
								//open member file specified in argv[i] to get data from
								fd_data = open(orig_filename, O_RDWR);
																
								num_read = 0;
								num_written = 0;
								//lseek(fd_data, 0, SEEK_SET);
								while (num_written < st.st_size)
								{
									read(fd_data, buf_rw, BLOCKSIZE);
									write(fd, buf_rw, BLOCKSIZE);
								
									if (num_written != num_read)
									{
										perror("Error writing file");
										unlink(filename);
										exit(-1);
									}
									num_read++;
									num_written++;
								}

								//set position of current byte ready to add more elements if available
								filesize_int = st.st_size;
								if (filesize_int % 2 == 1) //if filesize is odd number; if so, increment for proper offset position
								{
									write(fd, "\n", BLOCKSIZE);
								}
								close(fd_data);
														
							}
						}
					
					lseek(fd, -1, SEEK_END);

					close(fd);
					}
				}
				break;
			}
			
			case 'd': //delete archive member files; if named, extract those, otherwise extract all
			{
				int k;
				
				if (argc <= 3) //no members selected, no change to archive, just exit
				{
					exit(1);
				}
				else //add members to new archive, exclude member arguments from copy
				{
				
					fd = open(archiveFile, O_RDWR);
					if (fd == -1) //the file didn't open correctly
					{   
						perror("There was an error opening the archive file. Check the path and file name.\n");
						exit(-1);
					}
					filecheck(fd);
					unlink(archiveFile);
					
					//open fd_newarchive to replace old archive
					fd_newarchive = open(archiveFile, O_CREAT | O_RDWR, 0666);
					
					if (fd_newarchive == -1) 
					{   
						perror("There was an error creating new archive file.\n");
						exit(-1);
					}
					
					//add armag header
					if (write(fd_newarchive, ARMAG, sizeof(ARMAG)) == -1)
					{
						perror("Error writing file");
						unlink(archiveFile);
						exit(-1);
					}				
					
					file_curbyte += SARMAG;
					file_endbyte = lseek(fd, 0, SEEK_END);				
					lseek(fd_newarchive, SARMAG, SEEK_SET); //set position for copy start after SARMAG
										
					while (file_curbyte < file_endbyte) //loops until no more data in file
					{
					
						/*=======================GET FILENAME FROM ORIG ARCHIVE==========================*/	
						filename = malloc(sizeof(h.ar_name) * sizeof(char));
						pread(fd, filename, sizeof(h.ar_name), file_curbyte); //get filename from orig archive; assign to filename variable
						j=0;
						for (i=0; i<sizeof(filename); i++)
						{
							if (filename[i] == '/')
							{
								filename[i] = '\0';
								filename = realloc(filename, j+1);
								break;
							}
							j++;
						}			
						
						
						//get filesize info for advancing the orig archive pointer if file is marked for deletion
						pread(fd, &filesize, sizeof(h.ar_size), (file_curbyte + hdr_filesize_offset));
						
						j=0;
						for (i=0; i<sizeof(filesize); i++)
						{
							if (filesize[i] == ' ')
							{
								filesize[i] = '\0';
								break;
							}
							j++;
						}	
							
						filesize_int = atoi(filesize);
						
						memflag = 1;
						for (i=3; i < argc; i++)
						{						
							if (strcmp(filename, argv[i]) != 0) //if no match found, file will not be copied to new archive
							{
								memflag = 0; //sets to copy
							}
						}
							
						if (memflag == 0) //copy file to new archive
						{
								
							/*=======================ADD FILENAME TO NEW ARCHIVE==========================*/
							strcat(filename, "/");
							write(fd_newarchive, filename, strlen(filename));
							
							spaces = malloc(30 * sizeof(char));
							fill_spaces = sizeof(h.ar_name) - strlen(filename);
							for (k=0; k<fill_spaces; k++)
							{
								strcat(spaces, " ");
							}
							write(fd_newarchive, spaces, strlen(spaces)); //adds appropriate num spaces to get to time offset

							
							/*=======================GET TIMESTAMP FROM ORIG ARCHIVE==========================*/
							file_curbyte += (hdr_modtime_offset - hdr_filename_offset); 				
							pread(fd, &mod_time, sizeof(h.ar_date), file_curbyte); //get time from orig archive
							
							/*=======================ADD TIME STAMP TO NEW ARCHIVE==========================*/							
							j=0;
							for (i=0; i<sizeof(mod_time); i++)
							{
								if (mod_time[i] == ' ')
								{
									mod_time[i] = '\0';
									break;
								}
								j++;
							}	
							
							write(fd_newarchive, mod_time, strlen(mod_time));								

							spaces = malloc(30 * sizeof(char));
							fill_spaces = sizeof(h.ar_date) - strlen(mod_time);
							for (k=0; k<fill_spaces; k++)
							{
								strcat(spaces, " ");
							}
							write(fd_newarchive, spaces, strlen(spaces)); //adds appropriate num spaces to get to userid offset

							
							/*=======================GET USERID FROM ORIG ARCHIVE==========================*/
							file_curbyte += (hdr_userid_offset - hdr_modtime_offset); 			
							pread(fd, &userid, sizeof(h.ar_uid), file_curbyte); //get userid from orig archive
						
							/*=======================ADD USERID TO NEW ARCHIVE==========================*/
							j=0;
							for (i=0; i<sizeof(userid); i++)
							{
								if (userid[i] == ' ')
								{
									userid[i] = '\0';
									break;
								}
								j++;
							}	
							
							write(fd_newarchive, userid, strlen(userid));						
													
							spaces = malloc(30 * sizeof(char));
							fill_spaces = sizeof(h.ar_uid) - strlen(userid);
							for (k=0; k<fill_spaces; k++)
							{
								strcat(spaces, " ");
							}
							write(fd_newarchive, spaces, strlen(spaces)); //adds appropriate num spaces to get to groupid offset

							
							/*====================GET GROUPID FROM ORIG ARCHIVE==========================*/
							file_curbyte += (hdr_groupid_offset - hdr_userid_offset);				
							pread(fd, &groupid, sizeof(h.ar_gid), file_curbyte);
						
							/*=====================ADD GROUPID TO NEW ARCHIVE==========================*/	
							j=0;
							for (i=0; i<sizeof(groupid); i++)
							{
								if (groupid[i] == ' ')
								{
									groupid[i] = '\0';
									break;
								}
								j++;
							}	
							
							write(fd_newarchive, groupid, strlen(groupid));
							
							spaces = malloc(30 * sizeof(char));
							fill_spaces = sizeof(h.ar_gid) - strlen(groupid);
							for (k=0; k<fill_spaces; k++)
							{
								strcat(spaces, " ");
							}
							write(fd_newarchive, spaces, strlen(spaces)); //adds appropriate num spaces to get to mode offset

							
							/*=======================GET MODE PERMISSIONS FROM ORIG ARCHIVE==========================*/					
							file_curbyte += (hdr_mode_offset - hdr_groupid_offset);				
							pread(fd, &mode, sizeof(h.ar_mode), file_curbyte);
						
							/*=======================ADD MODE PERMISSIONS TO NEW ARCHIVE ==========================*/
							j=0;
							for (i=0; i<sizeof(mode); i++)
							{
								if (mode[i] == ' ')
								{
									mode[i] = '\0';
									break;
								}
								j++;
							}	
							
							write(fd_newarchive, mode, strlen(mode));
							
							spaces = malloc(30 * sizeof(char));
							fill_spaces = sizeof(h.ar_mode) - strlen(mode);
							for (k=0; k<fill_spaces; k++)
							{
								strcat(spaces, " ");
							}
							write(fd_newarchive, spaces, strlen(spaces)); //adds appropriate num spaces to get to size offset	

							/*=======================GET FILESIZE FROM ORIG ARCHIVE==========================*/				
							file_curbyte += (hdr_filesize_offset - hdr_mode_offset);
							
							/*=======================ADD FILESIZE TO NEW ARCHIVE==========================*/
							write(fd_newarchive, filesize, strlen(filesize));
							
							spaces = malloc(30 * sizeof(char));
							fill_spaces = sizeof(h.ar_size) - strlen(filesize);
							for (k=0; k<fill_spaces; k++)
							{
								strcat(spaces, " ");
							}
							write(fd_newarchive, spaces, strlen(spaces)); //adds appropriate num spaces to get to arfmag offset						

							/*=======================GET MAGE FILE FROM ORIG ARCHIVE==========================*/				
							file_curbyte += (hdr_filemagic_offset - hdr_filesize_offset);
							
							/*=======================ADD MAGIC FILE TO NEW ARCHIVE==========================*/
							write(fd_newarchive, ARFMAG, strlen(ARFMAG));

							/*===================MOVE POINTER TO ORIG ARCHIVE DATA START========================*/
							file_curbyte += sizeof(h.ar_fmag); //moves to start of data
															
							/*=======================ADD FILE DATA TO NEW ARCHIVE==========================*/
							
							memset(buf_rw, 0, sizeof(buf_rw));
							num_read = 0;
							num_written = 0;
							lseek(fd, file_curbyte, SEEK_SET);
							while (num_written < filesize_int)
							{
								read(fd, buf_rw, BLOCKSIZE);
								write(fd_newarchive, buf_rw, BLOCKSIZE);
							
								if (num_written != num_read)
								{
									perror("Error writing file");
									unlink(filename);
									exit(-1);
								}
								num_read++;
								num_written++;
							}

							
							//moves orig archive pointer to start of next read section
							if (filesize_int % 2 == 1) //if filesize is odd number; if so, increment for proper offset position
							{
								file_curbyte += filesize_int + 1;
								write(fd_newarchive, "\n", BLOCKSIZE);
							}
							else
							{
								file_curbyte += filesize_int;
							}
													
						}
						else
						{
							//skipping file that won't be included (deleted from orig archive)
							if (filesize_int % 2 == 1) //if filesize is odd number; if so, increment for proper offset position
							{
								file_curbyte += 60 + filesize_int + 1;
							}
							else
							{
								file_curbyte += 60+ filesize_int;
							}
							
						}
					}
					
					close(fd);
					lseek(fd_newarchive, -1, SEEK_END);
					close(fd_newarchive);
				}
				break;
			}
		}
	}

	return 0;
}

void filecheck(int filedesc)
{
	off_t startbyte = 0;
	char checkARMAG[8];
	
	pread(filedesc, checkARMAG, SARMAG, startbyte);
	
	if(strcmp(checkARMAG, ARMAG) != 0)
	{
		printf("The archive you're attempting to open is corrupt.\n");
		exit(1);
	}
}