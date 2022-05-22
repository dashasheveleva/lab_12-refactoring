// Copyright 2022 Shevelyova Darya photodoshfy@gmail.com

#ifndef INCLUDE_EXAMPLE_HPP_
#define INCLUDE_EXAMPLE_HPP_

#include <iostream>
#include <algorithm>
#include <set>
#include <cstring>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <string_view>

//----------------------------------HISTOGRAM---------------------------------//
class Histogram{
 public:
  static Histogram& GetInstance();

  [[nodiscard]] int Get_num() const;

  [[nodiscard]] float Get_avg() const;

  void Set_svg(const float& avg_);

  void PlusNumSkip();

  void NewLap();
 private:
  Histogram() = default;
  Histogram( const Histogram&) = delete;
  Histogram& operator=( Histogram& ) = delete;

  int num = 0;
  float avg = 0;
};
//------------------------------------LOG-------------------------------------//
class Log {
 public:
  static Log& GetInstance();

  void Setting(bool level);

  void Write(const std::string_view& message) const;

  void WriteDebug(const std::string_view& message) const;

 private:
  Log(): level_(false), out_(&std::cout){}

  Log( const Log&) = delete;
  Log& operator=(Log& ) = delete;

  bool level_ = false;
  mutable std::ostream* out_;
};

struct Item {
  std::string id;
  std::string name;
  float score = 0;
};
//---------------------------------USEDMEMORY---------------------------------//
class UsedMemory {
 public:
  void OnDataLoad(const std::vector<Item>& old_items,
                  const std::vector<Item>& new_items);

  void OnRawDataLoad(const std::vector<std::string>& old_items,
                     const std::vector<std::string>& new_items);

  [[nodiscard]] size_t Used() const;

 private:
  size_t used_ = 0;
};
//---------------------------------STATSENDER---------------------------------//
class StatSender {
 public:
  void OnLoaded(const std::vector<Item>& new_items);
  void Skip(const Item& item);
  virtual ~StatSender() = default;

 private:
  virtual void AsyncSend(const std::vector<Item>& items, std::string_view path);
  std::ofstream fstr{"network", std::ios::binary};
};
//--------------------------------PAGECONTAINER-------------------------------//
constexpr size_t kMinLines = 10;

class PageContainer {
 public:
  void RawLoad(std::istream& file);

  [[nodiscard]] const Item& ByIndex(const size_t& i) const;

  [[nodiscard]] const Item& ById(const std::string& id) const;

  [[nodiscard]] size_t GetRawDataSize() const;

  [[nodiscard]] size_t GetDataSize() const;

  void DataLoad(const float& threshold);

  static bool IsCorrect(std::string& line);

  void PrintTable() const;

  explicit PageContainer(UsedMemory* memory_counter = new UsedMemory(),
                         StatSender* statistic_sender = new StatSender())
      : memory_counter_(memory_counter), statistic_sender_(statistic_sender){}

  ~PageContainer();

 private:
  UsedMemory* memory_counter_;
  StatSender* statistic_sender_;
  std::vector<Item> data_;
  std::vector<std::string> raw_data_;
};

#endif // INCLUDE_EXAMPLE_HPP_
