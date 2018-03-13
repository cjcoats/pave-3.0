#include <string.h>
#include "LinkedList.h"

class StringPair: public linkedList {
 public:
  StringPair(char *name, char *value);
  virtual ~StringPair();      	// Destructor

  int  match(void *target); 	// override baseType's 
                                // virtual match() 

  char *getClassName(void);     // override baseType's 
                                // virtual getClassName()
  
  void print(FILE *output); 	// override linkedList's print()

  char *getName(void) {return value_;};

 private:
  char *name_;
  char *value_;
};
