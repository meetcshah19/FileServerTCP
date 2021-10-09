// Header file for worker pool

/* WHY 33?
** Total students in classroom ~100
** One student reads, other may write, 
** third may publish their work on repository
*/
#define MAX_WORKERS 33

typedef void *thread_pool;

//this is used to declare a pointer to a function
typedef void (*dispatch_fn)(void *);

thread_pool create_thread_pool(int num_threads);

void dispatch(thread_pool workers, dispatch_fn d_funct, void *args);

void destroy_thread_pool(thread_pool to_be_dead_workers);