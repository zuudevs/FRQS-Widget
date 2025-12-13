#include "unit/point.hpp"
#include "unit/size.hpp"

using namespace frqs::widget ;

int main() {
	auto pt1 = Point(10, 20) ;
	auto pt2 = Point(20.5, 40.5f) ;
	auto pt3 = pt1 + 2 ;
}