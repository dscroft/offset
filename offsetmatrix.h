#ifndef OFFSETSTORE_H
#define OFFSETSTORE_H

#include <algorithm>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "offsetvector.h"

#if defined(BOOST)
	#include <boost/progress.hpp>
#endif

namespace offset
{

template <typename T>
class OffsetMatrix : private std::vector< OffsetVector<T> >
{
public:
	typedef OffsetVector<T> Row;
	size_t mn = 0;

	/* return a reference to the row requested. takes parameter row which is the desired 
		row number. if requested row does not exist then it will be created first */
	Row& get_row( size_t row );

	/* no bounds checking, 
		only use if row >= min() && row <= max() && !empty() */
	const Row& get_row( size_t row ) const;

public:
	using std::vector< OffsetVector<T> >::begin;
	using std::vector< OffsetVector<T> >::const_iterator;
	using std::vector< OffsetVector<T> >::end;
	using std::vector< OffsetVector<T> >::empty;
	using std::vector< OffsetVector<T> >::iterator;
	using std::vector< OffsetVector<T> >::resize;
	using std::vector< OffsetVector<T> >::size;

	T defaultValue = 0;

	OffsetMatrix( const T& defaultValue ) : defaultValue(defaultValue) {}

	/* get the number of rows, the min/max row numbers from the matrix */
	size_t min() const;
	size_t max() const;
	size_t values() const;
	size_t count( const T &val ) const;

	bool clear();

	//void shrink_to_fit();

	/* set the value at row, col,
		will resize the matrix is row, col doesn't currently exist */
	void set( size_t row, size_t col, T val );

	/* returns the value at row, col,
		if row, col doesn't exist then will return defaultValue */
	T get( size_t row, size_t col ) const;
	T operator()( size_t row, size_t col ) const;

	/* writes the currect Matrix as a binary file to filename.
		returns true if error, false if success.

		format is:
			[minimum row number]           [number of rows]
			 size_t                         size_t

			[minimum col number in 1st row][number of columns in 1st row]
			 size_t                         size_t
			[1st row, 1st col value][1st row, 2nd col value]...[1st row, last col value]
			 typedef T               typedef T                  typedef T

			[minimum col number in 2nd row][number of columns in 2nd row]
			 size_t                         size_t
			[2nd row, 1st col value][2nd row, 2nd col value]...[2nd row, last col value]
			 typedef T               typedef T                  typedef T

			...

			[minimum col number in last row][number of columns in last row]
			 size_t                         size_t
			[last row, 1st col value][last row, 2nd col value]...[last row, last col value]
			 typedef T                typedef T                   typedef T	
	*/
	bool save( std::string filename, bool verbose=false ) const;

	/* load matrix from filenames, assumes file name is a pickle() created file.
		returns true if error, false if success. 
		will overwrite curret Matrix contents */
	bool load( std::string filename, bool verbose=false, std::ostream& output=std::cout );
};

template <typename T>
typename OffsetMatrix<T>::Row& OffsetMatrix<T>::get_row( const size_t row )
{	
	/* if row is greater than current max,
		resize rows to fit and initialise the unique_ptrs */
	if( row > max() )
	{	
		const size_t originalSize = size();

		resize( row - min() +1 );
	}
	/* if row is less than the current min,
		resize rows to fit, move contents of front of vector to 
		the back (move() is fast and unique_ptrs safe), initialise
		the new unique_ptrs */
	else if( row < min() )
	{
		const size_t increase = min() - row;
		const size_t originalSize = size();

		resize( size() + increase );

		std::move_backward( begin(), next(begin(),originalSize), end() );
		
		mn = row;
	}
	/* if is empty resize to 1 and initialise the new unique_ptr */
	else if( empty() )
	{
		resize(1);
		mn = row;
	}

	// return reference to the correct Row
	return (*this)[row - min()];
}

template <typename T>
const typename OffsetMatrix<T>::Row& OffsetMatrix<T>::get_row( size_t row ) const
{
	return (*this)[row - min()];
}

template <typename T>
size_t OffsetMatrix<T>::min() const { return mn; }

template <typename T>
size_t OffsetMatrix<T>::max() const { return min() + size() -1; };

template <typename T>
size_t OffsetMatrix<T>::values() const
{
	return accumulate( begin(), end(), (size_t)0, 
		[]( size_t count, const Row &r ) { return count + r.size(); } );
}

template <typename T>
size_t OffsetMatrix<T>::count( const T &val ) const
{
	size_t c = 0;
	for( const Row &r : *this )
		c += std::count( r.begin(), r.end(), val );
	
	return c;
}


template <typename T>
bool OffsetMatrix<T>::clear()
{
	std::vector< OffsetVector<T> >::clear();
	mn = 0;
}

/*template <typename T>
void OffsetMatrix<T>::shrink_to_fit()
{
	rows.shrink_to_fit();
	for_each( rows.begin(), rows.end(), []( std::unique_ptr<Row> &r ){ r->shrink_to_fit(); } );
}*/

template <typename T>
void OffsetMatrix<T>::set( size_t row, size_t col, T val )
{
	Row &r = get_row( row );
	r.set( col, val, defaultValue );	
}

template <typename T>
T OffsetMatrix<T>::get( size_t row, size_t col ) const 
{
	if( row < min() || row > max() || empty() )
		return defaultValue;
	
	const Row &r = get_row( row );
	return r.get( col );
}

template <typename T>
T OffsetMatrix<T>::operator()( size_t row, size_t col ) const
{
	return get( row, col );
}

template <typename T>
bool OffsetMatrix<T>::save( std::string filename, bool verbose ) const
{
	std::ofstream file( filename, std::ios::binary );
	if( !file.good() ) return true;

	// write the minimum row number and the number of rows
	const size_t total = values();
	const size_t rowsMin = min();
	const size_t rowsNum = size();
	file.write( (char*)&total, sizeof(total) );
	file.write( (char*)&rowsMin, sizeof(rowsMin) );
	file.write( (char*)&rowsNum, sizeof(rowsNum) );

	size_t progress = 0;

	// for each row in order
	for( const Row &r : *this )
	{
		if( verbose )
		{
			std::cout << ++progress << "\r" << std::flush;
		}

		// write the minimum column number and the number of columns
		const size_t colsMin = r.min();
		const size_t colsNum = r.size();
		file.write( (char*)&colsMin, sizeof(colsMin) );
		file.write( (char*)&colsNum, sizeof(colsNum) );

		// write all the column values as a contiguous block
		//const T *buffer = &*(r.begin()); // get pointer to internal vector buffer
		//file.write( (char*)buffer, sizeof(T)*colsNum ); // write entire row in one go

		file.write( (char*)r.data(), sizeof(T)*colsNum );
	}

	if( verbose ) std::cout << std::endl;

	file.close();
	
	return false;
}

template <typename T>
bool OffsetMatrix<T>::load( std::string filename, bool verbose, std::ostream& output )
{
	std::ifstream file( filename, std::ios::binary );
	if( !file.good() ) return true;

	clear(); // make sure the matrix is empty first
	
	// read the minimum row number and the number of rows
	size_t total, rowsMin, rowsNum;
	file.read( (char*)&total, sizeof(total) );
	file.read( (char*)&rowsMin, sizeof(rowsMin) );
	file.read( (char*)&rowsNum, sizeof(rowsNum) );

	get_row( rowsMin );
	get_row( rowsMin + rowsNum -1 );

	auto read_row = [&file]( Row &r, T &defaultValue )
	{
		// read the minimum column number and the number of columns
		size_t colsMin, colsNum;
		file.read( (char*)&colsMin, sizeof(colsMin) );
		file.read( (char*)&colsNum, sizeof(colsNum) );

		r.set( colsMin, defaultValue ); // easiest way to set the min value
		r.resize( colsNum, defaultValue );

		file.read( (char*)r.data(), sizeof(T)*colsNum );

		return colsNum;
	};

	if( verbose )
	{
#if defined(BOOST)
		boost::progress_display show_progress( total, output );
#else
		size_t show_progress = 0;
#endif
		for( Row &r : *this )
			show_progress += read_row( r, defaultValue );
	}
	else
		for( Row &r : *this )
			read_row( r, defaultValue );

	//assert( values() == total );

	return false;
}

template <typename T>
std::ostream &operator<<( std::ostream &output, const OffsetMatrix<T> &m )
{
	output << "matrix: " << std::endl;

	if( m.empty() ) return output;

	for( size_t row=m.min(); row<=m.max(); ++row )
	{
		const typename OffsetMatrix<T>::Row &r = m.get_row(row);

		output << std::setw(2) << row << " (" << r.size() << "): ";

		for( size_t col=r.min(); !r.empty() && col<=r.max(); ++col )
		{
			output << col << "=" << (int)r.get(col) << ", ";
		}

		output << std::endl;
	}

    return output;
}

}

#endif