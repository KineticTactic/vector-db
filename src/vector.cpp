#include <vecdb/vector.hpp>

#include <iostream>

void Vector::print() const {
	std::cout << "(" << x << ", " << y << ", " << z << ")\n";
}
