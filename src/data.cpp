#include "sol.hpp"
#include "tools.hpp"
#include <random>

template <class S> void bench(S sol) {
  using namespace std;

  cout << "=== Benchmark a-a a-b: " << name_of(sol) << endl;
  string s1(20000, 'a');
  string s2(20000, 'a');
  string s3(20000, 'b');

  execute_lambda([&] {
    fail_not(sol.lev_dist(s1, s2) == 0);
    fail_not(sol.lev_dist(s1, s3) == 20000);
  });

  cout << "=== Benchmark random data: " << name_of(sol) << endl;
  auto gen = default_random_engine{100500};
  uniform_int_distribution<int> distribution{'A', 'z'};

  for (char &c : s1) {
    c = distribution(gen);
  }
  for (char &c : s2) {
    c = distribution(gen);
  }
  execute_lambda([&] {
    fail_not(sol.lev_dist(s1, s2) > 10);
    fail_not(sol.lev_dist(s1, s2) > 10);
  });
}

int main() {
  bench(sol3{});
  bench(sol3b{});
}
