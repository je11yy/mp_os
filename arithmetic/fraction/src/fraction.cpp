#include "../include/fraction.h"

int fraction::sign()
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

fraction fraction::sin(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::sin(fraction const &) const", "your code should be here...");
}

fraction fraction::cos(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::cos(fraction const &) const", "your code should be here...");
}

fraction fraction::tg(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::tg(fraction const &) const", "your code should be here...");
}

fraction fraction::ctg(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::ctg(fraction const &) const", "your code should be here...");
}

fraction fraction::sec(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::sec(fraction const &) const", "your code should be here...");
}

fraction fraction::cosec(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::cosec(fraction const &) const", "your code should be here...");
}

fraction fraction::arcsin(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arcsin(fraction const &) const", "your code should be here...");
}

fraction fraction::arccos(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arccos(fraction const &) const", "your code should be here...");
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
    throw not_implemented("fraction fraction::pow(size_t) const", "your code should be here...");
}

fraction fraction::root(
    size_t degree,
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::root(size_t, fraction const &) const", "your code should be here...");
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