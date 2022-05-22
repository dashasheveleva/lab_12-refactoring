// Copyright 2022 Shevelyova Darya photodoshfy@gmail.com

#include "example.hpp"

//----------------------------------HISTOGRAM---------------------------------//
int Histogram::Get_num() const {return num;}
Histogram& Histogram::GetInstance() {
  static Histogram instance;
  return instance;
}
void Histogram::Set_svg(const float& avg_) {avg = avg_;}
void Histogram::PlusNumSkip() {++num;}
void Histogram::NewLap() {num = 0;}
float Histogram::Get_avg() const {return avg;}
//-------------------------------------LOG------------------------------------//
void Log::Write(const std::string_view& message) const {
  *out_ << "[info] " << message << std::endl;
}
void Log::WriteDebug(const std::string_view& message) const {
  if (level_) *out_ << "[debug] " << message << std::endl;
}
Log& Log::GetInstance() {
  static Log instance;
  return instance;
}
void Log::Setting(bool level) {
  level_ = level;
}
//---------------------------------USEDMEMORY---------------------------------//
void UsedMemory::OnDataLoad(const std::vector<Item>& old_items,
                            const std::vector<Item>& new_items) {
  Log::GetInstance().WriteDebug("UsedMemory::OnDataLoad");
  for (const auto& item : new_items) {
    used_ += item.id.capacity();
    used_ += item.name.capacity();
    used_ += sizeof(item.score);
  }

  for (const auto& item : old_items) {
    used_ -= item.id.capacity();
    used_ -= item.name.capacity();
    used_ -= sizeof(item.score);
  }

  Log::GetInstance().Write("UsedMemory::OnDataLoad: new size = " +
                             std::to_string(used_));
}

void UsedMemory::OnRawDataLoad(const std::vector<std::string>& old_items,
                               const std::vector<std::string>& new_items) {
  Log::GetInstance().WriteDebug("UsedMemory::OnRawDataLoads");
  for (const auto& item : new_items) {
    used_ += item.capacity();
  }
  for (const auto& item : old_items) {
    used_ -= item.capacity();
  }
  Log::GetInstance().Write("UsedMemory::OnDataLoad: new size = " +
                             std::to_string(used_));
}

size_t UsedMemory::Used() const {
  return used_;
}
//---------------------------------STATSENDER---------------------------------//
void StatSender::OnLoaded(const std::vector<Item>& new_items) {
  Log::GetInstance().WriteDebug("StatSender::OnDataLoad");

  AsyncSend(new_items, "/items/loaded");
}

void StatSender::Skip(const Item& item) {
  AsyncSend({item}, "/items/skiped");
}

void StatSender::AsyncSend(const std::vector<Item>& items,
                          std::string_view path) {
  Log::GetInstance().Write(path);
  Log::GetInstance().Write("send stat " + std::to_string(items.size()));

  for (const auto& item : items) {
    Log::GetInstance().WriteDebug("send: " + item.id);
    // ... some code
    fstr << item.id << item.name << item.score;
    fstr.flush();
  }
}
//--------------------------------PAGECONTAINER-------------------------------//
const Item& PageContainer::ByIndex(const size_t& i) const {
  return data_[i];
}

const Item& PageContainer::ById(const std::string& id) const {
  auto it = std::find_if(std::begin(data_), std::end(data_),
                         [&id](const auto& i) { return id == i.id; });
  return *it;
}

void PageContainer::PrintTable() const {
  std::cout << "|\tid\t |\t\tname\t\t|\tscore\t|\n";
  std::string separator = "_________________________________________\n";
  std::cout << separator;
  for (size_t i = 0; i < data_.size(); ++i) {
    const auto& item = ByIndex(i);
    std::cout << "|   " << item.id << "\t |\t\t" <<
        item.name << "\t\t|\t" << item.score << "\t\t|" << std::endl;
    const auto& item2 = ById(std::to_string(i));
    std::cout << "|   " << item2.id << "\t |\t\t" <<
        item2.name << "\t\t|\t" << item2.score << "\t\t|" << std::endl;
    std::cout << separator;
  }
}

void PageContainer::RawLoad(std::istream& file) {
  std::vector<std::string> raw_data;

  if (!file) throw std::runtime_error("file don`t open");

  if (file.peek() == EOF)  throw std::runtime_error("file is empty");

  Log::GetInstance().WriteDebug("file open");

  while (!file.eof()) {
    std::string line;
    std::getline(file, line, '\n');
    if (IsCorrect(line)) raw_data.push_back(std::move(line));
  }

  if (raw_data.size() < kMinLines) {
    throw std::runtime_error("too small input stream");
  }

  memory_counter_->OnRawDataLoad(raw_data_, raw_data);
  raw_data_ = std::move(raw_data);
}

void PageContainer::DataLoad(const float& threshold) {
  Histogram::GetInstance().NewLap();
  std::vector<Item> data;
  std::set<std::string> ids;
  float sum = 0;
  size_t counter = 0;
  for (const auto& line : raw_data_) {
    std::stringstream stream(line);

    Item item;
    stream >> item.id >> item.name >> item.score;

    if (auto&& [_, inserted] = ids.insert(item.id); !inserted) {
      throw std::runtime_error("already seen");
    }

    if (item.score > threshold) {
      data.push_back(std::move(item));
      sum += item.score;
      ++counter;
    } else {
      statistic_sender_->Skip(item);
      Histogram::GetInstance().PlusNumSkip();
    }
  }
  Histogram::GetInstance().Set_svg(sum/counter);
  if (data.size() < kMinLines) {
    throw std::runtime_error("correct items less then const");
  }

  memory_counter_->OnDataLoad(data_, data);
  statistic_sender_->OnLoaded(data);
  data_ = std::move(data);
}

bool PageContainer::IsCorrect(std::string& line) {
  size_t counter = 0;
  bool status = true;
  for (auto& ch : line){
    if (ch == ' ') {
      ++counter;
    } else if (counter == 0) {
      status = (ch >= '0' && ch <= '9') && status;
    } else if (counter == 1) {
      status = ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) && status;
    } else if (counter == 2) {
      status = (ch >= '0' && ch <= '9') && status;
    }
  }
  status = status && (counter == 2);
  return status;
}
size_t PageContainer::GetRawDataSize() const { return raw_data_.size(); }
size_t PageContainer::GetDataSize() const { return data_.size(); }
PageContainer::~PageContainer() {
  delete memory_counter_;
  delete statistic_sender_;
}
