#ifndef INCLUDED_objective_storage_RotamerScores_HH
#define INCLUDED_objective_storage_RotamerScores_HH

#include "scheme/util/SimpleArray.hh"
#include "scheme/util/assert.hh"
#include <boost/lexical_cast.hpp>

#include <vector>

namespace scheme { namespace objective { namespace storage {

struct Empty {};

template<
	class _Data = uint16_t,
	int _RotamerBits = 9,
	int _Divisor = -13
>
struct RotamerScore {
 	BOOST_STATIC_ASSERT( _Divisor < 0 );
 	typedef RotamerScore< _Data, _RotamerBits, _Divisor > THIS;
	typedef _Data Data;
	static const int RotamerBits = _RotamerBits;
	static const int Divisor = _Divisor;
	static const int ScoreBits = sizeof(_Data)*8 - _RotamerBits;
	static const Data one = 1;
	static const Data RotamerMask = (( one << RotamerBits ) - one );
	static const bool UseSat = false;

	Data data_;
	RotamerScore(): data_( RotamerMask ) {}
	RotamerScore( Data data ): data_(data) {}
	RotamerScore( Data rot, float score ){
		Data sdat = score * _Divisor;
		assert( sdat < ( one<<ScoreBits ) );
		assert( rot < (one<<RotamerBits) );
		data_ = rot | (sdat<<RotamerBits);
	}
	float score() const { return data2float( get_score_data() ); }
	Data  rotamer() const { return data_ & RotamerMask; }
	void  set_score( float score ){ set_score_data( float2data( score ) ); }
	void  set_rotamer( Data rot ){ assert(rot < one<<RotamerBits); data_ = (data_& (~RotamerMask)) | rot; }

	void  set_score_data( Data sd ){ assert(sd < (one<<ScoreBits)); data_ = rotamer() | ( sd << RotamerBits ); }
	Data  get_score_data() const { return data_ >> RotamerBits; }
	static float divisor() { return _Divisor; }

	static float data2float( Data data ){ return float(data)/_Divisor; }
	static Data  float2data( float f ){ return Data( f*_Divisor ); }

	bool do_i_satisfy_anything() const {
		return false;
	}

	bool operator < ( THIS const & other ) const { return data_ > other.data_; } // reverse so low score is low
	bool operator== ( THIS const & other ) const { return data_ == other.data_; }
	bool operator!= ( THIS const & other ) const { return data_ != other.data_; }
	bool operator== ( Data const & other ) const { return data_ == other; }

	bool empty() const { return data_ == RotamerMask; }

 	void set_or_merge( THIS const & other, bool force ){
 		if( other < *this || force ){
 			*this = other;
 		}
 	}

	static std::string name() {
		static std::string const name = "RotamerScore< "
			 + std::string("data_size ") + boost::lexical_cast<std::string>(sizeof(Data)) + ", "
			 + boost::lexical_cast<std::string>(THIS::RotamerBits) + ", "
			 + boost::lexical_cast<std::string>(THIS::Divisor)	 +" >";
		return name;
	}

} __attribute__((packed));
template< class Data, int RBits, int Div > int  const RotamerScore<Data,RBits,Div>::RotamerBits;
template< class Data, int RBits, int Div > int  const RotamerScore<Data,RBits,Div>::Divisor;
template< class Data, int RBits, int Div > int  const RotamerScore<Data,RBits,Div>::ScoreBits;
template< class Data, int RBits, int Div > bool const RotamerScore<Data,RBits,Div>::UseSat;
template< class Data, int RBits, int Div >
std::ostream & operator << ( std::ostream & out, RotamerScore<Data,RBits,Div> const & val ){
	out << val.rotamer() << "<" << val.score() << ">";
	return out;
}

template<class _Dat=uint8_t >
struct SatisfactionDatum {
	using Dat = _Dat;
	Dat data_;
	static int const MAXVAL = std::numeric_limits<Dat>::max();
	SatisfactionDatum() : data_(MAXVAL) {}
	SatisfactionDatum( Dat d ) : data_(d) {}
	bool empty() const { return data_ == MAXVAL; }
	bool not_empty() const { return data_ != MAXVAL; }
	int target_sat_num() const { return (int)data_; }
	int rotamer_sat_num() const { return 0; }
	bool operator==(SatisfactionDatum const & o) const { return data_==o.data_; }
} __attribute__((packed));
template<class D >
std::ostream & operator << ( std::ostream & out, SatisfactionDatum<D> const & val ){
	out << (int)val.data_;
	return out;
}


template<
	class _Data = uint16_t,
	int _RotamerBits = 9,
	int _Divisor = -13,
	class _SatDatum=SatisfactionDatum<uint8_t>,
	int _NSat=2
>
struct RotamerScoreSat : public RotamerScore<_Data,_RotamerBits,_Divisor> {
	typedef RotamerScoreSat<_Data,_RotamerBits,_Divisor,_SatDatum,_NSat> THIS;
	typedef RotamerScore<_Data,_RotamerBits,_Divisor> BASE;
	typedef _Data Data;
	typedef _SatDatum SatDatum;
	static const bool UseSat = true;
	static const int RotamerBits = _RotamerBits;
	static const int Divisor = _Divisor;
	static const int NSat = _NSat;
	util::SimpleArray<NSat,SatDatum> sat_data_;
	RotamerScoreSat() : BASE() {}
	RotamerScoreSat( Data data ) : BASE( data ) {}
	RotamerScoreSat( Data rot, float score, int sat1=-1, int sat2=-1 ) : BASE(rot,score)
	{
		if( sat1 < 0 || NSat < 1 ) return;
		ALWAYS_ASSERT( sat1 < SatDatum::MAXVAL );
		sat_data_[0].data_ = (typename SatDatum::Dat)sat1;
		if( sat2 >= 0 && NSat >= 2 ){;
			ALWAYS_ASSERT( sat2 < SatDatum::MAXVAL );
			sat_data_[1].data_ = (typename SatDatum::Dat)sat2;
		}
		// std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " " << sat1 << "/" << sat_data_[0] << " " << sat2 << "/" << sat_data_[1] << std::endl;
	}
	static std::string name()
	{
		static std::string const name = std::string("RotamerScoreSat< dsize=")
		     + boost::lexical_cast<std::string>(sizeof(Data)) + ", "
			 + "rbit=" + boost::lexical_cast<std::string>(BASE::RotamerBits) + ", "
			 + "div=" + boost::lexical_cast<std::string>(BASE::Divisor)	 + ", "
			 + std::string("nsat=") + boost::lexical_cast<std::string>(sizeof(SatDatum)*NSat)	  +" >";
		return name;
	}
	bool do_i_satisfy_anything() const {
		for( int isat = 0; isat < NSat; ++isat ){
			if (sat_data_[isat].not_empty()){
				//return sat_data_[isat].target_sat_num();
				return true;
			}
		}
		return false;
	}
	template< class Array >
	void get_sat_groups_raw( Array & sat_groups_out ) const {
		for( int isat = 0; isat < NSat; ++isat ){
			sat_groups_out[isat] = sat_data_[isat].target_sat_num();
		}
	}
	void get_sat_groups( std::vector<int> & sat_groups_out ) const
	{
		for( int isat = 0; isat < NSat; ++isat ){
			if( sat_data_[isat].not_empty() ){
				sat_groups_out.push_back( sat_data_[isat].target_sat_num() );
				// std::cout << "Sat " << isat << " " << sat_data_[isat].target_sat_num() << std::endl;
			} else {
				// std::cout << "Sat " << isat << " Empty!!" << std::endl;
			}
		}
	}
    
    int get_requirement_num( ) const {
        if( NSat >= 1 && sat_data_[0].not_empty() ){
            return sat_data_[0].target_sat_num();
        } else {
            return -1;
        }
    }
    
	void mark_sat_groups( std::vector<bool> & sat_groups_mask ) const {
		for( int isat = 0; isat < NSat; ++isat ){
			if( sat_data_[isat].not_empty() ){
				int val = sat_data_[isat].target_sat_num();
				if (val < 0 || val >= sat_groups_mask.size()) {
					//std::cout << "Sat" << val << std::endl;
                    return;
				}
				sat_groups_mask[ sat_data_[isat].target_sat_num() ] = true;
			}
		}
	}
	bool is_new_sat( SatDatum const & sd ) const
	{
		if( sd.empty() ) return false;
		for( int i = 0; i < NSat; ++i ){
			if( sat_data_[i] == sd ) return false;
		}
		return true;
	}
	void set_or_merge( THIS const & other, bool force )
	{
		if( this->empty() || force ){
			*this = other;
		} else {
			// merge sat data iff same rotamer, mostly for bounding grids
			if( other.rotamer() == this->rotamer() ){
				int osat = 0;
				for( int isat = 0; isat < NSat; ++isat ){
					if( sat_data_[isat].empty() ){
						while( osat < NSat && ! is_new_sat(other.sat_data_[osat]) ) ++osat;
						if( osat >= NSat ) break;
						sat_data_[isat] = other.sat_data_[osat];
						++osat;
					}
				}
			}
			BASE::set_or_merge( other, force );
		}
	}

	bool operator==(THIS const & o) const { return this->data_==o.data_ && this->sat_data_==o.sat_data_; }
	bool operator!=(THIS const & o) const { return this->data_!=o.data_ || this->sat_data_!=o.sat_data_; }
	bool operator < ( THIS const & other ) const {
		int nsat=0,onsat=0;
		for( int i = 0; i < NSat; ++i ){
			 nsat += this->sat_data_[i].not_empty();
			onsat += other.sat_data_[i].not_empty();
		}
		if( nsat == onsat ) return this->data_ > other.data_;
		return nsat > onsat;
	} // reverse so low score is low


} __attribute__((packed));
template< class Data, int RBits, int Div, class Sat, int N > int  const RotamerScoreSat<Data,RBits,Div,Sat,N>::RotamerBits;
template< class Data, int RBits, int Div, class Sat, int N > int  const RotamerScoreSat<Data,RBits,Div,Sat,N>::Divisor;
template< class Data, int RBits, int Div, class Sat, int N > bool const RotamerScoreSat<Data,RBits,Div,Sat,N>::UseSat;
template< class Data, int RBits, int Div, class Sat, int N > int  const RotamerScoreSat<Data,RBits,Div,Sat,N>::NSat;
template< class Data, int RBits, int Div, class Sat, int N >
std::ostream & operator << ( std::ostream & out, RotamerScoreSat<Data,RBits,Div,Sat,N> const & val ){
	out << val.rotamer() << "<" << val.score() << ">";
	std::vector<int> sat;
	val.get_sat_groups(sat);
	for( int j = 0; j < sat.size(); ++j ){
		out << "," << sat[j];
	}
	out << "  ";

	return out;
}





template<
	int _N,
	class _RotamerScore = RotamerScore<>
>
struct RotamerScores {
	BOOST_STATIC_ASSERT(( _N > 0   ));
	BOOST_STATIC_ASSERT(( _N < 256 )); // arbitrary

	typedef _RotamerScore RotScore;
	typedef typename RotScore::Data Data;
	typedef RotamerScores< _N, RotScore > THIS;

	static int const N = _N;
	util::SimpleArray<N,RotScore> rotscores_;

	RotamerScores(){
		rotscores_.fill( RotScore::RotamerMask );
	}


	// void super_print( std::ostream & out, shared_ptr< RotamerIndex > rot_index_p ) const {
	// 	for( int i = 0; i < N; ++i ){
	// 		if ( rotscores_[i].empty()) break;

	// 		uint64_t irot = rotscores_[i].rotamer();


	// 		out << rot_index_p->resname( irot ) << " " << rotscores_[i].score() << std::endl;
	// 		// rotscores_[i].super_print( out, rot_index_p );
	// 	}
	// }
	// void add_rotamer( Data rot, float score ){
		// add_rotamer( RotScore(rot,score) );
	// }
	void add_rotamer( Data rot, float score, int sat1=-1, int sat2=-1, bool force=false )	{
		add_rotamer_impl< RotScore::UseSat >( rot, score, sat1, sat2, force );
	}
	void rotamer_sat_groups( int irot, std::vector<int> & sat_groups_out ) const	{
		rotamer_sat_groups_impl< RotScore::UseSat >( irot, sat_groups_out );
	}
	void mark_sat_groups( int irot, std::vector<bool> & sat_groups_mask ) const	{
		mark_sat_groups_impl< RotScore::UseSat >( irot, sat_groups_mask );
	}
	template<class Array>
	void get_sat_groups_raw( int irot, Array & a ) const	{
		get_sat_groups_raw_impl< RotScore::UseSat, Array >( irot, a );
	}
    int get_requirement_num( int irot ) const {
        return get_requirement_num_impl< RotScore::UseSat >( irot );
    }
	void add_rotamer( RotScore to_insert, bool force )
	{
		Data irot = to_insert.rotamer();
		int insert_pos = 0;
		RotScore worst( std::numeric_limits<Data>::max() );
		for( int i = 0; i < N; ++i ){
			// std::cout << " iter " << i << " " << rotscores_[i].score() << " " << rotscores_[i].rotamer() << " "
			          // << "cur " << rotscores_[i].data_ << " low " << worst.data_ << std::endl;
			// if rot already stored, this is the position we check
			if( rotscores_[i].rotamer() == irot ){
				insert_pos = i;
				// std::cout << "rotamer equal at " << i << std::endl;
				break;
			}
			// else we take the worst position
			if( worst < rotscores_[i] ){
				// std::cout << "worst is " << i << std::endl;
				worst = rotscores_[i];
				insert_pos = i;
			}
		}
		// std::cout << "insert_pos " << insert_pos << std::endl;
		// now insert if new val is better than worst stored val
		rotscores_[insert_pos].set_or_merge( to_insert, force );
	}
	template<int N2>
	void merge( RotamerScores<N2,RotScore> const & other, bool force=false )
	{
		for( int i = 0; i < N2; ++i ){
			if( other.empty(i) ) break;
			add_rotamer( other.rotscores_[i], force );
		}
	}
	float score_of_rotamer( int irot ) const
	{
		for( int i = 0; i < N; ++i ){
			if( rotscores_[i].rotamer() == irot ){
				return rotscores_[i].score();
			}
		}
		return 0.0f;
	}

	float score( int i ) const { assert(i<N); return rotscores_[i].score(); }
	Data rotamer( int i ) const { assert(i<N); return rotscores_[i].rotamer(); }
	bool do_i_satisfy_anything(int i) const { assert(i<N); return rotscores_[i].do_i_satisfy_anything(); }

	bool empty( int i ) const { return rotscores_[i].empty(); }

	static int maxsize(){ return _N; }

	int size() const { int i; for(i=0;i<_N;++i) if( rotscores_[i].empty() ) break; return i; }

	void sort_rotamers(){
		std::sort( rotscores_.begin(), rotscores_.end() );
	}

	bool is_sorted() const {
		for( int i = 1; i < _N; ++i )
			if( rotscores_[i] < rotscores_[i-1] )
				return false;
		return true;
	}

	int count_these_irots( int irot_low, int irot_high ) {
		int count = 0;
		for( int i = 0; i < N; ++i ){
			int rotamer = rotscores_[i].rotamer();
			if (rotamer < irot_low) continue;
			if (rotamer > irot_high) continue;
			count++;
		}
		return count;
	}

	static std::string name() {
		static std::string const name = std::string("RotamerScores< N=" )
		     + boost::lexical_cast<std::string>(_N) + ", "
			 + RotScore::name()	 +" >";
		return name;
	}

	bool operator==(THIS const & o) const { return rotscores_ == o.rotscores_; }
	bool operator!=(THIS const & o) const { return rotscores_ != o.rotscores_; }

	template< bool UseSat >	typename boost::enable_if_c< UseSat, void >::type
	add_rotamer_impl( Data rot, float score, int sat1, int sat2, bool force ){ add_rotamer( RotScore(rot,score,sat1,sat2), force ); }
	template< bool UseSat >	typename boost::disable_if_c< UseSat, void >::type
	add_rotamer_impl( Data rot, float score, int sat1, int sat2, bool force ){ add_rotamer( RotScore(rot,score), force ); }
	template< bool UseSat >	typename boost::enable_if_c< UseSat, void >::type
	rotamer_sat_groups_impl( int irot, std::vector<int> & sat_groups_out ) const { rotscores_[irot].get_sat_groups( sat_groups_out ); }
	template< bool UseSat >	typename boost::disable_if_c< UseSat, void >::type
	rotamer_sat_groups_impl( int irot, std::vector<int> & sat_groups_out ) const { return; }
	template< bool UseSat >	typename boost::enable_if_c< UseSat, void >::type
	mark_sat_groups_impl( int irot, std::vector<bool> & sat_groups_mask ) const { rotscores_[irot].mark_sat_groups( sat_groups_mask ); }
	template< bool UseSat >	typename boost::disable_if_c< UseSat, void >::type
	mark_sat_groups_impl( int irot, std::vector<bool> & sat_groups_mask ) const { return; }
	template< bool UseSat, class Array > typename boost::enable_if_c< UseSat, void >::type
	get_sat_groups_raw_impl( int irot, Array & a ) const { rotscores_[irot].get_sat_groups_raw( a ); }
	template< bool UseSat, class Array > typename boost::disable_if_c< UseSat, void >::type
	get_sat_groups_raw_impl( int irot, Array &   ) const { return; }

    template< bool UseSat > typename boost::enable_if_c< UseSat, int >::type
    get_requirement_num_impl( int irot ) const { return rotscores_[irot].get_requirement_num( ); }
    template< bool UseSat > typename boost::disable_if_c< UseSat, int >::type
    get_requirement_num_impl( int irot ) const { return -1; }

};

template< int N, class R >
std::ostream & operator << ( std::ostream & out, RotamerScores<N,R> const & val ){
	out << val.name() << "( ";
	for(int i = 0; i < val.size(); ++i){
		out << val.rotscores_[i] << " ";
	}
	out << ")";
	return out;
}


}}}

#endif
