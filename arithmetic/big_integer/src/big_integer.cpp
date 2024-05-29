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

std::string string_subtract(const std::string &first, const std::string &second) 
{
    std::string result;
    int carry = 0;

    auto len1 = first.size();
    auto len2 = second.size();

    for (auto i = 0; i < len1; i++) 
    {
        int digit1 = first[len1 - 1 - i] - '0';
        int digit2 = (i < len2) ? second[len2 - 1 - i] - '0' : 0;

        int curr = digit1 - digit2 - carry;

        if (curr < 0) 
        {
            curr += 10;
            carry = 1;
        } 
        else carry = 0;
        result.push_back(curr + '0');
    }

    while (result.size() > 1 and result.back() == '0') result.pop_back();

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

std::string string_divide(const std::string & first, const std::string & second) 
{
    if (second == "0") throw std::logic_error("Division by zero is not defined\n");
    if (first == "0") return "0";

    bool negative = (first[0] == '-') ^ (second[0] == '-');

    std::string dividend = (first[0] == '-') ? first.substr(1) : first;
    std::string divisor = (second[0] == '-') ? second.substr(1) : second;

    std::string result = "";

    std::string curr;

    for (char digit : dividend) 
    {
        curr += digit;
        while (curr.size() > 1 and curr[0] == '0') curr.erase(0, 1);
        int quotient = 0;
        while (curr.size() > divisor.size() || (curr.size() == divisor.size() and curr >= divisor)) 
        {
            curr = string_subtract(curr, divisor);
            quotient++;
        }
        result += std::to_string(quotient);
    }
    while (result.size() > 1 and result[0] == '0') result.erase(0, 1);

    std::string n1 = (first[0] == '-') ? first.substr(1) : first;
    std::string n2 = (second[0] == '-') ? second.substr(1) : second;

    if (negative and result != "0") result = "-" + result;

    return result;
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

    int size = value.get_size();

    unsigned int i;
    for (i = 1; i < size; ++i) 
    {
        res = string_add(res, string_mult(std::to_string(value._other_digits[i]), string_pow(i - 1, DF_base)));
    }

    if (value.sign()  < 0) 
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
    if (!first_negative && second_negative) return 1;


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

std::string big_integer::string_to_decimal(const std::string& number, int base) 
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

    if ((first_multiplier.sign() == -1 && second_copy.sign() == 1) || (first_multiplier.sign() == 1 && second_copy.sign() == -1))
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
    unsigned int const max_int = -1;
    size_t base = static_cast<size_t>(max_int) + 1;
    
    constexpr int shift = sizeof(unsigned int) << 2;
    constexpr unsigned int mask = (1U << shift) - 1;

    big_integer result("0");

    for (auto i = 0; i < 2 * first_size; ++i)
    {
        unsigned int remainder = 0;
        unsigned int first_number_half;

        auto number_1 = first_multiplier.get_digit(i / 2);
        if (i % 2 == 0) first_number_half = number_1 & mask;
        else first_number_half = (number_1 >> shift) & mask;
        if (first_number_half == 0) continue;
        for (auto j = 0; j < 2 * second_size; ++j)
        {
            std::vector<int> digits_array;
            unsigned int second_number_half;

            auto number_2 = second_copy.get_digit(j / 2);
            if (j % 2 == 0) second_number_half = number_2 & mask;
            else second_number_half = (number_2 >> shift) & mask;

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
    
    first_multiplier = std::move(result);

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
    dividend = std::move(big_integer(string_divide(dividend.big_integer_to_string(dividend), dividend.big_integer_to_string(divisor))));
    return dividend;
}

big_integer &big_integer::trivial_division::modulo(
    big_integer &dividend,
    big_integer const &divisor,
    big_integer::multiplication_rule multiplication_rule) const
{
    auto first = dividend.big_integer_to_string(dividend);
    auto second = dividend.big_integer_to_string(divisor);
    auto divide_result = string_divide(first, second);
    std::string n1 = (first[0] == '-') ? first.substr(1) : first;
    std::string n2 = (second[0] == '-') ? second.substr(1) : second;

    std::string mult_res;
    switch (multiplication_rule)
    {
        case big_integer::multiplication_rule::trivial:
            mult_res = string_mult(divide_result, n2);
            break;
    }
    std::string rem = string_subtract(n1, mult_res);
    if (first[0] == '-' && divide_result != "0")
    {
        divide_result = "-" + divide_result;
        rem = "-" + rem;
    }
    else if (first[0] == '-' && divide_result == "0") rem = "-" + rem;

    big_integer tmp = big_integer(rem);
    return tmp;
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

std::vector<int> big_integer::convert_to_base(std::string const &value_as_string, size_t base)
{
    int pos = 0;
    std::string value = value_as_string;
    if (value[0] == '-') pos = 1;
    while (value[pos] == '0' && ((pos == 0 && value.length() > 1) || (pos == 1 && value.length() > 2))) value.erase(pos, 1);

    std::vector<int> result = convert_string_to_vector(value, pos);
    if ((result[result.size() - 1] & (1 << ((sizeof(unsigned int) << 3) - 1))) != 0) result.push_back(0);

    if (value[0] == '-' && value[1] != '0') result[result.size() - 1] |= default_base;

    return result;

}

void big_integer::copy_from(
    big_integer const &other)
{
    _oldest_digit = other._oldest_digit;
    _other_digits = nullptr;
    if (!other._other_digits) return;

    try
    {
        _other_digits = static_cast<unsigned int *>(allocate_with_guard(sizeof(unsigned int), other.get_size()));
    }
    catch(const std::bad_alloc& e)
    {
        throw e;
    }
    
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
    
    try
    {
        _other_digits = static_cast<unsigned int *>(allocate_with_guard(sizeof(unsigned int), digits_count));
    }
    catch(const std::bad_alloc& e)
    {
        throw e;
    }
    
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

    try
    {
        _other_digits = static_cast<unsigned int *>(allocate_with_guard(sizeof(unsigned int), digits_count));
    }
    catch(const std::bad_alloc& e)
    {
        throw e;
    }

    *_other_digits = digits_count;

    for (auto i = 1; i < digits_count; ++i) _other_digits[i] = *reinterpret_cast<unsigned int const *>(&digits[i - 1]);
}

void big_integer::initialize_from(
    std::string const &value_as_string,
    size_t base)
{
    std::vector<int> res = convert_to_base(value_as_string, base);
    initialize_from(res, res.size());
}

big_integer::big_integer(
    int const *digits,
    size_t digits_count,
    allocator * allocator) : _allocator(allocator)
{
    initialize_from(digits, digits_count);
}

big_integer::big_integer(
    std::vector<int> const &digits,
    allocator * allocator) : _allocator(allocator)
{
    initialize_from(digits, digits.size());
}

big_integer::big_integer(
    std::string const &value_as_string,
    size_t base,
    allocator * allocator) : _allocator(allocator)
{
    initialize_from(value_as_string, base);
}

void big_integer::clear()
{
    _oldest_digit = 0;
    if (_other_digits) deallocate_with_guard(_other_digits);
    _allocator = nullptr;
    _other_digits = nullptr;
}

big_integer::~big_integer()
{
    clear();
}

big_integer::big_integer(
    big_integer const &other)
{
    clear();
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

big_integer::big_integer(
    big_integer &&other) noexcept
{
    clear();
    _other_digits = other._other_digits;
    other._other_digits = nullptr;
    _oldest_digit = other._oldest_digit;
    _allocator = other._allocator;
    other._allocator = nullptr;
    other.clear();
}
    
big_integer &big_integer::operator=(
    big_integer &&other) noexcept
{
    if (this != &other)
    {
        clear();
        _other_digits = other._other_digits;
        other._other_digits = nullptr;
        _oldest_digit = other._oldest_digit;
        _allocator = other._allocator;
        other._allocator = nullptr;
        other.clear();
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
    big_integer tmp("0", 10, other.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp += other.first;
}

big_integer &big_integer::operator-=(
    big_integer const &other)
{
    if (other.is_zero()) return *this;

    if (is_zero()) return *this = other;

    if (*this == other) 
    { 
        big_integer tmp("0");
        *this = std::move(tmp);
        return *this;
    }
    if (*this < other) 
    {
        big_integer res(other);
        res -= *this;
        res.change_sign();
        *this = std::move(res);
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
        *this = std::move(tmp);
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

    try
    {
        _other_digits = static_cast<unsigned int *>(allocate_with_guard(sizeof(unsigned int), size));
    }
    catch(const std::bad_alloc& e)
    {
        throw e;
    }
    
    *_other_digits = size;

    for (auto i = 0; i < size; ++i) _other_digits[i + 1] = result_digits[i];
    return *this;
}

big_integer big_integer::operator-(
    big_integer const &other) const
{
    return big_integer(*this) -= other;
}

big_integer big_integer::operator-(
    std::pair<big_integer, allocator *> const &other) const
{
    big_integer tmp("0", 10, other.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp -= other.first;
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
    big_integer tmp("0", 10, other.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp *= other.first;
}

big_integer &big_integer::operator/=(
    big_integer const &other)
{
    return divide(*this, other);
}

big_integer big_integer::operator/(
    big_integer const &other) const
{
    return big_integer(*this) /= other;
}

big_integer big_integer::operator/(
    std::pair<big_integer, allocator *> const &other) const
{
    big_integer tmp("0", 10, other.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp /= other.first;
}

big_integer &big_integer::operator%=(
    big_integer const &other)
{
    *this = *this - (*this/other) * other;
    if (sign() < 0) change_sign();
    return *this;
}

big_integer big_integer::operator%(
    big_integer const &other) const
{
    return big_integer(*this) %= other;
}

big_integer big_integer::operator%(
    std::pair<big_integer, allocator *> const &other) const
{
    big_integer tmp("0", 10, other.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp %= other.first;
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
    *this = std::move(tmp);

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
    big_integer tmp("0", 10, other.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp &= other.first;
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
    *this = std::move(tmp);

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
    big_integer tmp("0", 10, other.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp |= other.first;
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
    *this = std::move(tmp);

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
    big_integer tmp("0", 10, other.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp ^= other.first;
}

big_integer &big_integer::operator<<=(
    size_t shift)
{
    if (is_zero() || shift == 0) return *this;

    auto value_sign = sign();
    if (value_sign == -1) change_sign();

    auto added_other_digits_count = shift / (sizeof(unsigned int) << 3);
    shift %= (sizeof(unsigned int) << 3);

    auto added_oldest_digit = 0;
    if (_oldest_digit)
    {
        unsigned int oldest_digit = *reinterpret_cast<unsigned int *>(&_oldest_digit);
        int oldest_value_bit_index = 0;

        while (oldest_digit != 1)
        {
            oldest_digit >>= 1;
            ++oldest_value_bit_index;
        }

        if (oldest_value_bit_index + shift > (sizeof(int) << 3) - 2) ++added_oldest_digit;
    }

    if (added_oldest_digit || added_other_digits_count)
    {
        auto const added_digits_count = added_oldest_digit + added_other_digits_count;

        if (!_other_digits)
        {
            _other_digits = new unsigned int[added_digits_count + 1];
            *_other_digits = added_digits_count + 1;
            std::memset(_other_digits + 1, 0, sizeof(unsigned int) * (added_digits_count - 1));
            if (added_oldest_digit)
            {
                _other_digits[*_other_digits - 1] = _oldest_digit;
                _oldest_digit = 0;
            }
            else _other_digits[*_other_digits - 1] = 0;
        }
        else
        {
            auto *new_digits = new unsigned int[added_digits_count + *_other_digits];
            std::memset(new_digits + 1, 0, sizeof(unsigned int) * added_digits_count);
            if (added_oldest_digit != 0)
            {
                new_digits[added_digits_count + *_other_digits - 1] = _oldest_digit;
                _oldest_digit = 0;
            }
            std::memcpy(new_digits + 1 + added_other_digits_count, _other_digits + 1, sizeof(unsigned int) * (*_other_digits - 1));
            *new_digits = *_other_digits + added_digits_count;

            delete[] _other_digits;
            _other_digits = new_digits;
        }
    }

    if (shift != 0)
    {
        auto size = get_size();
        unsigned int next_digit = 0;
        for (auto i = 0; i < size; ++i)
        {
            auto digit_value = get_digit(i);
            unsigned int * digit_address;
            if (i == size - 1) digit_address = reinterpret_cast<unsigned int *>(&_oldest_digit);
            else digit_address = _other_digits + 1 + i;

            *digit_address <<= shift;
            *digit_address |= next_digit;
            next_digit = digit_value >> ((sizeof(unsigned int) << 3) - shift);
        }
    }

    if (value_sign == -1) change_sign();

    return *this;
}

big_integer big_integer::operator<<(
    size_t shift) const
{
    return big_integer(*this) <<= shift;
}

big_integer big_integer::operator<<(
    std::pair<size_t, allocator *> const &shift) const
{
    big_integer tmp("0", 10, shift.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp <<= shift.first;
}

big_integer &big_integer::operator>>=(
    size_t shift)
{
    if (is_zero() || shift == 0) return *this;

    auto value_sign = sign();
    if (value_sign < 0) change_sign();

    auto const to_remove = shift / (sizeof(unsigned int) << 3);

    shift %= (sizeof(unsigned int) << 3);

    if (to_remove > 0) 
    {
        if (to_remove >= *_other_digits) 
        {
            deallocate_with_guard(_other_digits);
            _other_digits = nullptr;
            _oldest_digit = 0;
            return *this;
        }

        auto new_size = *_other_digits - to_remove;
        unsigned int * new_digits;
        try
        {
            new_digits = static_cast<uint *>(allocate_with_guard(sizeof(uint), new_size + 1));
        }
        catch(const std::bad_alloc& e)
        {
            throw e;
        }
        
        std::memcpy(new_digits + 1, _other_digits + 1 + to_remove, sizeof(unsigned int) * (new_size - 1));
        *new_digits = new_size;
        deallocate_with_guard(_other_digits);
        _other_digits = new_digits;
    }

    if (shift) 
    {
        unsigned int part_to_move_to_previous_digit = 0;
        auto const digits_count = get_size();
        for (auto i = digits_count - 1; i >= 0; --i) 
        {
            unsigned int * digit_address;
            if (i == digits_count - 1) digit_address = reinterpret_cast<unsigned int *>(&_oldest_digit);
            else digit_address = _other_digits + i + 1;

            unsigned int current_digit = get_digit(i);
            *(digit_address) >>= shift;
            *(digit_address) |= part_to_move_to_previous_digit;
            part_to_move_to_previous_digit = (current_digit << ((sizeof(unsigned int) << 3) - shift));
        }
    }
    if (value_sign == -1) change_sign();
    return *this;
}

big_integer big_integer::operator>>(
    size_t shift) const
{
    return big_integer(*this) >>= shift;
}

big_integer big_integer::operator>>(
    std::pair<size_t, allocator *> const &other) const
{
    big_integer tmp("0", 10, other.second);
    if (_other_digits)
    {
        tmp._other_digits = static_cast<uint *>(tmp.allocate_with_guard(sizeof(uint), get_size()));
        std::memcpy(tmp._other_digits, _other_digits, sizeof(uint) * (get_size()));
    }
    return tmp >>= other.first;
}

big_integer &big_integer::multiply(
    big_integer &first_multiplier,
    big_integer const &second_multiplier,
    allocator *allocator,
    big_integer::multiplication_rule multiplication_rule)
{
    big_integer::multiplication * _mult;
    switch (multiplication_rule)
    {
        case multiplication_rule::trivial:
            try
            {
                if (allocator) _mult = static_cast<big_integer::trivial_multiplication *>(allocator->allocate(sizeof(big_integer::trivial_multiplication), 1));
                else _mult = new big_integer::trivial_multiplication;
            }
            catch(const std::bad_alloc& e)
            {
                throw e;
            }
            first_multiplier = _mult->multiply(first_multiplier, second_multiplier);
    }
    if (allocator) allocator->deallocate(_mult);
    else delete _mult;
    return first_multiplier;
}

big_integer big_integer::multiply(
    big_integer const &first_multiplier,
    big_integer const &second_multiplier,
    allocator *allocator,
    big_integer::multiplication_rule multiplication_rule)
{
    big_integer::multiplication * _mult;
    big_integer * tmp;
    switch (multiplication_rule)
    {
        case multiplication_rule::trivial:
            try
            {
                if (allocator) _mult = static_cast<big_integer::trivial_multiplication *>(allocator->allocate(sizeof(big_integer::trivial_multiplication), 1));
                else _mult = new big_integer::trivial_multiplication;
            }
            catch(const std::bad_alloc& e)
            {
                throw e;
            }
            *tmp = first_multiplier;
            *tmp = _mult->multiply(*tmp, second_multiplier);
    }
    if (allocator) allocator->deallocate(_mult);
    else delete _mult;
    return *tmp;
}

big_integer &big_integer::divide(
    big_integer &dividend,
    big_integer const &divisor,
    allocator *allocator,
    big_integer::division_rule division_rule,
    big_integer::multiplication_rule multiplication_rule)
{
    big_integer::division * _div;
    switch (multiplication_rule)
    {
        case multiplication_rule::trivial:
            try
            {
                if (allocator) _div = static_cast<big_integer::trivial_division *>(allocator->allocate(sizeof(big_integer::trivial_division), 1));
                else _div = new big_integer::trivial_division;
            }
            catch(const std::bad_alloc& e)
            {
                throw e;
            }
            dividend =_div->divide(dividend, divisor, multiplication_rule);
            break;
    }
    if (allocator) allocator->deallocate(_div);
    else delete _div;
    return dividend;
}

big_integer big_integer::divide(
    big_integer const &dividend,
    big_integer const &divisor,
    allocator *allocator,
    big_integer::division_rule division_rule,
    big_integer::multiplication_rule multiplication_rule)
{
    big_integer::division * _div;
    big_integer * tmp;
    switch (multiplication_rule)
    {
        case multiplication_rule::trivial:
            try
            {
                if (allocator) _div = static_cast<big_integer::trivial_division *>(allocator->allocate(sizeof(big_integer::trivial_division), 1));
                else _div = new big_integer::trivial_division;
            }
            catch(const std::bad_alloc& e)
            {
                throw e;
            }
            *tmp = dividend;
            *tmp = _div->divide(*tmp, divisor, multiplication_rule);
            break;
    }
    if (allocator) allocator->deallocate(_div);
    else delete _div;
    return *tmp;
}

big_integer &big_integer::modulo(
    big_integer &dividend,
    big_integer const &divisor,
    allocator *allocator,
    big_integer::division_rule division_rule,
    big_integer::multiplication_rule multiplication_rule)
{
    big_integer::division * _div;
    switch (multiplication_rule)
    {
        case multiplication_rule::trivial:
            try
            {
                if (allocator) _div = static_cast<big_integer::trivial_division *>(allocator->allocate(sizeof(big_integer::trivial_division), 1));
                else _div = new big_integer::trivial_division;
            }
            catch(const std::bad_alloc& e)
            {
                throw e;
            }
            return _div->modulo(dividend, divisor, multiplication_rule);
    }
}

big_integer big_integer::modulo(
    big_integer const &dividend,
    big_integer const &divisor,
    allocator *allocator,
    big_integer::division_rule division_rule,
    big_integer::multiplication_rule multiplication_rule)
{
    big_integer::division * _div;
    switch (multiplication_rule)
    {
        case multiplication_rule::trivial:
            try
            {
                if (allocator) _div = static_cast<big_integer::trivial_division *>(allocator->allocate(sizeof(big_integer::trivial_division), 1));
                else _div = new big_integer::trivial_division;
            }
            catch(const std::bad_alloc& e)
            {
                throw e;
            }
            big_integer tmp(dividend);
            return _div->modulo(tmp, divisor, multiplication_rule);
    }
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
    value = std::move(tmp);
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

allocator *big_integer::get_allocator() const noexcept
{
    return _allocator;
}