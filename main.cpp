#include <iostream>
#include "Visitable.hpp"



struct Box;
struct Cylinder;
struct Cube;

struct Geometry : misc::Visitable<boost::mpl::vector<Box, Cylinder, Cube>>
{
protected:
	template<class ActualType>
	Geometry(misc::type<ActualType> t)
		:misc::Visitable<boost::mpl::vector<Box, Cylinder, Cube>>(t) {}
};


struct Box : Geometry
{
	char data[100];
	Box() :Geometry(misc::type<Box>()) {}

protected:
	template<class ActualType>
	Box(misc::type<ActualType> t) :Geometry(t) {}

	friend bool operator==(Box const& a, Box const& b)
	{
		return &a == &b;
	}
};

struct Cube : Box
{
	Cube() :Box(misc::type<Cube>()) {}

	friend bool operator==(Cube const& a, Cube const& b)
	{
		return &a == &b;
	}
};

struct Cylinder : Geometry
{
	char data[200];
	Cylinder() :Geometry(misc::type<Cylinder>()) {}

	friend bool operator==(Cylinder const& a, Cylinder const& b)
	{
		return &a == &b;
	}
};


 
struct SizeofVisitor
{
	template<class T>
	std::size_t operator()(T const&) { return sizeof(T); }
};

struct TestConst
{
	double operator()(Geometry&) { return 20.0; }
	char const* operator()(Geometry const&) { return "Const"; }
};

 
template<typename>
struct TypeDisplayer;

template<typename T>
void ShowType(T&& t)
{
	TypeDisplayer<T>();
	TypeDisplayer<decltype(t)>();
}

struct Test2DVisit
{
	template<class T, class U>
	int operator()(T const&, U&) {
		return sizeof(T) + sizeof(U);
	}
};


struct EqualityVisitor
{
	template<class T, class U>
	bool operator()(T const&, U const&) { return false; }

	template<class T>
	bool operator()(T const& a, T const& b) { return a == b; }
};

template<class SmartPtr>
struct CopyVisitor
{
	template<class T>
	SmartPtr operator()(T const& t)
	{
		return SmartPtr(new T(t));
	}
};

struct PrintType
{
	void operator()(Box const&) { std::cout << "Box\n"; }
	void operator()(Cylinder const&) { std::cout << "Cylinder\n"; }
	void operator()(Cube const&) { std::cout << "Cube\n"; }
};

#include <memory>

int main()
{
	Box b;
	Cylinder c;
	Geometry & g = b;
	Geometry const& cg = c;
	visit(TestConst(), g);
	std::cout << visit(TestConst(), g) << "\n"
		<< visit(TestConst(), cg)  << "\n";

	std::cout << visit(EqualityVisitor(), g, b);
	auto copied = visit(CopyVisitor<std::unique_ptr<Geometry>>(), cg);
	visit(PrintType(), *copied);
	
	getchar();
	return 0;
}
