// Copyright 2022 Shevelyova Darya photodoshfy@gmail.com

#ifndef INCLUDE_EXAMPLE_HPP_
#define INCLUDE_EXAMPLE_HPP_

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <string_view>

class Log {
 public:
  void Write(std::string_view message) const;

  void WriteDebug(std::string_view message) const;

  explicit Log(size_t level);

 private:
  size_t level_ = 0;
  mutable std::ostream* out_;
};

struct Item {
  std::string id;
  std::string name;
  float score = 0;
};

class UsedMemory {
 public:
  explicit UsedMemory(const Log& log);

  void OnDataLoad(const std::vector<Item>& old_items,
                  const std::vector<Item>& new_items);

  void OnRawDataLoad(const std::vector<std::string>& old_items,
                     const std::vector<std::string>& new_items);

  size_t used() const;

 private:
  const Log* log_;
  size_t used_ = 0;
};

class StatSender {
 public:
  explicit StatSender(const Log& log);
  void OnLoaded(const std::vector<Item>& new_items);

  void Skip(const Item& item);

 private:
  void AsyncSend(const std::vector<Item>& items, std::string_view path);

  const Log* log_;
  std::ofstream fstr{"network", std::ios::binary};
};

class PageContainer {
 public:
  void Load(std::istream& io, float threshold);
  const Item& ByIndex(size_t i) const;

  const Item& ById(const std::string& id) const;

  void Reload(float threshold);

  PageContainer(const Log& log, UsedMemory* memory_counter);

 private:
  const Log* log_;
  UsedMemory* memory_counter_;
  StatSender stat_sender_;
  std::vector<Item> data_;
  std::vector<std::string> raw_data_;
};

#endif // INCLUDE_EXAMPLE_HPP_
