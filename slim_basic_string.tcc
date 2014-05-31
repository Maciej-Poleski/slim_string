#include "slim_basic_string.hpp"

#include <boost/smart_ptr/allocate_shared_array.hpp>

namespace detail
{

template<typename CharT>
static std::size_t strlen ( const CharT *s )
{
    std::size_t result=0;
    while ( *s++!=CharT() ) {
        ++result;
    }
    return result;
}

template<typename size_type>
static size_type size_guard(size_type a, size_type b)
{
    if(a<=b)
	return 0;
    return a-b;
}
}

template<typename CharT,
	 typename Traits,
	 typename Allocator>
std::shared_ptr<typename slim_basic_string<CharT,Traits,Allocator>::data_t> 
slim_basic_string<CharT,Traits,Allocator>::allocate_data (
    const std::size_t size,
    const Allocator &allocator )
{
    typedef typename std::allocator_traits<Allocator>::template rebind_traits<CharT> allocator_traits;
    typedef typename allocator_traits::allocator_type allocator_type;
    allocator_type alloc(allocator);
    return std::allocate_shared<typename slim_basic_string<CharT,Traits,Allocator>::data_t> (
	alloc,
	nullptr,
	size,
	alloc);
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator>::slim_basic_string (
    size_type count,
    CharT ch,
    const Allocator &alloc ) : _data(allocate_data(count,alloc))
{
    _data->_chunk=boost::allocate_shared<const CharT[]>(_data->_allocator,count,ch);
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator>::slim_basic_string(
    slim_basic_string other,
    size_type pos,
    size_type count,
    const Allocator &allocator)
{
    if(pos==0 && count>=other._data->_length)
    {
	_data=other._data;
    } 
    else if(pos==0)
    {
	typedef typename std::allocator_traits<Allocator>::template rebind_traits<CharT> allocator_traits;
	typedef typename allocator_traits::allocator_type allocator_type;
	allocator_type alloc(allocator);
	_data=std::allocate_shared<data_t>(
	    alloc,
	    other._data->_chunk,
	    count,
	    alloc);
    }
    else
    {
	typedef typename std::allocator_traits<Allocator>::template rebind_traits<CharT> allocator_traits;
	typedef typename allocator_traits::allocator_type allocator_type;
	typedef typename allocator_traits::const_pointer const_pointer;
	allocator_type alloc(allocator);
	_data=std::allocate_shared<data_t>(
	    alloc,
	    boost::shared_ptr<const CharT[]>(
		other._data->_chunk.get()+pos,
		[other] (const_pointer) {},
		other._data->_allocator),
	    std::min(detail::size_guard<size_type>(other._data->_length,pos),count),
	    alloc);
    }
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator>::slim_basic_string(
    const CharT *s,
    const Allocator &alloc)
: slim_basic_string(s, detail::strlen(s), alloc) {}

template<typename CharT,
         typename Traits,
         typename Allocator>
template<class ForwardIt>
slim_basic_string<CharT,Traits,Allocator>::slim_basic_string (
    ForwardIt first,
    ForwardIt last,
    const Allocator &alloc ) : _data(allocate_data(size_type(std::distance(first,last)),alloc))
{
    auto chunk=boost::allocate_shared_noinit<CharT[]>(_data->_allocator,_data->_length);
    const auto begin=chunk.get();
    for ( auto i = begin, e = i + _data->_length; i != e; ++i ) {
        *i = *first++;
    }
    _data->_chunk=chunk;
}

template<typename CharT,
         typename Traits,
         typename Allocator>
typename slim_basic_string<CharT,Traits,Allocator>::allocator_type
slim_basic_string<CharT,Traits,Allocator>::get_allocator() const
{
    return _data->_allocator;
}

template<typename CharT,
         typename Traits,
         typename Allocator>
typename slim_basic_string<CharT,Traits,Allocator>::const_reference
slim_basic_string<CharT,Traits,Allocator>::at ( size_type pos ) const
{
    if ( pos>=_data->_length ) {
        throw std::out_of_range ( std::to_string ( pos ) +">="+std::to_string ( _data->_length ) );
    }
    return _data->_chunk[pos];
}

template<typename CharT,
         typename Traits,
         typename Allocator>
typename slim_basic_string<CharT,Traits,Allocator>::const_pointer
slim_basic_string<CharT,Traits,Allocator>::c_str() const
{
    if(!_data->_c_str)
    {
	auto c_str=boost::allocate_shared_noinit<CharT[]>(_data->_allocator,_data->_length+1);
	auto first=_data->_chunk.get();
	for ( auto i = c_str.get(), e = i + _data->_length; i != e; ++i ) {
	    *i = *first++;
	}
	c_str[_data->_length]=CharT();
	_data->_c_str=c_str;
    }
    return _data->_c_str.get();
}

template<typename CharT,
         typename Traits,
         typename Allocator>
typename slim_basic_string<CharT,Traits,Allocator>::size_type
slim_basic_string<CharT,Traits,Allocator>::max_size() const noexcept
{
    return allocator_traits::max_size(_data->_allocator);
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator> 
slim_basic_string<CharT,Traits,Allocator>::append(
    size_type count, 
    CharT ch) const
{
    auto ndata=allocate_data(size()+count,_data->_allocator);
    auto chunk=boost::allocate_shared_noinit<CharT[]>(ndata->_allocator,ndata->_length);
    std::fill_n(std::copy(data(),data()+size(),chunk.get()),count,ch);
    ndata->_chunk=chunk;
    return slim_basic_string(ndata);
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator> 
slim_basic_string<CharT,Traits,Allocator>::append(
    slim_basic_string str) const
{
    return append(str,0,str.size());
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator> 
slim_basic_string<CharT,Traits,Allocator>::append(
    slim_basic_string str,
    size_type pos,
    size_type count) const
{
    if(pos>=str.size())
    {
	throw std::out_of_range(std::to_string(pos) +">="+std::to_string (str._data->_length));
    }
    return append(str.data()+pos,str.data()+pos+std::min(str.size()-pos,count));
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator> 
slim_basic_string<CharT,Traits,Allocator>::append(
    const CharT* s,
    size_type count) const
{
    return append(s,s+count);
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator> 
slim_basic_string<CharT,Traits,Allocator>::append(
    const CharT* s) const
{
    return append(s,detail::strlen(s));
}

template<typename CharT,
         typename Traits,
         typename Allocator>
template<class ForwardIt>
slim_basic_string<CharT,Traits,Allocator> 
slim_basic_string<CharT,Traits,Allocator>::append(
    ForwardIt first, 
    ForwardIt last) const
{
    size_type rsize=size_type(std::distance(first,last));
    auto ndata=allocate_data(size()+rsize,_data->_allocator);
    auto chunk=boost::allocate_shared_noinit<CharT[]>(ndata->_allocator,ndata->_length);
    std::copy(first,last,std::copy(data(),data()+size(),chunk.get()));
    ndata->_chunk=chunk;
    return slim_basic_string(ndata);
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator> 
slim_basic_string<CharT,Traits,Allocator>::append(
    std::initializer_list<CharT> ilist) const
{
    return append(ilist.begin(),ilist.end());
}


template<typename CharT,
         typename Traits,
         typename Allocator>
typename slim_basic_string<CharT,Traits,Allocator>::size_type
slim_basic_string<CharT,Traits,Allocator>::copy(
    CharT* dest,
    size_type count,
    size_type pos) const
{
    if(pos>=size())
    {
	throw std::out_of_range(std::to_string(pos) +">="+std::to_string (_data->_length));
    }
    auto r=std::min(count,size()-pos);
    std::copy_n(data()+pos,r,dest);
    return r;
}

template<typename CharT,
         typename Traits,
         typename Allocator>
int
slim_basic_string<CharT,Traits,Allocator>::compare_impl(
    const value_type* s1,
    size_type s1_size,
    const value_type* s2,
    size_type s2_size)
{
    size_type rlen=std::min(s1_size,s2_size);
    int r=traits_type::compare(s1,s2,rlen);
    if(r!=0)
	return r;
    if(s1_size<s2_size)
	return -1;
    if(s1_size==s2_size)
	return 0;
    return 1;
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator>
slim_basic_string<CharT,Traits,Allocator>::concat_impl(
    const CharT* s1,
    size_type s1_size,
    const CharT* s2,
    size_type s2_size, 
    const Allocator& alloc)
{
    auto ndata=allocate_data(s1_size+s2_size,alloc);
    auto chunk=boost::allocate_shared_noinit<CharT[]>(ndata->_allocator,ndata->_length);
    std::copy(s2,s2+s2_size,std::copy(s1,s1+s1_size,chunk.get()));
    ndata->_chunk=chunk;
    return slim_basic_string(ndata);
}

template<typename CharT,
         typename Traits,
         typename Allocator>
slim_basic_string<CharT,Traits,Allocator>
slim_basic_string<CharT,Traits,Allocator>::concat_impl(
    const CharT c1,
    const CharT* s2,
    size_type s2_size, 
    const Allocator& alloc)
{
    auto ndata=allocate_data(1+s2_size,alloc);
    auto chunk=boost::allocate_shared_noinit<CharT[]>(ndata->_allocator,ndata->_length);
    chunk[0]=c1;
    std::copy(s2,s2+s2_size,chunk.get()+1);
    ndata->_chunk=chunk;
    return slim_basic_string(ndata);
}

template<typename CharT,
typename Traits,
typename Allocator>
slim_basic_string<CharT,Traits,Allocator>
slim_basic_string<CharT,Traits,Allocator>::concat_impl(
    const CharT* s1,
    size_type s1_size,
    const CharT c2, 
    const Allocator& alloc)
{
    auto ndata=allocate_data(s1_size+1,alloc);
    auto chunk=boost::allocate_shared_noinit<CharT[]>(ndata->_allocator,ndata->_length);
    *std::copy(s1,s1+s1_size,chunk.get())=c2;
    ndata->_chunk=chunk;
    return slim_basic_string(ndata);
}

template<typename CharT,
         typename Traits,
         typename Allocator>
class slim_basic_string<CharT,Traits,Allocator>::iterator
    : std::iterator<
    std::random_access_iterator_tag,
    CharT,
    std::ptrdiff_t,
    const_pointer,
    const_reference>
{
public:

private:
    slim_basic_string<CharT,Traits,Allocator> _instance;
    difference_type _i;
};

template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(slim_basic_string<CharT,Traits,Alloc> lhs,
	  slim_basic_string<CharT,Traits,Alloc> rhs)
{
    return slim_basic_string<CharT,Traits,Alloc>::concat_impl(
	lhs.data(),
	lhs.size(),
	rhs.data(),
	rhs.size(),
	lhs._data->_allocator);
}

template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(const CharT* lhs,
	  slim_basic_string<CharT,Traits,Alloc> rhs)
{
    return slim_basic_string<CharT,Traits,Alloc>::concat_impl(
	lhs,
	detail::strlen(lhs),
	rhs.data(),
	rhs.size(),
	rhs._data->_allocator);
}

template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(CharT lhs,
	  slim_basic_string<CharT,Traits,Alloc> rhs)
{
    return slim_basic_string<CharT,Traits,Alloc>::concat_impl(
	lhs,
	rhs.data(),
	rhs.size(),
	rhs._data->_allocator);
}

template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(slim_basic_string<CharT,Traits,Alloc> lhs,
	  const CharT* rhs)
{
    return slim_basic_string<CharT,Traits,Alloc>::concat_impl(
	lhs.data(),
	lhs.size(),
	rhs,
	detail::strlen(rhs),
	lhs._data->_allocator);
}


template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(slim_basic_string<CharT,Traits,Alloc> lhs,
	  CharT rhs)
{
    return slim_basic_string<CharT,Traits,Alloc>::concat_impl(
	lhs.data(),
	lhs.size(),
	rhs,
	lhs._data->_allocator);
}

template<class CharT, class Traits, class Allocator>
bool operator==(slim_basic_string<CharT,Traits,Allocator> lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs)
{
    if(lhs._data==rhs._data)
	return true;
    if(lhs._data->_length!=rhs._data->_length)
	return false;
    return lhs.compare(rhs)==0;
}

template<class CharT, class Traits, class Allocator>
bool operator!=(slim_basic_string<CharT,Traits,Allocator> lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return !(lhs==rhs);
}

template<class CharT, class Traits, class Allocator>
bool operator<(slim_basic_string<CharT,Traits,Allocator> lhs,
	       slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return lhs.compare(rhs)<0;
}

template<class CharT, class Traits, class Allocator>
bool operator<=(slim_basic_string<CharT,Traits,Allocator> lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return lhs.compare(rhs)<=0;
}

template<class CharT, class Traits, class Allocator>
bool operator>(slim_basic_string<CharT,Traits,Allocator> lhs,
	       slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return lhs.compare(rhs)>0;
}

template<class CharT, class Traits, class Allocator>
bool operator>=(slim_basic_string<CharT,Traits,Allocator> lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return lhs.compare(rhs)>=0;
}

template<class CharT, class Traits, class Allocator>
bool operator==(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return rhs.compare(lhs)==0;
}

template<class CharT, class Traits, class Allocator>
bool operator==(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs)
{
    return lhs.compare(rhs)==0;
}

template<class CharT, class Traits, class Allocator>
bool operator!=(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return rhs.compare(lhs)!=0;
}

template<class CharT, class Traits, class Allocator>
bool operator!=(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs)
{
    return lhs.compare(rhs)!=0;
}

template<class CharT, class Traits, class Allocator>
bool operator<(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return rhs.compare(lhs)>0;
}

template<class CharT, class Traits, class Allocator>
bool operator<(slim_basic_string<CharT,Traits,Allocator> lhs,  const CharT* rhs)
{
    return lhs.compare(rhs)<0;
}

template<class CharT, class Traits, class Allocator>
bool operator<=(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return rhs.compare(lhs)>=0;
}

template<class CharT, class Traits, class Allocator>
bool operator<=(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs)
{
    return lhs.compare(rhs)<=0;
}

template<class CharT, class Traits, class Allocator>
bool operator>(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return rhs.compare(lhs)<0;
}

template<class CharT, class Traits, class Allocator>
bool operator>(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs)
{
    return lhs.compare(rhs)>0;
}

template<class CharT, class Traits, class Allocator>
bool operator>=(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs)
{
    return rhs.compare(lhs)<=0;
}

template<class CharT, class Traits, class Allocator>
bool operator>=(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs)
{
    return lhs.compare(rhs)>=0;
}

template <class CharT, class Traits, class Allocator>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os,
	   slim_basic_string<CharT, Traits, Allocator> str)
{
    return os<<str.c_str();
}
