#ifndef BASE_LOGGING_H_
#define BASE_LOGGING_H_

#include <iostream>
#include <vector>

struct LOG_t {
  ~LOG_t() { std::cerr << std::endl; }
};
#define VLOG(_verbose)                                                         \
  (LOG_t(), std::cerr << "F: " << __FILE__ << 'L' << __LINE__ << ": ")
#define LOG_E (LOG_t(), std::cerr << "Error: ")
#define LOG_W (LOG_t(), std::cerr << "Warning: ")

template <typename T>
static std::ostream &operator<<(std::ostream &o, const std::vector<T> &v) {
  o << "[ ";
  for (const auto &e : v)
    o << e << ' ';
  return o << ']';
}
template <typename T1, typename T2>
inline std::ostream &operator<<(std::ostream &o, const std::pair<T1, T2> p) {
  o << '(' << p.first << ':' << p.second << ')';
  return o;
}

#endif // BASE_LOGGING_H_
