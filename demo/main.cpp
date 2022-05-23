// Copyright 2022 Shevelyova Darya photodoshfy@gmail.com

#include <boost/program_options.hpp>
#include <example.hpp>

const char error_mes[] = "**********ERROR SYNTAX**********\n"
    "Look to --help or -h";

void CreatePo(boost::program_options::options_description& desc,
              boost::program_options::variables_map& vm,
              const int& argc, const char** argv){
  desc.add_options()
      ("help,h", "Help screen\n")
      ("log_debug,l", "Logger status (Release(if OFF) or Debug(if ON))\n")
      ("input,i", boost::program_options::value<std::string>()->default_value(
                              "data.txt"), "Name of input file\n")
      ("threshold,t", boost::program_options::value<int>()->
                               default_value(1), "Enter number\n");
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);
}

int main(const int argc, const char* argv[]) {
  try{
    boost::program_options::options_description desc{"Options"};
    boost::program_options::variables_map vm;
    CreatePo(desc, vm, argc, argv);
    if(vm.count("help")){
      std::cout << desc << std::endl;
    } else if(argc > 1){
      Log::GetInstance().Setting(vm.count("log_debug"));
      UsedMemory used_memory;

      PageContainer page{};
      std::ifstream in(vm["input"].as<std::string>());

      page.RawLoad(in);
      page.DataLoad(vm["threshold"].as<int>());

      Log::GetInstance().Write("Used memory: " +
                                 std::to_string(used_memory.Used()));
      Log::GetInstance().
          Write("Number skips in this lap: " +
                std::to_string(Histogram::GetInstance().Get_num()));
      Log::GetInstance().
          Write("AVG score: " +
                std::to_string(Histogram::GetInstance().Get_avg()));

      page.PrintTable();

      page.DataLoad(vm["threshold"].as<int>()+3);
      Log::GetInstance().Write("Used memory: " +
                                 std::to_string(used_memory.Used()));
      Log::GetInstance().
          Write("Number skips in this lap: " +
                std::to_string(Histogram::GetInstance().Get_num()));
      Log::GetInstance().
          Write("AVG score: " +
                std::to_string(Histogram::GetInstance().Get_avg()));
    } else {
      throw boost::program_options::error(error_mes);
    }
  } catch (const boost::program_options::error &ex) {
    std::cout << error_mes << "\n";
  }
}