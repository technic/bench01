#include <chrono>
#include <cmath>
#include <iostream>
#include <iterator>
#include <thread>
#include <vector>
#include <boost/core/demangle.hpp>

void fail_not(bool b) {
  if (!b) {
    std::cerr << "check fail!\n";
    std::abort();
  }
}

template <class R> void execute(R &runner) {
  using namespace std;
  vector<double> times;
  for (unsigned int i = 0; i < 6; i++) {
    const auto t1 = chrono::high_resolution_clock::now();
    runner.run();
    const auto t2 = chrono::high_resolution_clock::now();
    times.push_back(chrono::duration<double, milli>{t2 - t1}.count());
  }
  // ignore first - warm up
  double t0 = *min_element(++times.cbegin(), times.cend());
  double var = 0.0;
  for (auto it = ++times.cbegin(); it != times.cend(); ++it) {
    auto t = *it;
    var += (t - t0) * (t - t0);
  }
  var = sqrt(var / (times.size() - 1));

  cout << "results: ";
  copy(times.begin(), times.end(), ostream_iterator<double>(cout, "ms "));
  cout << "\n";
  cout << "time: " << t0 << " ms ± " << var << " ms (" << var / t0 * 100.
       << "% error)\n"
       << endl;

  // cool down
  this_thread::sleep_for(chrono::milliseconds(1000));
}

template <typename F> struct LambdaRunner {
  F f;
  [[gnu::noinline]] void run() { f(); }
};

template <typename F> void execute_lambda(F f) {
  auto r = LambdaRunner<F>{f};
  execute(r);
}

template <typename S> std::string name_of(const S &sol) {
  return boost::core::demangle(typeid(S).name()) + " - " + S::comment;
}
