#include <cinttypes>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>
#include <omp.h>
#include "hit.h"

const char* INPUT = "--input";
const std::size_t SIZE_INPUT = std::strlen(INPUT);
const char* OUTPUT = "--output";
const std::size_t SIZE_OUTPUT = std::strlen(OUTPUT);
const char* NO_OMP = "--no-omp";
const std::size_t SIZE_NO_OMP = std::strlen(NO_OMP);
const char* OMP_THREADS = "--omp-threads";
const std::size_t SIZE_OMP_THREADS = std::strlen(OMP_THREADS);
const char* DEFAULT = "default";
const std::size_t SIZE_DEFAULT = std::strlen(DEFAULT);

const long double MOD = INT64_MAX;

struct Point {
  Point() = default;
  Point(const float x, const float y, const float z);
  float x_;
  float y_;
  float z_;
};

Point::Point(const float x, const float y, const float z)
: x_(x)
, y_(y)
, z_(z) {
}

struct ParseArguments {
public:
  ParseArguments() = default;
  ParseArguments(const int argc, char** argv);
  void Parse(const int argc, char** argv);
  bool CheckCorrect() const;
  int GetNumber(const char* ch);

  std::size_t threads_;
  std::string input_;
  std::string output_;
  bool is_correct_ = true;
  bool is_input_set_ = false;
  bool is_output_set_ = false;
  bool is_threads_set_ = false;
  bool is_default_set_ = false;
  bool is_no_omp = false;
};

ParseArguments::ParseArguments(const int argc, char** argv) {
  Parse(argc, argv);
}

inline bool ParseArguments::CheckCorrect() const {
  if (!is_correct_) {
    return false;
  }
  if (is_input_set_ && is_output_set_ && (is_threads_set_ || is_default_set_ || is_no_omp)) {
    return true;
  }

  return false;
}

int ParseArguments::GetNumber(const char* ch) {
  std::size_t i = 0;
  int ans = 0;
  int pow = 1;
  while (i++ != std::strlen(ch)) {
    ans += pow * (*(ch + std::strlen(ch) - i) - '0');
    pow *= 10;
  }

  return ans;
}

void ParseArguments::Parse(const int argc, char** argv) {
  for (std::size_t i = 1; i < argc; ++i) {
    if (!std::strncmp(argv[i], INPUT, SIZE_INPUT) && std::strlen(argv[i]) == SIZE_INPUT) {
      ++i;
      if (i == argc) {
        is_correct_ = false;
        break;
      }
      input_ = argv[i];
      is_input_set_ = true;
    } else if (!std::strncmp(argv[i], OUTPUT, SIZE_OUTPUT) && std::strlen(argv[i]) == SIZE_OUTPUT) {
      ++i;
      if (i == argc) {
        is_correct_ = false;
        break;
      }
      output_ = argv[i];
      is_output_set_ = true;
    } else if (!std::strncmp(argv[i], NO_OMP, SIZE_NO_OMP) && std::strlen(argv[i]) == SIZE_NO_OMP) {
      is_no_omp = true;
    } else if (!std::strncmp(argv[i], OMP_THREADS, SIZE_OMP_THREADS) && std::strlen(argv[i]) == SIZE_OMP_THREADS) {
      ++i;
      if (i == argc) {
        is_correct_ = false;
        break;
      }
      if (!std::strncmp(argv[i], DEFAULT, SIZE_DEFAULT) && std::strlen(argv[i]) == SIZE_DEFAULT) {
        is_default_set_ = true;
        continue;
      }
      is_threads_set_ = true;
      threads_ = GetNumber(argv[i]);
    }
  }
}

class OpenMP {
public:
  OpenMP() = default;
  OpenMP(const ParseArguments& parse);
  bool Calculate();
  
private:
  ParseArguments parse_;

  const float GetSmartRandom(uint64_t& seed, const float min_value, const float max_value) const;
  Point GeneratePoint(uint64_t& seed, const float* arr) const;
  void GetIntoOperations(const uint64_t it, const float* arr);
  const float FigureSize(const float* arr) const;
  void OutputVolume(const float volume);
};

OpenMP::OpenMP(const ParseArguments& parse)
: parse_(parse) {
}

inline const float OpenMP::FigureSize(const float* arr) const {
  return (*(arr + 1) - *arr) * (*(arr + 3) - *(arr + 2)) * (*(arr + 5) - *(arr + 4));
}

Point OpenMP::GeneratePoint(uint64_t& seed, const float* arr) const {
  Point point;
  point.x_ = GetSmartRandom(seed, *arr, *(arr + 1));
  point.y_ = GetSmartRandom(seed, *(arr + 2), *(arr + 3));
  point.z_ = GetSmartRandom(seed, *(arr + 4), *(arr + 5));
  
  return point;
}

void OpenMP::OutputVolume(const float volume) {
  std::ofstream stream(parse_.output_, std::ios::out | std::ios::binary);
  if (!stream) {
    std::cerr << "ERROR:\nCannot open an output file\n";
    std::exit(EXIT_FAILURE);
  }
  stream << volume;
  stream.close();
}

void OpenMP::GetIntoOperations(const uint64_t it, const float* arr) {
  uint32_t threads = 0;
  uint32_t points = 0;
  if (parse_.threads_) {
    omp_set_num_threads(parse_.threads_);
  }
  auto start_time = omp_get_wtime();
  #pragma omp parallel default(none) shared(threads, points, it, arr)
  {
    uint64_t seed = omp_get_wtime();
    uint64_t sum = 0;
    #pragma omp for schedule(static, 50)
    for (std::size_t i = 0; i < it; ++i) {
      Point point = GeneratePoint(seed, arr);
      if (hit_test(point.x_, point.y_, point.z_)) {
        ++sum;
      }
    }
    #pragma omp atomic
      points += sum;
    #pragma omp atomic
      ++threads;
  }
  auto end_time = omp_get_wtime();
  printf("Time (%i thread(s)): %g ms\n", threads, (end_time - start_time) * 1000);//ms
  OutputVolume(FigureSize(arr) * points / it);
}

bool OpenMP::Calculate() {
  std::string str;
  std::ifstream stream(parse_.input_, std::ios::in | std::ios::binary);
  if (!stream) {
    std::cerr << "Unable to open the file\n";
    return false;
  }
  stream >> str;
  GetIntoOperations(std::stoi(str), get_axis_range());
  stream.close();

  return true;
}

const float OpenMP::GetSmartRandom(uint64_t& seed, const float min_value, const float max_value) const {
  seed ^= seed << 13;
	seed ^= seed >> 7;
	seed ^= seed << 17;

	return min_value + static_cast<float>(seed) /
            (static_cast<float>(UINT64_MAX) / static_cast<float>(max_value - min_value));
}

int main(int argc, char** argv) {
  ParseArguments arguments(argc, argv);
  if (!arguments.CheckCorrect()) {
    std::cerr << "Incorrect input\n";
    return EXIT_FAILURE;
  }
  OpenMP mp(arguments);
  if (!mp.Calculate()) {
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
