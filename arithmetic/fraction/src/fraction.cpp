#include "../include/fraction.h"

int fraction::sign() const noexcept
{
    return _denominator.sign();
}

big_integer fraction::find_denominator(const fraction& a, const fraction& sec)
{
    big_integer gcd = a._denominator;
    big_integer b = sec._denominator;
    if (gcd < b) std::swap(gcd, b);
    while (b != big_integer("1") && !b.is_zero())
    {
        big_integer temp = b;
        b = gcd % b;
        gcd = temp;
    }
    return (a._denominator * sec._denominator) / (gcd);
}

/*
    Функция для сокращения дроби
*/
void fraction::fraction_reducing()
{
    if (_denominator == big_integer("1")) return;
    bool is_negative = false;
    if (_denominator.sign() < 0)
    {
        _denominator.change_sign();
        is_negative = true;
    }

    big_integer gcd = _denominator, b = _numerator;
    if (gcd < b) std::swap(gcd, b);
    while (!b.is_zero())
    {
        big_integer temp = b;
        b = gcd % b;
        gcd = temp;
    }
    _numerator /= gcd;
    _denominator /= gcd;

    if (is_negative) _denominator.change_sign();
}

fraction::fraction(
    big_integer &&numerator,
    big_integer &&denominator):
        _numerator(std::forward<big_integer>(numerator)),
        _denominator(std::forward<big_integer>(denominator))
{
    if (_numerator.sign() < 0) throw std::logic_error("Sign should be in denominator!\n");
    if (_denominator == big_integer("0")) throw std::logic_error("Denominator should not be zero\n");

    fraction_reducing();
}

fraction::fraction(
    fraction const &other):
        _numerator(other._numerator),
        _denominator(other._denominator)
{}

fraction &fraction::operator=(
    fraction const &other)
{
    if (*this == other) return *this;
    _numerator = other._numerator;
    _denominator = other._denominator;
    return *this;
}

fraction::fraction(
    fraction &&other) noexcept:
        _numerator(std::move(other._numerator)),
        _denominator(std::move(other._denominator))
{}

fraction &fraction::operator=(
    fraction &&other) noexcept
{
    if (*this == other) return *this;
    _numerator = std::move(other._numerator);
    _denominator = std::move(other._denominator);
    return *this;
}

fraction &fraction::operator+=(
    fraction const &other)
{
    big_integer deletil = find_denominator(*this, other);

    big_integer first_multiplyer = deletil / _denominator;
    big_integer second_multiplyer = deletil / other._denominator;

    _denominator = deletil;

    _numerator *= first_multiplyer;
    _numerator += (other._numerator * second_multiplyer);

    fraction_reducing();
    return *this;
}

fraction fraction::operator+(
    fraction const &other) const
{
    return fraction(*this) += other;
}

fraction &fraction::operator-=(
    fraction const &other)
{
    big_integer deletil = find_denominator(*this, other);
    big_integer first_multiplyer = deletil / _denominator;
    big_integer second_multiplyer = deletil / other._denominator;

    _denominator = deletil;
    _numerator *= first_multiplyer;
    _numerator -= (other._numerator * second_multiplyer);

    fraction_reducing();
    return *this;
}

fraction fraction::operator-(
    fraction const &other) const
{
    return fraction(*this) -= other;
}

fraction &fraction::operator*=(
    fraction const &other)
{
    _numerator *= other._numerator;
    _denominator *= other._denominator;
    fraction_reducing();
    return *this;
}

fraction fraction::operator*(
    fraction const &other) const
{
    return fraction(*this) *= other;
}

fraction &fraction::operator/=(
    fraction const &other)
{
    _numerator *= other._denominator;
    _denominator *= other._numerator;

    fraction_reducing();
    return *this;
}

fraction fraction::operator/(
    fraction const &other) const
{
    return fraction(*this) /= other;
}

bool fraction::operator==(
    fraction const &other) const
{
    return (_numerator == other._numerator && _denominator == other._denominator);
}

bool fraction::operator!=(
    fraction const &other) const
{
    return (_numerator != other._numerator && _denominator != other._denominator);
}

bool fraction::operator>=(
    fraction const &other) const
{
    return (_numerator * other._denominator > other._numerator * _denominator) || (_numerator == other._numerator && _denominator == other._denominator);
}

bool fraction::operator>(
    fraction const &other) const
{
    return _numerator * other._denominator > other._numerator * _denominator;
}

bool fraction::operator<=(
    fraction const &other) const
{
    return (_numerator * other._denominator < other._numerator * _denominator) || (_numerator == other._numerator && _denominator == other._denominator);
}

bool fraction::operator<(
    fraction const &other) const
{
    return _numerator * other._denominator < other._numerator * _denominator;
}

std::ostream &operator<<(
    std::ostream &stream,
    fraction const &obj)
{
    big_integer denominator_copy = obj._denominator;
    if (denominator_copy.sign() < 0)
    {
        stream << "-";
        denominator_copy.change_sign();
    }
    stream << obj._numerator << "/" << denominator_copy;
    return stream;
}

std::istream &operator>>(
    std::istream &stream,
    fraction &obj)
{
    stream >> obj._numerator;
    char slash;
    if (slash != '/') throw std::logic_error("Unexpected symbol\n");
    stream >> obj._denominator;
    if (obj._numerator.sign() < 0)
    {
        obj._numerator.change_sign();
        obj._denominator.change_sign();
    }
    return stream;
}

bool fraction::is_zero() const
{
    return _numerator.is_zero();
}

int fraction::is_valid_eps(fraction const &eps) const noexcept
{
    return !((eps.sign() == -1) || eps.is_zero());
}

fraction fraction::abs() const
{
    if (sign() > 0) return *this;
    fraction tmp(*this);
    tmp._denominator.change_sign();
    return tmp;
}

fraction fraction::sin(
    fraction const &epsilon) const
{
    if (!is_valid_eps(epsilon)) throw std::logic_error("Invalid epsilon\n");

    fraction term = *this;
    fraction result(big_integer(0), big_integer(1));
    int n = 1;
    while (term.abs() >= epsilon) 
    {
        result += term;
        fraction minus_term = term;
        minus_term._denominator.change_sign();
        term = (minus_term) * (*this) * (*this) / fraction(big_integer((2 * n ) * (2 * n + 1)), big_integer(1));
        n++;
    }

    return result;
}

fraction fraction::cos(
    fraction const &epsilon) const
{
    if (!is_valid_eps(epsilon)) throw std::logic_error("invalid epsilon");

    fraction term(big_integer(1), big_integer(1));
    fraction result = term;
    int n = 1;

    while (term.abs() >= epsilon) 
    {
        fraction minus_term = term;
        term = minus_term * (*this) * (*this) / fraction(big_integer((n) * (n + 1)), big_integer(1));
        result += term;
        n += 2;
  }

  return result;
}

fraction fraction::tg(
    fraction const &epsilon) const
{
	return sin(epsilon) / cos(epsilon);
}

fraction fraction::ctg(
    fraction const &epsilon) const
{
    return cos(epsilon) / sin(epsilon);
}

fraction fraction::sec(
    fraction const &epsilon) const
{
    // 1 / cos(x)
    return fraction(big_integer("1"), big_integer("1")) / cos(epsilon);
}

fraction fraction::cosec(
    fraction const &epsilon) const
{
    // 1 / sin(x)
    return fraction(big_integer("1"), big_integer("1")) / sin(epsilon);
}

fraction fraction::arcsin(
    fraction const &epsilon) const
{
    if (*this < fraction(big_integer("1"), big_integer("-1")) || *this > fraction(big_integer("1"), big_integer("1"))) 
        throw std::logic_error("Invalid range\n");
    
    fraction result = *this;
    fraction term = *this;
    int n = 1;

    while (term.abs() > epsilon) 
    {
        term = term * (*this) * (*this) * fraction(big_integer(std::to_string(n)), big_integer("1")) / fraction(big_integer(std::to_string(n + 1)), big_integer("1")) / fraction(big_integer(std::to_string(n + 2)), big_integer("1"));
        result += term;
        n += 2;
    }

    return result;
}

fraction fraction::arccos(
    fraction const &epsilon) const
{
    if (*this < fraction(big_integer("1"), big_integer("-1")) || *this > fraction(big_integer("1"), big_integer("1"))) 
        throw std::logic_error("Invalid range\n");

    
    
}

fraction fraction::arctg(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arctg(fraction const &) const", "your code should be here...");
}

fraction fraction::arcctg(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arcctg(fraction const &) const", "your code should be here...");
}

fraction fraction::arcsec(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arcsec(fraction const &) const", "your code should be here...");
}

fraction fraction::arccosec(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arccosec(fraction const &) const", "your code should be here...");
}

fraction fraction::pow(
    size_t degree) const
{
    if (degree == 0) return fraction(big_integer("1"), big_integer("1")); 
    if (degree < 0) return fraction(big_integer("1"), big_integer("1")) / pow(-degree);

    fraction base(*this);
    fraction result(big_integer(1), big_integer(1));

    while (degree > 0) 
    {
        if (degree % 2 == 1) 
        {
            result *= base;
            degree--;
        }
        base *= base;
        degree /= 2; 
    }

    return result;
}

fraction fraction::root(
    size_t degree,
    fraction const &epsilon) const
{
    fraction x = (*this);
    bool swapped;
    if (x._numerator > x._denominator)
    {
        std::swap(x._numerator, x._denominator);
        swapped = true;
    }
    fraction alpha = fraction(big_integer("1"), big_integer(std::to_string(degree)));
    x -= fraction(big_integer("1"), big_integer("1"));

    fraction result = fraction(big_integer("1"), big_integer("1"));
    fraction term = fraction(big_integer("2"), big_integer("1")) * epsilon;
    size_t iteration = 1;
    size_t factorial = 1;

    while (term.abs() > epsilon)
    {
        fraction precalc = alpha;
        for (int i = 1; i < iteration; ++i) precalc *= (alpha - fraction(big_integer(std::to_string(i)), big_integer("1")));

        term = precalc;
        term *= x.pow(iteration);
        term *= fraction(big_integer("1"), big_integer(std::to_string(factorial)));
        result += term;
        ++iteration;
        factorial *= iteration;
    }
    if (swapped) std::swap(result._denominator, result._numerator);
    return result;
}

fraction fraction::log2(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::log2(fraction const &) const", "your code should be here...");
}

fraction fraction::ln(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::ln(fraction const &) const", "your code should be here...");
}

fraction fraction::lg(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::lg(fraction const &) const", "your code should be here...");
}