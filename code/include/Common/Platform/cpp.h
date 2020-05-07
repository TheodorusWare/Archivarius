/*
Copyright (C) 2018-2020 Theodorus Software

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef _platform_cpp_h_
#define _platform_cpp_h_

// no operation
#define noop ((void) 0)

// condition
#define when if
#define elif else if

#define forn(n)    for(uint i = 0; i < n; i++)
#define form(m, n) for(uint m = 0; m < n; m++)
#define foru(m, n) for(     m = 0; m < n; m++)

// safe delete pointers
#ifdef TAA_ALLOCATOR
#define safe_delete(p)       (p ? (allocatorSrc(__FILE__,__LINE__,1,(uint)p),   delete    p, p = 0) : 0)
#define safe_delete_array(p) (p ? (allocatorSrc(__FILE__,__LINE__,1,(uint)p),   delete [] p, p = 0) : 0)
#define safe_delete_ao(p)    (p ? (allocatorSrc(__FILE__,__LINE__,1,(uint)p-4), delete [] p, p = 0) : 0)
#else
#define safe_delete(p)       (p ? (delete    p, p = 0) : 0)
#define safe_delete_array(p) (p ? (delete [] p, p = 0) : 0)
#define safe_delete_ao(p)    (p ? (delete [] p, p = 0) : 0)
#endif

#define safe_delete_fix_array_ptr(p, n)  (p ? (forn(n) safe_delete(p[i])) : 0)
#define safe_delete_dyn_array_ptr(p, n)  (p ? ((forn(n) safe_delete(p[i])), safe_delete_array(p)) : 0)
#define safe_delete_dyn_array_ptrs(p, n) (p ? ((forn(n) safe_delete_array(p[i])), safe_delete_array(p)) : 0)
#define safe_delete_vector(p) ((forn(p.size()) safe_delete(p[i])), p.clear())

#define _malloc(n) new char[n]

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define ALIGN_BYTE_PTR(p, n) ((char*)((uint_t)(p) + (n-1) & ~(n-1)))
#define ALIGN_TYPE_PTR(t, p, n) ((t*)((uint_t)(p) + (n-1) & ~(n-1)))
#define ALIGN_PTR(p, n) ((n) - ((uint_t)(p) & (n-1)))

#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#endif