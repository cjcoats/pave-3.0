#include "LinkedList.h"

class Alias: public linkedList {
 public:
  Alias(char *name, int ntoken, char **token, int *tflag);
  virtual ~Alias();      	// Destructor

  int  match(void *target); 	// override baseType's 
                                // virtual match() 

  char *getClassName(void);     // override baseType's 
                                // virtual getClassName()
  
  void print(FILE *output); 	// override linkedList's print()

  int getNtoken() { return ntoken_;};
  int *getTflag() { return tflag_;};
  char **getToken() { return token_;};

 private:
  char *name_;
  char **token_;
  int ntoken_;
  int *tflag_;
};
