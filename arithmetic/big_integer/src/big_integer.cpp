#include "../include/big_integer.h"

std::string string_add(std::string const & first, std::string const & second) 
{
    std::string result;
    int carry = 0, sum = 0; 
    int i = first.size() - 1, j = second.size() - 1;
    while (i >= 0 || j >= 0 || carry) 
    {
        sum = carry + (i >= 0 ? first[i--] - '0' : 0) + (j >= 0 ? second[j--] - '0' : 0);
        carry = sum / 10;
        result.push_back(sum % 10 + '0');
    }
    std::reverse(result.begin(), result.end());

    return result;
}

std::string string_mult(const std::string & first, const std::string & second) 
{
    if (first == "0" || second == "0") return "0";

    std::vector <int> res_arr(first.size() + second.size(), 0);

    std::string first_rev = std::string(first.rbegin(), first.rend());
    std::string second_rev = std::string(second.rbegin(), second.rend());

    for (auto i = 0; i < first_rev.size(); ++i) 
    {
        for (size_t j = 0; j < second_rev.size(); ++j) 
        {
            int mul = (first_rev[i] - '0') * (second_rev[j] - '0');
            res_arr[i + j] += mul;
            res_arr[i + j + 1] += res_arr[i + j] / 10;  
            res_arr[i + j] %= 10; 
        }
    }

    while (res_arr.size() > 1 && res_arr.back() == 0) res_arr.pop_back();

    std::string res_str;
    for (auto it = res_arr.rbegin(); it != res_arr.rend(); ++it) res_str.push_back(*it + '0');

    return res_str;
}

std::string string_pow(uint num, const std::string &base)
{
    if (num == 0 && base == "0") throw std::runtime_error("0^0 is undefined\n");

    if (num == 0) return "1";

    if (base == "0") return "0";

    std::string result = "1";
    std::string power = base;
    unsigned int curr_num = num;

    while (curr_num > 0) 
    {
        if (curr_num % 2 == 1) result = string_mult(result, power);
        power = string_mult(power, power);
        curr_num /= 2;
    }

    return result;
}

std::string big_integer::big_integer_to_string(big_integer const & value) const
{
    std::string res = "0";

    if (value.is_zero()) return res;

    if (!value._other_digits) 
    {
        if (value.sign() == -1) return "-" + std::to_string(value._oldest_digit);
        else return std::to_string(value._oldest_digit);
    }

    int size = value.get_size();
    if (size == 1) 
    {
        if (value.sign() == -1) return "-" + std::to_string(value._oldest_digit);
        else return std::to_string(value._oldest_digit);
    }

    unsigned int i;
    for (i = 1; i < size; ++i) 
    {
        res = string_add(res, string_mult(std::to_string(value._other_digits[i]), string_pow(i - 1, DF_base)));
    }

    if (value.sign() == -1) 
    {
        int old_d = value._oldest_digit;
        old_d = old_d ^ (1 << ((sizeof(int) << 3) - 1));
        std::string mystr = std::to_string(old_d);
        res = string_add(res, string_mult(mystr, string_pow(i - 1, DF_base)));
        res = "-" + res;
    } 
    else res = string_add(res, string_mult(std::to_string(value._oldest_digit), string_pow(i - 1, DF_base)));

    return res;
}

int big_integer::big_int_cmp(big_integer const & first, big_integer const & second) const
{
    bool first_negative = (first.sign() == -1);
    bool second_negative = (second.sign() == -1);

    if (first_negative && !second_negative) return -1;
    if (!first_negative && second_negative) return -1;


    big_integer first_copy(first);
    big_integer second_copy(second);

    if (first.get_size() != second.get_size())
    {
        bool res = (first.get_size() > second.get_size());
        if (!first_negative && res) return 1;
        else return -1;
    }
    
    if (first_negative)
    {
        first_copy.change_sign();
        second_copy.change_sign();
    }

    auto size = first.get_size();

    int cmp_res = big_integer_to_string(first_copy).compare(big_integer_to_string(second_copy));
    
    if (first_negative) cmp_res = -cmp_res;
    return cmp_res;
}

int char_to_int(char c) 
{
    if (c >= '0' and c <= '9') return c - '0';
    else if (c >= 'A' and c <= 'Z') return c - 'A' + 10;
    else if (c >= 'a' and c <= 'z') return c - 'a' + 10;
    throw std::invalid_argument("Invalid character in string\n");
}

std::string string_to_decimal(const std::string& number, int base) 
{
    if (base < 2 or base > 36) throw std::invalid_argument("Base must be in the range [2...36]");

    std::string result = "0";
    int power = 0;

    for (int i = number.size() - 1; i >= 0; --i) 
    {
        int digitValue = char_to_int(number[i]);
        if (digitValue >= base) throw std::invalid_argument("Invalid character in the number for the given base");

        std::string tmp = string_mult(string_pow(power, std::to_string(base)), std::to_string(digitValue));
        result = string_add(result, tmp);
        ++power;
    }

    return result;
}

big_integer &big_integer::trivial_multiplication::multiply(
    big_integer &first_multiplier,
    big_integer const &second_multiplier) const
{
    big_integer second_copy(second_multiplier);

    bool negative;

    if (first_multiplier.sign() == -1 && second_copy.sign() == 1 || first_multiplier.sign() == 1 && second_copy.sign() == -1)
    {
        negative = true;
        first_multiplier.sign() == -1 ? first_multiplier.change_sign() : second_copy.change_sign();
    }
    else if (first_multiplier.sign() == -1 && second_copy.sign() == -1)
    {
        negative = false;
        first_multiplier.change_sign();
        second_copy.change_sign();
    }
    else negative = false;

    int first_size = first_multiplier.get_size();
    int second_size = second_copy.get_size();
    
    constexpr int shift = sizeof(unsigned int) << 2;
    constexpr unsigned int mask = (1U << shift) - 1;

    big_integer result("0");

    for (auto i = 0; i < 2 * first_size; ++i)
    {
        unsigned int remainder = 0;
        unsigned int first_number_half;

        if (i % 2 == 0)
        {
            auto number = first_multiplier.get_digit(i / 2);
            first_number_half = number & mask;
        }
        else
        {
            auto number = first_multiplier.get_digit(i / 2);
            first_number_half = (number >> shift) & mask;
        }

        for (auto j = 0; j < 2 * second_size; ++j)
        {
            std::vector<int> digits_array;
            unsigned int second_number_half;

            if (j % 2 == 0)
            {
                auto number = second_copy.get_digit(j / 2);
                second_number_half = number & mask;
            }
            else
            {
                auto number = second_copy.get_digit(j / 2);
                second_number_half = (number >> shift) & mask;
            }

            unsigned int operation_result = (first_number_half * second_number_half + remainder) & mask;
            remainder = (first_number_half * second_number_half + remainder) >> shift;

            digits_array.push_back(operation_result);

            big_integer multiply_result(digits_array);

            multiply_result <<= (shift * (i + j));;
            result += multiply_result;
        }
        if (remainder)
        {
            std::vector<int> remainder_vector(1);
            remainder_vector[0] = remainder;
            big_integer add_remainder(remainder_vector);
            add_remainder <<= (shift * (2 * second_size + i));
            result += add_remainder;
        }
    }
    
    first_multiplier.copy_from(result);
    ~result;

    if (negative) first_multiplier.change_sign();
    return first_multiplier;
}

big_integer &big_integer::Karatsuba_multiplication::multiply(
    big_integer &first_multiplier,
    big_integer const &second_multiplier) const
{
    throw not_implemented("big_integer &big_integer::Karatsuba_multiplication::multiply(big_integer &, big_integer const &)", "your code should be here...");
}

big_integer &big_integer::Schonhage_Strassen_multiplication::multiply(
    big_integer &first_multiplier,
    big_integer const &second_multiplier) const
{
    throw not_implemented("big_integer &big_integer::Schonhage_Strassen_multiplication::multiply(big_integer &, big_integer const &)", "your code should be here...");
}

big_integer &big_integer::trivial_division::divide(
    big_integer &dividend,
    big_integer const &divisor,
    big_integer::multiplication_rule multiplication_rule) const
{
    throw not_implemented("big_integer &big_integer::trivial_division::divide(big_integer &, big_integer const &, big_integer::multiplication_rule)", "your code should be here...");
}

big_integer &big_integer::trivial_division::modulo(
    big_integer &dividend,
    big_integer const &divisor,
    big_integer::multiplication_rule multiplication_rule) const
{
    throw not_implemented("big_integer &big_integer::trivial_division::modulo(big_integer &, big_integer const &, big_integer::multiplication_rule)", "your code should be here...");
}

big_integer &big_integer::Newton_division::divide(
    big_integer &dividend,
    big_integer const &divisor,
    big_integer::multiplication_rule multiplication_rule) const
{
    throw not_implemented("big_integer &big_integer::Newton_division::divide(big_integer &, big_integer const &, big_integer::multiplication_rule)", "your code should be here...");
}

big_integer &big_integer::Newton_division::modulo(
    big_integer &dividend,
    big_integer const &divisor,
    big_integer::multiplication_rule multiplication_rule) const
{
    throw not_implemented("big_integer &big_integer::Newton_division::modulo(big_integer &, big_integer const &, big_integer::multiplication_rule)", "your code should be here...");
}

big_integer &big_integer::Burnikel_Ziegler_division::divide(
    big_integer &dividend,
    big_integer const &divisor,
    big_integer::multiplication_rule multiplication_rule) const
{
    throw not_implemented("big_integer &big_integer::Burnikel_Ziegler_division::divide(big_integer &, big_integer const &, big_integer::multiplication_rule)", "your code should be here...");
}

big_integer &big_integer::Burnikel_Ziegler_division::modulo(
    big_integer &dividend,
    big_integer const &divisor,
    big_integer::multiplication_rule multiplication_rule) const
{
    throw not_implemented("big_integer &big_integer::Burnikel_Ziegler_division::modulo(big_integer &, big_integer const &, big_integer::multiplication_rule)", "your code should be here...");
}

std::vector<int> big_integer::convert_string_to_vector(std::string value_as_string, size_t index)
{
    std::vector<int> result;
    size_t converted = 0;

    unsigned int max_int = -1;
    size_t base = static_cast<size_t>(max_int) + 1;

    while (index != value_as_string.length())
    {
        std::string next_number_to_divide("");

        while (converted < base)
        {
            if (index == value_as_string.length()) break;

            converted = converted * 10 + (value_as_string[index] - '0');
            ++index;
        }
        if (index == value_as_string.length())
        {
            if (converted >= base)
            {
                result.push_back(converted % base);
                converted /= base;
            }
            result.push_back(converted);
            return result;
        }

        while (index != value_as_string.length())
        {
            if (converted >= base)
            {
                next_number_to_divide.push_back(converted / base + '0');
                converted %= base;
            }
            else next_number_to_divide.push_back('0');

            if (index != value_as_string.length()) converted = converted * 10 + (value_as_string[index] - '0');

            ++index;
        }
        if (converted >= base)
        {
            next_number_to_divide.push_back(converted / base + '0');
            converted %= base;
        }
        else next_number_to_divide.push_back('0');

        result.push_back(converted);
        value_as_string = std::move(next_number_to_divide);
        converted = 0;
        index = 0;
    }

    return result;
}

void big_integer::copy_from(
    big_integer const &other)
{
    _oldest_digit = other._oldest_digit;
    _other_digits = nullptr;
    if (!other._other_digits) return;

    _other_digits = new unsigned int[other.get_size()];
    std::memcpy(_other_digits, other._other_digits, sizeof(uint) * (*other._other_digits));
}

void big_integer::initialize_from(
    int const *digits,
    size_t digits_count)
{
    if (!digits) throw std::logic_error("Pointer to digits array must not be nullptr\n");
    if (digits_count < 1) throw std::logic_error("Digits array length must be at least 1\n");


    _oldest_digit = digits[digits_count - 1];
    if (digits_count == 1) return;
    
    _other_digits = new unsigned int[digits_count];
    
    *_other_digits = (size_t)digits_count;
    std::memcpy(_other_digits + 1, digits, sizeof(unsigned int) * (digits_count - 1));
}

void big_integer::initialize_from(
    std::vector<int> const &digits,
    size_t digits_count)
{
    if (digits.empty() || !digits_count) throw std::logic_error("std::vector<int> of digits should not be empty\n");

    _oldest_digit = digits[digits_count - 1];
    if (digits_count == 1) return;

    _other_digits = new unsigned int[digits_count];

    *_other_digits = digits_count;

    for (auto i = 1; i < digits_count; ++i) _other_digits[i] = *reinterpret_cast<unsigned int const *>(&digits[i - 1]);
}

void big_integer::initialize_from(
    std::string const &value_as_string,
    size_t base)
{
    if (base < 2 or base > 36) throw std::logic_error("Base must be in range [2..36]\n");
    if (value_as_string.empty()) throw std::logic_error("String value must not be empty\n");

    std::string value = value_as_string;

    while (value[0] == '0' and value.size() > 1) value.erase(0, 1);

    int index = 0;
    if (value_as_string[0] == '-') index = 1;

    std::string decimal_value = value;
    if (base != 10) decimal_value = string_to_decimal(value, base);

    std::vector<int> res = convert_string_to_vector(decimal_value, index);
    initialize_from(res, res.size());
}

big_integer::big_integer(
    int const *digits,
    size_t digits_count)
{
    initialize_from(digits, digits_count);
}

big_integer::big_integer(
    std::vector<int> const &digits)
{
    initialize_from(digits, digits.size());
}

big_integer::big_integer(
    std::string const &value_as_string,
    size_t base)
{
    initialize_from(value_as_string, base);
    if (value_as_string[0] == '-') change_sign();
}

void big_integer::clear()
{
    _oldest_digit = 0;
    if (_other_digits) delete[] _other_digits;
    _other_digits = nullptr;
}

big_integer::~big_integer()
{
    clear();
}

big_integer::big_integer(
    big_integer const &other)
{
    copy_from(other);
}

big_integer &big_integer::operator=(
    big_integer const &other)
{
    if (this != &other)
    {
        clear();
        copy_from(other);
    }

    return *this;
}

bool big_integer::operator==(
    big_integer const &other) const
{
    if (big_int_cmp(*this, other) == 0) return true;
    return false;
}

bool big_integer::operator!=(
    big_integer const &other) const
{
    return !(*this == other);
}

bool big_integer::operator<(
    big_integer const &other) const
{
    if (big_int_cmp(*this, other) < 0) return true;
    return false;
}

bool big_integer::operator>(
    big_integer const &other) const
{
    if (big_int_cmp(*this, other) > 0) return true;
    return false;
}

bool big_integer::operator<=(
    big_integer const &other) const
{
    if (big_int_cmp(*this, other) <= 0) return true;
    return false;
}

bool big_integer::operator>=(
    big_integer const &other) const
{
    if (big_int_cmp(*this, other) >= 0) return true;
    return false;
}

big_integer big_integer::operator-() const
{
    return big_integer(*this).change_sign();
}

big_integer &big_integer::operator+=(
    big_integer const &other)
{
    if (other.is_zero()) return *this;
    if (is_zero()) return *this = other;


    if (sign() == -1)
    {
        change_sign();
        *this += -other;
        return change_sign();
    }

    if (other.sign() == -1) return *this -= -other;

    auto const size = std::max(get_size(), other.get_size());

    unsigned int operation_result = 0;

    constexpr int shift = sizeof(unsigned int) << 2;
    constexpr int mask = (1 << shift) - 1;

    std::vector<int> result(size + 1);

    for (int i = 0; i < size; ++i)
    {
        unsigned int first_digit = get_digit(i);
        unsigned int second_digit = other.get_digit(i);
        result[i] = 0;

        for (int j = 0; j < 2; ++j)
        {
            operation_result += (first_digit & mask) + (second_digit & mask);
            first_digit >>= shift;
            second_digit >>= shift;
            *reinterpret_cast<unsigned int *>(&result[i]) |= ((operation_result & mask) << shift * j);
            operation_result >>= shift;
        }
    }

    auto result_size = result.size();

    if (operation_result == 1)
    {
        if ((*reinterpret_cast<uint *>(&result[result_size - 1]) >> ((sizeof(uint) << 3) - 1)) == 0) *reinterpret_cast<uint *>(&result[result_size - 1]) |= (1u << ((sizeof(uint) << 3) - 1));
        else result.back() = 1;
    }
    else if ((*reinterpret_cast<uint *>(&result[result_size - 1]) >> ((sizeof(uint) << 3) - 1)) == 0) --result_size;

    clear();
    initialize_from(result, result_size);
    return *this;
}

big_integer big_integer::operator+(
    big_integer const &other) const
{
    return big_integer(*this) += other;
}

big_integer big_integer::operator+(
    std::pair<big_integer, allocator *> const &other) const
{
    throw not_implemented("big_integer big_integer::operator+(std::pair<big_integer, allocator *> const &) const", "your code should be here...");
}

big_integer &big_integer::operator-=(
    big_integer const &other)
{
    if (other.is_zero()) return *this;

    if (is_zero()) return *this = other;

    if (*this == other) 
    { 
        big_integer tmp("0");
        clear();
        copy_from(tmp);
        return *this;
    }
    if (*this < other) 
    {
        big_integer res(other);
        res -= *this;
        res.change_sign();
        clear();
        copy_from(res);
        return *this;
    }
    if (sign() == -1)
    {
        if (other.sign() == -1)
        {
            big_integer tmp(other);
            tmp.change_sign();
            *this += tmp;
            return *this;
        }
        change_sign();
        big_integer tmp(other);
        tmp.change_sign();
        tmp -= *this;
        clear();
        copy_from(tmp);
        return *this;
    }
    else if (other.sign() == -1)
    {
        big_integer tmp(other);
        tmp.change_sign();
        *this += tmp;
        return *this;
    }

    int first_size = get_size();
    int second_size = other.get_size();
    int size = std::max(first_size, second_size);

    std::vector <unsigned int> result_digits(size - 1);

    bool is_taken = false;
    for (auto i = 0; i < size - 1; ++i) 
    {
        unsigned int first_digit = get_digit(i);
        unsigned int second_digit = other.get_digit(i);
        if (is_taken) first_digit--;

        if (first_digit == second_digit) 
        {
            result_digits[i] = 0;
            is_taken = false;
        } 
        else 
        {
            result_digits[i] = first_digit - second_digit;
            if (first_digit < second_digit) is_taken = true;
            else is_taken = false;
        }
    }
    int first_oldest_digit = _oldest_digit;
    int second_oldest_digit = other._oldest_digit;

    if (is_taken && first_size > second_size) first_oldest_digit--;
    else if (is_taken && first_size == second_size) first_oldest_digit -= (second_oldest_digit + 1);
    else if (first_size == second_size) first_oldest_digit -= second_oldest_digit;

    while (result_digits.back() == 0) result_digits.pop_back();

    clear();

    _other_digits = nullptr;
    _oldest_digit = first_oldest_digit;

    _other_digits = new unsigned int[size];
    *_other_digits = size;

    for (auto i = 0; i < size; ++i) _other_digits[i + 1] = result_digits[i];
    return *this;
}

big_integer big_integer::operator-(
    big_integer const &other) const
{
    return big_integer(*this) -= other;
}

/*
    в чем смысл?
*/
big_integer big_integer::operator-(
    std::pair<big_integer, allocator *> const &other) const
{
    // что тут должно быть?
}

big_integer &big_integer::operator*=(
    big_integer const &other)
{
    return multiply(*this, other);
}

big_integer big_integer::operator*(
    big_integer const &other) const
{
    return big_integer(*this) *= other;
}

big_integer big_integer::operator*(
    std::pair<big_integer, allocator *> const &other) const
{
    // ??
}

big_integer &big_integer::operator/=(
    big_integer const &other)
{
    // TODO CLASS
}

big_integer big_integer::operator/(
    big_integer const &other) const
{
    throw not_implemented("big_integer big_integer::operator/(big_integer const &) const", "your code should be here...");
}

big_integer big_integer::operator/(
    std::pair<big_integer, allocator *> const &other) const
{
    throw not_implemented("big_integer big_integer::operator/(std::pair<big_integer, allocator *> const &) const", "your code should be here...");
}

big_integer &big_integer::operator%=(
    big_integer const &other)
{
    throw not_implemented("big_integer &big_integer::operator%=(big_integer const &)", "your code should be here...");
}

big_integer big_integer::operator%(
    big_integer const &other) const
{
    throw not_implemented("big_integer big_integer::operator%(big_integer const &) const", "your code should be here...");
}

big_integer big_integer::operator%(
    std::pair<big_integer, allocator *> const &other) const
{
    throw not_implemented("big_integer big_integer::operator%(std::pair<big_integer, allocator *> const &) const", "your code should be here...");
}

big_integer big_integer::operator~() const
{
    auto size = get_size();

    int * new_digits = new int[size];
    

    for (auto i = 0; i < size; ++i)
    {
        auto current = get_digit(i);
        current = ~_other_digits[i];
        new_digits[i] = current;
    }

    return big_integer(new_digits, size);
}

big_integer &big_integer::operator&=(
    big_integer const &other)
{
    auto size = std::min(this->get_size(), other.get_size());

    int * new_digits = new int[size];

    for(int i = 0; i < size; ++i) new_digits[i] = this->get_digit(i) & other.get_digit(i);

    clear();
    big_integer tmp(new_digits, size);
    copy_from(tmp);
    ~tmp;

    return *this;
}

big_integer big_integer::operator&(
    big_integer const &other) const
{
    return big_integer(*this) &= other;
}

big_integer big_integer::operator&(
    std::pair<big_integer, allocator *> const &other) const
{
    throw not_implemented("big_integer big_integer::operator&(std::pair<big_integer, allocator *> const &) const", "your code should be here...");
}

big_integer &big_integer::operator|=(
    big_integer const &other)
{
    int size =  std::max(get_size(), other.get_size());

    int * new_digits = new int[size];

    for (auto i = 0; i < size; ++i)
    {
        if (i != get_size() && i != other.get_size()) new_digits[i] = this->get_digit(i) | other.get_digit(i);
        else new_digits[i] = i == get_size() ? other.get_digit(i) : this->get_digit(i);
    }
    clear();
    big_integer tmp(new_digits, size);
    copy_from(tmp);
    ~tmp;

    return *this;
}

big_integer big_integer::operator|(
    big_integer const &other) const
{
    return big_integer(*this) |= other;
}

big_integer big_integer::operator|(
    std::pair<big_integer, allocator *> const &other) const
{
    throw not_implemented("big_integer big_integer::operator|(std::pair<big_integer, allocator *> const &) const", "your code should be here...");
}

big_integer &big_integer::operator^=(
    big_integer const &other)
{
    int size =  std::max(get_size(), other.get_size());

    int * new_digits = new int[size];

    for (auto i = 0; i < size; ++i)
    {
        if (i != get_size() && i != other.get_size()) new_digits[i] = this->get_digit(i) | other.get_digit(i);
        else new_digits[i] = i == get_size() ? other.get_digit(i) ^ 0 : this->get_digit(i) ^ 0;
    }

    clear();
    big_integer tmp(new_digits, size);
    copy_from(tmp);
    ~tmp;

    return *this;
}

big_integer big_integer::operator^(
    big_integer const &other) const
{
    return big_integer(*this) ^= other;
}

big_integer big_integer::operator^(
    std::pair<big_integer, allocator *> const &other) const
{
    throw not_implemented("big_integer big_integer::operator^(std::pair<big_integer, allocator *> const &) const", "your code should be here...");
}

big_integer &big_integer::operator<<=(
    size_t shift)
{
    throw not_implemented("big_integer &big_integer::operator<<=(size_t)", "your code should be here...");
}

big_integer big_integer::operator<<(
    size_t shift) const
{
    throw not_implemented("big_integer big_integer::operator<<(size_t) const", "your code should be here...");
}

big_integer big_integer::operator<<(
    std::pair<size_t, allocator *> const &shift) const
{
    throw not_implemented("big_integer big_integer::operator<<(std::pair<size_t, allocator *> const &) const", "your code should be here...");
}

big_integer &big_integer::operator>>=(
    size_t shift)
{
    throw not_implemented("big_integer &big_integer::operator>>=(size_t)", "your code should be here...");
}

big_integer big_integer::operator>>(
    size_t shift) const
{
    throw not_implemented("big_integer big_integer::operator>>(size_t) const", "your code should be here...");
}

big_integer big_integer::operator>>(
    std::pair<size_t, allocator *> const &other) const
{
    throw not_implemented("big_integer big_integer::operator>>(std::pair<size_t, allocator *> const &) const", "your code should be here...");
}

big_integer &big_integer::multiply(
    big_integer &first_multiplier,
    big_integer const &second_multiplier,
    allocator *allocator,
    big_integer::multiplication_rule multiplication_rule)
{
}

big_integer big_integer::multiply(
    big_integer const &first_multiplier,
    big_integer const &second_multiplier,
    allocator *allocator,
    big_integer::multiplication_rule multiplication_rule)
{
    return multiply(big_integer(first_multiplier), second_multiplier);
}

big_integer &big_integer::divide(
    big_integer &dividend,
    big_integer const &divisor,
    allocator *allocator,
    big_integer::division_rule division_rule,
    big_integer::multiplication_rule multiplication_rule)
{
    throw not_implemented("big_integer &big_integer::divide(big_integer &, big_integer const &, allocator *, big_integer::division_rule, big_integer::multiplication_rule)", "your code should be here...");
}

big_integer big_integer::divide(
    big_integer const &dividend,
    big_integer const &divisor,
    allocator *allocator,
    big_integer::division_rule division_rule,
    big_integer::multiplication_rule multiplication_rule)
{
    throw not_implemented("big_integer big_integer::divide(big_integer const &, big_integer const &, allocator *, big_integer::division_rule, big_integer::multiplication_rule)", "your code should be here...");
}

big_integer &big_integer::modulo(
    big_integer &dividend,
    big_integer const &divisor,
    allocator *allocator,
    big_integer::division_rule division_rule,
    big_integer::multiplication_rule multiplication_rule)
{
    throw not_implemented("big_integer &big_integer::modulo(big_integer &, big_integer const &, allocator *, big_integer::division_rule, big_integer::multiplication_rule)", "your code should be here...");
}

big_integer big_integer::modulo(
    big_integer const &dividend,
    big_integer const &divisor,
    allocator *allocator,
    big_integer::division_rule division_rule,
    big_integer::multiplication_rule multiplication_rule)
{
    throw not_implemented("big_integer big_integer::modulo(big_integer const &, big_integer const &, allocator *, big_integer::division_rule, big_integer::multiplication_rule)", "your code should be here...");
}

std::ostream &operator<<(
    std::ostream &stream,
    big_integer const &value)
{
    std::string res = value.big_integer_to_string(value);
    stream << res;
    
    return stream;
}

std::istream &operator>>(
    std::istream &stream,
    big_integer &value)
{
    std::string source;
    stream >> source;
    big_integer tmp(source);
    value.copy_from(tmp);
    ~tmp;
    return stream;
}

big_integer &big_integer::change_sign()
{
    _oldest_digit ^= (1 << ((sizeof(int) << 3) - 1));
    return *this;
}

inline bool big_integer::is_zero() const noexcept
{
    return _oldest_digit == 0 && !_other_digits;
}

inline int big_integer::sign() const noexcept
{
    if (is_zero()) return 0;
    return 1 - (static_cast<int>((*reinterpret_cast<unsigned int const *>(&_oldest_digit) >> ((sizeof(int) << 3) - 1))) << 1);
}

inline unsigned int big_integer::get_digit(int index) const noexcept
{
    if (!_other_digits) return index == 0 ? _oldest_digit : 0;

    int const digits_count = get_size();
    if (index < digits_count - 1) return _other_digits[index + 1];
    if (index == digits_count - 1) return _oldest_digit;

    return 0;
}

inline int big_integer::get_size() const noexcept
{
    return static_cast<int>(!_other_digits ? 1 : *_other_digits);
}