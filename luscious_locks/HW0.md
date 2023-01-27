# Welcome to Homework 0!

For these questions you'll need the mini course and  "Linux-In-TheBrowser" virtual machine (yes it really does run in a web page using javascript) at -

http://cs-education.github.io/sys/

Let's take a look at some C code (with apologies to a well known song)-
```C
// An array to hold the following bytes. "q" will hold the address of where those bytes are.
// The [] mean set aside some space and copy these bytes into teh array array
char q[] = "Do you wanna build a C99 program?";

// This will be fun if our code has the word 'or' in later...
#define or "go debugging with gdb?"

// sizeof is not the same as strlen. You need to know how to use these correctly, including why you probably want strlen+1

static unsigned int i = sizeof(or) != strlen(or);

// Reading backwards, ptr is a pointer to a character. (It holds the address of the first byte of that string constant)
char* ptr = "lathe"; 

// Print something out
size_t come = fprintf(stdout,"%s door", ptr+2);

// Challenge: Why is the value of away equal to 1?
int away = ! (int) * "";


// Some system programming - ask for some virtual memory

int* shared = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
munmap(shared,sizeof(int*));

// Now clone our process and run other programs
if(!fork()) { execlp("man","man","-3","ftell", (char*)0); perror("failed"); }
if(!fork()) { execlp("make","make", "snowman", (char*)0); execlp("make","make", (char*)0)); }

// Let's get out of it?
exit(0);
```

## So you want to master System Programming? And get a better grade than B?
```C
int main(int argc, char** argv) {
	puts("Great! We have plenty of useful resources for you, but it's up to you to");
	puts(" be an active learner and learn how to solve problems and debug code.");
	puts("Bring your near-completed answers the problems below");
	puts(" to the first lab to show that you've been working on this.");
	printf("A few \"don't knows\" or \"unsure\" is fine for lab 1.\n"); 
	puts("Warning: you and your peers will work hard in this class.");
	puts("This is not CS225; you will be pushed much harder to");
	puts(" work things out on your own.");
	fprintf(stdout,"This homework is a stepping stone to all future assignments.\n");
	char p[] = "So, you will want to clear up any confusions or misconceptions.\n";
	write(1, p, strlen(p) );
	char buffer[1024];
	sprintf(buffer,"For grading purposes, this homework 0 will be graded as part of your lab %d work.\n", 1);
	write(1, buffer, strlen(buffer));
	printf("Press Return to continue\n");
	read(0, buffer, sizeof(buffer));
	return 0;
}
```
## Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

http://cs-education.github.io/sys/

The coursebook:

http://cs341.cs.illinois.edu/coursebook/index.html

Questions? Comments? Use Ed: (you'll need to accept the sign up link I sent you)
https://edstem.org/

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, *so copy your code to a separate document.* The post-video challenges (e.g. Haiku poem) are not part of homework 0 but you learn the most by doing (rather than just passively watching) - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Copy your answers into a text document (which the course staff will grade later) because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".

#include <stdio.h>
#include <unistd.h>

int main() {
	//Hello world
	write(1, "Hello world!\nI am Jin.\n", 23);
	write_triangle(3);
	return 0;
}

### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
   ```C
   *
   **
   ***
   ```
#include <stdio.h>
#include <unistd.h>


void write_triangle(int n){
	int count;
	for (count = 1; count <= n; count++){
		int dots;
		for (dots = count; dots; dots--){
			write(STDERR_FILENO, "*",1);
		}
		write(STDERR_FILENO,"\n",1);
	}
}

int main() {
	write_triangle(3);
	return 0;
}   
   

### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).
   
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	int fildes = open("message.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	write(fildes, "hello!\n", 7);
	close(fildes);
	return 0;
}

### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
   - Make sure to print to the file instead of standard out!

int main() {
	close(1);
	mode_t mode =S_IRUSR | S_IWUSR;
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	printf("printing to file\n");
	close(fildes);
	return 0;
}

5. What are some differences between `write()` and `printf()`?
Difference is that for write, we can specify the file descriptor we want to write to (stdout, stderr, own files etc). But for printf, it automatically writes to 2 (originally stdout), but if stdout is closed, it will still write to whatever file is being pointed to by 2

## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?
2. How many bytes are there in a `char`?
3. How many bytes the following are on your machine?
   - `int`, `double`, `float`, `long`, and `long long`

ANS:
For C programming on a Windows x86 system:
1. A byte contains at least 8 bits, but can contain more. (on my machine it is 8)
2. A char typically contains 1 byte.
3: 
int: 4 bytes
double: 8 bytes
float: 4 bytes
long: 4 bytes
long long: 8 bytes


### Follow the int pointer
4. On a machine with 8 byte integers:
```C
int main(){
    int data[8];
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?
0x7fbd9d48

5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?

It is a pointer that points to the memory location that is 12 (3 x 4 bytes per int) away from the starting address of the array data structure.

### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
```C
char *ptr = "hello";
*ptr = 'J';

The string "hello" is stored in a read-only section of memory. Attempting to modify it by assigning a new value to the first character, results in a violation of the memory protection

```
7. What does `sizeof("Hello\0World")` return?

12, 11 in the char array and 1 null ending character.

9. What does `strlen("Hello\0World")` return?

5, strlen stops counting after seeing the first ending character.

11. Give an example of X such that `sizeof(X)` is 3.

"Hi"

13. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.

size_t can be of size 4 or 8 depending on if it is a 32 or 64 bit system.

## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?

a.Using the sizeof operator: sizeof(argv) / sizeof(argv[0])
b.Using a loop to count the number of elements in argv until a null pointer is encountered.

3. What does `argv[0]` represent?

argv[0] is the name of the program being executed.

### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?

extern char** environ: This allows us to access the environment we are in.

### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?
size of ptr is 4, size of array is 6. When array is initialized, the size required to store "Hello" is preallocated. The size of ptr is simply the size of the pointer to the array, which is 4.

### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables?
Stack. When a function is called, the memory for its local variables is allocated on the stack, and when the function returns, that memory is deallocated from the stack

## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?
Put the data into the heap using malloc.

3. What are the differences between heap and stack memory?

heap memory is dynamically allocated at runtime and can be accessed by any function. Stack memory is first in first out. Once the function call ends, the memoru si released.

5. Are there other kinds of memory in a process?

Register/ Cache memory.

7. Fill in the blank: "In a good C program, for every malloc, there is a ___".
free. Memory that is allocated to heap must be freed once it is not needed to prevent mememory leaks.

### Heap allocation gotchas
5. What is one reason `malloc` can fail?
Once all memory in heap is allocated, malloc will be unable to allocate any more memory.


7. What are some differences between `time()` and `ctime()`?
time returns number of seconds since epoch while ctime converts that time into local time representation as a string.

9. What is wrong with this code snippet?
```C
free(ptr);
free(ptr);
```
We are trying to free memory from a location that has already been freed. There is nothing stored at the ptr location.

8. What is wrong with this code snippet?
```C
free(ptr);
printf("%s\n", ptr);
```
We are trying to print value that has already been deallocated. Once it is freed, the memory is gone.

9. How can one avoid the previous two mistakes? 
Ensuring that free(ptr) is the last instance that ptr is called upon, and making sure it is only called once. We can also set it to NULL once the memory is freed, so it doesnt point to a memory location.


### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).


struct Person {
		char* name;
		int age;
		struct Person **friends;
	};

typedef struct Person person_t;


12. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.

int main() {
	person_t* smith = (person_t*) malloc(sizeof(person_t));
	smith->name = "Agent Smith";
	smith->age = 128;

	person_t* moore = (person_t*) malloc(sizeof(person_t));
	moore->name = "Sonny Moore";
	moore->age = 256;

	smith->friends = malloc(sizeof(person_t *));
	smith->friends[0] = moore;
	moore->friends = malloc(sizeof(person_t *));
	moore->friends[0] = smith;

	free(smith->friends);
	free(moore->friends);
	free(smith);
	free(moore);
	return 0;
}

### Duplicating strings, memory allocation and deallocation of structures
Create functions to create and destroy a Person (Person's and their names should live on the heap).
12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).
person_t* create(char* name, int age) {
	person_t* p1 = (person_t*) malloc(sizeof(person_t));
	p1->name = name;
	p1->age = age;
	p1->friends = malloc(10 * sizeof(person_t *));

	int i;
	for (i = 0; i < 10; i++) {
		p1->friends[i] = malloc(sizeof(person_t));
	}

	return p1;
}

14. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.
void destory(person_t* person) {
	int i;
	for (i = 0; i < 10; i++) {
		free(person->friends[i]);
	}
	free(person->friends);
	free(person);
}

## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?
We can get characters with getchar() and pwrite to stdout with putchar()

3. Name one issue with `gets()`.
You may not have enough space allocated for it since gets() does not tell you if the input is too large.

### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".

char* s = "Hello 5 World";
	
char buffer1[25];
int val = 0;
char buffer2[25];

sscanf(s, "%s %d %s", buffer1, &val, buffer2);

### `getline` is useful
4. What does one need to define before including `getline()`?

#define _GNU_SOURCE

6. Write a C program to print out the content of a file line-by-line using `getline()`.


## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?

gcc -g

3. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.


5. Are tabs or spaces used to indent the commands after the rule in a Makefile?

tabs
7. What does `git commit` do? What's a `sha` in the context of git?

git commit saves the work commited so far and added with git add on the local repo. SHA is a hashing algorithm used for the commit to make changes to the local repository to result the changes.

9. What does `git log` show you?

It shows a list of commits made in a repo.

11. What does `git status` tell you and how would the contents of `.gitignore` change its output?

It tells you which files have been staged with git add, ready to be committed, which files are untracked. The untracked files are under the .gitignore folder.

13. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?

Git push uploads the current version of the file from your local repository to a remote repository. The comments are too general and not very helpful to others working on the same repo.

15. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?
You need to fetch and merge the remote branch again as there are issues with overlapping commits.

## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Ed.
- Find, in your opinion, the best and worst C code on the web and post the link to Ed.
- Write a short C program with a deliberate subtle C bug and post it on Ed to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on Ed.
