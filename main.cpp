#include <print>

#include <vecdb/vector.hpp>

int main() {
	std::println("Hello VectorDB!");

	Vector v{1, 2, 3};
	v.print();

	return 0;
}
