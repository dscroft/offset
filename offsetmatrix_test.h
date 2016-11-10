#include <cxxtest/TestSuite.h>
#include "offsetmatrix.h"

using namespace offset;

class OffsetMatrixTest: public CxxTest::TestSuite
{
private:
	std::vector<int> testValues;
	size_t startingCol;
	int defaultValue;

public:
	void setUp()
	{
		testValues = {1,2,3,4,5,6,99};
		startingCol = 42;
		defaultValue = 999;
	}

	void tearDown()
	{

	}

	void test_test()
	{
		TS_ASSERT_EQUALS( 1, 1 );
	}

	void test_empty()
	{
		OffsetMatrix<int> store( defaultValue );

		TS_ASSERT( store.empty() );
	}

	/*void ttest_size()
	{
		auto values = {1,2,3,4,5,10};

		OffsetVector<int> vect;

		for( auto i : values )
		{
			vect.set( i, i );
			TS_ASSERT_EQUALS( i, vect.size() );
		}
	}

	void ttest_constructor()
	{
		const int defaultValue = 123;
		const size_t startingCol = 10;
		const size_t size = 42;

		OffsetVector<int,defaultValue> vect( startingCol, size );

		TS_ASSERT_EQUALS( startingCol, vect.min() );

		TS_ASSERT_EQUALS( size, vect.size() );

		for( auto i : vect )
			if( i != defaultValue )
			{
				TS_ASSERT_EQUALS( defaultValue, i );
				break;
			}
	}

	void ttest_iterator_constructor()
	{
		OffsetVector<int> vect( startingCol, testValues.begin(), testValues.end() );

		// test that all values got assigned
		TS_ASSERT_EQUALS( testValues.size(), vect.size() );

		// test that all values are correct
		auto ait=testValues.begin();
		auto bit=vect.begin();
		for( ; ait!=testValues.end(); ++ait, ++bit )
		{
			TS_ASSERT_EQUALS( *ait, *bit );
		}

	}

	void ttest_copy_constuctor()
	{
		OffsetVector<int> aVect( startingCol, testValues.begin(), testValues.end() );

		OffsetVector<int> bVect( aVect );

		OffsetVector<int> cVect;
		cVect = aVect;

		TS_ASSERT_EQUALS( aVect.size(), bVect.size() );
		TS_ASSERT_EQUALS( aVect.size(), cVect.size() );

		for( auto ait=aVect.begin(), bit=bVect.begin(), cit=cVect.begin(); 
			ait!=aVect.end(); ait++, bit++, cit++ )
		{
			TS_ASSERT_EQUALS( *ait, *bit );
			TS_ASSERT_EQUALS( *ait, *cit );
		}
	}

	void ttest_get()
	{
		OffsetVector<int> vect( startingCol, testValues.begin(), testValues.end() );

		for( size_t i=0; i<testValues.size(); ++i )
		{
			TS_ASSERT_EQUALS( testValues[i], vect.get(startingCol+i) );
		}
	}

	void ttest_set()
	{
		struct TestCase { size_t pos; int value; };
		std::vector< TestCase > testCases = { {42,42}, {20,10}, {40,1}, {42,69} };
	
		OffsetVector<int,defaultValue> vect;

		// set the test case values and immediately check that the value was recorded correctly
		for( auto testcase : testCases )
		{
			vect.set( testcase.pos, testcase.value );
			TS_ASSERT_EQUALS( vect.get(testcase.pos), testcase.value );
		}

		// check everything in the entire range that the test cases covered to make sure that the default value entries are also correct
		std::pair<size_t,size_t> range = accumulate( next(testCases.begin(),1), testCases.end(), 
												 	std::make_pair(testCases.begin()->pos, testCases.begin()->pos),
													[]( std::pair<size_t,size_t> &p, TestCase &c )
													{
														return std::make_pair( std::min(p.first,c.pos), std::max(p.second,c.pos) );
													} );

		// extend the range a bit to test either side of the set values but 
		range.first -= std::min( range.first, size_t(10) ); 
		range.second += std::min( std::numeric_limits<size_t>::max() - range.second, size_t(10) );

		for( size_t pos=range.first; pos<=range.second; ++pos )
		{
			auto testcase = find_if( testCases.rbegin(), testCases.rend(), [&pos]( TestCase &c ){ return pos == c.pos; } );
			const int expectedValue = testcase == testCases.rend() ? defaultValue : testcase->value;

			TS_ASSERT_EQUALS( expectedValue, vect.get(pos) );
		}
	}*/
};
