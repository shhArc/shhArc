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


#ifndef GOD_H
#define GOD_H


#include "../Common/SecureStl.h"
#include "../Common/Mutex.h"
#include "../File/Archive.h"
#include "Realm.h"




namespace shh {

	class God : public Realm
	{
		//DECLARE_ARCHIVE(God)

	public:

		static std::string ourCommandLineArgs;
		static std::string ourConfigFileName;
		static std::string ourBootFileName;
		static std::string ourUpdateFileName;
		static std::string ourGodRealm;
		static StringKeyDictionary ourConfigFileDict;
		static unsigned int ourVersion;


		static const GCPtr<God>& GetGod();
		static void DestroyGod();

		virtual bool Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd);
		bool Update(double timeSinceLastUpdate);

		const GCPtr<Realm> GetWorld(const ::std::string& name);
		bool CreateWorld(const ::std::string& name, const StringKeyDictionary& config, const GCPtr<Realm>& other);
		bool DestroyWorld (const ::std::string& name);
		const RealmMap& GetWorlds() const;
		void DestroyWorlds();

		void Write(IOInterface& io, int version) const;
		void Read(IOInterface& io, int version);

		inline void Lock();
		inline void Unlock();

	protected:

		RealmMap myWorlds;
		Mutex myMutex;

		God();
		virtual ~God();
		virtual bool Update(double until, unsigned int phase);

	private:

		static GCPtr<God> ourGod;
		double myPaceMaker;
		std::vector<std::string> myWorldsToDestroy;

		God(const God&);
		God& operator=(const God&);
		static void Boot();
		static bool BuildHierachry(Realm::DictMap& unorderedRealms);
	};


	inline void God::Lock() { myMutex.LockMutex(); }
	inline void God::Unlock() { myMutex.UnlockMutex(); }

}
#endif 
