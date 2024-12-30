Представление чисел

**Фиксированая точка:**
>- Был создан класс FixedPointClass, который содержит тип округления, количество битов, выделенных на целую и дробную части числа, и соответствующие функции.
```
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
```
В классе есть конструктор, в котором происходит инициализация элементов.
Остальные функции являются константными, так как не меняют параметров и будут раскрыты в отчёте далее.
>- После этапа парсинга, в котором инициализируются данные переменные и отлавливаются ошибки, начинается основная работа программы. Нахождение чисел A и B в записи A.B происходит в функции ```std::pair<int, int> FindPair(const char* str)```
>- Сначала заметим, что результат операции или представления числа можно представить, как число, равное числу, которое было взято с исходного с 1 по A + B биты, поделённое на 2^B. Соответственно, функция ```int64_t ConvertToInt(const char* str)``` из исходного класса находит такое представление числа, рассматривая биты с 1 по A + B включительно (назовем данное число N).
>- Далее, для вывода числа в стандартный поток вывода (функция Output), проверяется, будет ли оно являться отрицательным. Для этого проверяется, установлен ли ненулевой бит на позиции A + B у N. Если установлен, то число N декодируется в положительное.
```
int64_t whole = 0;
uint64_t pow = GetPowerTwo(a_ + b_ - 1);
bool is_neg = (value >= pow);
value = (!is_neg) ? (value) : value - 2 * pow;
```
>- Далее выделяется целая часть числа через последние A битов числа, а дробная часть вычисляется с помощью std::vector<int>.
```
std::vector<int> frac_digits;
RewriteFracToVector(frac_digits, GetFracValue(value));
GetFracVector(frac_digits);
PaddWithZeros(frac_digits);
FillZeros(frac_digits);
whole = (!is_neg) ? (std::abs(value) >> b_) : (~(std::abs(value) >> b_) + 1);
```
Функция ```int64_t GetFracValue(int64_t value)``` из исходного класса вычисляет значение value c 1 по B бит включительно.
Функция ```void RewriteFracToVector(std::vector<int>& vec, int frac)``` добавляет в конец вектора цифры числа, представляющего дробную часть.
В функции ```void GetFracVector(std::vector<int>& vec)``` происходит домножение числа на 5^B.
Функция ```void PaddWithZeros(std::vector<int>& vec)``` добавляет в начало вектора нули, если его длина после всех домножений стала не соответствовать количеству выделенного количества бит(B).
Функция ```void FillZeros(std::vector<int>& vec)``` добавляет в конец вектора три (незначащих) нуля, чтобы избежать ошибок, связанных с их количеством в дальнейшем.
>- В конце происходит округление (функция ```void RoundNumber(const int rounding, int64_t& whole, const bool is_neg, std::vector<int>& vec)``` вне класса).
>- Если тип округления к нулю, то ничего делать не надо и происходит выход из функции (конструкции switch - case).
>- Если тип округления к +infty, то проверяется целая часть. Если она отрицательна, то выход из функции. Иначе, вызывается функция вне класса ```void RoundUpAnswer(int64_t& whole, std::vector<int>& vec, const bool is_neg)``` если после 3 знаков в дробной части есть хотя бы одно ненулевое число (это проверяется в функции ```bool IsPositiveVector(const std::vector<int>& vec)```вне класса циклом по вектору), то она увеличивается на один в 3 разряде. В зависимости от этого последовательно могут увеличиваться более старшие разряды, если в них они становяться больше 9. Если же они вне "обнулились", то на один увеличивается целая часть whole. Например: 0.999->0.99"10"->0.9"10"0->0."10"00->1.000.
*Замечание: Чтобы не возникало коллизий со знаком нуля, то в функцию дополнительно передается параметр, явно указывающий на это*.
>- Округление к -infty делается аналогично к +infty, только увеличение разрядов происходит только если целая часть отрицательна.
```
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
```
Примеры:
1) Целая часть: `23`; Дробная часть: `234`000001.
Если округление к 0, то ответ 23.234;
Если округление к +infty, то ответ 23.235;
Если округление к -infty, то ответ 23.234.
2) Целая часть: `-1`; Дробная часть: `534`512.
Если округление к 0, то ответ -1.534;
Если округление к +infty, то ответ -1.534;
Если округление к -infty, то ответ -1.535.
>- Если тип округления к ближайшему четному числу, то вызывается функция ```void RoundToNearestEven(std::vector<int>& vec, const int is_neg, int64_t& whole)``` вне исходного класса. Сначала проверяется, какое число является ближайшим, округление происходит к нему. Если же такого числа найти не удается (то есть 4 знак у дроби равен 5, следующие равны нулю), то проверяется четна ли целая часть. Если она четна, то округлить дробную часть следует вниз, и происходит выход из функции. Иначе происходит обычное округление с учетом знака целой части.
```
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
```
1) Пример: Целая часть: `-1`; Дробная часть: `534`512.
Ответ: -1.535.
2) Целая часть: `21`; Дробная часть: `599`5000.
Ответ: 21.600.
>- В конце выводится знак минус, если целая часть не ноль, и число отрицательно, число по целой части, знаке точки и первым трем элементам вектора. 
**Операции:**
*Замечание(\*): в каждой операции в конце берется остаток от 2^(a+b), так как по условию результат операции должен быть в том же формате, что и исходные числа (тем самым отбрасываются лишние старшие разряды).*
>- Сложение: Реализована функция SumNumbers в исходном классе. Складываются два результата ConvertInt, берется остаток(\*), выводится результат в Output. Так происходит, так как результат должен быть равен a/2^B + b/2^B = (a + b) / 2^B, то есть можно найти a + b и привести его к виду с фиксированной запятой, что и реализуется.
```
void FixedPointClass::SumNumbers(const char* first, const char* second) const {
  uint64_t f_number = ConvertToInt(first);
  uint64_t s_number = ConvertToInt(second);
  Output((f_number + s_number) % GetPowerTwo(a_ + b_));
}
```
>- Вычитание: Реализована функция SubstractNumbers в исходном классе. Вычитаются два результата ConvertInt, далее проверяется его неотрицательность. Если он неотрицателен, то берется остаток(\*), и выводится результат в Output. Иначе, нужно взять противоположный знак (вычев из второго числа первое) и поменять итоговое число так, чтобы его знак в ходе вывода поменялся на противоположный, при этом его модуль остался бы тем же. Для этого реализована функция ```int64_t ChangeSignResult(int64_t res)``` в исходном классе. В итоге берем остаток(\*), и выводим результат в Output.
>- Умножение и деление: Реализована функция ```void AdvancedOperation(const char* first, const char* second, const bool tag_mult)``` в исходном классе с тегом в зависимости от операции.
Если стоит тег деления и второе число равно нулю, то выводим сообщение об ошибке.
Далее проверяем, являются ли исходные числа отрицательными. Это можно сделать по первому значащему биту формата A.B. Если они являются отрицательными, то меняем их так по функции ```int ChangeSignResult(int64_t res)```. Далее в зависимости от тега смотрим на остаток числа при делении и на само число. В случае умножения нужно умножить два числа и поделить на 2^B, так как мы хотим получить число (a/2\^B)\*(b/2^B) = (a\*b/2\^B)/2\^B. В случае деления нужно домножить первое число на 2^B и поделить на второе число, так как мы хотим получить число (a/2\^B)/(b/2\^B) = (a*2\^B/b)/2^B. В итоге округляем полученное число в зависимости от остатка с помощью функции void ```RoundLeastDigits(uint64_t& value, const int64_t carry, const int64_t measure)```.
>- В ней, если округление к нулю, то выход;
Если к +-infty, то проверяется, отрицательно ли число и положителен ли остаток (аналогично округлению в RoundNumber), в зависимости от этого результат может увеличиться на единицу.
```
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
```
Если к ближайшему четному числу, то если остаток меньше половины меры деления, то выход;
Если больше, то увеличиваем результат операции.
Если равен, то проверяем четность "целой" части, если нечетна, то увеличиваем результат на 1, иначе выход.
```
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
```
>- Модулярная арифметика(\*), далее если изначальные числа имели разный знак, то меняем знак с помощью ```ChangeSignResult```. Если равный, то не меняем.
Выводим получившееся число с помощью ```Output```.

**Плавающая точка:**
Был создан класс FloatingPointClass с константными методами и параметрами, показывающими на формат и тип округления. Также были созданы вспомогательные структуры TypeA и TypeB для удобного переключения между форматами.
```
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
```
В начале происходит инициализация элементов класса посредством делигирующих конструкторов, далее вызываются соответствующие методы.
>- Для начала, как в случае half precision floating point IEEE-754 (далее -- hfpr), так и в случае single precision floating point IEEE-754 (далее -- spfr) будем с помощью функции ``int64_t FloatingPointClass::ConvertToInt(const char* str) const`` (аналогу ``ConvertToInt`` из класса с фиксированной точкой) высчитывать число из первых 32 или 16 бит (в зависимости от формата). Далее разберем функцию Output из исходного класса.
>- Создадим функцию ``int CutBits(int64_t value, int number_bites)``, которая будет отсекать number_bits от заданного value. Отсечём 23 бита на мантиссу, далее поделим число на 2^23, отсечём 8 бит на экспоненту, поделим число на 2^8, отсечем один бит на знак.
>- Далее проверим крайние случаи (+-infty, nan). Это происходит, если у числа максимальная экспонента. Если мантисса нулевая, то смотрим на знак и выводим +-inf, иначе получаем неопределённость и выводим "nan". Далее, если знак у числа отрицательный (бит на него ненулевой), то выводим знак '-'.
```
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
```
Далее идет проверка числа на знак и на ноль. Если происходит последнее, то выход из функции.
>- Далее, если получается денормализованное число (число с наименьшим порядком, то есть число, меньшее чем наименьшее представимое нормализованное число), то тогда нормализуем его с помощью функции ``void FloatingPointClass::NormalizeNumber(uint64_t& mantissa, int& exp) const`` исходного класса, в которой мы увеличиваем мантиссу(и соответственно уменьшаем экспоненту числа) до тех пор, пока она не выйдет "за пределы" границы мантиссы и не перейдет в вид нормализованного числа (после взятия остатка в конце).
```
void FloatingPointClass::NormalizeNumber(uint64_t& mantissa, int& exp) const {
  while (mantissa < kMaxMantissa_) {
    mantissa *= 2;
    --exp;
  }
  mantissa *= 2;
  mantissa %= (2 * kMaxMantissa_);
}
```
>- Учитываем сдвиг для экспоненты и уменьшаем ее на 127 (для spfr, иначе на 15). Теперь можно выводить мантиссу и экспоненту. Для экспоненты просто выведем её в конце, если она положительна, то выводим '+'. Для мантиссы же реализована функция void ``void FloatingPointClass::PrintMantissa(const int mantissa) const`` исходного класса, которая берет по 4 бита в начале числа, переводит в "16-ричный" вид и выводит bits битов (для spfr -- 6, для hpfr -- 3).
**Операции:**
*Замечание: для всех операций нужно сначала достать значения знака, экспоненты и мантиссы (делалось ранее). Также для мантиссы добавим один единичный бит в начало (чтобы получить число типа 1,(...) и далее работать с ним). После такой "инициализации" идет проверка граничных случаев (nan, infty, 0). Если начальные число денормализованные, то идет их нормализация.*
```
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
int s_sign = CutBits(s_number, 1);
```
>- Для функции ``void FloatingPointClass::MultiplyNumbers(const char* first, const char* second) const`` приведем значения знак результат res_sign, как xor знаков множителей. Далее представим значения экспоненты и мантиссы результата.
>- Экспоненту результата представим, как сумму экспонент искомых чисел. При этом, чтобы при дальнейшем сдвиге сумма "сохранилась" нужно к необходимой функции добавить значение этого сдвига. Если она заходит за пределы границы экспоненты (после сдвига), то значит нужно отбросить максимальное значение от экспоненты.
Значение мантиссы же представим, как умножение мантисс множителей с учетом возможного переполнения. Также возьмем остаток, взятый при делении на (2 * 2 * MaxMantissa). В конце округлим числа с помощью функции ``void FloatingPointClass::RoundLeastDigits(uint64_t& value, const int64_t carry, const int64_t measure, const bool is_neg) const`` (аналогу функции округления в классе с фиксированной точкой, только негативность числа определяется не по функции, а по значению). Потом проверим число на денормилизованность, если денормилизованно, то нормилизуем его, иначе увеличиваюм общую степень числа. Также в конце происходит проверка числа на переполнение (когда общая экспонента имеет больше 10 (или 5) знаков в зависимости от формата в бинарной записи). Если оно произошло, то выводим бесконечность с учетом знака, тк формат больше не позволяет хранить большие числа.
```
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
```
```
  uint64_t res_mantissa = (f_mantissa * s_mantissa) / (2 * 2 * kMaxMantissa_);
  int carry_mantissa = (f_mantissa * s_mantissa) % (2 * 2 * kMaxMantissa_);
  RoundLeastDigits(res_mantissa, carry_mantissa, kMaxMantissa_, static_cast<bool>(res_sign));
```
>- В конце концов, вызывем функцию ``int64_t FloatingPointClass::BuildNumber(const int64_t sign, const int64_t exp, const int64_t mantissa) const``, которая строит число по его параметрам (знак, экспонента и мантисса).
```
int64_t FloatingPointClass::BuildNumber(const int64_t sign, const int64_t exp, const int64_t mantissa) const {
  int64_t mult = 1;
  int64_t res = 0;
  res += WriteNumber(mantissa, kBitsMantissa_, mult);
  res += WriteNumber(exp, kBitsExp_, mult);
  res += WriteNumber(sign, 1, mult);

  return res;
}
```
В функции ``int64_t WriteNumber(int64_t n, int number_bites, int64_t& mult)`` записывается число n в number_bits и выводится результат с учетом множителя mult.
>- В итоге, выводим получившееся число через функцию Output (рассмотрена ранее).
>- Для деления реализована функция ``void FloatingPointClass::DivideNumbers(const char* first, const char* second) const``. Почти все действия аналогичны функции умножения, только вместо суммы экспонент берется их разность, а вместо умножения мантисс -- их деление (домноженная на 2^(kBitsMantissa_)). Далее вызываем функцию RoundLeastDigits, указывая за measure -- s_number. В конце, в случае нулевой экспоненты следует нормализовать число, иначе если общая мантисса стала иметь больше, чем 24(11) знака, то значит вычитаем из неё 2^23 (или 2^10), чтобы отбросить первые биты и привести мантиссу в "нормальную" форму и увеличиваем общую экспоненту, далее отбрасываем лишние старшие биты и отбираем только последние 23(10). В конце, если происходит переполнение общей экспоненты, то возвращаем бесконечность с учетом знака. Далее строим число по функции BuildNumber, и выводим с помощью соответствующего Output число.
```
uint64_t res_mantissa = (f_mantissa * kMaxMantissa_ * 2) / s_mantissa;
int32_t carry_mantissa = (f_mantissa * kMaxMantissa_ * 2) % s_mantissa;
RoundLeastDigits(res_mantissa, carry_mantissa, s_mantissa, res_sign);
```
>- Для сложения и вычитания реализована функция ``void FloatingPointClass::SumNumbers(const char* first, const char* second, const bool tag_substract) const``. Если в ней tag_substract = true, то значит знак у второго числа следует поменять на противоположный (знак минус), иначе ничего дополнительного делать не следует. Рассмотрим экспоненты двух исходных чисел и выберем среди них максимальную. Обозначим 2 в степени модуля разности этих экспонент (или же, если она слишком велика, то возьмем такую степень, чтобы не было ни переполнения типа, ни потери точности общей мантиссы). Если модуль не нулевой, то тогда домножим число, у которого больше экспонента на эту разность, и выберем соответствующий знак у итогового числа. Иначе выберем числа так, чтобы итоговая мантисса была положительна. Далее возьмем 23(10) бита, начиная со второго (дополнительный незначающий бит = 1), остальные биты же будут остатком. Если остаток ненулевой, то увеличиваем общую экспоненту итогового числа. Далее происходит округление с помощью функции RoundLeastDigits исходного класса. Проверим общую экспоненту числа на переполнение. В конце строим число по его параметрам с помощью BuildNumber и выводим результат с помощью Output.
```
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
```
