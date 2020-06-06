// Tencent is pleased to support the open source community by making sluaunreal available.

// Copyright (C) 2018 THL A29 Limited, a Tencent company. All rights reserved.
// Licensed under the BSD 3-Clause License (the "License"); 
// you may not use this file except in compliance with the License. You may obtain a copy of the License at

// https://opensource.org/licenses/BSD-3-Clause

// Unless required by applicable law or agreed to in writing, 
// software distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and limitations under the License.
#pragma once
#include "CoreMinimal.h"
#include "lua.hpp"
#include "PropertyUtil.generated.h"


// copy code from UE, avoid be removed from version 4.22
UENUM()
enum class EPropertyClass
{
	Byte,
	Int8,
	Int16,
	Int,
	Int64,
	UInt16,
	UInt32,
	UInt64,
	UnsizedInt,
	UnsizedUInt,
	Float,
	Double,
	Bool,
	SoftClass,
	WeakObject,
	LazyObject,
	SoftObject,
	Class,
	Object,
	Interface,
	Name,
	Str,
	Array,
	Map,
	Set,
	Struct,
	Delegate,
	MulticastDelegate,
	Text,
	Enum,
};

namespace NS_SLUA {

    template<typename T>
    struct DeduceType;

    // convert T to EPropertyClass
    #define DefDeduceType(A,B) \
    template<> struct DeduceType<A> { \
        static const EPropertyClass value = EPropertyClass::B; \
    };\


    DefDeduceType(uint8, Byte);
    DefDeduceType(int8, Int8);
    DefDeduceType(int16, Int16);
    DefDeduceType(int, Int);
    DefDeduceType(int64, Int64);
    DefDeduceType(uint16, UInt16);
    DefDeduceType(uint32, UInt32);
    DefDeduceType(uint64, UInt64);
    DefDeduceType(float, Float);
    DefDeduceType(double, Double);
    DefDeduceType(bool, Bool);
    DefDeduceType(UObject*, Object);
    DefDeduceType(FString, Str);

    // definition of property
    struct SLUA_UNREAL_API PropertyProto {
        PropertyProto(EPropertyClass t) :type(t), cls(nullptr) {}
        PropertyProto(EPropertyClass t,UClass* c) :type(t), cls(c) {}

        template<typename T>
        static PropertyProto get() {
            return PropertyProto(DeduceType<T>::value);
        }

        EPropertyClass type;
        UClass* cls;

        // create FProperty by PropertyProto
        // returned FProperty should be collect by yourself
        static FProperty* createProperty(const PropertyProto& p);
    }; 


	/** Helper to manage a property pointer that may be potentially owned by a wrapper or stack instance */
	template <typename TPropType>
	class TPropOnScope
	{
		static_assert(TIsDerivedFrom<TPropType, FProperty>::IsDerived, "TPropType must be a FProperty-based type!");

	public:
		static TPropOnScope OwnedReference(TPropType* InProp)
		{
			return TPropOnScope(InProp, true);
		}

		static TPropOnScope ExternalReference(TPropType* InProp)
		{
			return TPropOnScope(InProp, false);
		}

		TPropOnScope() = default;

		~TPropOnScope()
		{
			Reset();
		}

		TPropOnScope(const TPropOnScope&) = delete;
		TPropOnScope& operator=(const TPropOnScope&) = delete;

		template <
			typename TOtherPropType,
			typename = decltype(ImplicitConv<TPropType*>((TOtherPropType*)nullptr))
		>
			TPropOnScope(TPropOnScope<TOtherPropType>&& Other)
		{
			bOwnsProp = Other.OwnsProp();
			Prop = Other.Release();

			Other.Reset();
		}

		template <
			typename TOtherPropType,
			typename = decltype(ImplicitConv<TPropType*>((TOtherPropType*)nullptr))
		>
			TPropOnScope& operator=(TPropOnScope<TOtherPropType>&& Other)
		{
			if (this != (void*)&Other)
			{
				Reset();

				bOwnsProp = Other.OwnsProp();
				Prop = Other.Release();

				Other.Reset();
			}
			return *this;
		}

		explicit operator bool() const
		{
			return IsValid();
		}

		bool IsValid() const
		{
			return Prop != nullptr;
		}

		operator TPropType* () const
		{
			return Prop;
		}

		TPropType& operator*() const
		{
			check(Prop);
			return *Prop;
		}

		TPropType* operator->() const
		{
			check(Prop);
			return Prop;
		}

		TPropType* Get() const
		{
			return Prop;
		}

		bool OwnsProp() const
		{
			return bOwnsProp;
		}

		TPropType* Release()
		{
			TPropType* LocalProp = Prop;
			Prop = nullptr;
			bOwnsProp = false;
			return LocalProp;
		}

		void Reset()
		{
			if (bOwnsProp)
			{
				delete Prop;
			}
			Prop = nullptr;
			bOwnsProp = false;
		}

		void AddReferencedObjects(FReferenceCollector& Collector)
		{
			if (Prop)
			{
				((typename TRemoveConst<TPropType>::Type*)Prop)->AddReferencedObjects(Collector);
			}
		}

	private:
		TPropOnScope(TPropType* InProp, const bool InOwnsProp)
			: Prop(InProp)
			, bOwnsProp(InOwnsProp)
		{
		}

		TPropType* Prop = nullptr;
		bool bOwnsProp = false;
	};
    
}