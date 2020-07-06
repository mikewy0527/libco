#define LIBCO_C
#include "libco.h"
#include "settings.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#ifdef __cplusplus
extern "C" {
#endif

static thread_local unsigned long co_active_buffer[64];
static thread_local cothread_t co_active_handle = 0;
void co_swap(cothread_t, cothread_t);

__asm__(
  ".text\n"
  ".align 4\n"
  ".type co_swap @function\n"
  "co_swap:\n"

  // Save/restore SP and RA.
  "sd sp, 0*8(a1)\n"
  "ld sp, 0*8(a0)\n"
  "sd ra, 1*8(a1)\n"
  "ld ra, 1*8(a0)\n"

  // Save/restore callee-save integer registers.
  "sd s0, 2*8(a1)\n"
  "ld s0, 2*8(a0)\n"
  "sd s1, 3*8(a1)\n"
  "ld s1, 3*8(a0)\n"
  "sd s2, 4*8(a1)\n"
  "ld s2, 4*8(a0)\n"
  "sd s3, 5*8(a1)\n"
  "ld s3, 5*8(a0)\n"
  "sd s4, 6*8(a1)\n"
  "ld s4, 6*8(a0)\n"
  "sd s5, 7*8(a1)\n"
  "ld s5, 7*8(a0)\n"
  "sd s6, 8*8(a1)\n"
  "ld s6, 8*8(a0)\n"
  "sd s7, 9*8(a1)\n"
  "ld s7, 9*8(a0)\n"
  "sd s8, 10*8(a1)\n"
  "ld s8, 10*8(a0)\n"
  "sd s9, 11*8(a1)\n"
  "ld s9, 11*8(a0)\n"
  "sd s10, 12*8(a1)\n"
  "ld s10, 12*8(a0)\n"
  "sd s11, 13*8(a1)\n"
  "ld s11, 13*8(a0)\n"

  // Save/restore callee-save floating point registers.
  "fsd fs0, 14*8(a1)\n"
  "fld fs0, 14*8(a0)\n"
  "fsd fs1, 15*8(a1)\n"
  "fld fs1, 15*8(a0)\n"
  "fsd fs2, 16*8(a1)\n"
  "fld fs2, 16*8(a0)\n"
  "fsd fs3, 17*8(a1)\n"
  "fld fs3, 17*8(a0)\n"
  "fsd fs4, 18*8(a1)\n"
  "fld fs4, 18*8(a0)\n"
  "fsd fs5, 19*8(a1)\n"
  "fld fs5, 19*8(a0)\n"
  "fsd fs6, 20*8(a1)\n"
  "fld fs6, 20*8(a0)\n"
  "fsd fs7, 21*8(a1)\n"
  "fld fs7, 21*8(a0)\n"
  "fsd fs8, 22*8(a1)\n"
  "fld fs8, 22*8(a0)\n"
  "fsd fs9, 23*8(a1)\n"
  "fld fs9, 23*8(a0)\n"
  "fsd fs10, 24*8(a1)\n"
  "fld fs10, 24*8(a0)\n"
  "fsd fs11, 25*8(a1)\n"
  "fld fs11, 25*8(a0)\n"

  "ret\n"

  ".size co_swap, .-co_swap\n"
);

cothread_t co_active() {
  if(!co_active_handle) co_active_handle = &co_active_buffer;
  return co_active_handle;
}

cothread_t co_derive(void* memory, unsigned int size, void (*entrypoint)(void)) {
  unsigned long* handle;
  if(!co_active_handle) co_active_handle = &co_active_buffer;

  if((handle = (unsigned long*)memory)) {
    unsigned int offset = (size & ~15);
    unsigned long* p = (unsigned long*)((unsigned char*)handle + offset);
    handle[0]  = (unsigned long)p;           /* x2 (stack pointer) */
    handle[1]  = (unsigned long)entrypoint;  /* x1 (return address) */
    handle[10] = (unsigned long)p;           /* x8 (frame pointer) */
  }

  return handle;
}

cothread_t co_create(unsigned int size, void (*entrypoint)(void)) {
  void* memory = malloc(size);
  if(!memory) return (cothread_t)0;
  return co_derive(memory, size, entrypoint);
}

void co_delete(cothread_t handle) {
  free(handle);
}

void co_switch(cothread_t handle) {
  cothread_t co_previous_handle = co_active_handle;
  co_swap(co_active_handle = handle, co_previous_handle);
}

int co_serializable() {
  return 1;
}

#ifdef __cplusplus
}
#endif
