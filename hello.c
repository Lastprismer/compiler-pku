// Really long code;
int n;

int getMost(int arr[]) {
  int count[1000];
  int i;
  i = 0;
  while (i < 1000) {
    count[i] = 0;
    i = i + 1;
  }
  i = 0;
  int max;
  int number;
  max = 0;
  while (i < n) {
    int num;
    num = arr[i];
    count[num] = count[num] + 1;
    if (count[num] > max) {
      max = count[num];
      number = num;
    }
    i = i + 1;
  }
  return number;
}

int main() {
  n = 32;
  int arr[32] = {0};
  int t = getMost(arr);
  putint(t);
  return 0;
}