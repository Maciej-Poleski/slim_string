#ifndef SLIM_BASIC_STRING_HPP
#define SLIM_BASIC_STRING_HPP

#include <memory>
#include <iterator>
#include <algorithm>

#include <boost/smart_ptr/shared_array.hpp>

template<class CharT,
         class Traits = std::char_traits<CharT>,
         class Allocator = std::allocator<CharT>>
class slim_basic_string;

namespace detail
{
struct no_op_deleter
{
    template<typename T>
    void operator()(T) noexcept
    {}
};

template<typename CharT>
static std::size_t strlen(const CharT *s);

}


template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(slim_basic_string<CharT,Traits,Alloc> lhs,
	  slim_basic_string<CharT,Traits,Alloc> rhs);

template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(const CharT* lhs,
	  slim_basic_string<CharT,Traits,Alloc> rhs);

template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(CharT lhs,
	  slim_basic_string<CharT,Traits,Alloc> rhs);

template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(slim_basic_string<CharT,Traits,Alloc> lhs,
	  const CharT* rhs);

template<class CharT, class Traits, class Alloc>
slim_basic_string<CharT,Traits,Alloc>
operator+(slim_basic_string<CharT,Traits,Alloc> lhs,
	  CharT rhs);

template<class CharT, class Traits, class Allocator>
bool operator==(slim_basic_string<CharT,Traits,Allocator> lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator!=(slim_basic_string<CharT,Traits,Allocator> lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator<(slim_basic_string<CharT,Traits,Allocator> lhs,
	       slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator<=(slim_basic_string<CharT,Traits,Allocator> lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator>(slim_basic_string<CharT,Traits,Allocator> lhs,
	       slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator>=(slim_basic_string<CharT,Traits,Allocator> lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator==(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator==(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);

template<class CharT, class Traits, class Allocator>
bool operator!=(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator!=(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);

template<class CharT, class Traits, class Allocator>
bool operator<(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator<(slim_basic_string<CharT,Traits,Allocator> lhs,  const CharT* rhs);

template<class CharT, class Traits, class Allocator>
bool operator<=(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator<=(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);

template<class CharT, class Traits, class Allocator>
bool operator>(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator>(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);

template<class CharT, class Traits, class Allocator>
bool operator>=(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);

template<class CharT, class Traits, class Allocator>
bool operator>=(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);

template <class CharT, class Traits, class Allocator>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os,
	   slim_basic_string<CharT, Traits, Allocator> str);

template <class CharT,
         class Traits,
         class Allocator>
class slim_basic_string
{
    typedef typename std::allocator_traits<Allocator>::template rebind_traits<CharT> allocator_traits;
public:
    typedef Traits traits_type;
    typedef typename Traits::char_type value_type;
    typedef typename allocator_traits::allocator_type allocator_type;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::difference_type difference_type;
    typedef typename allocator_traits::value_type &reference;
    typedef const typename allocator_traits::value_type &const_reference;
    typedef typename allocator_traits::pointer pointer;
    typedef typename allocator_traits::const_pointer const_pointer;
    class iterator;
    friend class iterator;
    typedef iterator const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static const size_type npos = std::numeric_limits<size_type>::max();
    
    template<size_type N>
    slim_basic_string(const CharT(&data)[N],
		      const Allocator& alloc = Allocator())
    : _data(allocate_data(N-1,alloc))
    {
	_data->_chunk=boost::shared_ptr<const CharT[]>(
	    data,
	    detail::no_op_deleter(),
	    _data->_allocator);
	_data->_c_str.reset(data,detail::no_op_deleter(),_data->_allocator);
    }

    slim_basic_string(size_type count,
                      CharT ch,
                      const Allocator &alloc = Allocator());

    slim_basic_string(slim_basic_string other,
                      size_type pos,
                      size_type count = npos,
                      const Allocator &alloc = Allocator());

    slim_basic_string(const CharT *s,
                      size_type count,
                      const Allocator &alloc = Allocator())
        : slim_basic_string(s, s + count, alloc) {}

    explicit slim_basic_string(const CharT *s,
                      const Allocator &alloc = Allocator());

    template<class ForwardIt>
    slim_basic_string(ForwardIt first,
                      ForwardIt last,
                      const Allocator &alloc = Allocator());

    slim_basic_string(std::initializer_list<CharT> init,
                      const Allocator &alloc = Allocator())
        : slim_basic_string(init.begin(), init.end(), alloc) {}
        
    template<class Alloc>
    slim_basic_string(const std::basic_string<CharT,Traits,Alloc>& basic_string,
		      const Allocator &alloc = Allocator())
	: slim_basic_string(basic_string.data(),basic_string.size(), alloc) {}
	
    explicit operator std::basic_string<CharT,Traits,Allocator>() const
    {
	return std::basic_string<CharT,Traits,Allocator>(data(),size());
    }
    
    allocator_type get_allocator() const;

    const_reference at(size_type pos) const;

    const_reference operator[](size_type pos) const
    {
        return _data->_chunk[pos];
    }

    const_reference front() const
    {
	return _data->_chunk[0];
    }

    const_reference back() const
    {
	return _data->_chunk[_data->_length-1];
    }

    const_pointer data() const
    {
        return _data->_chunk.get();
    }

    const_pointer c_str() const;

    const_iterator begin() const;
    const_iterator end() const;
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;

    bool empty() const noexcept
    {
        return _data->_length==0;
    }

    size_type size() const noexcept
    {
        return _data->_length;
    }

    size_type length() const noexcept
    {
        return _data->_length;
    }

    size_type max_size() const noexcept;
    
    int compare(slim_basic_string str) const
    {
	return compare(0,size(),str);
    }
    
    int compare(size_type pos1, size_type count1,
		slim_basic_string str) const
    {
	return compare(pos1,count1,str,0,str.size());
    }
    
    int compare(size_type pos1, size_type count1,
		slim_basic_string str,
		size_type pos2, size_type count2=npos) const
    {
	return compare(pos1,count1,str.data()+pos2,count2);
    }
    
    int compare(const CharT* s) const
    {
	return compare(0,size(),s);
    }
    
    int compare(size_type pos1, size_type count1,
		const CharT* s ) const
    {
	return compare(pos1,count1,s,detail::strlen(s));
    }
    
    int compare(size_type pos1, size_type count1,
		const CharT* s, size_type count2 ) const
    {
	return compare_impl(data()+pos1,count1,s,count2);
    }
    
    slim_basic_string append(size_type count, CharT ch) const;
    
    slim_basic_string append(slim_basic_string str) const;

    slim_basic_string append(slim_basic_string str,
			      size_type pos,
			      size_type count = npos) const;
    
    slim_basic_string append(const CharT* s,
			      size_type count) const;

    slim_basic_string append(const CharT* s) const;

    template<class ForwardIt>
    slim_basic_string append(ForwardIt first, ForwardIt last) const;

    slim_basic_string append(std::initializer_list<CharT> ilist) const;
    
    slim_basic_string substr(size_type pos = 0,
			     size_type count = npos ) const
    {
	return slim_basic_string(*this,pos,count);
    }
    
    size_type copy(CharT* dest,
                   size_type count,
                   size_type pos = 0) const;
    
private:    
    static int compare_impl(const value_type* s1,size_type s1_size,const value_type* s2, size_type s2_size);
    
    static slim_basic_string concat_impl(const CharT* s1, size_type s1_size, const CharT* s2, size_type s2_size, const Allocator& allo);
    static slim_basic_string concat_impl(const CharT c1, const CharT* s2, size_type s2_size, const Allocator& alloc);
    static slim_basic_string concat_impl(const CharT* s1, size_type s1_size, const CharT c2, const Allocator& alloc);
    
    friend slim_basic_string<CharT,Traits,Allocator>
    operator+<>(slim_basic_string<CharT,Traits,Allocator> lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend slim_basic_string<CharT,Traits,Allocator>
    operator+<>(const CharT* lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend slim_basic_string<CharT,Traits,Allocator>
    operator+<>(CharT lhs,
		slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend slim_basic_string<CharT,Traits,Allocator>
    operator+<>(slim_basic_string<CharT,Traits,Allocator> lhs,
		const CharT* rhs);
    
    
    friend slim_basic_string<CharT,Traits,Allocator>
    operator+<>(slim_basic_string<CharT,Traits,Allocator> lhs,
		CharT rhs);
    
    friend
    bool operator==<>(slim_basic_string<CharT,Traits,Allocator> lhs,
		      slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator!=<>(slim_basic_string<CharT,Traits,Allocator> lhs,
		      slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator< <>(slim_basic_string<CharT,Traits,Allocator> lhs,
		      slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator<=<>(slim_basic_string<CharT,Traits,Allocator> lhs,
		      slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator><>(slim_basic_string<CharT,Traits,Allocator> lhs,
		     slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator>=<>(slim_basic_string<CharT,Traits,Allocator> lhs,
		      slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator==<>(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator==<>(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);
    
    friend
    bool operator!=<>(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator!=<>(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);
    
    friend
    bool operator< <>(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator< <>(slim_basic_string<CharT,Traits,Allocator> lhs,  const CharT* rhs);
    
    friend
    bool operator<=<>(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator<=<>(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);
    
    friend
    bool operator><>(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator><>(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);
    
    friend
    bool operator>=<>(const CharT* lhs, slim_basic_string<CharT,Traits,Allocator> rhs);
    
    friend
    bool operator>=<>(slim_basic_string<CharT,Traits,Allocator> lhs, const CharT* rhs);
    
    friend
    std::basic_ostream<CharT, Traits>&
    operator<< <>(std::basic_ostream<CharT, Traits>& os,
	          slim_basic_string<CharT, Traits, Allocator> str);

private:
    struct data_t
    {
	boost::shared_ptr<const CharT[]> _chunk;
	boost::shared_ptr<const CharT[]> _c_str=nullptr; // Cached, lazy-initialized;
	const size_type _length;
	const allocator_type _allocator;
	
	data_t(const boost::shared_ptr<const CharT[]> chunk,
	       const size_type length,
	       const allocator_type& allocator) : _chunk(chunk), _length(length), _allocator(allocator) {}
	       
	data_t(const boost::shared_ptr<const CharT[]> chunk,
	       const boost::shared_ptr<const CharT[]> &&c_str,
	       const size_type length,
	       const allocator_type& allocator) : _chunk(chunk),_c_str(std::move(c_str)), _length(length), _allocator(allocator) {}
	       
	       
    };
    slim_basic_string(const std::shared_ptr<data_t> data) : _data(data) {}
    static std::shared_ptr<typename slim_basic_string<CharT,Traits,Allocator>::data_t> allocate_data (
	const std::size_t size,
	const Allocator &allocator );
    std::shared_ptr<data_t> _data;
};

#include "slim_basic_string.tcc"

#endif // SLIM_BASIC_STRING_HPP
