// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "CrimGameplayEffectContext.generated.h"

USTRUCT()
struct FCrimGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	bool IsCriticalHit() const {return bIsCriticalHit;}
	void SetIsCriticalHit (const bool bValue) {bIsCriticalHit = bValue;}

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
	bool bIsCriticalHit = false;
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
