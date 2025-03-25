#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define NAMELEN 32

typedef struct TNode *psTNode;
struct TNode
{
	char name[NAMELEN];
	int pid;
	int level;
	psTNode parent;
	bool is_printf;
};

int main(int argc, char *argv[])
{
	// for (int i = 0; i < argc; i++) {
	//   assert(argv[i]);
	//   printf("argv[%d] = %s\n", i, argv[i]);
	// }
	// assert(!argv[argc]);
	DIR *dir;
	const char basedir[] = "/proc";

	dir = opendir(basedir);
	if (dir == NULL)
	{
		assert(0);
		return -1;
	}

	struct dirent *dirinfo;

	while ((dirinfo = readdir(dir)) != NULL)
	{
		if (dirinfo->d_name[0] < '0' || dirinfo->d_name[0] > '9')
			continue;
		long int offset = telldir(dir);

		psTNode node = (psTNode)malloc(sizeof(struct TNode));
		char pidStatFile[30];
		FILE *fp;

		int proc_pid;
		char proc_name[50];
		int proc_parent;

		strcpy(pidStatFile, basedir);
		strcat(pidStatFile, "/");
		strcat(pidStatFile, dirinfo->d_name);
		strcat(pidStatFile, "/stat");

		if ((fp = fopen(pidStatFile, "r")) == NULL)
		{
			printf("stat file open worng\n");
			assert(0);
		}

		int get_num = fscanf(fp, "%d (%49[^)]) %d", &proc_pid, proc_name, &proc_parent);

		if(get_num == 3)
		{
			printf("%d\t%s\t%d\n",proc_pid,proc_name, proc_parent);
		}

		// seekdir(dir, offset);
		// printf("%s\t%ld\n", dirinfo->d_name, telldir(dir));
	}

	closedir(dir);
	return 0;
}
