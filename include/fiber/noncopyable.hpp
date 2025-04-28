#ifndef _NONCOPYABLE_H__
#define _NONCOPYABLE_H__

class Nonecopyable {
 public:
  Nonecopyable() = default;
  ~Nonecopyable() = default;
  Nonecopyable(const Nonecopyable &) = delete;
  Nonecopyable operator=(const Nonecopyable) = delete;
};

#endif