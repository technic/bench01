#include "sol.hpp"
#include "tools.hpp"
#include <iostream>
#include <string>
#include <vector>

template <class S> struct Runner {
  S sol;
  std::string s1, s2, s3;
  Runner(S sol) : sol(sol), s1(20000, 'a'), s2(20000, 'a'), s3(20000, 'b') {
    std::cout << "=== Benchmark soluton: " << name_of(sol) << std::endl;
  }
  [[gnu::noinline]] void run() {
    auto d12 = sol.lev_dist(s1, s2);
    fail_not(d12 == 0);
    auto d13 = sol.lev_dist(s1, s3);
    fail_not(d13 == 20000);
  }
};

template <class S> void bench(S sol) {
  auto r = Runner<S>{sol};
  execute(r);
}

int main() {
  bench(sol0{});
  bench(sol1{});
  bench(sol2{});
  bench(sol3{});
  bench(sol3b{});
}
