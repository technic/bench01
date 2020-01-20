#include <algorithm>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

struct sol0 {
  static constexpr auto comment = "original author solution";

  size_t lev_dist(const std::string &s1, const std::string &s2) {
    const auto m = s1.size();
    const auto n = s2.size();

    std::vector<int64_t> v0;
    v0.resize(n + 1);
    std::iota(v0.begin(), v0.end(), 0);
    auto v1 = v0;

    for (size_t i = 0; i < m; ++i) {
      v1[0] = i + 1;
      for (size_t j = 0; j < n; ++j) {
        auto delCost = v0[j + 1] + 1;
        auto insCost = v1[j] + 1;
        auto substCost = s1[i] == s2[j] ? v0[j] : (v0[j] + 1);
        v1[j + 1] = std::min({delCost, insCost, substCost});
      }
      std::swap(v0, v1);
    }

    return v0[n];
  }
};

struct sol1 {
  static constexpr auto comment = "with std::min fix";
  size_t lev_dist(const std::string &s1, const std::string &s2) {
    const auto m = s1.size();
    const auto n = s2.size();

    std::vector<int64_t> v0;
    v0.resize(n + 1);
    std::iota(v0.begin(), v0.end(), 0);
    auto v1 = v0;

    for (size_t i = 0; i < m; ++i) {
      v1[0] = i + 1;
      for (size_t j = 0; j < n; ++j) {
        auto delCost = v0[j + 1] + 1;
        auto insCost = v1[j] + 1;
        auto substCost = s1[i] == s2[j] ? v0[j] : (v0[j] + 1);
        v1[j + 1] = std::min(substCost, std::min(delCost, insCost));
      }
      std::swap(v0, v1);
    }

    return v0[n];
  }
};

struct sol2 {
  static constexpr auto comment = "with s1[i] fix";
  size_t lev_dist(const std::string &s1, const std::string &s2) {
    const auto m = s1.size();
    const auto n = s2.size();

    std::vector<int64_t> v0;
    v0.resize(n + 1);
    std::iota(v0.begin(), v0.end(), 0);
    auto v1 = v0;

    for (size_t i = 0; i < m; ++i) {
      v1[0] = i + 1;
      char c1 = s1[i];
      for (size_t j = 0; j < n; ++j) {
        auto delCost = v0[j + 1] + 1;
        auto insCost = v1[j] + 1;
        auto substCost = c1 == s2[j] ? v0[j] : (v0[j] + 1);
        v1[j + 1] = std::min({delCost, insCost, substCost});
      }
      std::swap(v0, v1);
    }

    return v0[n];
  }
};

struct sol3 {
  static constexpr auto comment = "with std::min and s1[i] fixes";
  size_t lev_dist(const std::string &s1, const std::string &s2) {
    const auto m = s1.size();
    const auto n = s2.size();

    std::vector<int64_t> v0;
    v0.resize(n + 1);
    std::iota(v0.begin(), v0.end(), 0);
    auto v1 = v0;

    for (size_t i = 0; i < m; ++i) {
      v1[0] = i + 1;
      char c1 = s1[i];
      for (size_t j = 0; j < n; ++j) {
        auto delCost = v0[j + 1] + 1;
        auto insCost = v1[j] + 1;
        auto substCost = c1 == s2[j] ? v0[j] : (v0[j] + 1);
        v1[j + 1] = std::min(substCost, std::min(delCost, insCost));
      }
      std::swap(v0, v1);
    }
    return v0[n];
  }
};

struct sol3a {
  static constexpr auto comment = "with better min and s1[i] fix";
  size_t lev_dist(const std::string &s1, const std::string &s2) {
    const auto m = s1.size();
    const auto n = s2.size();

    std::vector<int64_t> v0;
    v0.resize(n + 1);
    std::iota(v0.begin(), v0.end(), 0);
    auto v1 = v0;

    for (size_t i = 0; i < m; ++i) {
      v1[0] = i + 1;
      char c1 = s1[i];
      for (size_t j = 0; j < n; ++j) {
        auto delInsCost = std::min(v0[j + 1], v1[j]) + 1;
        auto substCost = c1 == s2[j] ? v0[j] : (v0[j] + 1);
        v1[j + 1] = std::min(delInsCost, substCost);
      }
      std::swap(v0, v1);
    }
    return v0[n];
  }
};

struct sol3b {
  static constexpr auto comment = "with inline min and s1[i] fix";
  size_t lev_dist(const std::string &s1, const std::string &s2) {
    const auto m = s1.size();
    const auto n = s2.size();

    std::vector<int> v0;
    v0.resize(n + 1);
    std::iota(v0.begin(), v0.end(), 0);
    auto v1 = v0;

    for (size_t i = 0; i < m; ++i) {
      v1[0] = i + 1;
      char c1 = s1[i];
      for (size_t j = 0; j < n; ++j) {
        auto substCost = c1 == s2[j] ? v0[j] : (v0[j] + 1);
        v1[j + 1] = std::min(substCost, std::min(v0[j + 1], v1[j]) + 1);
      }
      std::swap(v0, v1);
    }
    return v0[n];
  }
};
