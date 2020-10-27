#include "lua_shared_queue.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>

#define BUF_SIZE 1024 * 1024 * 5 // max msg size
#define SHARED_MEMORY_FILENAME "/shared_queue"

typedef struct {
  struct shmseg* shmseg;
} shared_queue_t;

struct shmseg {
  char buf[BUF_SIZE];
  sem_t consumers_mutex; // consumers wait for a msg in the buffer
  sem_t producers_mutex; // producers waits for consumers to be ready for a message
};

/*
 * consumer let the producers know that he is ready to take a msg (producers_mutex), and waits for producers to put a msg in the buffer (consumers_mutex)
 * @param[1] - shared_queue
 * @return msg 
 */
LUALIB_API int lua_shared_queue_consume(lua_State *L) {
  shared_queue_t* shared_queue = (shared_queue_t *)luaL_checkudata(L, 1, "shared_queue");

  if (sem_post(&shared_queue -> shmseg -> producers_mutex) < 0) {
    die(L, "consume - sem_post failed on producers_mutex - error: %d", errno);
  }
  if (sem_wait(&shared_queue -> shmseg -> consumers_mutex) < 0) {
    die(L, "consume - sem_wait failed on consumers_mutex - error: %d", errno);
  }
  lua_pushstring (L, shared_queue -> shmseg -> buf);

  return 1;
}

/*
 * producers wait for consumers to be ready to take a msg (producers_mutex), and after he puts the message there, he let the consumers know that they can take it (consumers_mutex)
 * @param[1] - shared_queue
 * @param[2] - msg
 */
LUALIB_API int lua_shared_queue_publish(lua_State *L) {
  shared_queue_t* shared_queue = (shared_queue_t *)luaL_checkudata(L, 1, "shared_queue");
  char* msg = (char *)luaL_checkstring(L, 2);
  int msg_len = strlen(msg);

  if (msg_len > BUF_SIZE - 1) {
    die(L, "message is exceeding the allowed size - msg size: %d, buffer suze: %d", msg_len, BUF_SIZE);
  }

  if (sem_wait(&shared_queue -> shmseg -> producers_mutex) < 0) {
    die(L, "publish - sem_wait failed on producers_mutex - error: %d", errno);
  }
  strcpy(shared_queue -> shmseg -> buf, msg);

  if (sem_post(&shared_queue -> shmseg -> consumers_mutex) < 0) {
    die(L, "publish - sem_post failed on consumers_mutex - error: %d", errno);
  }

  return 0;
}

/*
 * @param[1] - master - if its master then it inits all the shared segmant variables - it can only be called once with master = true
 * @return shared_queue object 
 */
LUALIB_API int lua_shared_queue_init(lua_State *L) {
  int fd, oflag;
  struct shmseg *shmp;
  int master = luaL_optboolean(L, 1, 0);

  if (master) {
    oflag = O_CREAT | O_RDWR; 
  } else {
    oflag = O_RDWR;
  }

  fd = shm_open(SHARED_MEMORY_FILENAME, oflag, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    die(L, "shm_open failed");
  }

  if (master && ftruncate(fd, sizeof(struct shmseg)) == -1) {
    throw_error(L, "ftruncate failed");
  }

  shmp = (struct shmseg*)mmap(NULL, sizeof(struct shmseg), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    die(L, "mmap failed - error: %d", errno);
  }

  if (master) {
    if (sem_init(&(shmp -> consumers_mutex), 1, 0) < 0) {
      die(L, "sem_init failed on consumers_mutex - error: %d", errno);
    }
    if (sem_init(&(shmp -> producers_mutex), 1, 0) < 0) {
      die(L, "sem_init failed on producers_mutex - error: %d", errno);
    }
  }

  shared_queue_t *shared_queue = (shared_queue_t *) lua_newuserdata(L, sizeof(shared_queue_t));
  setmeta(L, "shared_queue");

  shared_queue -> shmseg = shmp;

  return 1;
}

static const struct luaL_reg lua_shared_queue_reg[] = {
  { "publish",  lua_shared_queue_publish },
  { "consume",  lua_shared_queue_consume },
  { NULL, NULL }
};

static luaL_Reg lua_shared_queue_module[] = {
  { "init", lua_shared_queue_init },
  { NULL, NULL }
};

LUALIB_API int luaopen_shared_queue(lua_State *L) {
  lua_newtable(L);
  createmeta(L, "shared_queue", lua_shared_queue_reg);
  luaL_setfuncs(L, lua_shared_queue_module, 0);

  return 1;
}
