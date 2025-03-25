#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#define NAMELEN 32

typedef struct TNode *psTree;
struct TNode
{
	char name[NAMELEN];
	int pid;
	int level;
	psTree parent;
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
	dir = opendir('/proc');
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
		printf("%s\n",dirinfo->d_name);
	}

	closedir(dir);
	return 0;
}
