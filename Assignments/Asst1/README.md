# CS214-Asst1

## Description
Our implementation of malloc() and free()

### Design of malloc():
In any memory system, the computer needs to keep track of all the memory it 
has allocated. We keep track of the memory blocks allocated to the user by using a "linked list"
type of data structure. Each "node" of the linked list contains two important values: the size of
the memory block (int) and whether the memory block is free (bool/char). A programmer's call to
malloc(int) will see if there is a free memory block large enough to accommodate that request. If
a big enough free memory block exists, the function will break down the free memory block into
two separate memory blocks: one allocated for the user's request and the other free memory block
controlling the rest of the free space. If enough free space does not exist to create two
separate memory blocks, the user will get to use the existing free memory block with potentially
more space than requested by the user. Lastly, if enough space does not exist to accommodate the
user's request, then the malloc(int) function will return a null pointer.

### Design of free():
Unlike the free() in standard C library, our free(*ptr) function will check
the validity of the *ptr passed in by the user and will print different errors accordingly. The
free(*ptr) function will set the isFree flag to true in the associated metadata block (node).
Then, it will combine any free memory blocks to the left/right into one big free memory block.

### Errors Detected:
Our implementation of free() will throw an error if the user tries to free a pointer not allocated
by our malloc() function. The free() function does this by comparing all the allocated pointers to
the one passed in by the user.

## Running
```
# Run testcases
C> make
gcc -c mymalloc.c
gcc -o memgrind memgrind.c mymalloc.o
C> ./memgrind
Workload A Mean Run Time (microsecond): 2
Workload B Mean Run Time (microsecond): 38
Workload C Mean Run Time (microsecond): 15
Workload D Mean Run Time (microsecond): 459
Workload E Mean Run Time (microsecond): 4
```

Find a full list of workloads in 'memgrind.c'
