#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "fat32.h"
#include "lgraph.h"

#define CLUS_EMPTY_SIZE 10

ClusterGraph *graph;

ClusterGraph *create_graph(int num)
{
    ClusterGraph *new_graph = (ClusterGraph *)malloc(sizeof(ClusterGraph));
    new_graph->cluster_num = num;
    ClusterNode *clusters = (ClusterNode *)malloc(sizeof(ClusterNode) * num);
    new_graph->clusters = clusters;
    memset(new_graph->clusters, 0, sizeof(ClusterNode) * num);
    for (int i = 0; i < num; i++) {
        new_graph->clusters[i].cluster_id = i + 2;
    }
    

    ClusterEdge **adj_list = (ClusterEdge **)malloc(sizeof(ClusterEdge *) * num);
    for (int i = 0; i < num; i++) {
        adj_list[i] = NULL;
    }
    new_graph->adj_list = adj_list;
    return new_graph;
}

void add_edge(u32 source, u32 target, double prob)
{
    ClusterEdge *newEdge = (ClusterEdge *)malloc(sizeof(ClusterEdge));
    newEdge->adj_id = target;
    newEdge->probability = prob;
    newEdge->next = graph->adj_list[source];  // head insert 
    graph->adj_list[source]->next = newEdge;
    return;
}

int cluster_classify(u8 *data, size_t size, u32 cluster_num)
{
    /*0. unknown    1. dict     2. BMP head one cluster enough     3. BMP head one cluster not enough
      4. BMP body   5. cluster not use*/

    // BMP body, one cluster not enough use more cluster
    
    // cluster not use
    // printf("cluster_num %d\n",cluster_num);
    u8 *clus_offset = data;
    size_t i = 0;
    for (; i < CLUS_EMPTY_SIZE; i++) {
        uint32_t value = *((uint32_t *)(clus_offset));
        if( value != 0 )
            break;
        clus_offset += 4;
    }
    if(i == CLUS_EMPTY_SIZE)
    {
        graph->clusters[cluster_num].type = 5;
        return 5;
    }
    

    // BMP head
    if(data[0] == 0x42 && data[1] == 0x4d && 
         data[6] == 0 && data[7] == 0 && data[8] == 0 && data[9] == 0) { // head byte BM
        uint32_t bmp_size = *((uint32_t *)(data + 2));
        // one cluster not enough
        if( bmp_size > size )
        {
            graph->clusters[cluster_num].type = 3;
            return 3;
        }
        
        graph->clusters[cluster_num].type = 2;
        return 2;
    }

    

    // dirct
    int bmp_count=0;
    for(int i = 0; i < size; i += 32) {

        struct fat32dent *dir_entry = (struct fat32dent *)(data + i);
        // printf("dir_entry:%x DIR_Attr %x\n",dir_entry->DIR_Name[0],dir_entry->DIR_Attr);
        // printf("ggg %x\n",dir_entry->DIR_Name[0] & 0xF0);
        if (dir_entry->DIR_Name[0] == 0x00 ||
            dir_entry->DIR_Name[0] == 0xe5 ||
            dir_entry->DIR_Attr == ATTR_HIDDEN)
            continue;
        if ( ((dir_entry->DIR_Name[0] & 0xF0 ) == 0x40) && (dir_entry->DIR_Attr == 0x0F) && dir_entry->DIR_FstClusLO == 0 && dir_entry->DIR_NTRes == 0) { // is long name file
            bmp_count ++;
            int long_name_num = dir_entry->DIR_Name[0] & 0x0F;
            i += long_name_num * 32;
            struct fat32dent *short_dir_entry = dir_entry + long_name_num;
            // cluster_id - 2
            int bmp_num = short_dir_entry->DIR_FstClusLO - 2;
            graph->clusters[bmp_num].bmp_info.file_size = short_dir_entry->DIR_FileSize;
            // struct fat32dent *long_dir_entry = dir_entry;
            int len = 0;
            bool flag = false;
            for (int i = long_name_num-1; i >= 0; i--) {
                struct fat32ldent *long_dir_entry = (struct fat32ldent *)dir_entry + i;
                for (int j = 0; j < 5; j++) {
                    if (long_dir_entry->LDIR_Name1[j] == 0x0000) goto parse_end;
                    graph->clusters[bmp_num].bmp_info.name[len++] = (u8)long_dir_entry->LDIR_Name1[j];
                }
                for (int j = 0; j < 6; j++) {
                    if (long_dir_entry->LDIR_Name2[j] == 0x0000) goto parse_end;
                    graph->clusters[bmp_num].bmp_info.name[len++] = (u8)long_dir_entry->LDIR_Name2[j];
                }
                for (int j = 0; j < 2; j++) {
                    if (long_dir_entry->LDIR_Name3[j] == 0x0000) goto parse_end;
                    graph->clusters[bmp_num].bmp_info.name[len++] = (u8)long_dir_entry->LDIR_Name3[j];
                }
            }
        parse_end:
            assert( len < 50 );
            graph->clusters[bmp_num].bmp_info.name[len] = '\0';
        }
        else if( dir_entry->DIR_Attr == 0x20 && dir_entry->DIR_NTRes == 0 &&
             ((dir_entry->DIR_Name[0] >= 0x30 && dir_entry->DIR_Name[0] <= 0x39) ||
             (dir_entry->DIR_Name[0] >= 0x41 && dir_entry->DIR_Name[0] <= 0x5A)) ) {  // is short name file
            bmp_count ++;
            int bmp_num = (dir_entry->DIR_FstClusHI << 16) | dir_entry->DIR_FstClusLO;
            int len = 0;
            for (int i = 0; i<sizeof(dir_entry->DIR_Name); i++) {
                if( dir_entry->DIR_Name[i] != ' ' ) {
                    if(i == 8)
                        graph->clusters[bmp_num].bmp_info.name[len++] = '.';
                    graph->clusters[bmp_num].bmp_info.name[len++] = dir_entry->DIR_Name[i];    
                }
            }
            graph->clusters[bmp_num].bmp_info.name[len] = '\0';
            graph->clusters[bmp_num].bmp_info.file_size = dir_entry->DIR_FileSize;
        }
        // printf("77777\n");
        if( i > 32*4 && bmp_count == 0) // not dirct
            break;
    }

    if(bmp_count>0) {
        graph->clusters[cluster_num].type = 1;
        return 1;
    }

    graph->clusters[cluster_num].type = 4;
    return 4;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "no fat file");
        exit(1);
    }


    // get img size
    int fd = open(argv[1],O_RDONLY);
    if(fd == -1) {
        perror("file open fail");
        exit(1);
    }

    off_t size = lseek(fd, 0, SEEK_END);

    struct fat32hdr *hdr = mmap(NULL,size,PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if(hdr == MAP_FAILED) {
        perror("mmap open fail");
        exit(1);
    }
    
    close(fd);
    
    // first cluster
    // hdr->BPB_RootClus;

    // hdr->BPB_RsvdSecCnt + (hdr->BPB_NumFATs * hdr->BPB_FATSz32) + RootDirSectors
    // RootDirSectors is always 0 in FAT32



    // FAT after Reserved Sectors
    // u32 addr_FAT = hdr->BPB_RsvdSecCnt
    // FAT32 have BPB_NumFATs FAT, default 2
    // second FAT addr,  u32 FAT2_start = hdr->BPB_RsvdSecCnt + hdr->BPB_FATSz32
    
    // FAT entry 32bit, #entry->#cluster, entry's value -> #next cluster of this cluster

    // DATA Sec after FAT
    u32 FirstDataSector = hdr->BPB_RsvdSecCnt + hdr->BPB_NumFATs * hdr->BPB_FATSz32;
    
    // cluster number N, #cluster 0,1 is reserved, used from 2
    // u32 FirstSectorofCluster = (N - 2) * hdr->BPB_SecPerClus + FirstDataSector;
    // physical addr
    // (u8 *)hdr + FirstSectorofCluster*(hdr->BPB_BytsPerSec);

    int data_size = (size_t)size - (FirstDataSector) * hdr->BPB_BytsPerSec;

    int clus_num = data_size / (hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec);

    printf("%d\n",(FirstDataSector + 1 * hdr->BPB_SecPerClus) * hdr->BPB_BytsPerSec);
    printf("SecPerClus : %d\n", hdr->BPB_SecPerClus);
    printf("BytePerSec : %d\n", hdr->BPB_BytsPerSec);
    printf("BytePerClus : %d\n", hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec);
    printf("clus_num: %d\n",clus_num);
    printf("data_size: %d\n", data_size);
    printf("FirstDataSector: %d\n", FirstDataSector);
    // printf("%d\n", 0x3fB7 * hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec );
    printf("%d\n",(FirstDataSector + 0x61 * hdr->BPB_SecPerClus) * hdr->BPB_BytsPerSec);


    graph = create_graph(clus_num);
    graph->cluster_num = clus_num;

    for (size_t i = 0; i < clus_num; i++) {
        u8 *addr = (u8 *)hdr + (FirstDataSector + i * hdr->BPB_SecPerClus) * hdr->BPB_BytsPerSec;
        int type = cluster_classify(addr, hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec, i+2);
    }
    
    for (int i = 0; i < clus_num; i++) {
        if(graph->clusters[i].type == 2) {
            char output_filename[256];
            snprintf(output_filename,sizeof(output_filename), "./recovpic/%s",graph->clusters[i].bmp_info.name);
            FILE *fp = fopen(output_filename, "wb");
            if (!fp) {
                perror("Failed to open file");
                exit(1);
            }
            u8 *addr = (u8 *)hdr + (FirstDataSector + i * hdr->BPB_SecPerClus) * hdr->BPB_BytsPerSec;
            fwrite(addr, 1, graph->clusters[i].bmp_info.file_size, fp);
            fclose(fp);
        }
        if(graph->clusters[i].type == 3) {
            printf("clusterid: %d size: %d  name: %s\n",i, graph->clusters[i].bmp_info.file_size, graph->clusters[i].bmp_info.name);
        }
    }
    
    return 0;

}
