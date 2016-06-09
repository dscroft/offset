#ifndef OFFSETSTORE_H
#define OFFSETSTORE_H

#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <iostream>

namespace offset
{

template <typename T>
class OffsetVector : public std::vector<T>
{
private:
	size_t mn = 0;

public:
	size_t min() const { return mn; }
	size_t max() const { return mn + this->size() -1; }

	/* get value currently stored in column col,
		if no value is stored there then return defaultValue */
	T get( size_t col, T defaultValue=0 ) const;

	/* set the value in column col, if col is out of range then create it */
	void set( size_t col, T val, T defaultValue=0 );
};

template <typename T>
class OffsetMatrix
{
public:
	typedef OffsetVector<T> Row;
	std::vector< std::unique_ptr< Row > > rows;
	size_t mn = 0;

	/* return a reference to the row requested. takes parameter row which is the desired 
		row number. if requested row does not exist then it will be created first */
	Row& get_row( size_t row );

	/* no bounds checking, 
		only use if row >= min() && row <= max() && !empty() */
	const Row& get_row( size_t row ) const;

public:
	T defaultValue;

	OffsetMatrix( T defVal=0 ) : defaultValue(defVal) {}

	/* get the number of rows, the min/max row numbers from the matrix */
	size_t size() const;
	size_t min() const;
	size_t max() const;

	bool empty() const;

	bool clear();

	void shrink_to_fit();

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
	bool pickle( std::string filename ) const;

	/* load matrix from filenames, assumes file name is a pickle() created file.
		returns true if error, false if success. 
		will overwrite curret Matrix contents */
	bool unpickle( std::string filename );
};

template <typename T>
T OffsetVector<T>::get( size_t col, T defaultValue ) const
{
	if( col < min() || col > max() || this->empty() ) return defaultValue;

	return (*this)[ col - mn ];
}

template <typename T>
void OffsetVector<T>::set( size_t col, T val, T defaultValue )
{
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
typename OffsetMatrix<T>::Row& OffsetMatrix<T>::get_row( size_t row )
{	
	static const auto func = 
		[](std::unique_ptr<Row> &i){ i = std::unique_ptr<Row>(new Row); };

	/* if row is greater than current max,
		resize rows to fit and initialise the unique_ptrs */
	if( row > max() )
	{	
		const size_t originalSize = size();

		rows.resize( row - min() +1 );

		for_each( next(rows.begin(),originalSize), rows.end(), func );
	}
	/* if row is less than the current min,
		resize rows to fit, move contents of front of vector to 
		the back (move() is fast and unique_ptrs safe), initialise
		the new unique_ptrs */
	else if( row < min() )
	{
		const size_t increase = min() - row;
		const size_t originalSize = size();

		rows.resize( size() + increase );

		std::move_backward( rows.begin(), next(rows.begin(),originalSize), rows.end() );

		std::for_each( rows.begin(), next(rows.begin(),increase), func );
		
		mn = row;
	}
	/* if is empty resize to 1 and initialise the new unique_ptr */
	else if( empty() )
	{
		rows.resize(1);
		for_each( rows.begin(), rows.end(), func );
		mn = row;
	}

	// return reference to the correct Row
	return *(rows[row - min()]);
}

template <typename T>
const typename OffsetMatrix<T>::Row& OffsetMatrix<T>::get_row( size_t row ) const
{
	return *(rows[row - min()]);
}

template <typename T>
size_t OffsetMatrix<T>::size() const { return rows.size(); }

template <typename T>
size_t OffsetMatrix<T>::min() const { return mn; }

template <typename T>
size_t OffsetMatrix<T>::max() const { return min() + size() -1; };

template <typename T>
bool OffsetMatrix<T>::empty() const { return rows.empty(); }

template <typename T>
bool OffsetMatrix<T>::clear()
{
	rows.clear();
	mn = 0;
}

template <typename T>
void OffsetMatrix<T>::shrink_to_fit()
{
	rows.shrink_to_fit();
	for_each( rows.begin(), rows.end(), []( std::unique_ptr<Row> &r ){ r->shrink_to_fit(); } );
}

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
bool OffsetMatrix<T>::pickle( std::string filename ) const
{
	std::ofstream file( filename, std::ios::binary );
	if( !file.good() ) return true;

	// write the minimum row number and the number of rows
	const size_t rowsMin = min();
	const size_t rowsNum = size();
	file.write( (char*)&rowsMin, sizeof(rowsMin) );
	file.write( (char*)&rowsNum, sizeof(rowsNum) );

	// for each row in order
	for( const std::unique_ptr<Row> &r : rows )
	{
		// write the minimum column number and the number of columns
		const size_t colsMin = r->min();
		const size_t colsNum = r->size();
		file.write( (char*)&colsMin, sizeof(colsMin) );
		file.write( (char*)&colsNum, sizeof(colsNum) );

		// write all the column values as a contiguous block
		const T *buffer = &*(r->begin()); // get pointer to internal vector buffer
		file.write( (char*)buffer, sizeof(T)*colsNum ); // write entire row in one go
	}

	file.close();
	
	return false;
}

template <typename T>
bool OffsetMatrix<T>::unpickle( std::string filename )
{
	std::ifstream file( filename, std::ios::binary );
	if( !file.good() ) return true;

	clear(); // make sure the matrix is empty first
	
	// read the minimum row number and the number of rows
	size_t rowsMin, rowsNum;
	file.read( (char*)&rowsMin, sizeof(rowsMin) );
	file.read( (char*)&rowsNum, sizeof(rowsNum) );

	get_row( rowsMin );
	get_row( rowsMin + rowsNum -1 );

	for( std::unique_ptr<Row> &r : rows )
	{
		// read the minimum column number and the number of columns
		size_t colsMin, colsNum;
		file.read( (char*)&colsMin, sizeof(colsMin) );
		file.read( (char*)&colsNum, sizeof(colsNum) );

		r->set( colsMin, defaultValue ); // easiest way to set the min value
		r->resize( colsNum, defaultValue );

		const T *buffer = &*(r->begin()); // get pointer to internal vector buffer
		file.read( (char*)buffer, sizeof(T)*colsNum ); // read entire row in one go
	}

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

};

#endif
