/*
 * t2.c
 *
 *  Created on: Jan 24, 2018
 *      Author: im
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <langinfo.h>
#include <time.h>
#include <locale.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>

#define S_IFWHT 0160000
#define BUF_SIZE 256
DIR *dir;
struct dirent *dp;
struct stat statbuf;
struct passwd *pwd;
struct group *grp;
struct tm *tm;
struct tm *tm_acc;
struct tm *tm_stC;
struct stat sb;

int flag = 0;
int flagR = 0;
char optN, optI, optA, optD, cmpTime, cmpName, typeOfFile;
char datestring[256];
char datestring_acc[256];
char datestring_stC[256];
char dateDir[256];
char buf[256];
char bufLink[256];
int linkRead = 0;
__mode_t mode, m;
__blkcnt_t block;
__ino_t inode;
int total = 0;
int totalD = 0;
int count = 0;
int cnt = 0;
int c1 = 0;
int c2 = 0;
size_t len;
char month[5], month2[5];
int day, hh, mm, ss, year, numOfMonth, day2, hh2, mm2, ss2, year2, numOfMonth2;
int options[256];
int file_start = 1;

void printPermissions(__mode_t mode);
char checkTypeOfFile(__mode_t m);
int struct_cmp_by_name(const void *a, const void *b);
int struct_cmp_by_size(const void *a, const void *b);
int struct_cmp_by_timeM(const void *a, const void *b);
void getDirInfo(char *ch);

struct file {
	unsigned long link;
	char pwname[100];
	int st_id;
	char grname[100];
	int g_id;
	long int st_size;
	char date[100];
	char date_access[100];
	char date_stChange[100];
	char name[100];
	__mode_t mode;
	__ino_t inode;
};

struct file_command {
	char nameC[100];
	char type;
};

struct file_command f[100];

int main(int argc, char **argv) {

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			file_start = i;
			break;
		}
		for (int j = 0; j < strlen(argv[i]); j++) {
			options[argv[i][j]] = 1;
		}
	}

	for (int i = 0; i < 256; i++) {
		if (options[i] == 1) {
			cnt++;
		}
	}

	if (cnt == 0) {
		options['h'] = 1;
		cnt = 2;
	}

	for (int k = file_start; k < argc; k++) {
		strcpy(f[k].nameC, argv[k]);
		lstat(f[k].nameC, &statbuf);
		m = statbuf.st_mode;
		f[k].type = checkTypeOfFile(m);
	}

	for (int k = file_start; k < argc; k++) {
		if (f[k].type == 'd') {
			getDirInfo(f[k].nameC);
		} else if (f[k].type == 'r') {
			//getRegfileInfo(f[k].type);
		}
	}

	exit(0);
}

char checkTypeOfFile(__mode_t m) {

	if (S_ISDIR(m) != 0) {                   //directory
		typeOfFile = 'd';
	} else if (S_ISFIFO(m) != 0) {           //fifo
		typeOfFile = 'f';
	} else if (S_ISLNK(m) != 0) {            //symbolic link
		typeOfFile = 'l';
	} else if (S_ISSOCK(m) != 0) {           //socket
		typeOfFile = 's';
	} else if (m & S_IXUSR) {                //executable
		typeOfFile = 'e';
	} else if (((m) & S_IFMT) == S_IFWHT) {  //whiteout
		typeOfFile = 'w';
	} else if (S_ISCHR(m) != 0) {            //char special
		typeOfFile = 'c';
	} else if (S_ISBLK(m) != 0) {            //block special
		typeOfFile = 'b';
	} else if (S_ISREG(m) != 0) {            //regular
		typeOfFile = 'r';
	}
	return typeOfFile;
}

void printPermissions(__mode_t mode) {

	if (checkTypeOfFile(mode) == 'd') {
		printf("d");
	} else if (checkTypeOfFile(mode) == 'b') {
		printf("b");
	} else if (checkTypeOfFile(mode) == 'c') {
		printf("c");
	} else if (checkTypeOfFile(mode) == 'f') {
		printf("f");
	} else if (checkTypeOfFile(mode) == 'l') {
		printf("l");
	} else if (checkTypeOfFile(mode) == 's') {
		printf("s");
	} else {
		printf("-");
	}

	if ((mode & S_IRUSR) && (mode & S_IREAD)) {
		printf("r");
	} else {
		printf("-");
	}
	if ((mode & S_IWUSR) && (mode & S_IWRITE)) {
		printf("w");
	} else {
		printf("-");
	}
	if ((mode & S_IXUSR) && (mode & S_IEXEC)) {
		printf("x");
	} else {
		printf("-");
	}
	if ((mode & S_IRGRP) && (mode & S_IREAD)) {
		printf("r");
	} else {
		printf("-");
	}
	if ((mode & S_IWGRP) && (mode & S_IWRITE)) {
		printf("w");
	} else {
		printf("-");
	}

	if ((mode & S_IXGRP) && (mode & S_IEXEC)) {
		printf("x");
	} else {
		printf("-");
	}
	if ((mode & S_IROTH) && (mode & S_IREAD)) {
		printf("r");
	} else {
		printf("-");
	}
	if ((mode & S_IWOTH) && (mode & S_IWRITE)) {
		printf("w");
	} else {
		printf("-");
	}
	if ((mode & S_IXOTH) && (mode & S_IEXEC)) {
		printf("x");
	} else {
		printf("-");
	}
}

int struct_cmp_by_name(const void *a, const void *b) {
	struct file *ia = (struct file *) a;
	struct file *ib = (struct file *) b;
	if (cmpName == 'r')
		return strcmp(ib->name, ia->name);
	else
		return strcmp(ia->name, ib->name);
}

int struct_cmp_by_size(const void *a, const void *b) {
	struct file *ia = (struct file *) a;
	struct file *ib = (struct file *) b;
	return (int) (ib->st_size - ia->st_size);
}

int struct_cmp_by_timeM(const void *a, const void *b) {

	struct file *ia = (struct file *) a;
	struct file *ib = (struct file *) b;

	if (cmpTime == 't') {
		sscanf(ib->date, "%s %d %d:%d %d %d %d", month2, &day2, &hh2, &mm2, &year2, &numOfMonth2, &ss2);
		sscanf(ia->date, "%s %d %d:%d %d %d %d", month, &day, &hh, &mm, &year, &numOfMonth, &ss);
	} else if (cmpTime == 'c') {
		sscanf(ib->date_stChange, "%s %d %d:%d %d %d %d", month2, &day2, &hh2, &mm2, &year2, &numOfMonth2, &ss2);
		sscanf(ia->date_stChange, "%s %d %d:%d %d %d %d", month, &day, &hh, &mm, &year, &numOfMonth, &ss);
	} else if (cmpTime == 'u') {
		sscanf(ib->date_access, "%s %d %d:%d %d %d %d", month2, &day2, &hh2, &mm2, &year2, &numOfMonth2, &ss2);
		sscanf(ia->date_access, "%s %d %d:%d %d %d %d", month, &day, &hh, &mm, &year, &numOfMonth, &ss);
	}

	if ((year == year2) && (numOfMonth == numOfMonth2) && (day == day2) && (hh == hh2) && (mm == mm2)) {
		return (int) (ss < ss2);
	}
	if ((year == year2) && (numOfMonth == numOfMonth2) && (day == day2) && (hh == hh2)) {
		return (int) (mm < mm2);
	}
	if ((year == year2) && (numOfMonth == numOfMonth2) && (day == day2)) {
		return (int) (hh < hh2);
	}
	if ((year == year2) && (numOfMonth == numOfMonth2)) {
		return (int) (day < day2);
	}
	if (year == year2) {
		return (int) (numOfMonth < numOfMonth2);
	}
	if (year != year2) {
		return (int) (year < year2);
	}
}

void getDirInfo(char *ch) {

	int flagPD = 1;
	int c1 = 0, c2 = 0;

	dir = opendir(ch);

	while ((dp = readdir(dir)) != NULL) {
		if ((strcmp(dp->d_name, ".") == 0) || (strcmp(dp->d_name, "..") == 0)) {
			sprintf(buf, "%s/%s", ch, dp->d_name);
			lstat(buf, &statbuf);
			totalD = totalD + statbuf.st_blocks;
			if (flagR) {
				if (flagPD) {
					printf("\n");
					printf("%s/ ", ch);
					printf("\n");
					flagPD = 0;
				}
				printf("%s \n", dp->d_name);
			}
		} else {
			sprintf(buf, "%s/%s", ch, dp->d_name);
			lstat(buf, &statbuf);
			total = total + statbuf.st_blocks;
			count++;
			if (flagR) {
				if (flagPD) {
					printf("\n");
					printf("%s/ ", ch);
					printf("\n");
					flagPD = 0;
				}
				printf("%s ", dp->d_name);
				printf("\n");
			}
		}
	}

	c1 = count + 1;
	c2 = count + 2;
	count = 0;
	closedir(dir);
	opendir(ch);

	struct file files[c1 + 1];

	while ((dp = readdir(dir)) != NULL && c1 >= 0) {
		sprintf(buf, "%s/%s", ch, dp->d_name);
		lstat(buf, &statbuf);

		files[c1].link = statbuf.st_nlink;

		if ((pwd = getpwuid(statbuf.st_uid)) != NULL && flag == 0) {
			strcpy(files[c1].pwname, pwd->pw_name);
		} else {
			files[c1].st_id = statbuf.st_uid;
		}

		if ((grp = getgrgid(statbuf.st_gid)) != NULL && flag == 0) {
			strcpy(files[c1].grname, grp->gr_name);
		} else {
			files[c1].g_id = statbuf.st_gid;
		}

		files[c1].st_size = statbuf.st_size;

		files[c1].mode = statbuf.st_mode;

		files[c1].inode = statbuf.st_ino;

		tm = localtime(&statbuf.st_mtime);
		strftime(datestring, sizeof(datestring), "%b %d %H:%M %Y %m %S", tm);
		strcpy(files[c1].date, datestring);

		tm_acc = localtime(&statbuf.st_atime);
		strftime(datestring_acc, sizeof(datestring_acc), "%b %d %H:%M %Y %m %S", tm_acc);
		strcpy(files[c1].date_access, datestring_acc);

		tm_stC = localtime(&statbuf.st_ctime);
		strftime(datestring_stC, sizeof(datestring_stC), "%b %d %H:%M %Y %m %S", tm_stC);
		strcpy(files[c1].date_stChange, datestring_stC);

		strcpy(files[c1].name, dp->d_name);
		c1--;
	}

	closedir(dir);

	if (options['l'] || options['n']) {
		cmpTime = 't';
		if (options['c'])
			cmpTime = 'c';
		if (options['u'])
			cmpTime = 'u';
		if (options['n'])
			optN = 'n';
		if (options['i'])
			optI = 'i';

		if (options['d']) {
			lstat(ch, &statbuf);
			tm = localtime(&statbuf.st_mtime);
			strftime(dateDir, sizeof(dateDir), "%b %d %H:%M %Y %m %S", tm);
			printPermissions(statbuf.st_mode);
			printf("%4lu ", statbuf.st_nlink);
			if (!options['n'])
				printf("%-5.8s ", getpwuid(statbuf.st_uid)->pw_name);
			else
				printf("%-5d ", statbuf.st_uid);
			if (!options['n'])
				printf("%-5.8s ", getgrgid(statbuf.st_gid)->gr_name);
			else
				printf("%-5d ", statbuf.st_gid);
			if (options['h']) {
				if (statbuf.st_size < 1024) {
					printf("%5ld ", statbuf.st_size);
				} else if ((statbuf.st_size >= 1024) && (statbuf.st_size < 1000000)) {
					double size = (double) ((double) statbuf.st_size / 1024.0);
					printf("%5.1fK ", size);
				} else if ((statbuf.st_size >= 1000000)) {
					double size = (double) ((double) statbuf.st_size / 1000000.0);
					printf("%5.1fM ", size);
				}
			} else
				printf("%8ld ", statbuf.st_size);
			printf("%.12s ", dateDir);
			printf("%s \n", ch);
		}

		if (options['a'] || options['f']) {
			if (!options['d'])
				printf("total %d\n", (total + totalD) / 2);
		} else {
			if (!options['d'])
				printf("total %d\n", total / 2);
		}

		total = 0;
		totalD = 0;

		if (!options['f']) {
			if ((!options['t']) && (!options['S']) && (!options['r'])) {
				size_t structs_len = sizeof(files) / sizeof(struct file);
				qsort(files, structs_len, sizeof(struct file), struct_cmp_by_name);
			}
		}

		if ((options['t']) && (!options['f'])) {
			cmpTime = 't';
			size_t len = sizeof(files) / sizeof(struct file);
			qsort(files, len, sizeof(struct file), struct_cmp_by_timeM);
		}

		if (options['S'] && (!options['f'])) {
			size_t len = sizeof(files) / sizeof(struct file);
			qsort(files, len, sizeof(struct file), struct_cmp_by_size);
		}

		if (options['a'] || options['f']) {
			if (!options['d']) {
				for (int i = 0; i < c2; i++) {
					if (optI == 'i')
						printf("%ld ", files[i].inode);
					printPermissions(files[i].mode);
					printf(" %4lu ", files[i].link);

					if (optN != 'n')
						printf("%-5.8s ", files[i].pwname);
					else
						printf("%-5d ", statbuf.st_uid);

					if (optN != 'n')
						printf("%-5.8s ", files[i].grname);
					else
						printf("%-5d ", statbuf.st_gid);

					if (options['h']) {
						if (files[i].st_size < 1024) {
							printf("%5ld ", files[i].st_size);
						} else if ((files[i].st_size >= 1024) && (files[i].st_size < 1000000)) {
							double size = (double) ((double) files[i].st_size / 1024.0);
							printf("%5.1fK ", size);
						} else if ((files[i].st_size >= 1000000)) {
							double size = (double) ((double) files[i].st_size / 1000000.0);
							printf("%5.1fM ", size);
						}
					} else
						printf("%8ld ", files[i].st_size);

					if (cmpTime == 't')
						printf("%.12s ", files[i].date);
					else if (cmpTime == 'c')
						printf("%.12s ", files[i].date_stChange);
					else if (cmpTime == 'u')
						printf("%.12s ", files[i].date_access);

					if (options['F']) {
						char f;
						if (checkTypeOfFile(files[i].mode) == 'd')
							f = '/';
						else if (checkTypeOfFile(files[i].mode) == 'e')
							f = '*';
						else if (checkTypeOfFile(files[i].mode) == 'l')
							f = '@';
						else if (checkTypeOfFile(files[i].mode) == 'w')
							f = '%';
						else if (checkTypeOfFile(files[i].mode) == 's')
							f = '=';
						else if (checkTypeOfFile(files[i].mode) == 'f')
							f = '|';
						if (checkTypeOfFile(files[i].mode) == 'l') {
							linkRead = readlink(files[i].name, bufLink, BUF_SIZE);
							bufLink[linkRead] = '\n';
							printf("%s -> %s", files[i].name, bufLink);
						} else
							printf("%s%c \n", files[i].name, f);
						f = ' ';
					} else {
						if (checkTypeOfFile(files[i].mode) == 'l') {
							linkRead = readlink(files[i].name, bufLink, BUF_SIZE);
							bufLink[linkRead] = '\n';
							printf("%s -> %s", files[i].name, bufLink);
						}

						else
							printf("%s \n", files[i].name);
					}
				}
			}
		}

		if (!options['a']) {
			if (!options['d']) {
				for (int i = 0; i < c2; i++) {
					if ((strcmp(files[i].name, ".") == 0) || (strcmp(files[i].name, "..") == 0)) {
					} else {
						if (optI == 'i')
							printf("%ld ", files[i].inode);
						printPermissions(files[i].mode);
						printf(" %4lu ", files[i].link);

						if (optN != 'n')
							printf("%-5.8s ", files[i].pwname);
						else
							printf("%-5d ", statbuf.st_uid);

						if (optN != 'n')
							printf("%-5.8s ", files[i].grname);
						else
							printf("%-5d ", statbuf.st_gid);

						if (options['h']) {
							if (files[i].st_size < 1024) {
								printf("%5ld ", files[i].st_size);
							} else if ((files[i].st_size >= 1024) && (files[i].st_size < 1000000)) {
								double size = (double) ((double) files[i].st_size / 1024.0);
								printf("%5.1fK ", size);
							} else if ((files[i].st_size >= 1000000)) {
								double size = (double) ((double) files[i].st_size / 1000000.0);
								printf("%5.1fM ", size);
							}
						} else
							printf("%8ld ", files[i].st_size);

						if (cmpTime == 't')
							printf("%.12s ", files[i].date);
						else if (cmpTime == 'c')
							printf("%.12s ", files[i].date_stChange);
						else if (cmpTime == 'u')
							printf("%.12s ", files[i].date_access);

						if (options['F']) {
							char f;
							if (checkTypeOfFile(files[i].mode) == 'd')
								f = '/';
							else if (checkTypeOfFile(files[i].mode) == 'e')
								f = '*';
							else if (checkTypeOfFile(files[i].mode) == 'l')
								f = '@';
							else if (checkTypeOfFile(files[i].mode) == 'w')
								f = '%';
							else if (checkTypeOfFile(files[i].mode) == 's')
								f = '=';
							else if (checkTypeOfFile(files[i].mode) == 'f')
								f = '|';
							if (checkTypeOfFile(files[i].mode) == 'l') {
								linkRead = readlink(files[i].name, bufLink, BUF_SIZE);
								bufLink[linkRead] = '\n';
								printf("%s -> %s", files[i].name, bufLink);
							} else
								printf("%s%c \n", files[i].name, f);
							f = ' ';
						} else {
							if (checkTypeOfFile(files[i].mode) == 'l') {
								linkRead = readlink(files[i].name, bufLink, BUF_SIZE);
								bufLink[linkRead] = '\n';
								printf("%s -> %s", files[i].name, bufLink);
							} else
								printf("%s \n", files[i].name);
						}
					}
				}
			}
		}
	}

	if (options['S']) {
		if (!options['f']) {
			size_t len = sizeof(files) / sizeof(struct file);
			qsort(files, len, sizeof(struct file), struct_cmp_by_size);
		}
		if (!options['d']) {
			for (int i = 0; i < c2; i++) {
				if ((strcmp(files[i].name, ".") == 0) || (strcmp(files[i].name, "..") == 0)) {
				} else {
					if (cnt == 2) {
						printf("%s\n", files[i].name);
					}
				}
			}
		} else {
			if (cnt == 2)
				printf("%s\n", ch);
		}
	}

	if (options['t'] || options['c'] || options['u']) {
		if (options['c'])
			cmpTime = 'c';
		else if (options['u'])
			cmpTime = 'u';
		else if (options['t'])
			cmpTime = 't';
		if (!options['f']) {
			size_t len = sizeof(files) / sizeof(struct file);
			qsort(files, len, sizeof(struct file), struct_cmp_by_timeM);
		}
		if (!options['d']) {
			for (int i = 0; i < c2; i++) {
				if ((strcmp(files[i].name, ".") == 0) || (strcmp(files[i].name, "..") == 0)) {
				} else {
					if (cnt == 2) {
						printf("%s\n", files[i].name);
					}
				}
			}
		} else {
			if (cnt == 2) {
				printf("%s\n", ch);
			}
		}
	}

	if (options['i']) {
		if (!options['f']) {
			size_t structs_len = sizeof(files) / sizeof(struct file);
			qsort(files, structs_len, sizeof(struct file), struct_cmp_by_name);
		}
		if (cnt == 2) {
			if (!options['d']) {
				for (int i = 0; i < c2; i++) {
					if ((strcmp(files[i].name, ".") == 0) || (strcmp(files[i].name, "..") == 0)) {
					} else {
						printf("%ld ", files[i].inode);
						printf(" %s\n", files[i].name);
					}
				}
			} else {
				lstat(ch, &statbuf);
				printf("%ld", statbuf.st_ino);
				printf("%s\n", ch);
			}
		}
	}

	if (options['r']) {
		cmpName = 'r';
		if (!options['f']) {
			size_t structs_len = sizeof(files) / sizeof(struct file);
			qsort(files, structs_len, sizeof(struct file), struct_cmp_by_name);
		}
		if (cnt == 2) {
			if (!options['d']) {
				for (int i = 0; i < c2; i++) {
					if ((strcmp(files[i].name, ".") == 0) || (strcmp(files[i].name, "..") == 0)) {
					} else {
						printf("%s\n", files[i].name);
					}
				}
			} else
				printf("%s\n", ch);
		}
	}

	if ((options['a'] || options['A']) && cnt == 2) {
		if (options['A'])
			optA = 'A';
		if (!options['f']) {
			size_t structs_len = sizeof(files) / sizeof(struct file);
			qsort(files, structs_len, sizeof(struct file), struct_cmp_by_name);
		}
		if (!options['d']) {
			for (int i = 0; i < c2; i++) {

				if (optA == 'A' || options['k']) {
					if ((strcmp(files[i].name, ".") == 0) || (strcmp(files[i].name, "..") == 0)) {
					} else {
						printf("%s\n", files[i].name);
					}
				} else if (optA != 'A') {
					printf("%s\n", files[i].name);
				}
			}
		} else
			printf("%s\n", ch);
	}

	if (options['d']) {
		lstat(ch, &statbuf);
		if (cnt == 2)
			printf("%s\n", ch);
	}

	if (options['F'] && cnt == 2) {
		char f;
		if (!options['f']) {
			size_t structs_len = sizeof(files) / sizeof(struct file);
			qsort(files, structs_len, sizeof(struct file), struct_cmp_by_name);
		}

		if (!options['d']) {
			for (int i = 0; i < c2; i++) {
				if ((strcmp(files[i].name, ".") == 0) || (strcmp(files[i].name, "..") == 0)) {
				} else {
					if (checkTypeOfFile(files[i].mode) == 'd')
						f = '/';
					else if (checkTypeOfFile(files[i].mode) == 'e')
						f = '*';
					else if (checkTypeOfFile(files[i].mode) == 'l')
						f = '@';
					else if (checkTypeOfFile(files[i].mode) == 'w')
						f = '%';
					else if (checkTypeOfFile(files[i].mode) == 's')
						f = '=';
					else if (checkTypeOfFile(files[i].mode) == 'f')
						f = '|';
					if (cnt == 2)
						printf("%s%c\n", files[i].name, f);
					f = ' ';
				}
			}
		} else {
			printf("%s\n", ch);
		}
	}

	if (options['f']) {
		if (cnt == 2) {
			if (!options['d']) {
				for (int i = c2 - 1; i >= 0; i--) {
					printf("%s\n", files[i].name);
				}
			} else
				printf("%s\n", ch);
		}
	}

	if (options['h']) {
		size_t structs_len = sizeof(files) / sizeof(struct file);
		qsort(files, structs_len, sizeof(struct file), struct_cmp_by_name);
		if (!options['d']) {
			for (int i = 0; i < c2; i++) {
				if ((strcmp(files[i].name, ".") == 0) || (strcmp(files[i].name, "..") == 0)) {
				} else {
					if (cnt == 2)
						printf("%s \n", files[i].name);
				}
			}
		} else {
			if (cnt == 2)
				printf("%s \n", ch);
		}

	}

	if (options['R']) {
		if (cnt == 2)
			flagR = 1;
		if (!options['d']) {
			for (int i = 0; i < c2; i++) {
				if ((strcmp(files[i].name, ".") == 0) || (strcmp(files[i].name, "..") == 0)) {
				} else {
					if (checkTypeOfFile(files[i].mode) == 'd') {
						getDirInfo(files[i].name);
					}
				}
			}
		} else {
			printf("%s\n", ch);
		}
	}
}

