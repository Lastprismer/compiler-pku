int main() {
  int a = 1;
  {
    {
      { a + 2; }
    }
    return a;
  }
  a = a + 5;
  return a;
}