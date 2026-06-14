// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputSlot.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilityInputBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class CRIMABILITYSYSTEM_API UAbilityInputBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure, DisplayName = "MakeAbilityInputSlot")
	static FAbilityInputSlot MakeAbilityInputSlotFromTag(UPARAM(meta = (Categories = "Input")) const FGameplayTag InputTag);
	
	UFUNCTION(BlueprintPure, DisplayName = "MakeAbilityInputSlot")
	static FAbilityInputSlot MakeAbilityInputSlotFromIndex(const int32 SlotIndex = 0);
	
	UFUNCTION(BlueprintPure, DisplayName = "IsValid")
	static bool IsAbilityInputSlotValid(const FAbilityInputSlot& AbilityInputSlot);
	
	UFUNCTION(BlueprintPure, DisplayName = "GetString")
	static FString GetStringFromAbilityInputSlot(const FAbilityInputSlot& AbilityInputSlot);
};
