#ifndef __HIPPO_HSTRING_H_
#define __HIPPO_HSTRING_H_

#include <stdlib.h>
#include <string>
#include <vector>

namespace Hippo{


//string strore in UTF8;
class HString{

	typedef enum{
    	TokenState_eBeforeKey,
    	TokenState_eInKey,
    	TokenState_eAfterKey,
    	TokenState_eInTag,
    	TokenState_eBeforeValue,
    	TokenState_eInValue,
    	TokenState_eIgnore,
    }token_state_e;

public:
	friend inline bool operator==( HString& a, HString& b);
	friend inline bool operator==( HString& a, std::string& b);
	friend inline bool operator==( HString& a, const char* b);
	friend inline bool operator<( const HString& a, const HString& b );
public:
	HString( );
	HString( const char* str ): m_str( str ){}
	HString( std::string& str ):m_str(str) { }

	HString& operator= ( const HString& str );
	HString& operator= ( const std::string& str );
	HString& operator= ( const char* s );
	HString& operator= ( int c );
	HString& operator+= ( const HString& str );
	HString& operator+= ( const std::string& str );
	HString& operator+= ( const char* s );
	HString& operator+= ( char c );
	HString& operator+= ( int c );
	HString& format( );
	char& operator[]( size_t pos ) { return m_str[pos]; }
	int toInt32( ) { return atoi( m_str.c_str( ) );}

    HString& format(const char *pFormat, ...);

	//parse key=['|"| ]Value['|"| ] and return the in aKey and aValue.
	int tokenize( /*out*/ std::vector<HString>& aKey, /*out*/ std::vector<HString>& aValue, int aTag = '=' );
	int ioctlCommandSplit( HString& aInput, HString& aCmd, HString& aParam );

	bool equalIgnoringCase( const HString&, size_t n = 0xffffffff );
	bool equalIgnoringCase( const std::string&, size_t n = 0xffffffff );
	bool equalIgnoringCase( const char*, size_t n = 0xffffffff );

	void clear( ) { m_str.clear( ); }
	HString& erase( size_t aStartPos, size_t aEraseNum );
	HString& append( const char c ){ m_str += c; return *this; }
	const char* c_str( ) { return m_str.c_str();}
    size_t length( ) { return m_str.length();}
	bool empty( ) { return m_str.empty(); }
	int compare(const char* str){ return m_str.compare(str); }

	std::string::size_type find(const char* str){return m_str.find(str);}	//WZW modified to fix pc-lint warning 568
	HString substr(const size_t off, const size_t count);
	std::string::const_iterator begin(){return m_str.begin();}
	std::string::const_iterator end(){return m_str.end();}


private:
	std::string& impl( ) { return m_str; }
	std::string m_str; //the storage of HString.
};

inline HString& operator+( HString& str1, HString& str2 )
{
	str1 += str2;
	return str1;
}
inline HString& operator+( HString& str1, const char* s1 )
{
	str1 += s1;
	return str1;
}
inline HString& operator+( HString& str1, int c1 )
{
	str1 += c1;
	return str1;
}
inline bool operator==( HString& a, HString& b)
{
	return !(a.impl().compare( b.impl() ) );
}
inline bool operator==( HString& a,  std::string& b)
{
	return !( a.impl().compare( b ) );
	return true;
}
inline bool operator==( HString& a, const char* b)
{
	return !(a.impl().compare( b ) );
	return true;
}
inline bool operator< ( const HString& a, const HString& b )
{
	return a.m_str < b.m_str ;
}
}
#endif

