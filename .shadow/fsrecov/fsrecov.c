#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include "fat32.h"
#include "lgraph.h"

#define CLUS_EMPTY_SIZE 10


// int cluster_classify(u8 *data, size_t size)
// {
//     // dirct
//     // BMP head
//     // BMP body, one cluster not enough use more cluster
//     // cluster not use
//     u8 *clus_offset = data;
//     size_t i = 0;
//     for (; i < CLUS_EMPTY_SIZE; i++) {
//         uint32_t value = *((uint32_t *)(clus_offset));
//         if( value != 0 )
//             break;
//         clus_offset += 4;
//     }
//     if(i == CLUS_EMPTY_SIZE)
//         return 3;
    


//     if(data[0] == 0x42 && data[1] == 0x4d && 
//          data[6] == 0 && data[7] == 0 && data[8] == 0 && data[9] == 0) { // head byte BM
//         uint32_t bmp_size = *((uint32_t *)(data + 2));
//         // one cluster not enough
//         if( bmp_size > size )
//             return 4;
//         return 1;
//     }

//     int bmp_count=0;
//     for(int i = 0; i < size; i += 32) {
//         struct fat32dent *dir_entry = (struct fat32dent *)(data + i);
//         if (dir_entry->DIR_Name[0] == 0x00 ||
//             dir_entry->DIR_Name[0] == 0xe5 ||
//             dir_entry->DIR_Attr & ATTR_HIDDEN)
//             continue;
//         if ( (dir_entry->DIR_Name[0] && 0x40 ) && dir_entry->DIR_Attr == 0x0F) { // is long name file
//             int long_name_num = dir_entry->DIR_Name[0] & 0x0F;
//             // struct fat32dent *long_dir_entry = dir_entry;

//             for (int i = long_name_num-1; i >= 0; i--) {
//                 struct fat32ldent *long_dir_entry = dir_entry + i;
                
//             }

//         }
//     }


// }

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

    size_t data_size = (size_t)size - (FirstDataSector) * hdr->BPB_BytsPerSec;

    int clus_num = data_size / (hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec);

    printf("%d\n",(FirstDataSector + 1 * hdr->BPB_SecPerClus) * hdr->BPB_BytsPerSec);
    printf("SecPerClus : %d\n", hdr->BPB_SecPerClus);
    printf("BytePerSec : %d\n", hdr->BPB_BytsPerSec);
    printf("BytePerClus : %d\n", hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec);
    printf("clus_num: %d\n",clus_num);

    // printf("%d\n", 0x3fB7 * hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec );
    printf("%d\n",(FirstDataSector + 0x3FB6 * hdr->BPB_SecPerClus) * hdr->BPB_BytsPerSec);
    // for (size_t i = 0; i < clus_num; i++) {
    //     u8 *addr = (u8 *)hdr + (FirstDataSector + i * hdr->BPB_SecPerClus) * hdr->BPB_BytsPerSec;
    //     int type = cluster_classify(addr, hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec);
    // }
    


}
