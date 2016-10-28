/*!*********************************************************************************************************************
\file         PVRCore\RefCounted.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         A smart pointer implementation very close to the spec of std::shared_ptr but with some differences and tweaks to
              make it more suitable for the PowerVR Framework.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Types.h"
#include "PVRCore/Log.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <type_traits>

namespace pvr {

/*!*********************************************************************************************************************
\brief    An interface that represents a block of memory that will be doing the bookkeeping for an object that will be Reference
          Counted. This bit of memory holds the reference counts.
***********************************************************************************************************************/
struct IRefCountEntry
{
private:
	mutable volatile std::atomic<int32_t> count_; //!< Number of total references for this object
	mutable volatile std::atomic<int32_t> weakcount_; //!< Number of weak references for this object
	std::mutex mylock;
public:

	int32_t count() { return count_; } //!< Number of total references for this object
	int32_t weakcount() { return weakcount_; } //!< Number of weak references for this object

	void increment_count()
	{
		if (std::atomic_fetch_add(&count_, 1) == 0)
		{
			std::lock_guard<std::mutex> lockguard(mylock);
			if (count_.load() == 0)
			{
				assertion(false, "RefCounted::increment_count:  Tried to increment the count of an object but it had already been destroyed!");
			}
		}
	} //!< Number of total references for this object
	void increment_weakcount()
	{
		if (std::atomic_fetch_add(&weakcount_, 1) == 0)
		{
			std::lock_guard<std::mutex> lockguard(mylock);
			if (weakcount_.load() == 0)
			{
				assertion(false, "RefCounted::increment_weakcount:  Tried to increment the count of an object but it had already been destroyed!");
			}
		}
	}  //!< Number of weak references for this object

	void decrement_count()
	{
		bool deleter = false; //DO NOT DESTROY THE OBJECT WHILE THE LOCK IS HELD!
		int32_t cnt = 0;
		{
			if ((cnt = std::atomic_fetch_add(&count_, -1)) == 1)
			{
				std::lock_guard<std::mutex> lockguard(mylock);
				destroyObject();
				if (weakcount() == 0)
				{
					deleter = true;
				}
			}
		}
		if (deleter)
		{
			deleteEntry();
		}
	}   //!< Number of total references for this object
	void decrement_weakcount()
	{
		bool deleter = false; //DO NOT DESTROY THE OBJECT WHILE THE MUTEX IS USED!
		if (((std::atomic_fetch_add(&weakcount_, -1)) == 1))
		{
			std::lock_guard<std::mutex> lockguard(mylock);
			if (count_.load() == 0)
			{
				deleter = true;
			}
		}
		if (deleter)
		{
			deleteEntry();
		}
	}   //!< Number of weak references for this object


	virtual void deleteEntry() = 0; //!< Will be overriden with the actual code required to delete the bookkeeping entry
	virtual void destroyObject() = 0; //!< Will be overriden with the actual code required to delete the object
	virtual ~IRefCountEntry() {} //!< Will be overriden with the actual code required to delete the object
	IRefCountEntry() : count_(1), weakcount_(0) {}
};

/*!*********************************************************************************************************************
\brief        DO NOT USE DIRECTLY. The RefCountedResource uses this class when required.
              An Intrusive Refcount entry stores the object on the same block of memory as the reference counts.
\tparam       MyClass_ The type of reference counted object.
\description  This class is used to store the generated object on the same block of memory as the reference counts bookkeeping
              giving much better memory coherency. It is created when the "construct" method is used to create a new object inside
              a smart pointer. This is a much better solution than creating the object yourself and then just wrapping the
        pointer you have with a RefCountEntry, as if you use the intrusive class you save one level of indirection.
        This class is used when the user does RefCountEntry<MyClass> entry; entry.construct(...);
***********************************************************************************************************************/
template<typename MyClass_>
struct RefCountEntryIntrusive : public IRefCountEntry
{
	union
	{
		double _alignment_; //!<Unused, value irrelevent. Used to 4-byte align the following byte array.
		char entry[sizeof(MyClass_)]; //!<Represents enough bytes of memory to store a MyClass_ object.
	};

	/*!*****************************************************************************************************************
	\brief       Construct a new RefCountEntryIntrusive and the contained object.  Parameterless constructor.
	\param[out]  pointee After returning, pointee will contain the address of the newly constructed MyClass_ object.
	\description This constructor is called when the user calls construct() with no arguments.
	             Uses MyClass_'s parameterless constructor. Creates the class in place on our block of memory.
	*******************************************************************************************************************/
	RefCountEntryIntrusive(MyClass_*& pointee)
	{
		pointee = new(entry) MyClass_();
	}

	/*!*****************************************************************************************************************
	\brief       Construct a new RefCountEntryIntrusive and the contained object.  One Parameter constructor.
	\param[out] pointee After returning, pointee will contain the address of the newly constructed object
	\tparam P1 The type of the first argument required for MyClass_ one parameter constructor.
	\param[in] p1 The parameter that will be passed as MyClass_'s constructor first argument
	\description This constructor is called when the user calls construct() with one argument.
	             Uses MyClass_'s one-parameter constructor. Creates the class in place on our block of memory.
	*******************************************************************************************************************/
	template<typename P1> RefCountEntryIntrusive(MyClass_*& pointee, P1&& p1)
	{
		pointee = new(entry)MyClass_(std::forward<P1>(p1));
	}

	/*!*****************************************************************************************************************
	\brief       Construct a new RefCountEntryIntrusive and the contained object.  Two Parameter constructor.
	\param[out] pointee After returning, pointee will contain the address of the newly constructed object
	\tparam P1 The type of the first argument required for MyClass_ two parameter constructor.
	\tparam P2 The type of the second argument required for MyClass_ two parameter constructor.
	\param[in] p1 The parameter that will be passed as MyClass_'s constructor first argument
	\param[in] p2 The parameter that will be passed as MyClass_'s constructor second argument
	\description This constructor is called when the user calls construct() with two arguments.
	             Uses MyClass_'s two-parameter constructor. Creates the class in place on our block of memory.
	******************************************************************************************************************/
	template<typename P1, typename P2> RefCountEntryIntrusive(MyClass_*& pointee, P1&& p1, P2&& p2)
	{
		pointee = new(entry)MyClass_(std::forward<P1>(p1), std::forward<P2>(p2));
	}
	/*!*****************************************************************************************************************
	\brief       Construct a new RefCountEntryIntrusive and the contained object.  Three Parameter constructor.
	\param[out] pointee After returning, pointee will contain the address of the newly constructed object
	\tparam P1 The type of the first argument required for MyClass_ two parameter constructor.
	\tparam P2 The type of the second argument required for MyClass_ two parameter constructor.
	\tparam P3 The type of the third argument required for MyClass_ three parameter constructor.
	\param[in] p1 The parameter that will be passed as MyClass_'s constructor first argument
	\param[in] p2 The parameter that will be passed as MyClass_'s constructor second argument
	\param[in] p3 The parameter that will be passed as MyClass_'s constructor third argument
	\description This constructor is called when the user calls construct() with three arguments.
	             Uses MyClass_'s three-parameter constructor. Creates the class in place on our block of memory.
	******************************************************************************************************************/
	template<typename P1, typename P2, typename P3> RefCountEntryIntrusive(MyClass_*& pointee, P1&& p1, P2&& p2, P3&& p3)
	{
		pointee = new(entry)MyClass_(std::forward<P1>(p1), std::forward<P2>(p2), std::forward<P3>(p3));
	}

	/*!*****************************************************************************************************************
	\brief       Construct a new RefCountEntryIntrusive and the contained object.  Four Parameter constructor.
	\param[out] pointee After returning, pointee will contain the address of the newly constructed object
	\tparam P1 The type of the first argument required for MyClass_ two parameter constructor.
	\tparam P2 The type of the second argument required for MyClass_ two parameter constructor.
	\tparam P3 The type of the third argument required for MyClass_ three parameter constructor.
	\tparam P4 The type of the fourth argument required for MyClass_ fourth parameter constructor.
	\param[in] p1 The parameter that will be passed as MyClass_'s constructor first argument
	\param[in] p2 The parameter that will be passed as MyClass_'s constructor second argument
	\param[in] p3 The parameter that will be passed as MyClass_'s constructor third argument
	\param[in] p4 The parameter that will be passed as MyClass_'s constructor fourth argument
	\description This constructor is called when the user calls construct() with four arguments.
	             Uses MyClass_'s four-parameter constructor. Creates the class in place on our block of memory.
	******************************************************************************************************************/
	template<typename P1, typename P2, typename P3, typename P4> RefCountEntryIntrusive(MyClass_*& pointee, P1&& p1, P2&& p2, P3&& p3,
	    P4&& p4)
	{
		pointee = new(entry)MyClass_(std::forward<P1>(p1), std::forward<P2>(p2), std::forward<P3>(p3), std::forward<P4>(p4));
	}

	/*!*****************************************************************************************************************
	\brief       Construct a new RefCountEntryIntrusive and the contained object.  Five Parameter constructor.
	\param[out] pointee After returning, pointee will contain the address of the newly constructed object
	\tparam P1 The type of the first argument required for MyClass_ two parameter constructor.
	\tparam P2 The type of the second argument required for MyClass_ two parameter constructor.
	\tparam P3 The type of the third argument required for MyClass_ three parameter constructor.
	\tparam P4 The type of the fourth argument required for MyClass_ four parameter constructor.
	\tparam P4 The type of the fifth argument required for MyClass_ five parameter constructor.
	\param[in] p1 The parameter that will be passed as MyClass_'s constructor first argument
	\param[in] p2 The parameter that will be passed as MyClass_'s constructor second argument
	\param[in] p3 The parameter that will be passed as MyClass_'s constructor third argument
	\param[in] p4 The parameter that will be passed as MyClass_'s constructor fourth argument
	\param[in] p5 The parameter that will be passed as MyClass_'s constructor fifth argument
	\description This constructor is called when the user calls construct() with five arguments.
	             Uses MyClass_'s five-parameter constructor. Creates the class in place on our block of memory.
	******************************************************************************************************************/
	template<typename P1, typename P2, typename P3, typename P4, typename P5> RefCountEntryIntrusive(MyClass_*& pointee, P1&& p1,
	    P2&& p2, P3&& p3, P4&& p4, P5&& p5)
	{
		pointee = new(entry)MyClass_(std::forward<P1>(p1), std::forward<P2>(p2), std::forward<P3>(p3), std::forward<P4>(p4),
		                             std::forward<P5>(p5));
	}

	/*!*****************************************************************************************************************
	\brief       Destroys the RefcountEntryIntrusive object (entry and counters). Called when all references
	             (count + weak count) are  0. It assumes the object has already been destroyed - so it will NOT destroy
	       the object properly - only free its memory! destroyObject must be explicitly called otherwise
	       undefined behaviour occurs by freeing the memory of a live object!
	******************************************************************************************************************/
	void deleteEntry()
	{
		delete this;
	}

	/*!*****************************************************************************************************************
	\brief       Destroys the the held object. Called when all strong references are dead (count is zero).
	******************************************************************************************************************/
	void destroyObject()
	{
		((MyClass_*)entry)->~MyClass_();
	}
};


/*!*********************************************************************************************************************
\brief        DO NOT USE DIRECTLY. The RefCountedResource uses this class when required.
              A "simple" Refcount entry keeps a user-provided pointer to the object together with the reference counts.
\tparam       MyClass_ The type of the pointer kept.
\description  This class is used to store a user-provided pointer with the reference counts bookkeeping. This class is used when
              instead of calling "RefCountedResource::construct", the user provides a pointer to his object directly. This is
        worse than construct because it has worse memory coherency (the object lives "far" from the refcount information).
        The best solution is to use "construct", which creates a new object on a RefCountEntryIntrusive.
        On destruction, calls "delete" on the pointer.
***********************************************************************************************************************/
template<typename MyClass_>
struct RefCountEntry : public IRefCountEntry
{
	MyClass_* ptr; //!<Pointer to the object
	RefCountEntry() : ptr(NULL) { }
	RefCountEntry(MyClass_* ptr) : ptr(ptr) { }
	void deleteEntry() { delete this; }
	void destroyObject() { delete ptr; }
};

/*!*********************************************************************************************************************
\brief        DO NOT USE DIRECTLY. The RefCountedResource follows this class.
\tparam       MyClass_ The type of the pointer kept.
\description  This class stores a pointer alias to the object to avoid the double-indirection of the RefCountEntry.
              Broken off to make specialising for the MyClass_ = void easier.
***********************************************************************************************************************/
template<typename MyClass_>
class Dereferenceable
{
protected:
	MyClass_* pointee;
	Dereferenceable(const MyClass_ * pointee)
	{
		this->pointee = const_cast<MyClass_*>(pointee);
	}

public:
	MyClass_& operator*()
	{
		debug_assertion(pointee != 0, "Dereferencing NULL pointer");
		return *pointee;
	}
	const MyClass_& operator*() const
	{
		debug_assertion(pointee != 0, "Dereferencing NULL pointer");
		return *pointee;
	}

	MyClass_* operator->()
	{
		debug_assertion(pointee != 0, "Dereferencing NULL pointer");
		return pointee;
	}
	const MyClass_* operator->() const
	{
		debug_assertion(pointee != 0, "Dereferencing NULL pointer");
		return pointee;
	}

	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator==(const Dereferenceable& rhs) const { return pointee == rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests inequality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator!=(const Dereferenceable& rhs) const { return pointee != rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator<(const Dereferenceable& rhs) const { return pointee < rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator>(const Dereferenceable& rhs) const { return pointee > rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator<=(const Dereferenceable& rhs) const { return pointee <= rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator>=(const Dereferenceable& rhs) const { return pointee >= rhs.pointee; }
};
//!\cond NO_DOXYGEN
template<>
class Dereferenceable<void>
{
protected:
	Dereferenceable(void* pointee) : pointee(pointee) {}
	void* pointee;
public:
	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator==(const Dereferenceable& rhs) const { return pointee == rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests inequality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator!=(const Dereferenceable& rhs) const { return pointee != rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator<(const Dereferenceable& rhs) const { return pointee < rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator>(const Dereferenceable& rhs) const { return pointee > rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator<=(const Dereferenceable& rhs) const { return pointee <= rhs.pointee; }

	/*!*********************************************************************************************************************
	\brief       Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator>=(const Dereferenceable& rhs) const { return pointee >= rhs.pointee; }
};

template<typename MyClass_>
struct RefcountEntryHolder
{
	template <typename> friend class RefCountedResource;
	template<typename> friend class EmbeddedRefCountedResource;

protected:
	//Holding the actual entry (pointers, objects and everything). Use directly in the subclass.
	IRefCountEntry* refCountEntry;

	//Create a new intrusive refcount entry, and on it, construct a new object with its zero-parameter constructor.
	void construct(MyClass_*& pointee)
	{
		RefCountEntryIntrusive<MyClass_>* ptr = new RefCountEntryIntrusive<MyClass_>(pointee);
		refCountEntry = ptr;
	}
	//Create a new intrusive refcount entry, and on it, construct a new object with its one-parameter constructor.
	template <typename P1> void construct(MyClass_*& pointee, P1&& p1)
	{
		RefCountEntryIntrusive<MyClass_>* ptr = new RefCountEntryIntrusive<MyClass_>(pointee, p1);
		refCountEntry = ptr;
	}
	//Create an new intrusive refcount entry, and on it, construct a new object with its two-parameter constructor.
	template <typename P1, typename P2> void construct(MyClass_*& pointee, P1&& p1, P2&& p2)
	{
		RefCountEntryIntrusive<MyClass_>* ptr = new RefCountEntryIntrusive<MyClass_>(pointee, p1, p2);
		refCountEntry = ptr;
	}
	//Create an new intrusive refcount entry, and on it, construct a new object with its three-parameter constructor.
	template <typename P1, typename P2, typename P3> void construct(MyClass_*& pointee, P1&& p1, P2&& p2, P3&& p3)
	{
		RefCountEntryIntrusive<MyClass_>* ptr = new RefCountEntryIntrusive<MyClass_>(pointee, p1, p2, p3);
		refCountEntry = ptr;
	}
	//Create an new intrusive refcount entry, and on it, construct a new object with its four-parameter constructor.
	template <typename P1, typename P2, typename P3, typename P4> void construct(MyClass_*& pointee, P1&& p1, P2&& p2, P3&& p3,
	    P4&& p4)
	{
		RefCountEntryIntrusive<MyClass_>* ptr = new RefCountEntryIntrusive<MyClass_>(pointee, p1, p2, p3, p4);
		refCountEntry = ptr;
	}
	//Create an new intrusive refcount entry, and on it, construct a new object with its five-parameter constructor.
	template <typename P1, typename P2, typename P3, typename P4, typename P5> void construct(MyClass_*& pointee, P1&& p1, P2&& p2,
	    P3&& p3, P4&& p4, P5&& p5)
	{
		RefCountEntryIntrusive<MyClass_>* ptr = new RefCountEntryIntrusive<MyClass_>(pointee, p1, p2, p3, p4, p5);
		refCountEntry = ptr;
	}

	//Create this entry with an already constructed reference-counted entry.
	RefcountEntryHolder(IRefCountEntry* refCountEntry) : refCountEntry(refCountEntry) {}
};

template<>
struct RefcountEntryHolder<void>
{
protected:
	IRefCountEntry* refCountEntry;
	RefcountEntryHolder(IRefCountEntry* refCountEntry) : refCountEntry(refCountEntry) {}
};
//!\endcond

template<typename MyClass_> class RefCountedWeakReference;

template<typename MyClass_> class EmbeddedRefCount;

/*!*********************************************************************************************************************
\brief        "Embedded" subset of the Reference counted smart pointer. See RefCountedResource. It can be used very
        similarly to the C++11 shared_ptr, but with some differences, especially as far as conversions go.
        The EmbeddedRefCountedResource is used when the user must not have access to the .construct(...) functions
        of the RefCountedResource (especially, when a class is designed around reference counting and cannot stand
        widthout it). A RefCountedResource is-a EmbeddedRefCountedResource and is safe to "slice" to
        it as well (see details).
\description  In order for objects of a class to be used as EmbeddedRefCountedResource, it will need to inherit from
        EmbeddedRefCounted<Class>, and then implement destroyObject (a function that conceptually "clears" an object
        by freeing its resources, without freeing the memory of the object itself - like a custom destructor),
        and MyClass::createNew() static function (or other factory function that forwards arguments to the correct
        EmbeddedRefCount::createNew overloads, one for each "constructor" that needs to be exposed).
        Also see RefCountedResource.
\tparam       MyClass_ The type of object held. If a createNew function returning an EmbeddedRefCounted is called, this
        must be the type of the object that will be created.
***********************************************************************************************************************/
template<typename MyClass_>
class EmbeddedRefCountedResource : public Dereferenceable<MyClass_>, public RefcountEntryHolder<MyClass_>
{
	template<typename> friend class EmbeddedRefCount;
	template<typename> friend class RefCountedResource;
	template<typename> friend class EmbeddedRefCountedResource;
	template<typename> friend class RefCountedWeakReference;
public:
	/*!*********************************************************************************************************************
	\brief   Type of the object that this object contains. Can be a superclass of the actual type of the object.
	***********************************************************************************************************************/
	typedef MyClass_ ElementType;

	/*!*********************************************************************************************************************
	\return True if this object contains a reference to a non-null object (is safely dereferenceable).
	        Equivalent to !isNull().
	***********************************************************************************************************************/
	bool isValid() const { return RefcountEntryHolder<MyClass_>::refCountEntry != NULL && RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0; }

	/*!*********************************************************************************************************************
	\return True if this object does not contain a reference to a non-null object (is not safely dereferenceable).
	        Equivalent to !isValid().
	***********************************************************************************************************************/
	bool isNull() const { return RefcountEntryHolder<MyClass_>::refCountEntry == NULL || RefcountEntryHolder<MyClass_>::refCountEntry->count() == 0; }

	/*!*********************************************************************************************************************
	\brief    Get a raw pointer to the pointed-to object.
	\return   A raw pointer to the pointed-to object.
	***********************************************************************************************************************/
	MyClass_* get() { return Dereferenceable<MyClass_>::pointee; }

	/*!*********************************************************************************************************************
	\brief    Get a const raw pointer to the pointed-to object.
	\return   A const raw pointer to the pointed-to object.
	***********************************************************************************************************************/
	const MyClass_* get() const { return Dereferenceable<MyClass_>::pointee; }

	/*!*********************************************************************************************************************
	\brief    Swap the contents of this RefCountedResource object with another RefCountedResource object of the same type.
	***********************************************************************************************************************/
	void swap(EmbeddedRefCountedResource& rhs)
	{
		IRefCountEntry* tmpRefCountEntry = RefcountEntryHolder<MyClass_>::refCountEntry;
		RefcountEntryHolder<MyClass_>::refCountEntry = rhs.refCountEntry;
		rhs.refCountEntry = tmpRefCountEntry;
		MyClass_ * tmpPointee = Dereferenceable<MyClass_>::pointee;
		Dereferenceable<MyClass_>::pointee = rhs.pointee;
		rhs.pointee = tmpPointee;
	}

	/*!*********************************************************************************************************************
	\brief    Default constructor. Points to NULL.
	***********************************************************************************************************************/
	EmbeddedRefCountedResource() : Dereferenceable<MyClass_>(NULL), RefcountEntryHolder<MyClass_>(0) { }

	/*!*********************************************************************************************************************
	\brief    Copy constructor. Increments reference count.
	***********************************************************************************************************************/
	EmbeddedRefCountedResource(const EmbeddedRefCountedResource& rhs) : Dereferenceable<MyClass_>(rhs), RefcountEntryHolder<MyClass_>(rhs)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_count();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0 && RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0, "BUG - Count was negative.");
		}
	}

	/*!*********************************************************************************************************************
	\brief    Wrap an already existing object with a RefCountedResource.
	\tparam   MyPointer_ the type of the class pointed to by the pointer. May be a subclass of MyClass_.
	\param    ref The pointer to wrap into the RefCountedResource. Will be deleted when refcount reaches zero.
	\brief    Wrap an already existing object with a RefCountedResource. Will call delete on a MyPointer_ type when the reference
	          count reaches zero. MyPointer_ may be MyClass_ or a subclass of MyClass_. delete MyPointer_ will happen, not MyClass_.
	***********************************************************************************************************************/
	template<typename MyPointer_>
	explicit EmbeddedRefCountedResource(MyPointer_* ref) : Dereferenceable<MyClass_>(ref), RefcountEntryHolder<MyClass_>(new RefCountEntry<MyPointer_>(ref))
	{
		RefcountEntryHolder<MyClass_>::refCountEntry->count_ = 1;
	}

	/*!*********************************************************************************************************************
	\brief       Copy Assignment operator. Increments reference count.
	\description "this" (the left-hand-side of the operator) object will be released (if not NULL, reference count will be
	             decreased as normal and, if zero, the object will be deleted). Reference count of the right-hand-side object
	       will be increased and this RefCountedResource will be pointing to it.
	***********************************************************************************************************************/
	EmbeddedRefCountedResource& operator=(EmbeddedRefCountedResource rhs)
	{
		swap(rhs);
		debug_assertion(!RefcountEntryHolder<MyClass_>::refCountEntry ||
		                (RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0 &&
		                 RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0),
		                "BUG - Count was negative.");
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief       Implicit Copy Conversion constructor.
	\description  A RefCountedResource is implicitly convertible to any type that its current ElementType is implicitly convertible
	       to. For example, a pointer to a subclass is implicitly convertible to a pointer to superclass.
	       Reference counting keeps working normally on the original object regardless of the type of the pointer.
	\param rhs   The object to convert
	\description The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and is not used.
	***********************************************************************************************************************/
	template<class OldType_>
	EmbeddedRefCountedResource(const EmbeddedRefCountedResource<OldType_>& rhs,
	                           typename std::enable_if<std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* = 0)
		: Dereferenceable<MyClass_>(rhs.pointee), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		//Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<OldType_*>(Dereferenceable<MyClass_>::pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0 && RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0, "BUG - Count was negative.");
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_count();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0 && RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0, "BUG - Count was negative.");
		}
	}

	/*!*********************************************************************************************************************
	\brief       Explicit Copy Conversion constructor.
	\description A RefCountedResource is explicitly convertible to any type that its current ElementType is explicitly convertible to.
	             For example, a pointer to void is explicitly convertible to a pointer to any class.
	       Conversion used is static_cast.
	       Reference counting keeps working normally on the original object regardless of the type of the pointer.
	\param rhs   The object to convert
	\description The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and is not used.
	***********************************************************************************************************************/
	template<class OldType_>
	explicit EmbeddedRefCountedResource(const EmbeddedRefCountedResource<OldType_>& rhs,
	                                    typename std::enable_if < !std::is_convertible<OldType_*, MyClass_*>::value, void* >::type* = 0)
		: Dereferenceable<MyClass_>(static_cast<MyClass_*>(rhs.pointee)), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		//Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<MyClass_*>(Dereferenceable<MyClass_>::pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0 && RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0, "BUG - Count was negative.");
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_count();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0 && RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0, "BUG - Count was negative.");
		}
	}

	/*!*********************************************************************************************************************
	\brief       Destructor. Releases the object (decreases reference count and deletes it if zero).
	***********************************************************************************************************************/
	virtual ~EmbeddedRefCountedResource() { reset(); }

	/*!*********************************************************************************************************************
	\brief       Tests inequality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.
	***********************************************************************************************************************/
	bool operator!=(const EmbeddedRefCountedResource& rhs) const
	{
		return get() != rhs.get();
	}

	/*!*********************************************************************************************************************
	\brief       Decrements reference count. If it is the last pointer, destroys the pointed-to object. Else, abandons it. Then,
	             wraps the provided pointer and points to it. Equivalent to destroying the original smart pointer and creating a
	       new one.
	\tparam MyPointer_ The type of the new pointer to wrap. Must be implicitly convertible to MyClass_.
	\param ref   The new pointer to wrap.
	***********************************************************************************************************************/
	template<typename MyPointer_>
	void reset(MyPointer_* ref)
	{
		reset();
		RefcountEntryHolder<MyClass_>::refCountEntry = new RefCountEntry<MyPointer_>(ref);
		Dereferenceable<MyClass_>::pointee = ref;
	}

	/*!*********************************************************************************************************************
	\brief       Decrements reference count. If it is the last pointer, destroys the pointed-to object. Else, abandons it. Then,
	             resets to NULL.
	***********************************************************************************************************************/
	void reset()
	{
		releaseOne();
	}
protected:
	template <typename OriginalType>
	EmbeddedRefCountedResource(IRefCountEntry* entry, OriginalType* pointee)
		: Dereferenceable<MyClass_>(static_cast<MyClass_*>(pointee)), RefcountEntryHolder<MyClass_>(entry)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0 && RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0, "BUG - Count was negative.");
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_count();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0 && RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0, "BUG - Count was negative.");
		}
	}

private:
	void releaseOne()
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			IRefCountEntry* tmp = RefcountEntryHolder<MyClass_>::refCountEntry;
			RefcountEntryHolder<MyClass_>::refCountEntry = 0;
			Dereferenceable<MyClass_>::pointee = 0;
			tmp->decrement_count();
		}
	}
};


/*!*********************************************************************************************************************
\brief        Reference counted smart pointer. It can be used very similarly to the C++11 shared_ptr, but with some differences.
\description  This reference counted smart resource will keep track of how many references to a specific object exist, and
release it when this is zero. Use its .construct(...) method overloads to efficiently construct an object together
with its ref-counting bookkeeping information. If this is not practical, you can also assign an already created
pointer to it, as long as it is safe to call "delete" on it.
Copy construction, move and assignment all work as expected and can freely be used.
It can very easily be used polymorphically.
        It is intended that this class will mainly be used with the .construct() method. If .construct() is used
        to create the object held (as opposed to wrapping a pre-existing pointer)
\tparam       MyClass_ The type of object held. If .construct(...) is called, this is the type of the object that will be created.
***********************************************************************************************************************/
template<typename MyClass_>
class RefCountedResource : public EmbeddedRefCountedResource<MyClass_>
{
	template<typename> friend class EmbeddedRefCount;
	template<typename> friend class RefCountedResource;
	template<typename> friend class EmbeddedRefCountedResource;
	template<typename> friend class RefCountedWeakReference;
public:
	typedef typename EmbeddedRefCountedResource<MyClass_>::ElementType ElementType;

	/*!*********************************************************************************************************************
	\brief    Swap the contents of this RefCountedResource object with another RefCountedResource object of the same type.
	***********************************************************************************************************************/
	void swap(RefCountedResource& rhs)
	{
		IRefCountEntry* tmpRefCountEntry = RefcountEntryHolder<MyClass_>::refCountEntry;
		RefcountEntryHolder<MyClass_>::refCountEntry = rhs.refCountEntry;
		rhs.refCountEntry = tmpRefCountEntry;
		MyClass_ * tmpPointee = Dereferenceable<MyClass_>::pointee;
		Dereferenceable<MyClass_>::pointee = rhs.pointee;
		rhs.pointee = tmpPointee;
	}

	/*!*********************************************************************************************************************
	\brief    Default constructor. Points to NULL.
	***********************************************************************************************************************/
	RefCountedResource() { }

	/*!*********************************************************************************************************************
	\brief    Copy constructor. Increments reference count.
	***********************************************************************************************************************/
	RefCountedResource(const RefCountedResource& rhs) : EmbeddedRefCountedResource<MyClass_>(rhs)
	{
	}

	/*!*********************************************************************************************************************
	\brief    Wrap an already existing object with a RefCountedResource.
	\tparam   MyPointer_ the type of the class pointed to by the pointer. May be a subclass of MyClass_.
	\param    ref The pointer to wrap into the RefCountedResource. Will be deleted when refcount reaches zero.
	\brief    Wrap an already existing object with a RefCountedResource. Will call delete on a MyPointer_ type when the reference
	count reaches zero. MyPointer_ may be MyClass_ or a subclass of MyClass_. delete MyPointer_ will happen, not MyClass_.
	***********************************************************************************************************************/
	template<typename MyPointer_>
	explicit RefCountedResource(MyPointer_* ref) : EmbeddedRefCountedResource<MyClass_>(ref)
	{
		RefcountEntryHolder<MyClass_>::refCountEntry->count = 1;
	}

	/*!*********************************************************************************************************************
	\brief       Copy Assignment operator. Increments reference count.
	\description "this" (the left-hand-side of the operator) object will be released (if not NULL, reference count will be
	decreased as normal and, if zero, the object will be deleted). Reference count of the right-hand-side object
	will be increased and this RefCountedResource will be pointing to it.
	***********************************************************************************************************************/
	RefCountedResource& operator=(RefCountedResource rhs) { swap(rhs); return *this; }

	/*!*********************************************************************************************************************
	\brief       Implicit Copy Conversion constructor.
	\description  A RefCountedResource is implicitly convertible to any type that its current ElementType is implicitly convertible
	to. For example, a pointer to a subclass is implicitly convertible to a pointer to superclass.
	Reference counting keeps working normally on the original object regardless of the type of the pointer.
	\param rhs   The object to convert
	\description The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and is not used.
	***********************************************************************************************************************/
	template<class OldType_>
	RefCountedResource(const RefCountedResource<OldType_>& rhs,
	                   typename std::enable_if<std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* t = 0)
		: EmbeddedRefCountedResource<MyClass_>(rhs, t)
	{
		//Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<OldType_*>(Dereferenceable<MyClass_>::pointee);
		(void)checkMe;
	}

	/*!*********************************************************************************************************************
	\brief       Explicit Copy Conversion constructor.
	\description A RefCountedResource is explicitly convertible to any type that its current ElementType is explicitly convertible to.
	For example, a pointer to void is explicitly convertible to a pointer to any class.
	Conversion used is static_cast.
	Reference counting keeps working normally on the original object regardless of the type of the pointer.
	\param rhs   The object to convert
	\description The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and is not used.
	***********************************************************************************************************************/
	template<class OldType_>
	explicit RefCountedResource(const RefCountedResource<OldType_>& rhs,
	                            typename std::enable_if < !std::is_convertible<OldType_*, MyClass_*>::value, void* >::type* t = 0)
		: EmbeddedRefCountedResource<MyClass_>(rhs, t)
	{
		//Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<MyClass_*>(Dereferenceable<MyClass_>::pointee);
		(void)checkMe;
	}

	/*!*********************************************************************************************************************
	\brief       Destructor. Releases the object (decreases reference count and deletes it if zero).
	***********************************************************************************************************************/
	virtual ~RefCountedResource() { /*release called from EmbeddedRefCountedResource*/ }

	/*!*********************************************************************************************************************
	\brief       Decrements reference count. If it is the last pointer, destroys the pointed-to object. Else, abandons it. Then,
	wraps the provided pointer and points to it. Equivalent to destroying the original smart pointer and creating a
	new one.
	\tparam MyPointer_ The type of the new pointer to wrap. Must be implicitly convertible to MyClass_.
	\param ref   The new pointer to wrap.
	***********************************************************************************************************************/
	template<typename MyPointer_>
	void reset(MyPointer_* ref)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<MyClass_>::refCountEntry = new RefCountEntry<MyPointer_>(ref);
		Dereferenceable<MyClass_>::pointee = ref;
	}

	/*!*********************************************************************************************************************
	\brief       Decrements reference count. If it is the last pointer, destroys the pointed-to object. Else, abandons it.
	***********************************************************************************************************************/
	void reset()
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
	}

	/*!*********************************************************************************************************************
	\brief      -- Use this method to construct a new instance of ElementType using its default constructor, and use
	            reference counting with it. ---   PARAMETERLESS CONSTRUCTOR OVERLOAD
	\description Use this method or its overloads to create a new object in the most efficient way possible and wrap it in a
	            RefCountedResource object. Use this whenever possible instead of wrapping a user-provided pointer in order to get
	      the best memory locality, as this method will create both the object and the necessary reference counters in one
	      block of memory.
	      If an object is already owned by this object, it will first be properly released before constructing the new one.
	***********************************************************************************************************************/
	void construct()
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<MyClass_>::construct(Dereferenceable<MyClass_>::pointee);
	}

	/*!*********************************************************************************************************************
	\brief      -- Use this method to construct a new instance of ElementType using its one-parameter constructor, and use
	            reference counting with it. ---   ONE ARGUMENT CONSTRUCTOR OVERLOAD
	\description Use this method or its overloads to create a new object in the most efficient way possible and wrap it in a
	            RefCountedResource object. Use this whenever possible instead of wrapping a user-provided pointer in order to get
	      the best memory locality, as this method will create both the object and the necessary reference counters in one
	      block of memory.
	      If an object is already owned by this object, it will first be properly released before constructing the new one.
	\tparam     ParamType  The type of the argument accepted by MyClass_'s constructor
	\param      param  The argument that will be forwarded to MyClass_'s constructor
	***********************************************************************************************************************/
	template <typename ParamType>
	void construct(ParamType&& param)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<MyClass_>::construct(Dereferenceable<MyClass_>::pointee, std::forward<ParamType>(param));
	}


	/*!*********************************************************************************************************************
	\brief      -- Use this method to construct a new instance of ElementType using its two-parameter constructor, and use
	            reference counting with it. ---   TWO-ARGUMENT CONSTRUCTOR OVERLOAD
	\description Use this method or its overloads to create a new object in the most efficient way possible and wrap it in a
	            RefCountedResource object. Use this whenever possible instead of wrapping a user-provided pointer in order to get
	      the best memory locality, as this method will create both the object and the necessary reference counters in one
	      block of memory.
	      If an object is already owned by this object, it will first be properly released before constructing the new one.
	\tparam     ParamType1  The type of the first argument accepted by MyClass_'s constructor
	\param      param1  The argument that will be forwarded to MyClass_'s constructor as the first argument
	\tparam     ParamType2  The type of the second argument accepted by MyClass_'s constructor
	\param      param2  The argument that will be forwarded to MyClass_'s constructor as the second argument
	***********************************************************************************************************************/
	template <typename ParamType1, typename ParamType2>
	void construct(ParamType1&& param1, ParamType2&& param2)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<MyClass_>::construct(Dereferenceable<MyClass_>::pointee, std::forward<ParamType1>(param1),
		    std::forward<ParamType2>(param2));
	}

	/*!*********************************************************************************************************************
	\brief      -- Use this method to construct a new instance of ElementType using its three-parameter constructor, and use
	            reference counting with it. ---   THREE-ARGUMENT CONSTRUCTOR OVERLOAD
	\description Use this method or its overloads to create a new object in the most efficient way possible and wrap it in a
	            RefCountedResource object. Use this whenever possible instead of wrapping a user-provided pointer in order to get
	      the best memory locality, as this method will create both the object and the necessary reference counters in one
	      block of memory.
	      If an object is already owned by this object, it will first be properly released before constructing the new one.
	\tparam     ParamType1  The type of the first argument accepted by MyClass_'s constructor
	\param      param1  The argument that will be forwarded to MyClass_'s constructor as the first argument
	\tparam     ParamType2  The type of the second argument accepted by MyClass_'s constructor
	\param      param2  The argument that will be forwarded to MyClass_'s constructor as the second argument
	\tparam     ParamType3  The type of the third argument accepted by MyClass_'s constructor
	\param      param3  The argument that will be forwarded to MyClass_'s constructor as the third argument
	***********************************************************************************************************************/
	template <typename ParamType1, typename ParamType2, typename ParamType3>
	void construct(ParamType1&& param1, ParamType2&& param2, ParamType3&& param3)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<MyClass_>::construct(Dereferenceable<MyClass_>::pointee, std::forward<ParamType1>(param1),
		    std::forward<ParamType2>(param2), std::forward<ParamType3>(param3));
	}

	/*!*********************************************************************************************************************
	\brief      -- Use this method to construct a new instance of ElementType using its four-parameter constructor, and use
	            reference counting with it. ---   FOUR-ARGUMENT CONSTRUCTOR OVERLOAD
	\descriptionUse this method or its overloads to create a new object in the most efficient way possible and wrap it in a
	            RefCountedResource object. Use this whenever possible instead of wrapping a user-provided pointer in order to get
	      the best memory locality, as this method will create both the object and the necessary reference counters in one
	      block of memory.
	      If an object is already owned by this object, it will first be properly released before constructing the new one.
	\tparam     ParamType1  The type of the first argument accepted by MyClass_'s constructor
	\param      param1  The argument that will be forwarded to MyClass_'s constructor as the first argument
	\tparam     ParamType2  The type of the second argument accepted by MyClass_'s constructor
	\param      param2  The argument that will be forwarded to MyClass_'s constructor as the second argument
	\tparam     ParamType3  The type of the third argument accepted by MyClass_'s constructor
	\param      param3  The argument that will be forwarded to MyClass_'s constructor as the third argument
	\tparam     ParamType4  The type of the fourth argument accepted by MyClass_'s constructor
	\param      param4  The argument that will be forwarded to MyClass_'s constructor as the fourth argument
	***********************************************************************************************************************/
	template <typename ParamType1, typename ParamType2, typename ParamType3, typename ParamType4>
	void construct(ParamType1&& param1, ParamType2&& param2, ParamType3&& param3, ParamType4&& param4)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<MyClass_>::construct(Dereferenceable<MyClass_>::pointee, std::forward<ParamType1>(param1),
		    std::forward<ParamType2>(param2), std::forward<ParamType3>(param3), std::forward<ParamType4>(param4));
	}

	/*!*********************************************************************************************************************
	\brief      -- Use this method to construct a new instance of ElementType using its FIVE-parameter constructor, and use
	            reference counting with it. ---   FIVE-ARGUMENT CONSTRUCTOR OVERLOAD
	\descriptionUse this method or its overloads to create a new object in the most efficient way possible and wrap it in a
	            RefCountedResource object. Use this whenever possible instead of wrapping a user-provided pointer in order to get
	      the best memory locality, as this method will create both the object and the necessary reference counters in one
	      block of memory.
	      If an object is already owned by this object, it will first be properly released before constructing the new one.
	\tparam     ParamType1  The type of the first argument accepted by MyClass_'s constructor
	\param      param1  The argument that will be forwarded to MyClass_'s constructor as the first argument
	\tparam     ParamType2  The type of the second argument accepted by MyClass_'s constructor
	\param      param2  The argument that will be forwarded to MyClass_'s constructor as the second argument
	\tparam     ParamType3  The type of the third argument accepted by MyClass_'s constructor
	\param      param3  The argument that will be forwarded to MyClass_'s constructor as the third argument
	\tparam     ParamType4  The type of the fourth argument accepted by MyClass_'s constructor
	\param      param4  The argument that will be forwarded to MyClass_'s constructor as the fourth argument
	\tparam     ParamType5  The type of the fifth argument accepted by MyClass_'s constructor
	\param      param5  The argument that will be forwarded to MyClass_'s constructor as the fifth argument
	***********************************************************************************************************************/
	template <typename ParamType1, typename ParamType2, typename ParamType3, typename ParamType4, typename ParamType5>
	void construct(ParamType1&& param1, ParamType2&& param2, ParamType3&& param3, ParamType4&& param4, ParamType5&& param5)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<MyClass_>::construct(Dereferenceable<MyClass_>::pointee, std::forward<ParamType1>(param1),
		    std::forward<ParamType2>(param2), std::forward<ParamType3>(param3), std::forward<ParamType4>(param4),
		    std::forward<ParamType5>(param5));
	}

	/*!*********************************************************************************************************************
	\brief      Use this function to share the refcounting between two unrelated classes that share lifetime (for example,
	the node of a read-only list). This function will use the "parent" objects' entry for the child object, so
	that the reference count to the child object will keep the parent alive.
	\description WARNING. This function does NOT cause lifetime dependencies, it only EXPRESSES them if they already exist.
	i.e. The main purpose of this method is to allow the user to express hierarchical lifetime relationships
	between objects. For example, if we had a RefCountedResource<std::vector<pvr::assets::Model>> (that is
	never modified), this method would allow us to get smart resources to the Mesh objects of the models, with shared
	refcounting to the vector object, and not keep the original RefCountedResource at all, allowing the vector to be destroyed
	automatically when no references to its Meshes exist anymore - the Meshes would all be destroyed together
	as normal when the vector was destroyed, in turn deleting the Models, in turn deleting the Meshes.
	WARNING: All that this method does is make sure that as long as pointers to children exist, the parent object
	stays alive. If the parent objects are in fact unrelated (For example, have a Mesh share refcounting to a std::vector<Mesh>
	that does NOT contain it, then a) The object will not be properly destroyed when its reference is released, and b1) Potentially
	result in a memory leak if the object actually needed to be released an/or b2) Cause undefined behaviour when the object was released
	by its "normal" lifetime, since the shared resource was NOT really keeping it alive.
	***********************************************************************************************************************/
	template<typename OriginalType>
	void shareRefCountFrom(const RefCountedResource<OriginalType>& resource, typename EmbeddedRefCountedResource<MyClass_>::ElementType* pointee)
	{
		shareRefCountingFrom(RefcountEntryHolder<OriginalType>(resource).refCountEntry, pointee);
	}

private:
	void shareRefCountingFrom(IRefCountEntry* entry, ElementType* pointee)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<ElementType>::refCountEntry = entry;
		entry->increment_count();
		debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0 && RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0, "BUG - Count was negative.");
		Dereferenceable<ElementType>::pointee = pointee;
	}

	template <typename OriginalType>
	RefCountedResource(const IRefCountEntry* entry, OriginalType* pointee)
		: EmbeddedRefCountedResource<MyClass_>(entry, pointee) { }
};



/*!*********************************************************************************************************************
\brief      A RefCountedWeakReference is a "Weak Reference" to a Reference counted object.
\description Weak references are the same as a normal RefCountedResource with a few key differences:
            1) They cannot keep the object alive. If an object only has weak references pointing to it, it is destroyed.
      2) Weak reference can still be safely queried to see if their object is "alive" (i.e. has strong references pointing
      to it)
      3) You cannot .construct() an object on a weak reference, only strong references.
      Weak references are used to avoid Cyclic Dependencies which would sometimes make objects undeleteable and hanging
      even when no application references exist to them (If "A" holds a reference to "B" and "B" holds a reference to
      "A" then even when the user abandons all references to A and B the objects will be dangling in memory, keeping each
      other alive). In those cases one object must hold a weak reference to the other ("A" holds a reference to "B" but
      "B" holds a weak reference to "A", then when the last reference to "A" is abandoned from the application, "A"
      will be deleted, freeing the reference to "B" and allowing it to also be deleted.
      Can only be created from an already existing RefCountedResource or another RefCountedWeakReference.
\tparam     MyClass_  The type of the element that this RefCountedWeakReference points to. Can be polymorphic.
***********************************************************************************************************************/
template<typename MyClass_>
class RefCountedWeakReference : public Dereferenceable<MyClass_>, public RefcountEntryHolder<MyClass_>
{
	template<typename> friend class EmbeddedRefCount;
	template<typename> friend class RefCountedResource;
	template<typename> friend class EmbeddedRefCountedResource;
	template<typename> friend class RefCountedWeakReference;
public:
	typedef MyClass_ ElementType; //!< The type of the element pointed to. Can be polymorphic.

	/*!*********************************************************************************************************************
	\brief     Test if this reference points to a valid object.
	\return    True if this reference points to a valid object.
	***********************************************************************************************************************/
	bool isValid() const
	{
		return RefcountEntryHolder<MyClass_>::refCountEntry != NULL &&
		       RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0;
	}

	/*!*********************************************************************************************************************
	\brief     Test if this reference does not point to a valid object.
	\return    True if this reference does not points to a valid object.
	***********************************************************************************************************************/
	bool isNull() const
	{
		return RefcountEntryHolder<MyClass_>::refCountEntry == NULL ||
		       RefcountEntryHolder<MyClass_>::refCountEntry->count() == 0;
	}

	/*!*********************************************************************************************************************
	\brief     Get a raw pointer to the pointed-to object.
	\return    Get a raw pointer of MyClass_* type to the pointed-to object.
	***********************************************************************************************************************/
	MyClass_* get() { return Dereferenceable<MyClass_>::pointee; }

	/*!*********************************************************************************************************************
	\brief     Get a const raw pointer to the pointed-to object.
	\return    Get a const raw pointer of MyClass_* type to the pointed-to object.
	***********************************************************************************************************************/
	const MyClass_* get() const { return Dereferenceable<MyClass_>::pointee; }

	void swap(RefCountedWeakReference& rhs)
	{
		IRefCountEntry* tmpRefCountEntry = RefcountEntryHolder<MyClass_>::refCountEntry;
		RefcountEntryHolder<MyClass_>::refCountEntry = rhs.refCountEntry;
		rhs.refCountEntry = tmpRefCountEntry;
		MyClass_ * tmpPointee = Dereferenceable<MyClass_>::pointee;
		Dereferenceable<MyClass_>::pointee = rhs.pointee;
		rhs.pointee = tmpPointee;
	}

	/*!*********************************************************************************************************************
	\brief     Default constructor. Constructed object points to NULL.
	***********************************************************************************************************************/
	RefCountedWeakReference() : Dereferenceable<MyClass_>(NULL), RefcountEntryHolder<MyClass_>(0) { }

	/*!*********************************************************************************************************************
	\brief     Copy constructor. Implements normal reference counting (increases Weak reference count).
	***********************************************************************************************************************/
	RefCountedWeakReference(const RefCountedWeakReference& rhs) : Dereferenceable<MyClass_>(rhs), RefcountEntryHolder<MyClass_>(rhs)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "BUG - Count was nonpositive.");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "BUG - Count was nonpositive.");
		}
	}

	/*!*********************************************************************************************************************
	\brief     Copy conversion constructor from Normal (non-weak) reference. Implements normal reference counting (increases
	           Weak reference count).
	***********************************************************************************************************************/
	RefCountedWeakReference(const EmbeddedRefCountedResource<MyClass_>& rhs) : Dereferenceable<MyClass_>(rhs), RefcountEntryHolder<MyClass_>(rhs)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	/*!*********************************************************************************************************************
	\brief     Copy assignment operator. Implements normal reference counting (increases Weak reference count).
	***********************************************************************************************************************/
	RefCountedWeakReference& operator=(RefCountedWeakReference rhs) { swap(rhs); return *this; }

	/*!*********************************************************************************************************************
	\brief     Implicit Copy Conversion operator. Converts to this type any RefCountedWeakReference to a type that is implicitly
	           convertible to MyClass_.
	\tparam OldType_ The type that the right hand side wraps. Must be implicitly convertible to MyType_
	\param     rhs The object to convert
	\description The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and is not used.
	***********************************************************************************************************************/
	template<class OldType_>
	RefCountedWeakReference(const RefCountedWeakReference<OldType_>& rhs,
	                        typename std::enable_if<std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* = 0)
		: Dereferenceable<MyClass_>(rhs.pointee), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		//Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<OldType_*>(Dereferenceable<MyClass_>::pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	/*!*********************************************************************************************************************
	\brief     Explicit Copy Conversion operator. Will converts to this type any RefCountedWeakReference to a type that is
	           explicitly convertible to MyClass_.
	\tparam OldType_ The type that the right hand side wraps. Must be explicitly convertible (static_cast) to MyType_
	\param     rhs The object to convert
	\description The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and is not used.
	***********************************************************************************************************************/
	template<class OldType_>
	explicit RefCountedWeakReference(const RefCountedWeakReference<OldType_>& rhs,
	                                 typename std::enable_if < !std::is_convertible<OldType_*, MyClass_*>::value, void* >::type* = 0)
		: Dereferenceable<MyClass_>((MyClass_*)(rhs.pointee)), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		//Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<MyClass_*>(Dereferenceable<MyClass_>::pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	/*!*********************************************************************************************************************
	\brief   Destructor. Reduces weak references by one, freeing the bookkeeping block if total references reach 0.
	***********************************************************************************************************************/
	virtual ~RefCountedWeakReference() { release(); }

	/*!*********************************************************************************************************************
	\brief   Decrements weak reference count. If it is the last pointer, the object must have already been destroyed, but it
	         destroys the reference counts block. Then, points to NULL.
	***********************************************************************************************************************/
	void release()
	{
		releaseOne();
		RefcountEntryHolder<MyClass_>::refCountEntry = 0;
		Dereferenceable<MyClass_>::pointee = 0;
	}

	/*!*********************************************************************************************************************
	\brief   Decrements weak reference count. If it is the last pointer, the object must have already been destroyed, but it
	         destroys the reference counts block. Then, points to NULL.
	***********************************************************************************************************************/
	void reset()
	{
		release();
	}

private:
	void retainOne()
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	void releaseOne()
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			IRefCountEntry* tmp = RefcountEntryHolder<MyClass_>::refCountEntry;
			RefcountEntryHolder<MyClass_>::refCountEntry = 0;
			Dereferenceable<MyClass_>::pointee = 0;
			tmp->decrement_weakcount();

		}
	}

	template <typename OriginalType>
	RefCountedWeakReference(IRefCountEntry* entry, OriginalType* pointee)
		: Dereferenceable<MyClass_>(static_cast<MyClass_*>(pointee)), RefcountEntryHolder<MyClass_>(entry)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}
};

/*!*********************************************************************************************************************
\brief      EmbeddedRefCount is the class that must be inherited from in order to have a class that automatically has
      refcounted members, with awareness and access to their ref counting by keeping their ref-counting bookkeeping
      entries embedded in the main class
\tparam     MyClass_  The type of the class that this EmbeddedRefCount belongs to. CRTP parameter. MUST be the actual class
      that inherits from EmbeddedRefCount (as in class MyClass: public EmbeddedRefCount<MyClass>{}, even if the
      pointer returned to the user is ultimately of another kind.
\description EmbeddedRefCount has two uses:
      First, it is a performance optimization, for all intents and purposes identical to using .construct
***********************************************************************************************************************/
template<typename MyClass_>
class EmbeddedRefCount : public IRefCountEntry
{
public:
	typedef EmbeddedRefCountedResource<MyClass_> StrongReferenceType;
	typedef RefCountedWeakReference<MyClass_> WeakReferenceType;
	WeakReferenceType getWeakReference() { return WeakReferenceType(static_cast<MyClass_*>(this), static_cast<MyClass_*>(this)); }
	StrongReferenceType getReference() { return StrongReferenceType(static_cast<MyClass_*>(this), static_cast<MyClass_*>(this)); }
protected:
	EmbeddedRefCount() {}
	static StrongReferenceType createNew()
	{
		MyClass_* item = new MyClass_();
		StrongReferenceType ptr(item, item);
		item->decrement_count();
		return ptr;
	}
	template<typename T1>
	static StrongReferenceType createNew(T1&& t1)
	{
		MyClass_* item = new MyClass_(std::forward<T1>(t1));
		StrongReferenceType ptr(item, item);
		item->decrement_count();
		return ptr;
	}
	template<typename T1, typename T2>
	static StrongReferenceType createNew(T1&& t1, T2&& t2)
	{
		MyClass_* item = new MyClass_(std::forward<T1>(t1), std::forward<T2>(t2));
		StrongReferenceType ptr(item, item);
		item->decrement_count();
		return ptr;
	}
	template<typename T1, typename T2, typename T3>
	static StrongReferenceType createNew(T1&& t1, T2&& t2, T3&& t3)
	{
		MyClass_* item = new MyClass_(std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3));
		StrongReferenceType ptr(item, item);
		item->decrement_count();
		return ptr;
	}
	template<typename T1, typename T2, typename T3, typename T4>
	static StrongReferenceType createNew(T1&& t1, T2&& t2, T3&& t3, T4&& t4)
	{
		MyClass_* item = new MyClass_(std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3), std::forward<T4>(t4));
		StrongReferenceType ptr(item, item);
		item->decrement_count();
		return ptr;
	}
	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	static StrongReferenceType createNew(T1&& t1, T2&& t2, T3&& t3, T4&& t4, T5&& t5)
	{
		MyClass_* item = new MyClass_(std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3), std::forward<T4>(t4), std::forward<T5>(t5));
		StrongReferenceType ptr(item, item);
		item->decrement_count();
		return ptr;
	}

	void deleteEntry()
	{
		delete static_cast<MyClass_*>(this);
	}
};

//!\cond NO_DOXYGEN
template<typename T>
struct IsRefCountedType
{
	template<typename U, void(U::*)()> struct SFINAE {};
	template<typename U> static char Test(SFINAE<U, &U::construct>*);
	template<typename U> static int Test(...);
	enum { value = (sizeof(Test<T>(0)) == sizeof(char)) };
};
}
//!\endcond
