#ifndef _SRC_REPL_LOOPER_H_
#define _SRC_REPL_LOOPER_H_

#define MAX_BUFFER_SIZE 102400

enum repl_error {
  REPL_SUCCESS = 0,
  REPL_MALLOC_FAIL,
  REPL_UNEXPECTED_MODULE_FAIL,
  REPL_SIGINT,
};

enum repl_error looper();

#endif // _SRC_REPL_LOOPER_H_
