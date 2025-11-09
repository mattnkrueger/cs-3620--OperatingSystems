#include <string.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

typedef struct
{
  unsigned short tid;
  unsigned short challenge;
} 
tinput_t;

unsigned int NSOLUTIONS = 8;

// ============================ //
//    GLOBAL SOLUTIONS FOUND    //
// ============================ //
// these are accessed by worker threads 
unsigned short found_solutions = 0;
unsigned long* solutions;

// MUTEX
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
usigned

/**
 * @brief return 0 if number is divisible by [1000000..1500000] otherwise 1
 * 
 * @param n - number to check
 * @return unsigned short 
 */
unsigned short divisibility_check(unsigned long n)
{
  for (unsigned long i=1000000; i<=1500000; i++)
  {
    if (n%i == 0)
    {
      return 0;
    }
  }

  return 1;
}

/**
 * @brief computes SHA-256 of attempted_solution, checking if the first 2 bytes of the hash match challenge value
 * 
 * @param challenge - 2 byte value
 * @param attempted_solution - solution to check
 * @return short 
 */
short try_solution(unsigned short challenge, unsigned long attempted_solution)
{
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, &attempted_solution, 8);
  SHA256_Final(digest, &ctx);

  if ((*(unsigned short*)digest) == challenge)
  {
    return 1;
  } 
  else 
  {
    return 0;
  }
}

/**
 * @brief brute forces integers from [0..1000000000] to check against target. Records NSOLUTIONS valid solutions within global solutions array.
 * 
 * @param tinput_void - input thread structure
 * @return void* 
 */
void* worker_thread_function(void *tinput_void) 
{
  tinput_t* tinput = (tinput_t*) tinput_void;

  unsigned long start = 0;
  unsigned long end = start + 1000000000l;
  unsigned long attempted_solution; 

  for (attempted_solution=start; attempted_solution<end; attempted_solution++)
  {

    pthread_mutex_lock(&lock);
    if (found_solutions >= NSOLUTIONS)
    {
      pthread_mutex_unlock(&lock);
      return NULL;
    }
    
    if (try_solution(tinput->challenge, attempted_solution))
    {
      short bad_solution = 0;

      for (int i=0; i<found_solutions; i++)
      {
        if (attempted_solution%10 == solutions[i]%10)
        {
          bad_solution = 1;
        }
      }

      if (bad_solution) continue;


      if (!divisibility_check(attempted_solution)) continue;

      if (found_solutions < NSOLUTIONS)
      {
        solutions[found_solutions] = attempted_solution;
        found_solutions++;
      }
      pthread_mutex_unlock(&lock);
    }
  }
}

/**
 * @brief launch n worker threads to solve challenge 
 * 
 * @param challenge - target hash prefix (shorts are 2 bytes)
 * @param nthread - thread to solve challenge on
 */
void solve_one_challenge(unsigned short challenge, unsigned short nthread)
{
  pthread_t threads[nthread];
  tinput_t inputs[nthread];

  found_solutions = 0;
  solutions = (unsigned long*) malloc(NSOLUTIONS * (sizeof(unsigned long)));

  for (int i=0; i<nthread; i++)
  {
    solutions[i] = 0;
  }

  for (int i=0; i<nthread; i++)
  {
    inputs[i].tid = i;
    inputs[i].challenge = challenge;

    pthread_create(                           // CREATE A NEW THREAD
      &(threads[i]),                          // output thread id
      NULL,                                   // optional attribs
      worker_thread_function,                 // function to run in new thread
      &(inputs[i])                            // arguments passed to function
    );
  }

  for (int i=0; i<nthread; i++) 
  {
    pthread_join(threads[i], NULL);
  }

  printf("%d", challenge);
  for (int i=0; i<NSOLUTIONS; i++)
  {
    printf("%ld", solutions[i]);
  }

  printf("\n");
  free(solutions);
}

/**
 * @brief entry point. Ex: `./task 3 12 1000 123` will use 3 threads to solve the challenges 12, 1000, 123.
 * 
 * @param argc - count of arguments
 * @param argv - contains the number of worker threads (argv[1]) and challenges to solve
 * @return int 
 */
int main(int argc, char* argv[]) 
{
  unsigned short nthread = strtol(argv[1], NULL, 10);   

  for (int i=2; i<argc; i++)
  {
    unsigned short challenge = strtol(argv[i], NULL, 10);
    solve_one_challenge(challenge, nthread);
  }

  return 0;
}