#ifndef INCLUDED_scheme_nest_NEST_concepts_HH
#define INCLUDED_scheme_nest_NEST_concepts_HH

#include <cstddef>
#include <assert.h>
#include <Eigen/Core>

namespace scheme {
namespace nest {
namespace concept {


struct ValueArchitype { };

template<int N>
struct ArrayValueArchitype {
	double array[N];
	double       & operator[](size_t i)       { assert(i<N); return array[i]; }
	double const & operator[](size_t i) const { assert(i<N); return array[i]; }
};


	///@brief Parameter to Value Map Policy Class Concept
	///@detail JUST A DEFINITION OF THE CONCEPT AND PLACEHOLDER!
	///@tparam DIM the dimension number of the input parameter space
	///@tparam Value the output value type, default Eigen Matrix
	///@tparam Index index type, default size_t
	///@tparam Float float type, default double
	template<
		int DIM,
		class Value=double,
		class Index=size_t,
		class Float=double
	>
	struct ParamMapArchitype {
		static int const DIMENSION = DIM;
		typedef Value ValueType ;
		typedef Float FloatType ;		
		typedef Index IndexType ;		
		typedef Eigen::Array<Index,DIM,1> Indices;
		typedef Eigen::Array<Float,DIM,1> Params;

		///@brief sets value to parameters without change
		///@return false iff invalid parameters
		bool params_to_value(
			Params const & /*params*/,
			Index /*cell_index*/,
			Value & /*value*/
		) const { return false; }

		///@brief aka covering radius max distance from bin center to any value within bin
		Float bin_circumradius(Index /*resl*/) const { return 0; }

		///@brief cell size
		Index cell_size() const { return 0; }
	};


	///@brief Parameter to Value Map Policy Class Concept
	///@detail JUST A DEFINITION OF THE CONCEPT AND PLACEHOLDER!
	///@tparam DIM the dimension number of the input parameter space
	///@tparam Value the output value type, default Eigen Matrix
	///@tparam Index index type, default size_t
	///@tparam Float float type, default double
	template<
		int DIM,
		class Value=Eigen::Array<double,DIM,1>,
		class Index=size_t,
		class Float=double
	>
	struct ParamMapInvertableArchitype {
		static int const DIMENSION = DIM;
		typedef Value ValueType ;
		typedef Float FloatType ;		
		typedef Index IndexType ;		
		typedef Eigen::Array<Index,DIM,1> Indices;
		typedef Eigen::Array<Float,DIM,1> Params;

		///@brief sets value to parameters without change
		///@return false iff invalid parameters
		bool params_to_value(
			Params const & /*params*/,
			Index /*cell_index*/,
			Value & /*value*/
		) const { return false; }

		///@brief sets params/cell_index from value
		///@note necessary for value lookup and neighbor lookup
		bool value_to_params(
			Value const & /*value*/,
			Params & /*params*/,
			Index & /*cell_index*/
		) const { return false; }

		///@brief get parameter space repr of Value for particular cell
		///@note necessary only for neighbor lookup		
		void value_to_params_for_cell(
			Value const & /*value*/,
			Params & /*params*/
		) const {}

		///@brief return the cell_index of neighboring cells within radius of value
		///@note delta parameter is in "Parameter Space"
		template<class OutIter>
		void get_neighboring_cells(Value const & /*value*/, Float /*radius*/, OutIter /*out*/) const {}

		///@brief aka covering radius max distance from bin center to any value within bin
		Float bin_circumradius(Index /*resl*/) const { return 0; }

		///@brief maximum distance from the bin center which must be within the bin
		Float bin_inradius(Index /*resl*/) const { return 0; }

		///@brief cell size
		Index cell_size() const { return 0; }
	};

}
}
}

#endif