#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define NAMELEN 32
#define MAXLEVEL 10

typedef struct TNode *psTNode;
struct TNode
{
	char name[NAMELEN];
	int pid;
	int level;
	int parent_pid;
	bool is_printf;
	psTNode level_node;
};

psTNode node_insert(psTNode node, psTNode levelTree)
{
	psTNode endNode = levelTree;
	while (endNode->level_node != NULL)
		endNode = endNode->level_node;

	endNode->level_node = node;
	node->level_node = NULL;
	return levelTree;
}

int find_level(int parent_pid, psTNode levelTree)
{
	psTNode endNode;
	if(parent_pid == 0)
		return 0;
	for (int i = 0; i < MAXLEVEL; i++)
	{
		endNode = levelTree[i].level_node;
		while (endNode != NULL )
		{
			if(endNode->pid == parent_pid)
				return (endNode->level + 1);
			endNode = endNode->level_node;
		}
	}
	assert(0);
	return 0;
}

int print_sonTree(psTNode parentNode, psTNode levelTree, int foreSpace)
{
	int ret = 1;
	int is_end;
	for (int i = 0; i < foreSpace; i++)
		printf(" ");

	printf("%s(%d)    ",parentNode->name,parentNode->pid);
	// printf("%s    ",parentNode->name);
	int pidLen=0;
	for ( int i = 1; i <= parentNode->pid; i *= 10)
	{
		pidLen++;
	}
	pidLen = pidLen + 2;

	if(foreSpace < 0)
		foreSpace += 200;
	foreSpace += strlen(parentNode->name) + 4 + pidLen;
	bool first = true;
	int pPid = parentNode->pid;
	int level = parentNode->level + 1;
	psTNode sonNode = levelTree[level].level_node;
	while (sonNode != NULL)
	{
		if(sonNode->parent_pid == pPid)
		{
			ret = 0;
			if (first)
			{
				is_end = print_sonTree(sonNode,levelTree,foreSpace-200);
				first = false;
			}
			else
			{
				is_end = print_sonTree(sonNode,levelTree,foreSpace);

			}
			if( is_end )
				printf("\n");
		}
		sonNode = sonNode->level_node;
	}
	return ret;
}

void free_levelTree(psTNode levelTree)
{
	psTNode node;
	psTNode next_node;
	for (int i = 0; i < MAXLEVEL; i++)
	{
		node = levelTree[i].level_node;
		if (node == NULL)
			break;
		
		next_node = node->level_node;
		free(node);

		while (next_node != NULL)
		{
			node = next_node;
			next_node = node->level_node;
			free(node);
		}
	}
	return;
}

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

	struct TNode levelTree[MAXLEVEL];
	for (int i = 0; i < MAXLEVEL; i++)
	{
		levelTree[i].is_printf = false;
		levelTree[i].level = i;
		strcpy(levelTree[i].name,"\0");
		levelTree[i].parent_pid = -1;
		levelTree[i].pid = -1;
		levelTree[i].level_node = NULL;
	}

	struct dirent *dirinfo;

	while ((dirinfo = readdir(dir)) != NULL)
	{
		if (dirinfo->d_name[0] < '0' || dirinfo->d_name[0] > '9')
			continue;

		char pidStatFile[30];
		FILE *fp;

		int proc_pid;
		char proc_name[50];
		char tmp_char;
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

		int get_num = fscanf(fp, "%d (%49[^)]) %c %d", &proc_pid, proc_name, &tmp_char, &proc_parent);
		
		fclose(fp);

		if(proc_pid == 2 || proc_parent == 2)
			continue;

		psTNode node = (psTNode)malloc(sizeof(struct TNode));
		if(get_num == 4)
		{
			node->is_printf = false;
			strcpy(node->name,proc_name);
			node->pid = proc_pid;
			node->parent_pid = proc_parent;
			node->level = find_level(proc_parent,levelTree);
			node_insert(node, &levelTree[node->level]);
		}
	}

	print_sonTree(levelTree[0].level_node, levelTree, 0);

	free_levelTree(levelTree);
	closedir(dir);
	return 0;
}
