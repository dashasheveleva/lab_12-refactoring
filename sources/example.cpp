// Copyright 2022 Shevelyova Darya photodoshfy@gmail.com

#include <algorithm>
#include <example.hpp>
#include <iostream>
#include <set>
#include <sstream>

void Log::Write(std::string_view message) const {
  *out_ << message << std::endl;
}

void Log::WriteDebug(std::string_view message) const {
  if (level_ > 0) *out_ << message << std::endl;
}

Log::Log(size_t level) : level_(level) { out_ = &std::cout; }

constexpr size_t kMinLines = 10;

UsedMemory::UsedMemory(const Log& log) : log_(&log) {}

void UsedMemory::OnDataLoad(const std::vector<Item>& old_items,
                const std::vector<Item>& new_items) {
  log_->WriteDebug("UsedMemory::OnDataLoad");
  for (const auto& item : old_items) {
    used_ -= item.id.capacity();
    used_ -= item.name.capacity();
    used_ -= sizeof(item.score);
  }

  for (const auto& item : new_items) {
    used_ += item.id.capacity();
    used_ += item.name.capacity();
    used_ += sizeof(item.score);
  }
  log_->Write("UsedMemory::OnDataLoad: new size = " + std::to_string(used_));
}

void UsedMemory::OnRawDataLoad(const std::vector<std::string>& old_items,
                   const std::vector<std::string>& new_items) {
  log_->WriteDebug("UsedMemory::OnRawDataLoads");
  for (const auto& item : old_items) {
    used_ -= item.capacity();
  }

  for (const auto& item : new_items) {
    used_ += item.capacity();
  }
  log_->Write("UsedMemory::OnDataLoad: new size = " + std::to_string(used_));
}

size_t UsedMemory::used() const { return used_; }

StatSender::StatSender(const Log& log) : log_(&log) {}

void StatSender::OnLoaded(const std::vector<Item>& new_items) {
  log_->WriteDebug("StatSender::OnDataLoad");

  AsyncSend(new_items, "/items/loaded");
}

void StatSender::Skip(const Item& item) { AsyncSend({item}, "/items/skiped"); }

void StatSender::AsyncSend(
    const std::vector<Item>& items,
    std::string_view path) {
  log_->Write(path);
  log_->Write("send stat " + std::to_string(items.size()));

  for (const auto& item : items) {
    log_->WriteDebug("send: " + item.id);
    // ... some code
    fstr << item.id << item.name << item.score;
    fstr.flush();
  }
}

void PageContainer::Load(std::istream& io, float threshold) {
  std::vector<std::string> raw_data;

  while (!io.eof()) {
    std::string line;
    std::getline(io, line, '\n');
    raw_data.push_back(std::move(line));
  }

  if (raw_data.size() < kMinLines) {
    throw std::runtime_error("too small input stream");
  }

  memory_counter_->OnRawDataLoad(raw_data_, raw_data);
  raw_data_ = std::move(raw_data);

  std::vector<Item> data;
  std::set<std::string> ids;
  for (const auto& line : raw_data_) {
    std::stringstream stream(line);

    Item item;
    stream >> item.id >> item.name >> item.score;

    if (auto&& [_, inserted] = ids.insert(item.id); !inserted) {
      throw std::runtime_error("already seen");
    }

    if (item.score > threshold) {
      data.push_back(std::move(item));
    } else {
      stat_sender_.Skip(item);
    }
  }

  if (data.size() < kMinLines) {
    throw std::runtime_error("oops");
  }

  memory_counter_->OnDataLoad(data_, data);
  stat_sender_.OnLoaded(data);
  data_ = std::move(data);
}

const Item& PageContainer::ByIndex(size_t i) const { return data_[i]; }

const Item& PageContainer::ById(const std::string& id) const {
  auto it = std::find_if(std::begin(data_), std::end(data_),
                         [&id](const auto& i) { return id == i.id; });
  return *it;
}

void PageContainer::Reload(float threshold) {
  std::vector<Item> data;
  std::set<std::string> ids;
  for (const auto& line : raw_data_) {
    std::stringstream stream(line);

    Item item;
    stream >> item.id >> item.name >> item.score;

    if (auto&& [_, inserted] = ids.insert(item.id); !inserted) {
      throw std::runtime_error("already seen");
    }

    if (item.score > threshold) {
      data.push_back(std::move(item));
    } else {
      stat_sender_.Skip(item);
    }
  }

  if (data.size() < kMinLines) {
    throw std::runtime_error("oops");
  }

  memory_counter_->OnDataLoad(data_, data);
  stat_sender_.OnLoaded(data);
  data_ = std::move(data);
}

PageContainer::PageContainer(const Log& log, UsedMemory* memory_counter)
    : log_(&log), memory_counter_(memory_counter), stat_sender_(*log_) {}
