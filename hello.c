void f1d(int arr[]) {
  arr[1] = 1;
}

int main() {
  int a[10];
  f1d(a);
  return a[1];
}