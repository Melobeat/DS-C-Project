#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED 'R'
#define BLACK 'B'
#define ERROR -1

typedef struct node {
  char *word;
  char *translation;
  char color;
  struct node *left;
  struct node *right;
  struct node *parent;
} node;

struct node tNilNode;
node *tNil = &tNilNode;
node *fRoot = NULL;
int returnNumber = 0;

void freeTree(node *root) {
  if (root != tNil) {
    if (root->left != tNil) freeTree(root->left);
    if (root->right != tNil) freeTree(root->right);

    free(root->word);
    free(root->translation);
    free(root);
  }
}

void quit(char *message) {
  fprintf(stderr, "ERROR: %s\n", message);
  freeTree(fRoot);
  
  exit(2);
}

node *newNode(char *word, char *translation) {
  node *temp = (node *)malloc(sizeof(node));
  if (temp == 0) {
    quit("Memory allocation failed. (newNode)");

    return NULL;
  } else {
    temp->word = word;
    temp->translation = translation;
    temp->color = RED;
    temp->left = NULL;
    temp->right = NULL;
    temp->parent = NULL;

    return temp;
  }
}

void rotateLeft(node **t, node *x) {
  node *y = x->right;  // set y
  x->right = y->left;  // turn y's left subtree into x's right subtree{
  if (y->left != tNil) y->left->parent = x;
  y->parent = x->parent;  // link x's parent to y
  if (x->parent == tNil)
    *t = y;
  else if (x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;
  y->left = x;  // put x on y's left
  x->parent = y;
}

void rotateRight(node **t, node *y) {
  node *x = y->left;   // set x
  y->left = x->right;  // turn x's right subtree into y's left subtree{
  if (x->right != tNil) x->right->parent = y;
  x->parent = y->parent;  // link y's parent to x
  if (y->parent == tNil)
    *t = x;
  else if (y == y->parent->right)
    y->parent->right = x;
  else
    y->parent->left = x;
  x->right = y;  // put y on x's right
  y->parent = x;
}

void redBlackInsertFixup(node **t, node *z) {
  node *y;
  while (z->parent->color == RED) {
    if (z->parent == z->parent->parent->left) {
      y = z->parent->parent->right;
      if (y->color == RED) {
        z->parent->color = BLACK;
        y->color = BLACK;
        z->parent->parent->color = RED;
        z = z->parent->parent;
      } else {
        if (z == z->parent->right) {
          z = z->parent;
          rotateLeft(t, z);
        }
        z->parent->color = BLACK;
        z->parent->parent->color = RED;
        rotateRight(t, z->parent->parent);
      }
    } else {
      y = z->parent->parent->left;
      if (y->color == RED) {
        z->parent->color = BLACK;
        y->color = BLACK;
        z->parent->parent->color = RED;
        z = z->parent->parent;
      } else {
        if (z == z->parent->left) {
          z = z->parent;
          rotateRight(t, z);
        }
        z->parent->color = BLACK;
        z->parent->parent->color = RED;
        rotateLeft(t, z->parent->parent);
      }
    }
  }
  t[0]->color = BLACK;
}

void redBlackInsert(node **t, char *word, char *translation) {
  node *z = newNode(word, translation);
  node *y = tNil;
  node *x = *t;

  // Find where to Insert new node Z into the binary search tree
  while (x != tNil) {
    y = x;
    int cmp = strcmp(z->word, x->word);
    if (cmp == 0) {

    } else if (cmp < 0)
      x = x->left;
    else
      x = x->right;
  }

  z->parent = y;
  if (y == tNil)
    *t = z;
  else if (strcmp(z->word, y->word) < 0)
    y->left = z;
  else
    y->right = z;

  // Init z as a red leaf
  z->left = tNil;
  z->right = tNil;
  z->color = RED;

  // Ensure the Red-Black property is maintained
  redBlackInsertFixup(t, z);
}

char *searchTree(node *root, char *word) {
  node *currentNode = root;

  while (currentNode != tNil) {
    int cmp = strcmp(currentNode->word, word);
    if (cmp == 0) {
      return currentNode->translation;
    }

    if (cmp > 0) {
      currentNode = currentNode->left;
    } else if (cmp < 0) {
      currentNode = currentNode->right;
    }
  }

  return NULL;
}

void printTree(node *root) {
  printf("%s:%s %c (Left:%s Right:%s) --", root->word, root->translation,
         root->color, root->left->word, root->right->word);
  if (root->left != tNil) printTree(root->left);
  if (root->right != tNil) printTree(root->right);
}

enum states { StateWord, StateTranslation };

node *readDictionary(char *filepath) {
  FILE *fp = fopen(filepath, "r");
  if (!fp) {
    quit("Failed to open dictionary. (readDictionary)");
  }

  bool stop = false;
  int state = StateWord;
  node *root = tNil;
  size_t len = 0;
  size_t max = 20;
  char *tempWord = malloc(max);
  if (tempWord == 0) {
    quit("Memory allocation failed. (readDirectory)");
  }
  char *word = NULL;
  char *translation = NULL;

  while (!stop) {
    // Iteriere ueber das Woerterbuch
    int current = fgetc(fp);
    switch (state) {
      case StateWord:
      // StateWord, wenn das deutsche Wort verarbeitet wird
        if (islower(current)) {
          // Wenn kleiner Buchstabe, zum Wort hinzufuegen
          tempWord[len++] = current;
          if (len + 1 == max) {
            // Wenn Buffergrenze erreicht, dann verdopple Buffer
            max += max;
            void *tmp = realloc(tempWord, max);
            if (NULL == tmp) {
              quit("Memory reallocation failed. (readDirectory)");
            } else {
              tempWord = tmp;
            }
          }
        } else if (current == ':' && len > 0) {
          // Wenn Doppelpunkt und Wort laenger als 0, dann wechseln in StateTranslation
          tempWord[len] = '\0';
          word = malloc(len + 1);
          if (word == 0) {
            quit("Memory allocation failed. (readDirectory)");
          }
          strcpy(word, tempWord);
          // Speichere das deutsche Wort in word
          len = 0;
          max = 20;
          void *tmp = realloc(tempWord, max);
          if (NULL == tmp) {
            quit("Memory reallocation failed. (readDirectory)");
          } else {
            tempWord = tmp;
          }
          state = StateTranslation;
        } else if (current == EOF) {
          // Wenn EOF, beende Schleife
          free(tempWord);
          stop = true;
        } else {
          state = ERROR;
        }
        break;
      case StateTranslation:
      // StateTranslation, wenn die Uebersetzung verarbeitet wird
        if (islower(current)) {
          tempWord[len++] = current;
          if (len + 1 == max) {
            max += max;
            void *tmp = realloc(tempWord, max);
            if (NULL == tmp) {
              quit("Memory reallocation failed. (readDirectory)");
            } else {
              tempWord = tmp;
            }
          }
        } else if (current == '\n' && len > 0) {
          // Wenn newline und Uebersetzung groeßer 0, dann speichere Uebersetzung und fuege
          // Eintrag dem Rot-Schwarz-Baum hinzu
          tempWord[len] = '\0';
          translation = malloc(len + 1);
          if (translation == 0) {
            quit("Memory allocation failed. (readDirectory)");
          }
          strcpy(translation, tempWord);
          redBlackInsert(&root, word, translation);
          len = 0;
          max = 20;
          void *tmp = realloc(tempWord, max);
          if (NULL == tmp) {
            quit("Memory reallocation failed. (readDirectory)");
          } else {
            tempWord = tmp;
          }
          state = StateWord;
        } else {
          state = ERROR;
        }
        break;
      default:
      // Wenn irgendeine falsche Eingabe kommt
        free(tempWord);
        quit("Falsche Formatierung im Wörterbuch.");
        stop = true;
        break;
    }
  }

  fclose(fp);

  return root;
}

int readText(node *root) {
  bool inWord = false;
  bool inclUpper = false;
  size_t max = 20;
  size_t len = 0;
  bool con = true;
  char *word = malloc(max);
  if (word == 0) {
    quit("Memory allocation failed. (readText)");
  }
  while (con) {
    // Iteriere durch den Text
    int ch = fgetc(stdin);

    // Pruefe auf richtige Eingabe
    if (ch < 32 || ch > 126)
    {
      if (ch != 10 && ch != EOF)
      {
        quit("Falsches Zeichen im Eingabetext.");
      }
    }
    
    if (islower(ch) || isupper(ch)) {
      if (isupper(ch)) {
        // Speichere, dass ein Wort einen Großbuchstaben enthaelt
        inclUpper = true;
      }
      inWord = true;
      // Speichere, ob man sich in einem Wort befindet
      word[len++] = ch;
      if (len + 1 == max) {
        max += max;
        void *tmp = realloc(word, max);
        if (NULL == tmp) {
          quit("Memory reallocation failed. (readText)");
        } else {
          word = tmp;
        }
      }
    } else {
      if (inWord) {
        word[len] = '\0';
        char *translation = NULL;
        if (inclUpper) {
          // Formatiere Wort zu Kleinbuchstaben fuer das suchen im Wb
          char *temp = malloc(len + 1);
          if (temp == 0) {
            quit("Memory allocation failed. (readText)");
          } else {
            for (size_t i = 0; i < len; ++i) {
              temp[i] = tolower(word[i]);
            }
            temp[len] = '\0';
            translation = searchTree(root, temp);
          }

          if (translation != NULL) {
            // Gebe Uebersetzung aus wenn gefunden, mit Groß Klein
            char *c = translation;
            if (isupper(word[0])) {
              char firstChar = toupper(translation[0]);
              c++;
              fputc(firstChar, stdout);
            }
            while (*c) fputc(*c++, stdout);
          } else {
            fputc('<', stdout);
            fputs(word, stdout);
            fputc('>', stdout);
          }

          inclUpper = false;
          free(temp);
        } else {
          // Gib Uebersetzung aus
          translation = searchTree(root, word);
          if (translation != NULL) {
            fputs(translation, stdout);
          } else {
            returnNumber = 1;
            fputc('<', stdout);
            fputs(word, stdout);
            fputc('>', stdout);
          }
        }

        if (ch == EOF) {
          // Terminiere Schleife
          con = false;
        } else {
          fputc(ch, stdout);
        }

        inWord = false;
        max = 20;
        len = 0;
        // void *tmp = realloc(word, max);
        // if (NULL == tmp) {
        //   quit("Memory reallocation failed. (readText)");
        // } else {
        //   word = tmp;
        // }
        free(word);
        word = malloc(max);
        if (word == 0) {
          quit("Memory allocation failed. ()");
        }

      } else {
        if (ch == EOF) {
          con = false;
        } else {
          fputc(ch, stdout);
        }
      }
    }
  }
  free(word);

  return 0;
}

int main(int argc, char *argv[]) {
  node *root = fRoot;

  if (argc == 2) {
    root = readDictionary(argv[1]);
    readText(root);
  } else {
    quit("Not enough arguments. (407)");
  }

  freeTree(root);

  return returnNumber;
}
