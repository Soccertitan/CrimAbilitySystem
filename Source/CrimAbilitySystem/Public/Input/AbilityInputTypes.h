// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbilityTypes.h"

#include "AbilityInputTypes.generated.h"

/** Used internally by the AbilityInputManagerComponent to store the handle and EventData. */
USTRUCT()
struct FAbilityInputHandle
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
