#include <iostream>
#include <ostream>
using namespace std;

class complex {
public:
	complex(int a = 0, int b = 0) 
		:_a(a), _b(b) {

	}

	complex(const complex &c) 
		:_a(c._a), _b(c._b)
	{

	}

	~complex() {}

	const complex operator+(const complex &c) const {
		return complex(_a + c._a, _b + c._b);
	}

	const complex operator-(const complex &c) const {
		return complex(_a - c._a, _b - c._b);
	}

	complex &operator=(const complex &c) {
		_a = c._a;
		_b = c._b;
		return *this;
	}

	complex &operator+=(const complex &c) {
		_a += c._a;
		_b += c._b;
		return *this;
	}

	complex &operator++() {
		_a++;	
		_b++;
		return *this;
	}

	const complex operator++(int) {
		complex ret = *this;
		_a++;
		_b++;
		return ret;
	}

	void show() const {
		cout << "a : " << _a << ", b :" << _b << endl;
	}

	friend istream &operator>>(istream &in, complex &c);
	friend ostream &operator<<(ostream &out, const complex &c);

private:
	int _a;
	int _b;
};

ostream &operator<<(ostream &out, const complex &c)
{
	out << c._a << " " << c._b;
	return out;
}

istream &operator>>(istream &in, complex &c) {
	in >> c._a >> c._b;
	return in;
}

int main()
{
	complex c1(1, 1);

	c1.show();
	complex c2 = c1++;
	c2.show();

	complex c3 = c1 + c2;
	c3.show();

	complex c4 = c3 - c1;
	c4.show();

	complex c5 = ++c1;
	c5.show();

	c5 += c1;
	c5.show();

	cout << c5 << endl;

	cin >> c5;
	cout << c5 << endl;


	
	return 0;
}
