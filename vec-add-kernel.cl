/*
 * Modification of the vec-add-soln.cl file from
 * Andreas Kloeckners code available at
 *   https://github.com/hpc12/tools
 * Modified by Lucas Wilcox
 * See README.md for more info.
 */

#pragma OPENCL EXTENSION cl_khr_fp64: enable

kernel void sum(
    long N,
    global const double *a,
    global const double *b,
    global double *c)
{
  // long gid = get_local_size(dim)*get_group_id(dim) + get_local_id(dim)
  long gid = get_global_id(0);
  if (gid < N)
    c[gid] = a[gid] + b[gid];
}
