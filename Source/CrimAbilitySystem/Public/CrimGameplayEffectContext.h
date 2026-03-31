// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimGameplayEffectContext.generated.h"

USTRUCT()
struct FCrimGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	const TArray<FInstancedStruct>& GetCustomDataFragments() const;
	TArray<FInstancedStruct>& GetMutableCustomDataFragments();
	
	template <typename T>
	const T* FindCustomDataFragment() const
	{
		for (const auto& Fragment : CustomDataFragments)
		{
			if (const T* TypedFragment = Fragment.GetPtr<T>())
			{
				return TypedFragment;
			}
		}
		return nullptr;
	}
	
	template <typename T>
	T* FindMutableCustomDataFragment()
	{
		for (auto& Fragment : CustomDataFragments)
		{
			if (T* TypedFragment = Fragment.GetMutablePtr<T>())
			{
				return TypedFragment;
			}
		}
		return nullptr;
	}

	/** Adds a Custom Data Fragment to the CustomDataFragments array. Returns a pointer to the added item. */
	template <typename T>
	T* AddCustomDataFragment(const T& Fragment)
	{
		FInstancedStruct InstancedStruct;
		InstancedStruct.InitializeAs<T>();
		T& Mutable = InstancedStruct.GetMutable<T>();
		Mutable = Fragment;

		int32 Index = CustomDataFragments.Add(MoveTemp(InstancedStruct));
		return CustomDataFragments[Index].GetMutablePtr<T>();
	}

	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FGameplayEffectContext* Duplicate() const override
	{
		FCrimGameplayEffectContext* NewContext = new FCrimGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}
	
	/** Returns the actual struct used for serialization, subclasses must override this!*/
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	/** Overridden to serialize new fields */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

protected:
	
	UPROPERTY()
	TArray<FInstancedStruct> CustomDataFragments{};
};

template<>
struct TStructOpsTypeTraits<FCrimGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FCrimGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
