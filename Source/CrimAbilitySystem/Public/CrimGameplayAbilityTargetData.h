// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimGameplayAbilityTargetData.generated.h"

/** A generic struct that can hold instanced structs for custom data. */
USTRUCT(BlueprintType)
struct FCrimGameplayAbilityTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInstancedStruct> CustomDataFragments{};
	
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
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FCrimGameplayAbilityTargetData::StaticStruct();
	}

	virtual FString ToString() const override
	{
		return TEXT("FCrimGameplayAbilityTargetData");
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};
template<>
struct TStructOpsTypeTraits<FCrimGameplayAbilityTargetData> : public TStructOpsTypeTraitsBase2<FCrimGameplayAbilityTargetData>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};
