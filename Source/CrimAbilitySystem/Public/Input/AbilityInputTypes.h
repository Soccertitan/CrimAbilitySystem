// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputSlot.h"
#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbilityTypes.h"

#include "AbilityInputTypes.generated.h"

class UAbilityInput;
/** Used internally by the AbilityInputManagerComponent to store the handle and EventData. */
USTRUCT()
struct CRIMABILITYSYSTEM_API FAbilityInputHandle
{
	GENERATED_BODY()
	
	FAbilityInputHandle(){}
	FAbilityInputHandle(const FGameplayAbilitySpecHandle& InHandle) : Handle(InHandle){}
	
	FGameplayAbilitySpecHandle Handle;
	
	bool bSendGameplayEventData = false;
	
	FGameplayEventData EventData;
	
	FORCEINLINE bool operator == (FAbilityInputHandle const& Other) const
	{
		return Handle == Other.Handle;
	}

	FORCEINLINE bool operator != (FAbilityInputHandle const& Other) const
	{
		return Handle != Other.Handle;
	}
};

USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilityInputParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbilityInputSlot Slot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilityInput> Ability;
	
	bool IsValid() const;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, meta = (EditCondition=false, EditConditionHides))
	FString EditorDisplayName;
#endif
	
#if WITH_EDITOR
	void PostSerialize(const FArchive& Ar);
#endif
};
template<>
struct TStructOpsTypeTraits<FAbilityInputParams> : public TStructOpsTypeTraitsBase2<FAbilityInputParams>
{
	enum
	{
		WithPostSerialize = true,
   };
};
