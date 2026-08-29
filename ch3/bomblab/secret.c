typedef struct Node{
  int value;
  struct Node *left;
  struct Node *right;
} Node;

// initial x value = 0x24
int fun7(Node *x, long input) {
  if (!x) {
    return 0xffffffff;
  }
  if (x->value > input) {
    return fun7(x->left, input) * 2;
  } else if (x->value == input) {
    return 0;
  } else {
    return fun7(x->right, input) * 2 + 1;
  }
}
