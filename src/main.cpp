#include "unit/rect.hpp"
#include <print>

using namespace frqs::widget ;

int main() {
	auto pt1 = Point(10, 20) ;
	auto pt2 = Point(20.5, 40.5f) ;
	auto pt3 = pt1 + 2 + pt2 ;

	auto sz1 = Size(10, 20) ;
	auto sz2 = Size(20.5, 40.5f) ;
	auto sz3 = sz1 + 2 + sz2 ;
	
	auto d1 = pt3 + sz3 ;
	auto d2 = sz3 + pt3 ;

	auto r1 = Rect(10, 20u, 30.5, 40.5f) ;
	auto r2 = Rect(pt1, sz1) ;
	auto r3 = r1 + r2 ;


	std::println("{}\nsize data : {}", pt1.debug(), sizeof(pt1)) ;
	std::println("{}\nsize data : {}", pt2.debug(), sizeof(pt2)) ;
	std::println("{}\nsize data : {}", pt3.debug(), sizeof(pt3)) ;

	std::println("{}\nsize data : {}", sz1.debug(), sizeof(sz1)) ;
	std::println("{}\nsize data : {}", sz2.debug(), sizeof(sz2)) ;
	std::println("{}\nsize data : {}", sz3.debug(), sizeof(sz3)) ;

	std::println("{}\ntype : {}\nsize data : {}", d1.debug(), typeid(d1).name(), sizeof(d1)) ;
	std::println("{}\ntype : {}\nsize data : {}", d2.debug(), typeid(d2).name(), sizeof(d2)) ;
}