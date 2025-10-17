///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 David K Bhowmik. All rights reserved.
// This file is part of shhArc.
//
// This Software is available under the MIT License with a No Modification clause.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this file and associated documentation files (the "Software"), to use the
// Software for any purpose, including commercial applications, provided that:
//
//   1. You do NOT modify, alter, or create derivative works of this file.
//   2. Redistributions must include this notice and may only distribute it 
//      unmodified.
//
// The full MIT License text, including this Custom No Modification clause,
// is available in the LICENSE file in the root of the project.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
//
// You may alternatively use this source under the terms of a specific 
// version of the shhArc Unrestricted License provided you have obtained 
// such a license from the copyright holder.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
#ifndef GCPTRBASE_H
#define GCPTRBASE_H


#include "SecureStl.h"
#include "MemoryPtr.h"
#include <algorithm>


namespace shh {

	class GCObjectBase;
	template<class BASE> class GCObjectBaseInterface;

	// GCInfo ///////////////////////////////////////////////////////////////////////

	class GCInfo : public MemoryInfo
	{

		//DECLARE_MEMORY_MANAGED;

		friend class MemoryFrame;
		friend class Chunk;

	public:

		GCInfo(GCObjectBase* object, bool memoryManaged, int count = 0, bool valid = true);
		GCInfo(void* object, bool memoryManaged, int count = 0, bool valid = true);
		GCInfo(MemoryLocator& l, MemoryOffset offset, bool memoryManaged, int count = 0, bool valid = true);
		~GCInfo();

		inline void* GetObject() const { return myObject; }
		inline GCObjectBase* GetGCObject() const { return myGCObject; }

		static GCInfo* CreateNull();

	protected:

		virtual void RegisterMoveAll(MemorySize offsetDelta = 0, MemoryLocator::Direction offsetDir = MemoryLocator::None);
		virtual void RegisterMoveSingle(MemoryLocator* locator, MemoryOffset& offset);
	};



	// GCObjectBase /////////////////////////////////////////////////////////////////////

	class GCObjectBase
	{
		friend class GCInfo;
		template<class BASE> friend class GCObjectInterface;

	public:

		int myCollectable;


		GCObjectBase();
		virtual ~GCObjectBase();
		virtual bool Finalize(GCObjectInterface<GCObjectBase>* gc);

		void VoidGCInfo();
		void Init();
		bool IsDying() const;
		bool IsDeleteable() const;
		inline int GetReferenceCount() const;
		void MovePointers(GCObjectBase* object);

	protected:
		
		void NullInfo();
		void SetGCMemoryStart(void* mem);

	private:

		template<class T> friend GCInfo* GetGCInfoFromObject(T* object, bool memoryManaged);
		friend GCInfo* GetGCInfoFromObject(GCObjectBase* object, bool memoryManaged);
		GCInfo* GetGCInfo(bool memoryManaged);
		GCInfo* myGCInfo;
		
		// Declared but not defined
		GCObjectBase(GCObjectBase const&);
		GCObjectBase& operator=(GCObjectBase const&);

	private:

		void* myMemoryStart; // need to set this when there is multiople inheritance

	};



	// GCPtrBase ///////////////////////////////////////////////////////////////////////////

	template< typename T > class GCPtrBase : public MemoryPtr
	{

		friend typename T;

	public:

		GCPtrBase();
		explicit GCPtrBase(T* object);
		template< typename Other > GCPtrBase(GCPtrBase<Other> const& other);
		template< typename Other > GCPtrBase(GCPtrBase<Other> const& other, bool dummy);
		GCPtrBase(GCPtrBase const& other);
		~GCPtrBase();


		inline void Swap(GCPtrBase& other);
		template< typename Other > inline bool DynamicCast(GCPtrBase<Other> const& other);
		template< typename Other > inline bool StaticCast(GCPtrBase<Other> const& other);
		template< typename Other > inline GCPtrBase& operator=(GCPtrBase< Other > const& other);
		inline GCPtrBase& operator=(GCPtrBase const& other);

		void BlankInit();
		void Init(GCObjectBase* const& object);
		
		inline void Destroy();
		inline void SetNull();

		inline T* operator->() const;
		inline T& operator*() const;

		inline GCInfo* GetGCInfo() const;
		inline T* GetObject() const;
		inline bool IsMemoryManaged() const;

	protected:

		static bool ourMemoryManaged;
		T* myObject;
		void* myInfoObject;

		template< typename Other > inline  GCPtrBase& Assign(GCPtrBase< Other > const& other);
		inline T* SetObject() const;

	};



	template<typename T> bool GCPtrBase<T>::ourMemoryManaged = false;
	


	// General Functions //////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	GetGCInfoFromObject
	// Description:	get objects gcinfo
	// Arguments:	object to get from, if memory managed
	// Returns:		gcinfo
	// --------------------------------------------------------------------------

	inline GCInfo* GetGCInfoFromObject(GCObjectBase* object, bool memoryManaged)
	{
		return object ? object->GetGCInfo(memoryManaged) : GCInfo::CreateNull();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetGCInfoFromObject
	// Description:	get objects gcinfo
	// Arguments:	object to get from, if memory managed
	// Returns:		gcinfo
	// --------------------------------------------------------------------------
	inline GCInfo* GetGCInfoFromObject(void* object, bool memoryManaged)
	{
		return object ? new GCInfo(object, memoryManaged) : GCInfo::CreateNull();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetGCInfoFromObject
	// Description:	get objects gcinfo
	// Arguments:	object to get from, if memory managed
	// Returns:		gcinfo
	// --------------------------------------------------------------------------
	template< typename T >
	inline GCInfo* GetGCInfoFromObject(T* o, bool memoryManaged)
	{
		GCObjectBase* object = dynamic_cast<GCObjectBase*>(o);
		return object ? object->GetGCInfo(memoryManaged) : o ? new GCInfo(object, memoryManaged) : GCInfo::CreateNull();
	}


	// --------------------------------------------------------------------------						
	// Function:	==
	// Description:	compare is same object pointed to
	// Arguments:	object a, object b
	// Returns:		if same
	// --------------------------------------------------------------------------
	template< typename A, typename B >
	inline bool operator==(GCPtrBase<A> const& a, GCPtrBase<B> const& b)
	{
		return a.GetGCInfo() == b.GetGCInfo();
	}


	// --------------------------------------------------------------------------						
	// Function:	!=
	// Description:	compare is not same object pointed to
	// Arguments:	object a, object b
	// Returns:		if not same
	// --------------------------------------------------------------------------
	template< typename A, typename B >
	inline bool operator!=(GCPtrBase<A> const& a, GCPtrBase<B> const& b)
	{
		return a.GetGCInfo() != b.GetGCInfo();
	}


	// --------------------------------------------------------------------------						
	// Function:	Swap
	// Description:	swaps the two GCPtrBases objects
	// Arguments:	GCPtrBase a, GCPtrBase b
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T >
	inline void swap(shh::GCPtrBase<T>& a, shh::GCPtrBase<T>& b)
	{
		a.Swap(b);
	}



	// GCObjectBase  //////////////////////////////////////////////////////////

	

	// --------------------------------------------------------------------------						
	// Function:	GetReferenceCount
	// Description:	returns number of GCPtrBase referencing this object
	// Arguments:	none
	// Returns:		count
	// --------------------------------------------------------------------------
	inline int GCObjectBase::GetReferenceCount() const { return myGCInfo->GetReferenceCount(); }



	// GCPtrBase ////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	GCPtrBase
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > GCPtrBase<T>::GCPtrBase() : 
		MemoryPtr(GCInfo::CreateNull()), myObject(NULL), myInfoObject(NULL) 
	{}
	

	// --------------------------------------------------------------------------						
	// Function:	GCPtrBase
	// Description:	constructor
	// Arguments:	object to this will point to
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > GCPtrBase<T>::GCPtrBase(T* object) :
		MemoryPtr(shh::GetGCInfoFromObject(object, ourMemoryManaged)), 
		myObject(NULL), 
		myInfoObject(NULL) 
	{ 
		SetObject(); 
	}
	


	// --------------------------------------------------------------------------						
	// Function:	GCPtrBase
	// Description:	constructor
	// Arguments:	ptr to object this will point to
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > 
	template< typename Other > GCPtrBase<T>::GCPtrBase(GCPtrBase<Other> const& other) : 
		MemoryPtr(other.GetInfo()), 
		myObject(NULL), 
		myInfoObject(NULL) 
	{ 
		SetObject(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GCPtrBase
	// Description:	constructor
	// Arguments:	ptr to object this will point to
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > 
	template< typename Other > GCPtrBase<T>::GCPtrBase(GCPtrBase<Other> const& other, bool dummy) : 
		MemoryPtr(other.GetInfo()), 
		myObject(NULL), 
		myInfoObject(NULL) 
	{ 
		SetObject(); 
	} 	//Static-cast constructor
	

	// --------------------------------------------------------------------------						
	// Function:	GCPtrBase
	// Description:	constructor
	// Arguments:	ptr to object this will point to
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > GCPtrBase<T>::GCPtrBase(GCPtrBase const& other) : 
		MemoryPtr(other.GetInfo()), 
		myObject(NULL), 
		myInfoObject(NULL) 
	{ 
		SetObject(); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	~GCPtrBase
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > GCPtrBase<T>::~GCPtrBase() 
	{ 
		if (myInfo->GetReferenceCount() == 1) 
			Destroy(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Swap
	// Description:	swaps the object the two pointers are pointing to
	// Arguments:	object to swap with
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > inline void GCPtrBase<T>::Swap(GCPtrBase& other)
	{
		MemoryInfo* info = myInfo;
		LOCKEXCHANGEGCOBJECT(myInfo, other.myInfo);
		LOCKEXCHANGEGCOBJECT(other.myInfo, info);
		SetObject();
		other.SetObject();
	}


	// --------------------------------------------------------------------------						
	// Function:	DynamicCast
	// Description:	cast object sent in and assigns to this pointer
	// Arguments:	object to cast
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	template< typename T >
	template< typename Other > inline bool GCPtrBase<T>::DynamicCast(GCPtrBase<Other> const& other)
	{
		T* object = dynamic_cast<T*>(other.GetObject());
		if (object)
		{
			void* infoObject = other.GetGCInfo()->GetObject();
			LOCKEXCHANGEGCOBJECT(myInfoObject, infoObject);
			MemoryInfo* info = other.GetInfo();
			LOCKEXCHANGEGCOBJECT(myInfo, info);
			LOCKEXCHANGEGCOBJECT(myObject, object);
			myInfo->IncrementReferenceCount();
		}
		return object != 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	StaticCast
	// Description:	cast object sent in and assigns to this pointer
	// Arguments:	object to cast
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	template< typename T >
	template< typename Other > inline bool GCPtrBase<T>::StaticCast(GCPtrBase<Other> const& other)
	{
		T* object = static_cast<T*>(other.GetObject());
		if (object)
		{
			void* infoObject = other.GetGCInfo()->GetObject();
			LOCKEXCHANGEGCOBJECT(myInfoObject, infoObject);
			MemoryInfo* info = other.GetInfo();
			LOCKEXCHANGEGCOBJECT(myInfo, info);
			LOCKEXCHANGEGCOBJECT(myObject, object);
			myInfo->IncrementReferenceCount();
		}
		return object != 0;
	}



	// --------------------------------------------------------------------------						
	// Function:	=
	// Description:	assigns pointer
	// Arguments:	pointer to set to
	// Returns:		this
	// --------------------------------------------------------------------------
	template< typename T >
	template< typename Other >	inline GCPtrBase<T>& GCPtrBase<T>::operator=(GCPtrBase< Other > const& other) 
	{ 
		return Assign(other); 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	=
	// Description:	assigns pointer
	// Arguments:	pointer to set to
	// Returns:		this
	// --------------------------------------------------------------------------
	template< typename T > inline GCPtrBase<T>& GCPtrBase<T>::operator=(GCPtrBase const& other) 
	{ 
		return Assign(other); 
	}


	// --------------------------------------------------------------------------						
	// Function:	BlankInit
	// Description:	initializes blank pointerf
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > void GCPtrBase<T>::BlankInit()
	{
		myInfo = GCInfo::CreateNull();
		myInfo->IncrementReferenceCount();
	}


	// --------------------------------------------------------------------------						
	// Function:	Init
	// Description:	initializes to point to object passed in
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > void GCPtrBase<T>::Init(GCObjectBase* const& object)
	{
		GCInfo* i = myInfo;
		myInfo = GetGCInfoFromObject(object, ourMemoryManaged);
		myInfo->IncrementReferenceCount();
		if (i != NULL)
		{
			if (i->DecrementReferenceCount() == 0)
				delete i;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	Destroy
	// Description:	destroys object
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > inline void GCPtrBase<T>::Destroy()
	{
		if (IsValid() && !IsDying())
		{
			myInfo->SetDying(true);
			DestroyObjectVirtuallyIfPossible(GetObject(), GetObject());
			myInfo->SetValid(false);
			SetNull();
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	SetNull
	// Description:	clear pointer
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template< typename T > inline void GCPtrBase<T>::SetNull()
	{
		GCPtrBase temp;
		temp.Swap(*this);
	}



	// --------------------------------------------------------------------------						
	// Function:	->
	// Description:	dereference
	// Arguments:	none
	// Returns:		object pointed to
	// --------------------------------------------------------------------------
	template< typename T > inline T* GCPtrBase<T>::operator->() const
	{
		if (!IsValid())
			Exception::Throw("Bad GCPtrBase dereference");
		return GetObject();
	}



	// --------------------------------------------------------------------------						
	// Function:	*
	// Description:	dereference
	// Arguments:	none
	// Returns:		object pointed to
	// --------------------------------------------------------------------------
	template< typename T > inline T& GCPtrBase<T>::operator*() const
	{
		if (!IsValid())
			Exception::Throw("Bad GCPtrBase dereference");
		return *GetObject();
	}



	// --------------------------------------------------------------------------						
	// Function:	GetGCInfo
	// Description: returns gcinfo of pointer
	// Arguments:	none
	// Returns:		gcinfo
	// --------------------------------------------------------------------------
	template< typename T > inline GCInfo* GCPtrBase<T>::GetGCInfo() const 
	{ 
		return reinterpret_cast<GCInfo*>(myInfo); 
	}



	// --------------------------------------------------------------------------						
	// Function:	GetObject
	// Description:	dereference
	// Arguments:	none
	// Returns:		object pointed to
	// --------------------------------------------------------------------------
	template< typename T > inline T* GCPtrBase<T>::GetObject() const
	{
		return (IsValid() && GetGCInfo()->GetObject() == myInfoObject) ? myObject : SetObject();
	}



	// --------------------------------------------------------------------------						
	// Function:	IsMemoryManaged
	// Description:	returns if memory managed
	// Arguments:	none
	// Returns:		if managed
	// --------------------------------------------------------------------------
	template< typename T > inline bool GCPtrBase<T>::IsMemoryManaged() const
	{ 
		return myInfo->IsMemoryManaged();
	}



	// --------------------------------------------------------------------------						
	// Function:	Assign
	// Description:	assigns this pointer to point at object passed in by pointer
	// Arguments:	ptr to object to set to
	// Returns:		this
	// --------------------------------------------------------------------------
	template< typename T > 
	template< typename Other > inline GCPtrBase<T>& GCPtrBase<T>::Assign(GCPtrBase< Other > const& other)
	{
		GCPtrBase temp(other);
		Swap(temp);
		return *this;
	}



	// --------------------------------------------------------------------------						
	// Function:	SetObject
	// Description:	sets the myObject based on configuration by memory info etc
	// Arguments:	none
	// Returns:		object pointed to
	// --------------------------------------------------------------------------
	template< typename T > inline T* GCPtrBase<T>::SetObject() const
	{
		if (!IsValid())
			return 0;

		void* infoObject = GetGCInfo()->GetObject();
		LOCKEXCHANGEGCOBJECT(myInfoObject, infoObject);
		GCObjectBase* gco = GetGCInfo()->GetGCObject();
		if (gco)
		{
			T* object = dynamic_cast<T*>(gco);
			LOCKEXCHANGEGCOBJECT(myObject, object);
			return myObject;
		}
		else
		{
			T* object = static_cast<T*>(myInfoObject);
			LOCKEXCHANGEGCOBJECT(myObject, object);
			return myObject;
		}
	}


}

#endif // GCPtrBase_H
