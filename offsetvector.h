#ifndef OFFSETVECTOR_H
#define OFFSETVECTOR_H

#include <vector>
#include <algorithm>

namespace offset
{

template <typename T>
class OffsetVector : private std::vector<T>
{
private:
	size_t mn = 0;
	T defaultValue;

public:
	using std::vector<T>::front;
	using std::vector<T>::back;
	
	using std::vector<T>::iterator;
	using std::vector<T>::begin;
	using std::vector<T>::end;
	
	using std::vector<T>::rbegin;
	using std::vector<T>::rend;

	using std::vector<T>::const_iterator;
	using std::vector<T>::cbegin;
	using std::vector<T>::cend;
	
	using std::vector<T>::empty;
	
	using std::vector<T>::resize;
	using std::vector<T>::size;

	using std::vector<T>::data;

	// empty constructor
	OffsetVector( const T& defaultValue=0 );

	// constructor
	OffsetVector( const size_t col, const size_t s, const T& defaultValue=0 );

	// copy constructor
	OffsetVector( const OffsetVector<T>& other );

	// iterator constructor
	template <typename iterator>
	OffsetVector( const size_t col, iterator begin, iterator end, const T& defaultValue=0 ) : 
		mn(col), defaultValue(defaultValue), std::vector<T>( begin, end ) {}

	// destructor
	~OffsetVector();

	// copy assignment
	OffsetVector<T>& operator=( const OffsetVector<T>& other );
	// move assignment
	OffsetVector<T>& operator=( OffsetVector<T>&& other ) noexcept;

	void clear()
	{
		mn = 0;
		std::vector<T>::clear();
	}

	size_t min() const { return mn; }
	size_t max() const { return mn + this->size() -1; }

	bool is_in( const size_t col ) const;

	/* get value currently stored in column col,
		if no value is stored there then return defaultValue */
	T get( const size_t col, const T& defaultValue ) const;
	T get( const size_t col ) const;


	/* set the value in column col, if col is out of range then create it */
	void set( const size_t col, const T val, const T& defaultValue );
	void set( const size_t col, const T val );
};

/*template <typename T>
template <typename iterator>
OffsetVector<T>::OffsetVector( const size_t col, iterator begin, iterator end )
{

}*/

// empty constructor
template <typename T>
OffsetVector<T>::OffsetVector( const T& defaultValue ) : defaultValue(defaultValue), std::vector<T>() {}

// constructor
template <typename T>
OffsetVector<T>::OffsetVector( const size_t col, const size_t s, const T& defaultValue ) :
	mn(col), std::vector<T>( s, defaultValue ) {}

// copy constructor
template <typename T>
OffsetVector<T>::OffsetVector( const OffsetVector<T>& other ) : mn( other.mn ), std::vector<T>( other ) {}

// destructor
template <typename T>
OffsetVector<T>::~OffsetVector() {}

// copy assignment
template <typename T>
OffsetVector<T>& OffsetVector<T>::operator=( const OffsetVector<T>& other )
{
	mn = other.mn;
	defaultValue = other.defaultValue;
	std::vector<T>::operator=(other);
}

// move assignment
template <typename T>
OffsetVector<T>& OffsetVector<T>::operator=( OffsetVector<T>&& other ) noexcept
{
	mn = std::move(other.mn);
	defaultValue = std::move(other.defaultValue);
	std::vector<T>::operator=( std::move(other) );

	return *this;
}

template <typename T>
bool OffsetVector<T>::is_in( const size_t col ) const 
{ 
	return col >= min() && col <= max(); 
}

template <typename T>
T OffsetVector<T>::get( const size_t col, const T& defaultValue ) const
{
	if( col < min() || col > max() || empty() ) return defaultValue;

	return (*this)[ col - mn ];
}

template <typename T>
T OffsetVector<T>::get( const size_t col ) const
{
	return get( col, defaultValue );
}



template <typename T>
void OffsetVector<T>::set( const size_t col, const T val, const T& defaultValue )
{
	/* if val is the default value then don't both actually saving anything */
	if( val == defaultValue ) return;

	/* column is greater than current max
		resize vector to fit all fill new space with defaultValue */
	if( col > max() )
	{
		this->resize( col -mn +1, defaultValue );
	}
	/* column is less than current min
		resize vector to fit, move current contents to back of vector,
		fill front of vector with defaultValue */
	else if( col < min() )
	{
		const size_t increase = min() - col;
		const size_t originalSize = this->size();

		this->resize( this->size() + increase );

		std::move_backward( this->begin(), next(this->begin(),originalSize), 
								 this->end() );

		fill( next(this->begin()), next(this->begin(),increase), defaultValue );

		mn = col;
	}
	/* vector is currently empty,
		resize to fit one element */
	else if( this->empty() )
	{
		mn = col;
		this->resize( 1 );
	}

	(*this)[ col - mn ] = val;
}

template <typename T>
void OffsetVector<T>::set( const size_t col, const T val )
{
	return set( col, val, defaultValue );
}

}

#endif