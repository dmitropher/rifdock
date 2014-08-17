#ifndef INCLUDED_numeric_util_HH
#define INCLUDED_numeric_util_HH

#include <limits>

namespace scheme { namespace numeric {


template<class Position>
bool approx_eq(Position const & a, Position const & b){
	return a.isApprox(b);
}

///@brief return sum of highest two elements in vector
template<class Vector, class Index> void
max2(
	Vector vector,
	typename Vector::Scalar & mx1,
	typename Vector::Scalar & mx2,
	Index & argmax_1,
	Index & argmax_2
){
	// TODO: is there a faster way to do this?
	mx1 = vector.maxCoeff(&argmax_1);
	vector[argmax_1] = std::numeric_limits<typename Vector::Scalar>::min();
	mx2 = vector.maxCoeff(&argmax_2);
}


}}

#endif
