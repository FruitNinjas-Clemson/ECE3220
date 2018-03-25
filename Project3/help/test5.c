#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#define NUMBUFS 50000
#define RANDOMS 300
#define MAX 9000

//not as thorough as test #5 BE WARNED

int buf_sizes[NUMBUFS] = { 0 };
int orig_sizes[NUMBUFS] = { 0 };
uint8_t* buf_ptrs[NUMBUFS] = { NULL };

int main() {

  clock_t begin = clock();

  srand(time(NULL));
  void* first_break = sbrk(0);

  free(NULL); //just for kicks

  // randomly allocate
  int i, j;
  for (i = 0; i < NUMBUFS; ++i) {
    orig_sizes[i] = buf_sizes[i] = rand() % MAX + 1;
    //allocate the next block
    buf_ptrs[i] = malloc(buf_sizes[i]);
    assert(buf_ptrs[i] != NULL); //should never return NULL
    //write some data into the buffer
    memset(buf_ptrs[i], i % 256, buf_sizes[i]);
  }

  for (i = 0; i < RANDOMS; i++) {
    int index = rand() % NUMBUFS;
    int option = rand() % 2;
    if (option == 0) {
      // realloc a random factor memory
      buf_sizes[index] = rand() % MAX + 1;
      if (orig_sizes[index] > buf_sizes[index]) orig_sizes[index] = buf_sizes[index];
      uint8_t* new_ptr = realloc(buf_ptrs[index], buf_sizes[index]);
      if (new_ptr) buf_ptrs[index] = new_ptr;
    } else if (option == 1) {
      // free and calloc a random amount of memory
      orig_sizes[index] = buf_sizes[index] = rand() % MAX + 1;
      free(buf_ptrs[index]);
      buf_ptrs[index] = calloc(sizeof(uint8_t), buf_sizes[index]);
      memset(buf_ptrs[index], index % 256, buf_sizes[index]);
    }
  }

  // free everything
  for (i = 0; i < NUMBUFS; ++i) {
    //check whether or not the memory is still intact
    for (j = 0; j < orig_sizes[i]; ++j) {
      assert(buf_ptrs[i][j] == i % 256);
    }
    free(buf_ptrs[i]);
  }

  void* last_break = sbrk(0);

  //verify that the program break never moved up.
  assert(first_break == last_break);

  clock_t end = clock();
  fprintf(stderr, "\n\nTime to run = %f\n", (double) (end - begin) / CLOCKS_PER_SEC);


  return 0;
}
