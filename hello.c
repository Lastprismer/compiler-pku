int main() {
  int a = 1, b = 0;
  if (a || b) {
    return 0;
  }
  if (a && b) {
    return 55;
  }
  return 199;
}