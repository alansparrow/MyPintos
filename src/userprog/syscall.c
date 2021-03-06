#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);
static void check_address(void *addrr);
static void get_argument(void *esp, int *arg, int count);

static void halt(void);
static void exit(int status);
static bool create(const char *file, unsigned initial_size);
static bool remove(const char *file);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  
  int system_number = 0;
  uint32_t *esp = f->esp; // because in user's stack, it uses 32 bit word
  uint32_t *arg = NULL;
  
  printf ("system call!\n");
  check_address(esp);

  system_number = *((int *) esp);
  
  

  switch (system_number) {
  case SYS_HALT: //0
    printf("%\n  SYS_HALT\n");

    halt();
    break;
  case SYS_EXIT: //1
    printf("%\n  SYS_EXIT\n");
    
    // argument is in the 32-bit word, here we have 1 arg (status)
    arg = (int *) malloc(sizeof(uint32_t));
    get_argument(esp, arg, 1);
    
    exit((int) arg[0]);
    
    free(arg);
    break;
  case SYS_CREATE: //4
    printf("%\n  SYS_CREATE\n");
    
    // argument is in the 32-bit word, here we have 2 args(filename, file_size)
    arg = (int *) malloc(sizeof(uint32_t)*2);
    get_argument(esp, arg, 2);

    // check if the pointer's value is in user's memory area
    check_address((char *) arg[0]);

    // call function and set return value at eax
    f->eax = create((char *) arg[0], (int) arg[1]);

    free(arg);
    break;
  case SYS_REMOVE: //5
    printf("%\n  SYS_REMOVE\n");
    
    // argument is in the 32-bit word, here we have 1 arg(filename)
    arg = (int *) malloc(sizeof(uint32_t));
    get_argument(esp, arg, 1);

    // check if the pointer's value is in user's memory area
    check_address((char *) arg[0]);
    
    // call function and set return value at eax
    f->eax = remove((char *) arg[0]);

    free(arg);
    break;
  }
 
  thread_exit ();
}

static void check_address(void *addrr)
{

  //if (addrr >= 0xc0000000 && addrr <= 0x08048000) {
  if (addrr != NULL && is_user_vaddr(addrr)) {
    printf("\n  Checking esp's value..\n");
    printf("  OK! %p is in user's memory space\n", addrr);
  } else {
    printf("\n  Error: Don't hang out outside user's memory space!\n");
    printf("\n  Your are at: %p\n", addrr);
    exit(-1);
  }
}

static void get_argument(void *esp, int *arg, int count)
{
  int i = 0;
  // cast esp into 32 bit 
  uint32_t *esp_copy = esp; 
  // skip the first one which is system call number, +1 mean go up 32 bit
  esp_copy += 1; 

  for (i = 0; i < count; i++) {
    arg[i] = *(esp_copy+i);
  }
}


void halt(void)
{
  shutdown_power_off();
}

void exit(int status)
{
  printf("\n  %s: exit(%d)\n", thread_name(), status); 
  thread_exit();
}

bool create(const char *file, unsigned initial_size)
{
  return filesys_create(file, initial_size);
}

bool remove(const char *file)
{
  return filesys_remove(file);
}
