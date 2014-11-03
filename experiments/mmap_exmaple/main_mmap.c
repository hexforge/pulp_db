//*M*E*T*A*S*T*A*R*T*/4/16/224.0.65.10/50005/00:20:09.177/1361406009.177703//*M*E*T*A*E*N*D*177703��S!

/*But after thinking about it we need size ip port and time.   
The best I can think of is know the index of the first char of size, 
though it varies in string length.  
We need ip and port which are just beside it but may also vary.  
The raw time stamp is a fixed size away from there.  
And the point of the data is a fixed size away from there.   

4 always there
length variable length
ip=optional variable length
port=varibale length
ignore
get raw time fixed
meta end fixed

"D*" DEBUG CHECK

 _FILE_OFFSET_BITS=64
*/

#include <stdio.h>    
#include <sys/mman.h>   // memory map (mmap, madvise and flags)

// Needed for open
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <errno.h>

// No idea
#include <stdlib.h>      

#define FILEPATH "pb.pb"
#define BLOCKSIZE (1024*4L)

int main(int argc, char *argv[])
{
    int fd;
    unsigned char madvise_ret;
    unsigned long i, j;   //, number_of_blocks, block_num;
    unsigned char *map;  /* mmapped array of int's */
    
    // OPEN THE FILE
    fd = open(FILEPATH, O_RDONLY);
    if (fd == -1) 
    {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    // GET THE FILE SIZE
    off_t filesize = lseek(fd, 0L, SEEK_END);   // Find end of file
    off_t curpos = lseek(fd, 0L, SEEK_SET);
    
    // MMAP the file
    map = mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) 
    {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
    madvise_ret = madvise(map, filesize, MADV_SEQUENTIAL);
    if (madvise_ret != 0) 
    {
        close(fd);
        perror("Error madvise the file");
        exit(EXIT_FAILURE);
    }

    /* Read the file char-by-char from the mmap */

    for (i = 1; i<=filesize; ++i)
    {
        printf("%ld: %c\n", i, map[i]);
        // Get rid of blocks we have moved past need

        
        j = i & BLOCKSIZE-1;     
        if (j==0 && i>=BLOCKSIZE)
        {
            printf("BLAH i%d j%d bottom%d top%d\n", i, j, i-BLOCKSIZE, i-1);
            
            madvise(map+i-BLOCKSIZE, BLOCKSIZE-1, MADV_DONTNEED);
            if (madvise_ret != 0) 
            {
                close(fd);
                perror("Error madvise the file");
                exit(EXIT_FAILURE);
            }
            
        }
        
    }

    if (munmap(map, filesize) == -1) 
    {
        perror("Error un-mmapping the file");
    }
    close(fd);
    return 0;
}



