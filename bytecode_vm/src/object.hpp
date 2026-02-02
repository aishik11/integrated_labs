#ifndef OBJECT_HPP
#define OBJECT_HPP

enum ObjectType { OBJ_PAIR, OBJ_FUNCTION, OBJ_CLOSURE };

struct Object {
  bool marked;
  ObjectType type;
  Object *next;

  union {
    struct {
      Object *head;
      Object *tail;
    } pair;

    struct {
      // Function specific data if any (e.g. address)
      long address;
    } func;

    struct {
      Object *fn;
      Object *env;
    } closure;
  };
};

#endif // OBJECT_HPP
