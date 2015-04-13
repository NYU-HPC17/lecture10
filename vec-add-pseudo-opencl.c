#include <stdio.h>
#include <stdlib.h>
#include "timing.h"

void vecadd(size_t N, const double * restrict a, const double * restrict b,
    double * restrict c)
{
#if 0
  a = __builtin_assume_aligned(a, 32);
  b = __builtin_assume_aligned(b, 32);
  c = __builtin_assume_aligned(c, 32);
#endif


  /*
   * size of a group
   */
  size_t  local_size[] = { 128 };

  /*
   * total number of iterations to be executed
   */
  size_t global_size[] = { ((N + local_size[0] - 1)/local_size[0])
                           *local_size[0] };

  /*
   * number of groups
   */
  size_t group_size[] = { global_size[0]/local_size[0] };

  for(size_t group_id = 0, global_id = 0; group_id < group_size[0]; ++group_id)
  {
    for(size_t local_id = 0; local_id < local_size[0]; ++local_id, ++global_id)
    {
#if 1
      int gid = global_id;
#else
      int gid = local_size[0]*group_id + local_id;
#endif
      if(gid < N)
        c[gid] = a[gid] + b[gid];
    }
  }
}

int main (int argc, char *argv[])
{
  double *a, *b, *c;

  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s size_of_vector num_adds\n", argv[0]);
    abort();
  }

  const size_t N = (size_t) atol(argv[1]);
  const int num_adds = atoi(argv[2]);

  posix_memalign((void**)&a, 32, N*sizeof(double));
  if (!a) { fprintf(stderr, "alloc a"); abort(); }
  posix_memalign((void**)&b, 32, N*sizeof(double));
  if (!b) { fprintf(stderr, "alloc b"); abort(); }
  posix_memalign((void**)&c, 32, N*sizeof(double));
  if (!c) { fprintf(stderr, "alloc c"); abort(); }

  for(size_t n = 0; n < N; ++n)
  {
    a[n] = n;
    b[n] = 2*n;
  }

  timestamp_type tic, toc;
  get_timestamp(&tic);
  for(int add = 0; add < num_adds; ++add)
  {
    vecadd(N, a, b, c);
  }
  get_timestamp(&toc);

  double elapsed = timestamp_diff_in_seconds(tic,toc)/num_adds;
  printf("%f s\n", elapsed);
  printf("%f GB/s\n", 3*N*sizeof(double)/1e9/elapsed);

  for(size_t i = 0; i < N; ++i)
    if(c[i] != 3*i)
    {
      printf("BAD %zd\n", i);
      abort();
    }
  printf("GOOD\n");


  free(a);
  free(b);
  free(c);
  return 0;
}
