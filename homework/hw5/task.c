#include <string.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct {
  unsigned short tid;
  unsigned short challenge;
  unsigned short nthread;
} tinput_t;

const unsigned int NSOLUTIONS = 8;

// ================== //
//  GLOBAL SOLUTIONS  //
// ================== //
unsigned short found_solutions = 0;
unsigned long* solutions;

// =============== //
//  NEXT TO CHECK  //
// =============== //
unsigned long next_candidate = 0;

// ======= //
//  LOCKS  //
// ======= //
pthread_rwlock_t solutions_lock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t work_lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief return 0 if number is divisible by [1000000..1500000] otherwise 1
 * 
 * @param n - number to check
 * @return unsigned short 
 */
unsigned short divisibility_check(unsigned long n) {
  const unsigned long START = 1000000;
  const unsigned long END = 1500000;
  
  if (n < START) return 1;
  if (n <= END) return 0;
  
  for (unsigned long i = START; i <= END; i++) 
  {
    if (n % i == 0) 
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
short try_solution(unsigned short challenge, unsigned long attempted_solution) {
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, &attempted_solution, 8);
  SHA256_Final(digest, &ctx);
  
  return (*(unsigned short*)digest == challenge) ? 1 : 0;
}

/**
 * @brief round robin execution across search space by threads
 * 
 * @param tinput_void - input thread structure
 * @return void* 
 */
void* worker_thread_function(void *tinput_void) {
  tinput_t* tinput = (tinput_t*) tinput_void;
  
  while (1) {
    // get candidate to check 
    pthread_mutex_lock(&work_lock);
    unsigned long attempted_solution = next_candidate++;
    pthread_mutex_unlock(&work_lock);
    
    // if all solutions already found -> exit
    pthread_rwlock_rdlock(&solutions_lock);
    if (found_solutions >= NSOLUTIONS) {
      pthread_rwlock_unlock(&solutions_lock);
      break;
    }
    pthread_rwlock_unlock(&solutions_lock);
    
    // try new solution
    if (!try_solution(tinput->challenge, attempted_solution)) continue;
    if (!divisibility_check(attempted_solution)) continue;
    
    pthread_rwlock_wrlock(&solutions_lock);
    
    // if all found -> exit
    if (found_solutions >= NSOLUTIONS) {
      pthread_rwlock_unlock(&solutions_lock);
      break;
    }
    
    // BAD SOLUTION - least significant digits match
    short bad_solution = 0;
    for (int i = 0; i < found_solutions; i++) {
      if (attempted_solution % 10 == solutions[i] % 10) {
        bad_solution = 1;
        break;
      }
    }
    
    // SOLUTION FOUND
    if (!bad_solution) {
      solutions[found_solutions] = attempted_solution;
      found_solutions++;
    }
    
    pthread_rwlock_unlock(&solutions_lock);
  }
  
  return NULL;
}

/**
 * @brief launch n worker threads to solve challenge 
 * 
 * @param challenge - target hash prefix (shorts are 2 bytes)
 * @param nthread - thread to solve challenge on
 */
void solve_one_challenge(unsigned short challenge, unsigned short nthread) {
  pthread_t threads[nthread];
  tinput_t inputs[nthread];
  
  found_solutions = 0;
  next_candidate = 0;
  solutions = (unsigned long*) malloc(NSOLUTIONS * sizeof(unsigned long));
  
  for (int i = 0; i < NSOLUTIONS; i++) {
    solutions[i] = 0;
  }
  
  // Create worker threads
  for (int i = 0; i < nthread; i++) {
    inputs[i].tid = i;
    inputs[i].challenge = challenge;
    inputs[i].nthread = nthread;
    
    pthread_create(                           // CREATE A NEW THREAD
      &(threads[i]),                          // output thread id
      NULL,                                   // optional attribs
      worker_thread_function,                 // function to run in new thread
      &(inputs[i])                            // arguments passed to function
    );
  }
  
  for (int i = 0; i < nthread; i++) {
    pthread_join(threads[i], NULL);
  }
  
  printf("%hu", challenge);
  for (int i = 0; i < NSOLUTIONS; i++) {
    printf(" %lu", solutions[i]);
  }
  printf("\n");
  
  free(solutions);
}

/**
 * @brief entry point. Ex: `./task 3 12 1000 123`
 * 
 * @param argc - count of arguments
 * @param argv - contains the number of worker threads (argv[1]) and challenges to solve
 * @return int 
 */
int main(int argc, char* argv[]) {
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s <nthreads> <challenge1> [challenge2...]\n", argv[0]);
    return 1;
  }
  
  unsigned short nthread = strtol(argv[1], NULL, 10);

  bool valid = (nthread > 0 && nthread <= 100) ? true : false;
  if (!valid) 
  {
    fprintf(stderr, "Number of threads (%hu) must be between [1-100]\n", nthread);
    return 1;
  }
  
  for (int i = 2; i < argc; i++) {
    unsigned short challenge = strtol(argv[i], NULL, 10);
    solve_one_challenge(challenge, nthread);
  }
  
  return 0;
}