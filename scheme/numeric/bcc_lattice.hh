#ifndef INCLUDED_scheme_numeric_bcc_lattice_HH
#define INCLUDED_scheme_numeric_bcc_lattice_HH

#include "scheme/util/SimpleArray.hh"
#include "scheme/util/template_loop.hh"

namespace scheme { namespace numeric {

template< int DIM, class Float, class Index = uint64_t >
struct BCC {
	typedef util::SimpleArray<DIM,Index> Indices;
	typedef util::SimpleArray<DIM,Float> Floats;
	BOOST_STATIC_ASSERT(DIM > 2);

	Indices sizes_, sizes_prefsum_;
	Floats lower_, width_, lower_cen_, half_width_;

	BCC(){}

	template<class Sizes>
	BCC(
		Sizes const & sizes,
		Floats lower=Floats(0), 
		Floats upper=Floats(1)
	) :
		sizes_(sizes),
		lower_(lower)
	{
		for(size_t i = 0; i < DIM; ++i)
			sizes_prefsum_[i] = sizes_.prod(i);
		width_ = (upper-lower_)/sizes_.template cast<Float>();
		half_width_ = width_ / 2.0;
		lower_cen_ = lower_ + half_width_;
	}

	Index
	size() const { 
		return sizes_.prod()*2;
	}

	Floats 
	operator[](
		Index index
	) const {
		bool odd = index & 1;
		Indices indices = ( (index>>1) / sizes_prefsum_ ) % sizes_;
		return lower_cen_ + width_ * indices.template cast<Float>() + (odd? half_width_ : 0);
	}

	Indices
	get_indices(
		Floats value,
		bool & odd
	) const {
		value = (value-lower_)/width_;
		Indices const indices = value.template cast<Index>();
		value = value - indices.template cast<Float>() - 0.5;
		Indices const corner_indices = indices - (value < 0);
		odd = (0.25 * DIM) < fabs( ( value.sign() * value ).sum() );
		return  odd ? corner_indices : indices;
	}

	Index
	operator[](
		Floats const & value
	) const {
		bool odd;
		Indices indices = get_indices(value,odd);
		Index index = (sizes_prefsum_*indices).sum();
		return (index<<1) + odd;
	}

	template<class Iiter>
	void neighbors(Index index, Iiter iter, bool edges=false, bool edges2=false) const {
		*iter++ = index;
		bool odd = index & 1;
		Indices indices = ( (index>>1) / sizes_prefsum_ ) % sizes_;
		// std::cout << indices << std::endl;
		for(Index i = 0; i < DIM; ++i){
			indices[i] += 1;
			// std::cout << indices << " " << i1 << std::endl;			
			if( (indices<sizes_).sum()==DIM )
				*iter++ = (sizes_prefsum_*indices).sum()<<1 | odd;
			indices[i] -= 2;
			// std::cout << indices << " " << i2 << std::endl;
			if( (indices<sizes_).sum()==DIM )
				*iter++ = (sizes_prefsum_*indices).sum()<<1 | odd;
			indices[i] += 1; // reset
		}
		odd = !odd;
		Index sodd = odd? -1 : 1;
		for(Index i = 0; i < (1<<DIM); ++i){
			Indices corner(indices);
			for(int d = 0; d < DIM; ++d) corner[d] += ((i>>d)&1)? sodd : 0;
			// std::cout << corner << std::endl;
			if( (corner<sizes_).sum()==DIM )
				*iter++ = (sizes_prefsum_*corner).sum()<<1 | odd;
		}
		if(edges){
			odd = !odd;
			for(Index i =   0; i < DIM-1; ++i){
			for(Index j = i+1; j < DIM  ; ++j){
				indices[i] += 1;
				indices[j] += 1; // +1,+1
				// std::cout << indices << " " << i1 << std::endl;			
				if( (indices<sizes_).sum()==DIM ) *iter++ = (sizes_prefsum_*indices).sum()<<1 | odd;
				indices[i] -= 2; // -1,+1
				// std::cout << indices << " " << i2 << std::endl;
				if( (indices<sizes_).sum()==DIM ) *iter++ = (sizes_prefsum_*indices).sum()<<1 | odd;
				indices[j] -= 2; // -1,-1
				// std::cout << indices << " " << i2 << std::endl;
				if( (indices<sizes_).sum()==DIM ) *iter++ = (sizes_prefsum_*indices).sum()<<1 | odd;
				indices[i] += 2; // +1,-1
				// std::cout << indices << " " << i2 << std::endl;
				if( (indices<sizes_).sum()==DIM ) *iter++ = (sizes_prefsum_*indices).sum()<<1 | odd;
				// reset
				indices[i] -= 1;
				indices[j] += 1;
			}}
		}
	}
};


template< int DIM, class Float, class Index = uint64_t >
struct Cubic {
	typedef util::SimpleArray<DIM,Index> Indices;
	typedef util::SimpleArray<DIM,Float> Floats;
	BOOST_STATIC_ASSERT(DIM > 2);

	Indices sizes_, sizes_prefsum_;
	Floats lower_, width_, lower_cen_, half_width_;

	Cubic(){}

	template<class Sizes>
	Cubic(
		Sizes const & sizes,
		Floats lower=Floats(0), 
		Floats upper=Floats(1)
	) :
		sizes_(sizes),
		lower_(lower)
	{
		for(size_t i = 0; i < DIM; ++i)
			sizes_prefsum_[i] = sizes_.prod(i);
		width_ = (upper-lower_)/sizes_.template cast<Float>();
		half_width_ = width_ / 2.0;
		lower_cen_ = lower_ + half_width_;
	}

	Index
	size() const { 
		return sizes_.prod();
	}

	Floats 
	operator[](
		Index index
	) const {
		Indices indices = ( index / sizes_prefsum_ ) % sizes_;
		return lower_cen_ + width_ * indices.template cast<Float>();
	}

	Indices
	get_indices(
		Floats value
	) const {
		value = (value-lower_)/width_;
		return value.template cast<Index>();
	}

	Index
	operator[](
		Floats const & value
	) const {
		Indices indices = get_indices(value);
		return (sizes_prefsum_*indices).sum();
	}

	template<class Iiter>
	void neighbors(Index index, Iiter iter, bool=false) const {
		Indices idx0 = ( index / sizes_prefsum_ ) % sizes_;
		Indices threes(1);
		for(int d = 1; d < DIM; ++d) threes[d] = 3*threes[d-1];
		for(int i = 0; i < threes[DIM-1]*3; ++i){
			Indices idx(idx0);
			for(int d = 0; d < DIM; ++d) idx[d] += ((i/threes[d]) % 3) - 1;
			// std::cout << i << " " << (idx-idx0).template cast<int>()+1 << std::endl;
			if( (idx<sizes_).sum()==DIM ) *iter++ = (sizes_prefsum_*idx).sum();
		}
	}
};

}}

#endif