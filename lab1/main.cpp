#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cinttypes>

const int kSystemBase = 16;

const int kShortArguments = 4;

const int kLongArguments = 6;

const char kDotSymbol = '.';

const int kMaxRounding = 3;

const int kPow16 = 4;

const int kMaxNumberDigits = 32;

const char kMultSign = '*';

class FixedPointClass {
public:
  FixedPointClass(const int a, const int b, const int rounding);
  void Output(int64_t value) const;
  int64_t ConvertToInt(const char* str) const;
  void SumNumbers(const char* first, const char* second) const;
  void SubstractNumbers(const char* first, char* second) const;
  void AdvancedOperation(const char* first, const char* second, const bool tag_mult) const;
private:
  int64_t GetFracValue(int64_t value) const;
  void GetFracVector(std::vector<int>& vec) const;
  void PaddWithZeros(std::vector<int>& vec) const;
  bool is_neg(const int value) const;
  void RoundLeastDigits(uint64_t& value, const int64_t carry, const int64_t measure) const;
  inline int64_t ChangeSignResult(int64_t res) const;
  int a_;
  int b_;
  int rounding_;
};

FixedPointClass::FixedPointClass(const int a, const int b, const int rounding)
: a_(a)
, b_(b)
, rounding_(rounding) {
}

struct TypeA {};
struct TypeB {};

class FloatingPointClass {
public:
  FloatingPointClass(const int rounding, const int max_exp, const int max_mantissa,
                                         const int bits_exp, const int bits_mantissa, const int digits);
  FloatingPointClass(const int rounding, TypeA): FloatingPointClass(rounding, 128, 4194304, 8, 23, 8) {};
  FloatingPointClass(const int rounding, TypeB): FloatingPointClass(rounding, 16, 512, 5, 10, 4) {};
  int64_t ConvertToInt(const char* str) const;
  void Output(int64_t value) const;
  void SumNumbers(const char* first, const char* second, const bool tag_substract) const;
  void MultiplyNumbers(const char* first, const char* second) const;
  void DivideNumbers(const char* first, const char* second) const;
  void SetParameters(const int rounding);
private:
  void NormalizeNumber(uint64_t& mantissa, int& exp) const;
  void PrintMantissa(const int mantissa) const;
  int64_t BuildNumber(const int64_t sign, const int64_t exp, const int64_t mantissa) const;
  void RoundLeastDigits(uint64_t& value, const int64_t carry, const int64_t measure, const bool is_neg) const;
  int digits_;
  int rounding_;
  int kMaxExp_;
  int kMaxMantissa_;
  int kBitsExp_;
  int kBitsMantissa_;
};

FloatingPointClass::FloatingPointClass(const int rounding, const int max_exp, const int max_mantissa,
                                       const int bits_exp, const int bits_mantissa, const int digits)
: rounding_(rounding)
, kMaxExp_(max_exp)
, kMaxMantissa_(max_mantissa)
, kBitsExp_(bits_exp)
, kBitsMantissa_(bits_mantissa)
, digits_(digits) {
}

inline int64_t GetPowerTwo(const int pow) {
  int64_t res = 1;
  for (int i = 0; i < pow; ++i) {
    res *= 2;
  }

  return res;
}

void FloatingPointClass::SetParameters(const int rounding) {
  FloatingPointClass my_parameters(rounding, TypeB());
  this->rounding_ = my_parameters.rounding_;
  this->kMaxMantissa_ = my_parameters.kMaxMantissa_;
  this->kBitsMantissa_ = my_parameters.kBitsMantissa_;
  this->kMaxExp_ = my_parameters.kMaxExp_;
  this->kBitsExp_ = my_parameters.kBitsExp_;
  this->digits_ = my_parameters.digits_;
}

int CutBits(int64_t value, int number_bites) {
  int res = 0;
  int mult = 1;
  while (number_bites--) {
    res += mult * (value % 2);
    value /= 2;
    mult *= 2;
  }

  return res;
}

void FloatingPointClass::NormalizeNumber(uint64_t& mantissa, int& exp) const {
  while (mantissa < kMaxMantissa_) {
    mantissa *= 2;
    --exp;
  }
  mantissa *= 2;
  mantissa %= (2 * kMaxMantissa_);
}

void FloatingPointClass::PrintMantissa(const int mantissa) const {
  int i = 4 * ((kBitsMantissa_) / 4 + 1) - 4;
  while (i >= 0) {
    int k = (mantissa / GetPowerTwo(i)) % kSystemBase;
    i -= 4;
    if (k >= 10) {
      std::cout << static_cast<char>('a' + (k - 10));
      continue;
    }
    std::cout << k;
  }
}

void FloatingPointClass::Output(int64_t value) const {
  uint64_t mantissa = CutBits(value, kBitsMantissa_);
  value /= (2 * kMaxMantissa_);
  int exp = CutBits(value, kBitsExp_);
  value /= (2 * kMaxExp_);
  int sign = CutBits(value, 1);
  if (exp == 2 * kMaxExp_ - 1 && !mantissa) {
    if (!sign) {
      std::cout << "inf\n";
    } else {
      std::cout << "-inf\n";
    }
    return;
  } else if (exp == 2 * kMaxExp_ - 1) {
    std::cout << "nan\n";
    return;
  }
  if (sign) {
    std::cout << '-';
  }
  if (!mantissa && !exp && kBitsMantissa_ == 23) {
    std::cout << "0x0.000000p+0\n";
    return;
  }
  if (!mantissa && !exp) {
    std::cout << "0x0.000p+0\n";
    return;
  }
  std::cout << "0x1.";
  if (!exp) {
    NormalizeNumber(mantissa, exp);
  }
  exp -= (kMaxExp_ - 1);
  mantissa = (kBitsMantissa_ == 23) ? mantissa * 2 : mantissa * 4;
  PrintMantissa(mantissa);
  std::cout << 'p';
  if (exp >= 0) {
    std::cout << '+';
  }
  std::cout << exp << '\n';
}

inline int Get16thNumber(const char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  } else if (ch >= 'a') {
    return ch - 'a' + 10;
  }

  return ch - 'A' + 10;
}

bool IsPositiveVector(const std::vector<int>& vec) {
  bool is_pos = false;
  for (int i = 3; i < vec.size(); ++i) {
    if (vec[i]) {
      is_pos = true;
      break;
    }
  }

  return is_pos;
}

void RoundUpAnswer(int64_t& whole, std::vector<int>& vec, const bool is_neg) {
  if (!IsPositiveVector(vec)) {
    return;
  }
  ++vec[2];
  if (vec[2] == 10) {
    ++vec[1];
    vec[2] = 0;
    if (vec[1] == 10) {
      ++vec[0];
      vec[1] = 0;
      if (vec[0] == 10) {
        vec[0] = 0;
        if (!is_neg) {
          ++whole;
        } else {
          --whole;
        }
      }
    }
  }
}

void RoundToNearestEven(std::vector<int>& vec, const int is_neg, int64_t& whole) {
  bool is_rounddown = true;
  bool is_set = false;
  if (vec[3] > 5) {
    is_rounddown = false;
    is_set = true;
  } else if (vec[3] < 5) {
    is_rounddown = true;
    is_set = true;
  } else {
    for (std::size_t i = 4; i < vec.size(); ++i) {
      if (vec[i]) {
        is_rounddown = false;
        is_set = true;
        break;
      }
    }
  }
  if (is_set) {
    if (is_rounddown) {
      return;
    }
    RoundUpAnswer(whole, vec, is_neg);
  } else {
    if (vec[2] % 2 == 0) {
      return;
    }
    RoundUpAnswer(whole, vec, is_neg);
  }
}

void RoundNumber(const int rounding, int64_t& whole, const bool is_neg, std::vector<int>& vec) {
  switch (rounding) {
    case 0:
      break;
    case 1:
    {
      RoundToNearestEven(vec, is_neg, whole);
      break;
    }
    case 2:
      if (whole < 0 || (whole == 0 && is_neg)) {
        return;
      }
      RoundUpAnswer(whole, vec, is_neg);
      break;
    case 3:
      if (whole > 0 || (whole == 0 && !is_neg)) {
        return;
      }
      RoundUpAnswer(whole, vec, is_neg);
      break;
    default:
      std::cerr << "ERROR:\nIncorrect format for rounding\n";
      std::exit(EXIT_FAILURE);
  }
}

int64_t FixedPointClass::ConvertToInt(const char* str) const {
  uint64_t pow = 1;
  std::size_t i = 1;
  std::size_t len = std::strlen(str);
  int64_t res = 0;
  if (len < 2) {
    std::cerr << "ERROR:\nIncorrect input\n";
    std::exit(EXIT_FAILURE);
  }
  if (str[0] != '0' || str[1] != 'x') {
    std::cerr << "ERROR:\nIncorrect input\n";
    std::exit(EXIT_FAILURE);
  }
  while (i <= std::min(static_cast<int>(len - 2), ((a_ + b_ + kPow16 - 1) / kPow16))) {
    int curr_digit = Get16thNumber(*(str + len - i));
    if (curr_digit >= 0 && curr_digit < kSystemBase) {
      res += pow * curr_digit;
    } else {
      std::cerr << "ERROR:\nIncorrect input for number\n";
      std::exit(EXIT_FAILURE);
    }
    pow *= kSystemBase;
    ++i;
  }

  return res % GetPowerTwo(a_ + b_);
}

int64_t FloatingPointClass::ConvertToInt(const char* str) const {
  int64_t pow = 1;
  std::size_t i = 1;
  std::size_t len = std::strlen(str);
  uint64_t res = 0;
  if (len < 2) {
    std::cerr << "ERROR:\nIncorrect input\n";
    std::exit(EXIT_FAILURE);
  }
  if (str[0] != '0' || str[1] != 'x') {
    std::cerr << "ERROR:\nIncorrect input\n";
    std::exit(EXIT_FAILURE);
  }
  while (i <= std::min(static_cast<int>(len - 2), digits_)) {
    int curr_digit = Get16thNumber(*(str + len - i));
    if (curr_digit >= 0 && curr_digit < kSystemBase) {
      res += pow * curr_digit;
    } else {
      std::cerr << "ERROR:\nIncorrect input for number\n";
      std::exit(EXIT_FAILURE);
    }
    pow *= kSystemBase;
    ++i;
  }

  return res;
}

int64_t FixedPointClass::GetFracValue(int64_t value) const {
  int64_t res = 0;
  int i = 0;
  int pow = 1;
  while (i < b_) {
    res += std::abs((value % 2)) * pow;
    value /= 2;
    ++i;
    pow *= 2;
  }

  return res;
}

void FixedPointClass::GetFracVector(std::vector<int>& vec) const {
  for (int i = 0; i < b_; ++i) {
    int carry = 0;
    int j = vec.size() - 1;
    while (j >= 0) {
      int t = vec[j];
      vec[j] = (5 * vec[j] + carry) % 10;
      carry = (5 * t + carry) / 10;
      --j;
    }
    if (carry) {
      vec.insert(vec.begin(), carry);
    }
  }
}

void FillZeros(std::vector<int>& vec) {
  for (int i = 0; i < 3; ++i) {
    vec.push_back(0);
  }
}

void FixedPointClass::PaddWithZeros(std::vector<int>& vec) const {
  int size = vec.size();
  for (int i = 0; i < b_ - size; ++i) {
    vec.insert(vec.begin(), 0);
  }
}

void RewriteFracToVector(std::vector<int>& vec, int frac) {
  while (frac) {
    vec.insert(vec.begin(), frac % 10);
    frac /= 10;
  }
}

void FixedPointClass::Output(int64_t value) const {
  int64_t whole = 0;
  uint64_t pow = GetPowerTwo(a_ + b_ - 1);
  bool is_neg = (value >= pow);
  value = (!is_neg) ? (value) : value - 2 * pow;
  std::vector<int> frac_digits;
  RewriteFracToVector(frac_digits, GetFracValue(value));
  GetFracVector(frac_digits);
  PaddWithZeros(frac_digits);
  FillZeros(frac_digits);
  whole = (!is_neg) ? (std::abs(value) >> b_) : (~(std::abs(value) >> b_) + 1);
  RoundNumber(rounding_, whole, is_neg, frac_digits);
  whole = (!is_neg) ? (whole) : (~whole + 1);
  if (is_neg && (whole || frac_digits[0] || frac_digits[1] || frac_digits[2])) {
    std::cout << '-';
  }
  std::cout << whole << kDotSymbol << frac_digits[0] << frac_digits[1] << frac_digits[2] << '\n';
}

void FixedPointClass::SumNumbers(const char* first, const char* second) const {
  uint64_t f_number = ConvertToInt(first);
  uint64_t s_number = ConvertToInt(second);
  Output((f_number + s_number) % GetPowerTwo(a_ + b_));
}

void FixedPointClass::SubstractNumbers(const char* first, char* second) const {
  uint64_t f_number = ConvertToInt(first);
  uint64_t s_number = ConvertToInt(second);
  uint64_t res = f_number - s_number;
  if (s_number > f_number) {
    res = s_number - f_number;
    res = ChangeSignResult(res);
  }
  Output(res % GetPowerTwo(a_ + b_));
}

bool FixedPointClass::is_neg(const int value) const {
  if (value > GetPowerTwo(2 * a_ + b_ - 1)) {
    return true;
  }

  return false;
}

void FixedPointClass::RoundLeastDigits(uint64_t& value, const int64_t carry, const int64_t measure) const {
  switch (rounding_) {
    case 0:
      break;
    case 1: {
      if (carry * 2 < measure) {
        return;
      }
      if (carry * 2 > measure) {
        ++value;
        return;
      }
      value += (value % 2);
      break;
    }
    case 2:
      if (!is_neg(value) && carry) {
        ++value;
      }
      break;
    case 3:
      if (is_neg(value) && carry) {
        ++value;
      }
      break;
    default:
      std::cerr << "ERROR:\nIncorrect format for the rounding\n";
      std::exit(EXIT_FAILURE);
  }
}

void FixedPointClass::AdvancedOperation(const char* first, const char* second, const bool tag_mult) const {
  uint64_t f_number = ConvertToInt(first);
  uint64_t s_number = ConvertToInt(second);
  if (s_number == 0 && !tag_mult) {
    std::cerr << "ERROR:\nDivision by zero\n";
    std::exit(EXIT_FAILURE);
  }
  bool is_first_neg = false;
  bool is_second_neg = false;
  uint64_t changer_sign = GetPowerTwo(a_ + b_ - 1);
  if (f_number >= changer_sign) {
    is_first_neg = true;
    f_number = ChangeSignResult(f_number);
  }
  if (s_number >= changer_sign) {
    is_second_neg = true;
    s_number = ChangeSignResult(s_number);
  }
  uint64_t pow_b = GetPowerTwo(b_);
  uint64_t ans = 0;
  if (tag_mult) {
    ans = (f_number * s_number) / pow_b;
    int64_t carry = (f_number * s_number) % pow_b;
    RoundLeastDigits(ans, carry, pow_b);
  } else {
    ans = (pow_b * f_number) / s_number;
    int64_t carry = (pow_b * f_number) % s_number;
    RoundLeastDigits(ans, carry, s_number);
  }
  ans %= (2 * changer_sign);
  if (is_first_neg ^ is_second_neg) {
    ans = ChangeSignResult(ans);
  }
  Output(ans);
}

int64_t FixedPointClass::ChangeSignResult(int64_t res) const {
  res = ~res + 1;
  res += GetPowerTwo(a_ + b_);

  return res;
}

std::pair<int, int> FindPair(const char* str) {
  std::size_t i = 1;
  std::size_t len = std::strlen(str);
  int a = 0;
  int b = 0;
  int mult = 1;
  bool is_dot_set = false;
  bool is_correct = true;
  while (i <= len) {
    if (*(str + len - i) == kDotSymbol && !is_dot_set) {
      is_dot_set = true;
      mult = 1;
    } else if (!is_dot_set) {
      b += mult * (*(str + len - i) - '0');
      mult *= 10;
    } else if (*(str + len - i) != kDotSymbol && is_dot_set) {
      a += mult * (*(str + len - i) - '0');
      mult *= 10;
    } else {
      is_correct = false;
      break;
    }
    ++i;
  }
  if (!is_dot_set || !is_correct) {
    std::cerr << "ERROR:\nIncorrect input!\n";
    std::exit(EXIT_FAILURE);
  }

  return std::make_pair(a, b);
}

int64_t WriteNumber(int64_t n, int number_bites, int64_t& mult) {
  int64_t ans = 0;
  while (number_bites--) {
    ans += mult * (n % 2);
    n /= 2;
    mult *= 2;
  }

  return ans;
}

int64_t FloatingPointClass::BuildNumber(const int64_t sign, const int64_t exp, const int64_t mantissa) const {
  int64_t mult = 1;
  int64_t res = 0;
  res += WriteNumber(mantissa, kBitsMantissa_, mult);
  res += WriteNumber(exp, kBitsExp_, mult);
  res += WriteNumber(sign, 1, mult);

  return res;
}

void FloatingPointClass::MultiplyNumbers(const char* first, const char* second) const {
  int64_t f_number = ConvertToInt(first);
  int64_t s_number = ConvertToInt(second);
  int64_t f_mantissa = CutBits(f_number, kBitsMantissa_);
  int64_t s_mantissa = CutBits(s_number, kBitsMantissa_);
  f_number /= (2 * kMaxMantissa_);
  s_number /= (2 * kMaxMantissa_);
  int f_exp = CutBits(f_number, kBitsExp_);
  int s_exp = CutBits(s_number, kBitsExp_);
  f_number /= (2 * kMaxExp_);
  s_number /= (2 * kMaxExp_);
  int f_sign = CutBits(f_number, 1);
  int s_sign = CutBits(s_number, 1);
  int res_sign = f_sign ^ s_sign;
  int64_t res_exp = f_exp + s_exp + (kMaxExp_ - 1);
  if (f_exp + s_exp > 2 * kMaxExp_ - 1) {
    res_exp = f_exp + s_exp - (kMaxExp_ - 1);
  }
  if ((f_exp == 2 * kMaxExp_ - 1 && f_mantissa) || (s_exp == 2 * kMaxExp_ - 1 && s_mantissa)) { //one of numbers is nan
    Output(BuildNumber(0, 2 * kMaxExp_ - 1, 1));//nan
    return;
  } else if ((f_exp == 2 * kMaxExp_ - 1 && !s_exp && !s_mantissa) || (s_exp == 2 * kMaxExp_ - 1 && !f_exp && !f_mantissa)) { //one is infty, one iz zero
    Output(BuildNumber(0, 2 * kMaxExp_ - 1, 1));//nan
    return;
  } else if (f_exp == 2 * kMaxExp_ - 1 || s_exp == 2 * kMaxExp_ - 1) {
    Output(BuildNumber(res_sign, 2 * kMaxExp_ - 1, 0));//infty
    return;
  } else if ((!s_exp && !s_mantissa) || (!f_exp && !f_mantissa)) {
    Output(BuildNumber(res_sign,0, 0));//0
    return;
  }
  if (!s_exp) {
    while (s_mantissa < 2 * kMaxMantissa_) {
      s_mantissa *= 2;
      ++res_exp;
    }
    --res_exp;
    s_mantissa %= (2 * kMaxMantissa_);
  }
  if (!f_exp) {
    while (f_mantissa < 2 * kMaxMantissa_) {
      f_mantissa *= 2;
      --res_exp;
    }
    ++res_exp;
    f_mantissa %= (2 * kMaxMantissa_);
  }
  f_mantissa += kMaxMantissa_ * 2;
  s_mantissa += kMaxMantissa_ * 2;

  uint64_t res_mantissa = (f_mantissa * s_mantissa) / (2 * 2 * kMaxMantissa_);
  int carry_mantissa = (f_mantissa * s_mantissa) % (2 * 2 * kMaxMantissa_);
  RoundLeastDigits(res_mantissa, carry_mantissa, kMaxMantissa_, static_cast<bool>(res_sign));
  if (res_mantissa >= 2 * kMaxMantissa_) {
    ++res_exp;
  } else {
    while (res_mantissa < 2 * kMaxMantissa_) {
      res_mantissa *= 2;
      --res_exp;
    }
  }
  if (res_exp >= 2 * kMaxExp_) {
    Output(BuildNumber(res_sign, 2 * kMaxExp_ - 1, 0));//infty
    return;
  }
  Output(BuildNumber(res_sign, res_exp, res_mantissa));
}

void FloatingPointClass::RoundLeastDigits(uint64_t& value, const int64_t carry, const int64_t measure, const bool is_neg) const {
  switch (rounding_) {
    case 0:
      break;
    case 1: {
      if (carry * 2 < measure) {
        return;
      }
      if (carry * 2 > measure) {
        ++value;
        return;
      }
      value += (value % 2);
      break;
    }
    case 2:
      if (!is_neg && carry) {
        ++value;
      }
      break;
    case 3:
      if (is_neg && carry) {
        ++value;
      }
      break;
    default:
      std::cerr << "ERROR:\nIncorrect format for the rounding\n";
      std::exit(EXIT_FAILURE);
  }
}

void FloatingPointClass::DivideNumbers(const char* first, const char* second) const {
  uint64_t f_number = ConvertToInt(first);
  uint64_t s_number = ConvertToInt(second);
  uint64_t f_mantissa = CutBits(f_number, kBitsMantissa_);
  uint64_t s_mantissa = CutBits(s_number, kBitsMantissa_);
  f_number /= (2 * kMaxMantissa_);
  s_number /= (2 * kMaxMantissa_);
  int f_exp = CutBits(f_number, kBitsExp_);
  int s_exp = CutBits(s_number, kBitsExp_);
  f_number /= (2 * kMaxExp_);
  s_number /= (2 * kMaxExp_);
  int f_sign = CutBits(f_number, 1);
  int s_sign = CutBits(s_number, 1);
  int res_sign = f_sign ^ s_sign;
  int res_exp = (f_exp - s_exp) + (kMaxExp_ - 1);
  if ((f_exp == 2 * kMaxExp_ - 1 && f_mantissa) || (s_exp == 2 * kMaxExp_ - 1 && s_mantissa)) { //one of numbers is nan
    Output(BuildNumber(0, 2 * kMaxExp_ - 1, 1));//nan
    return;
  } else if (f_exp == 2 * kMaxExp_ - 1) { //first is infty
    if (s_exp == 2 * kMaxExp_ - 1) { //infty div infty
      Output(BuildNumber(0, 2 * kMaxExp_ - 1, 1)); //nan
      return;
    }
    Output(BuildNumber(res_sign, 2 * kMaxExp_ - 1, 0));//infty
    return;
  } else if (s_exp == 2 * kMaxExp_ - 1) { //second is infty
    Output(BuildNumber(res_sign, 0, 0));//0
    return;
  } else if (!f_exp && !f_mantissa && !s_exp && !s_mantissa) { //0 div 0
    Output(BuildNumber(0, 2 * kMaxExp_ - 1, 1)); //nan
    return;
  } else if (!f_exp && !f_mantissa) {
    Output(BuildNumber(res_sign, 0, 0));//0
    return;
  } else if (!s_exp && !s_mantissa) {
    Output(BuildNumber(res_sign, 2 * kMaxExp_ - 1, 0));//infty
    return;
  }
  if (!s_exp) {
    while (s_mantissa < 2 * kMaxMantissa_) {
      s_mantissa *= 2;
      ++res_exp;
    }
    --res_exp;
    s_mantissa %= (2 * kMaxMantissa_);
  }
  if (!f_exp) {
    while (f_mantissa < 2 * kMaxMantissa_) {
      f_mantissa *= 2;
      --res_exp;
    }
    ++res_exp;
    f_mantissa %= (2 * kMaxMantissa_);
  }
  f_mantissa += kMaxMantissa_ * 2;
  s_mantissa += kMaxMantissa_ * 2;
  uint64_t res_mantissa = (f_mantissa * kMaxMantissa_ * 2) / s_mantissa;
  int32_t carry_mantissa = (f_mantissa * kMaxMantissa_ * 2) % s_mantissa;
  RoundLeastDigits(res_mantissa, carry_mantissa, s_mantissa, res_sign);
  if (res_mantissa >= 2 * kMaxMantissa_) {
    if (res_mantissa >= 2 * 2 * kMaxMantissa_) {
      res_mantissa -= (2 * kMaxMantissa_);
      ++res_exp;
    }
    res_mantissa %= (2 * kMaxMantissa_);
  } else {
    while (res_mantissa < 2 * kMaxMantissa_) {
      res_mantissa *= 2;
      --res_exp;
    }
  }
  if (res_exp >= 2 * kMaxExp_) {
    Output(BuildNumber(res_sign, 2 * kMaxExp_ - 1, 0));//infty
    return;
  }
  Output(BuildNumber(res_sign, res_exp, res_mantissa));
}

void FloatingPointClass::SumNumbers(const char* first, const char* second, const bool tag_substract) const {
  int64_t f_number = ConvertToInt(first);
  int64_t s_number = ConvertToInt(second);
  int64_t f_mantissa = CutBits(f_number, kBitsMantissa_);
  int64_t s_mantissa = CutBits(s_number, kBitsMantissa_);
  f_mantissa += kMaxMantissa_ * 2;
  s_mantissa += kMaxMantissa_ * 2;
  f_number /= (2 * kMaxMantissa_);
  s_number /= (2 * kMaxMantissa_);
  int f_exp = CutBits(f_number, kBitsExp_);
  int s_exp = CutBits(s_number, kBitsExp_);
  f_number /= (2 * kMaxExp_);
  s_number /= (2 * kMaxExp_);
  int f_sign = CutBits(f_number, 1);
  int s_sign = std::abs(CutBits(s_number, 1) - static_cast<int>(tag_substract));
  if ((f_exp == 2 * kMaxExp_ - 1 && f_mantissa) || (s_exp == 2 * kMaxExp_ - 1 && s_mantissa)) {//one of numbers is nan
    Output(BuildNumber(0, 2 * kMaxExp_ - 1, 1));//nan
    return;
  } else if (f_exp == 2 * kMaxExp_ - 1 && s_exp == 2 * kMaxExp_ - 1 && f_sign != s_sign) {//infty-infty
    Output(BuildNumber(0, 2 * kMaxExp_ - 1, 1));//nan
    return;
  } else if (f_exp == 2 * kMaxExp_ - 1 && s_exp == 2 * kMaxExp_ - 1) {
    Output(BuildNumber(f_sign, 2 * kMaxExp_ - 1, 0));//infty
    return;
  } else if (f_exp == 2 * kMaxExp_ - 1) {
    Output(BuildNumber(f_sign, 2 * kMaxExp_ - 1, 0));//infty
    return;
  } else if (s_exp == 2 * kMaxExp_ - 1) {
    Output(BuildNumber(s_sign, 2 * kMaxExp_ - 1, 0));//infty
    return;
  } else if (!f_exp && !f_mantissa && !s_exp && !s_mantissa && f_sign != s_sign) {
    Output(BuildNumber(0, 0, 0));//+0-0=nan
    return;
  } else if (!f_exp && !f_mantissa && !s_exp && !s_mantissa) {
    Output(BuildNumber(f_sign, 2 * kMaxExp_ - 1, 1));//+0+0=+0 -0-0=-0
    return;
  }
  int res_sign = 0;
  int64_t res_exp = std::max(f_exp, s_exp);
  if (!s_exp) {
    while (s_mantissa < 2 * kMaxMantissa_) {
      s_mantissa *= 2;
      ++res_exp;
    }
    --res_exp;
    s_mantissa %= (2 * kMaxMantissa_);
  }
  if (!f_exp) {
    while (f_mantissa < 2 * kMaxMantissa_) {
      f_mantissa *= 2;
      --res_exp;
    }
    ++res_exp;
    f_mantissa %= (2 * kMaxMantissa_);
  }
  int64_t pow = GetPowerTwo(std::min(std::abs(f_exp - s_exp), 25));
  uint64_t res_mantissa = 0;
  if (f_exp > s_exp) {
    f_mantissa *= pow;
    res_sign = f_sign;
    if (f_sign != s_sign) {
      res_mantissa = f_mantissa - s_mantissa;
    } else {
      res_mantissa = f_mantissa + s_mantissa;
    }
  } else if (f_exp < s_exp) {
    s_mantissa *= pow;
    res_sign = s_sign;
    if (f_sign != s_sign) {
      res_mantissa = s_mantissa - f_mantissa;
    } else {
      res_mantissa = f_mantissa + s_mantissa;
    }
  } else { //==
    if (f_sign == 0 && s_sign == 1 && f_mantissa >= s_mantissa) {
      res_mantissa = f_mantissa - s_mantissa;
      res_sign = 0;
      res_exp = 0;
    } else if (f_sign == 0 && s_sign == 1) {
      res_mantissa = s_mantissa - f_mantissa;
      res_sign = 1;
      res_exp = 0;
    } else if (f_sign == s_sign) {
      res_mantissa = f_mantissa + s_mantissa;
      res_sign = f_sign;
    } else if (f_sign == 1 && s_sign == 0 && f_mantissa <= s_mantissa) {
      res_mantissa = s_mantissa - f_mantissa;
      res_sign = 0;
      res_exp = 0;
    } else if (f_sign == 1 && s_sign == 0) {
      res_mantissa = f_mantissa - s_mantissa;
      res_sign = 1;
      res_exp = 0;
    }
  }
  int carry_mantissa = 0;
  int mult = 1;
  while (res_mantissa > 2 * 2 * kMaxMantissa_) {
    carry_mantissa += mult * (res_mantissa % 2);
    mult *= 2;
    res_mantissa /= 2;
  }
  if (carry_mantissa) {
    ++res_exp;
  }
  RoundLeastDigits(res_mantissa, carry_mantissa, mult, res_sign);
  if (res_exp >= 2 * kMaxExp_) {
    Output(BuildNumber(res_sign, 2 * kMaxExp_ - 1, 0));//infty
    return;
  }
  Output(BuildNumber(res_sign, res_exp, res_mantissa));
}

int main(int argc, char** argv) {
  if (argc != kShortArguments && argc != kLongArguments) {
    std::cerr << "ERROR:\nIncorrect number of parameters\n";
    return EXIT_FAILURE;
  }
  if (argv[1][0] != 'h' && argv[1][0] != 'f') {
    std::pair<int, int> curr_pair = FindPair(argv[1]);
    if (std::strlen(argv[2]) != 1) {
      std::cerr << "Incorrect input for the second number\n";
      return EXIT_FAILURE;
    }
    int rounding = argv[2][0] - '0';
    if (rounding > kMaxRounding) {
      std::cerr << "ERROR:\nIncorrect input for the rounding\n";
      return EXIT_FAILURE;
    }
    FixedPointClass my_parameters(curr_pair.first, curr_pair.second, rounding);
    if (curr_pair.first + curr_pair.second > kMaxNumberDigits) {
      std::cerr << "ERROR:\nToo many digits for the fixed point numbers\n";
      return EXIT_FAILURE;
    }
    if (argc == kShortArguments) {
      int64_t ans = my_parameters.ConvertToInt(argv[3]);
      my_parameters.Output(ans);
      return EXIT_SUCCESS;
    }
    if (std::strlen(argv[4]) != 1) {
      std::cerr << "ERROR:\nThe operator has not founded\n";
      return EXIT_FAILURE;
    }
    switch (argv[4][0]) {
      case '+': {
        my_parameters.SumNumbers(argv[3], argv[5]);
        break;
      } case '-': {
        my_parameters.SubstractNumbers(argv[3], argv[5]);
        break;
      } case kMultSign: {
        my_parameters.AdvancedOperation(argv[3], argv[5], true);
        break;
      } case '/': {
        my_parameters.AdvancedOperation(argv[3], argv[5], false);
        break;
      } default: {
        std::cerr << "ERROR:\nThe operator has not founded\n";
        return EXIT_FAILURE;
      }
    }
    return EXIT_SUCCESS;
  }
  if (std::strlen(argv[1]) != 1) {
    std::cerr << "ERROR\nIncorrect format for rounding\n";
    return EXIT_FAILURE;
  }
  int rounding = argv[2][0] - '0';
  if (rounding < 0 && rounding > kMaxRounding) {
    std::cerr << "ERROR\nIncorrect format for rounding\n";
    return EXIT_FAILURE;
  }
  if (argc == kShortArguments) {
    int rounding = argv[1][0] - '0';
    if (argv[1][0] == 'f') {
      FloatingPointClass my_parameters(rounding, TypeA());
      int64_t value = my_parameters.ConvertToInt(argv[3]);
      my_parameters.Output(value);
    } else {
      FloatingPointClass my_parameters(rounding, TypeB());
      int64_t value = my_parameters.ConvertToInt(argv[3]);
      my_parameters.Output(value);
    }
    return EXIT_SUCCESS;
  }
  if (std::strlen(argv[4]) != 1) {
    std::cerr << "ERROR:\nIncorrect input for operation\n";
    return EXIT_FAILURE;
  }
  FloatingPointClass my_parameters(rounding, TypeA());
  if (argv[1][0] == 'h') {
    my_parameters.SetParameters(rounding);
  }
  switch (argv[4][0]) {
    case '+':
      my_parameters.SumNumbers(argv[3], argv[5], false);
      break;
    case '-':
      my_parameters.SumNumbers(argv[3], argv[5], true);
      break;
    case kMultSign:
      my_parameters.MultiplyNumbers(argv[3], argv[5]);
      break;
    case '/':
      my_parameters.DivideNumbers(argv[3], argv[5]);
      break;
    default:
      std::cerr << "ERROR:\nIncorrect input for operation\n";
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
