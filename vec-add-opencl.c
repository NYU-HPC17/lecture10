/*
 * Modification of the cl-demo.c file from
 * Andreas Kloeckners code available at
 *   https://github.com/hpc12/tools
 * Edited by Lucas Wilcox.
 *
 * See README.md for more info.
 */

#include <stdio.h>
#include <stdlib.h>
#include "timing.h"
#include "cl-helper.h"

int main (int argc, char *argv[])
{
  double *a, *b, *c;

  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s size_of_vector num_adds\n", argv[0]);
    abort();
  }

  const cl_long N = (cl_long) atol(argv[1]);
  const int num_adds = atoi(argv[2]);


  cl_context ctx;
  cl_command_queue queue;
  create_context_on(CHOOSE_INTERACTIVELY, CHOOSE_INTERACTIVELY, 0, &ctx, &queue, 0);

  print_device_info_from_queue(queue);

  // --------------------------------------------------------------------------
  // load kernels
  // --------------------------------------------------------------------------
  char *knl_text = read_file("vec-add-kernel.cl");
  cl_kernel knl = kernel_from_string(ctx, knl_text, "sum", NULL);
  free(knl_text);

  // --------------------------------------------------------------------------
  // allocate and initialize CPU memory
  // --------------------------------------------------------------------------
  posix_memalign((void**)&a, 32, N*sizeof(double));
  if (!a) { fprintf(stderr, "alloc a"); abort(); }
  posix_memalign((void**)&b, 32, N*sizeof(double));
  if (!b) { fprintf(stderr, "alloc b"); abort(); }
  posix_memalign((void**)&c, 32, N*sizeof(double));
  if (!c) { fprintf(stderr, "alloc c"); abort(); }

  for(cl_long n = 0; n < N; ++n)
  {
    a[n] = n;
    b[n] = 2*n;
  }

  // --------------------------------------------------------------------------
  // allocate device memory
  // --------------------------------------------------------------------------
  cl_int status;
  cl_mem buf_a = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
      sizeof(double) * N, 0, &status);
  CHECK_CL_ERROR(status, "clCreateBuffer");

  cl_mem buf_b = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
      sizeof(double) * N, 0, &status);
  CHECK_CL_ERROR(status, "clCreateBuffer");

  cl_mem buf_c = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
      sizeof(double) * N, 0, &status);
  CHECK_CL_ERROR(status, "clCreateBuffer");

  // --------------------------------------------------------------------------
  // transfer to device
  // --------------------------------------------------------------------------
  CALL_CL_SAFE(clEnqueueWriteBuffer(
        queue, buf_a, /*blocking*/ CL_TRUE, /*offset*/ 0,
        N * sizeof(double), a,
        0, NULL, NULL));

  CALL_CL_SAFE(clEnqueueWriteBuffer(
        queue, buf_b, /*blocking*/ CL_TRUE, /*offset*/ 0,
        N * sizeof(double), b,
        0, NULL, NULL));

  // --------------------------------------------------------------------------
  // run code on device
  // --------------------------------------------------------------------------

  CALL_CL_SAFE(clFinish(queue));

  timestamp_type tic, toc;
  get_timestamp(&tic);
  for(int add = 0; add < num_adds; ++add)
  {
    SET_4_KERNEL_ARGS(knl, N, buf_a, buf_b, buf_c);
    size_t  local_size[] = { 128 };
    size_t global_size[] = { ((N + local_size[0] - 1)/local_size[0])*
                             local_size[0] };
    CALL_CL_SAFE(clEnqueueNDRangeKernel(queue, knl, 1, NULL,
          global_size, local_size, 0, NULL, NULL));
  }
  CALL_CL_SAFE(clFinish(queue));
  get_timestamp(&toc);

  double elapsed = timestamp_diff_in_seconds(tic,toc)/num_adds;
  printf("%f s\n", elapsed);
  printf("%f GB/s\n", 3*N*sizeof(double)/1e9/elapsed);

  // --------------------------------------------------------------------------
  // transfer back & check
  // --------------------------------------------------------------------------
  CALL_CL_SAFE(clEnqueueReadBuffer(
        queue, buf_c, /*blocking*/ CL_TRUE, /*offset*/ 0,
        N * sizeof(double), c,
        0, NULL, NULL));



  for(cl_long i = 0; i < N; ++i)
    if(c[i] != 3*i)
    {
      printf("BAD %ld\n", (long)i);
      abort();
    }
  printf("GOOD\n");

  // --------------------------------------------------------------------------
  // clean up
  // --------------------------------------------------------------------------
  CALL_CL_SAFE(clReleaseMemObject(buf_a));
  CALL_CL_SAFE(clReleaseMemObject(buf_b));
  CALL_CL_SAFE(clReleaseMemObject(buf_c));
  CALL_CL_SAFE(clReleaseKernel(knl));
  CALL_CL_SAFE(clReleaseCommandQueue(queue));
  CALL_CL_SAFE(clReleaseContext(ctx));

  free(a);
  free(b);
  free(c);

  return 0;
}
